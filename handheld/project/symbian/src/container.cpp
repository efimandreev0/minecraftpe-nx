#include "appui.h"
#include "container.h"

#include <AknDef.h>
#include <eikdoc.h>
#include <aknappui.h>
#include <coecntrl.h>
#include <coemain.h>
#include <escapeutils.h>
#include <w32std.h>
#include <e32event.h>
#include <aknquerydialog.h>
#include <aknnotewrappers.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <minecraftpe.rsg>

#include <algorithm>

#define LPRINTF(FMT, ...) printf("mcpe: in file %s: in function %s: at line %d: " FMT "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#if !defined(EKeyLeftUpArrow) && \
	!defined(EKeyRightUpArrow) && \
	!defined(EKeyRightDownArrow) && \
	!defined(EKeyLeftDownArrow)
#define EKeyLeftUpArrow    EKeyDevice10  // Diagonal arrow event
#define EKeyRightUpArrow   EKeyDevice11  // Diagonal arrow event
#define EKeyRightDownArrow EKeyDevice12  // Diagonal arrow event
#define EKeyLeftDownArrow  EKeyDevice13  // Diagonal arrow event
#endif

#pragma mark - CMcpeContainer impl

CMcpeContainer::CMcpeContainer() : iImeShown(false), iApp(NULL), iAppCxt(), iAppPlat() {}

CMcpeContainer *CMcpeContainer::gInstance = NULL;

void CMcpeContainer::ConstructL(const TRect &aRect, CAknAppUi *aAppUi) {
	gInstance = this;

	iAppUi = aAppUi;

	CreateWindowL();
	Window().EnableAdvancedPointers();
	SetExtentToWholeScreen();
	ActivateL();

	mkdir("/data", 0777);
	mkdir("/data/Others", 0777);
	mkdir("/data/Others/minecraftpe", 0777);

	// in release builds, do *not* redirect standard streams
#ifndef PUBLISH
	freopen("/data/Others/minecraftpe/stdout.txt", "w+", stdout);
	freopen("/data/Others/minecraftpe/stderr.txt", "w+", stderr);

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
#endif

	const std::string storagePath{"/data/Others/minecraftpe"};

	iAppCxt.display = iEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (iEglDisplay == NULL) {
		_LIT(KGetDisplayFailed, "eglGetDisplay failed");
		User::Panic(KGetDisplayFailed, 0);
	}

	if (eglInitialize(iEglDisplay, NULL, NULL) == EGL_FALSE) {
		_LIT(KInitializeFailed, "eglInitialize failed");
		User::Panic(KInitializeFailed, 0);
	}

	eglBindAPI(EGL_OPENGL_ES_API);

	EGLConfig config;
	EGLint numOfConfigs = 0;

	TDisplayMode displayMode = Window().DisplayMode();
	TInt bufferSize = TDisplayModeUtils::NumDisplayModeBitsPerPixel(displayMode);

	// clang-format off
	const EGLint attribList[] = {
		EGL_BUFFER_SIZE, bufferSize,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
		EGL_SURFACE_TYPE, EGL_SWAP_BEHAVIOR_PRESERVED_BIT|EGL_WINDOW_BIT,
		EGL_DEPTH_SIZE, 1,
		EGL_STENCIL_SIZE, 1,
		EGL_NONE,
	};
	// clang-format on

	if (eglChooseConfig(iEglDisplay, attribList, &config, 1, &numOfConfigs) ==
			EGL_FALSE) {
		_LIT(KChooseConfigFailed, "eglChooseConfig failed");
		User::Panic(KChooseConfigFailed, 0);
	}

	if (numOfConfigs == 0) {
		_LIT(KNoConfig, "Can't find the requested config.");
		User::Panic(KNoConfig, 0);
	}

	iAppCxt.surface = iEglSurface = eglCreateWindowSurface(iEglDisplay, config, &Window(), NULL);

	if (iEglSurface == NULL) {
		_LIT(KCreateWindowSurfaceFailed, "eglCreateWindowSurface failed");
		User::Panic(KCreateWindowSurfaceFailed, 0);
	}

	eglSurfaceAttrib(iEglDisplay, iEglSurface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_DESTROYED);

	// clang-format off
	const EGLint contextAttrs[] = {EGL_NONE};
	// clang-format on

	iAppCxt.context = iEglContext =
			eglCreateContext(iEglDisplay, config, EGL_NO_CONTEXT, contextAttrs);
	if (iEglContext == NULL) {
		_LIT(KCreateContextFailed, "eglCreateContext failed");
		User::Panic(KCreateContextFailed, 0);
	}

	if (eglMakeCurrent(iEglDisplay, iEglSurface, iEglSurface, iEglContext) ==
			EGL_FALSE) {
		_LIT(KMakeCurrentFailed, "eglMakeCurrent failed");
		User::Panic(KMakeCurrentFailed, 0);
	}

	HandleResourceChange(KEikDynamicLayoutVariantSwitch);

	iApp = new NinecraftApp;
	if (!iApp) {
		_LIT(KAppFailed, "Create app failed!");
		User::Panic(KAppFailed, 0);
	}
	iApp->externalStoragePath = storagePath;
	iApp->externalCacheStoragePath = storagePath;

	iAppCxt.doRender = true;
	iAppCxt.platform = &iAppPlat;

	static_cast<App *>(iApp)->init(iAppCxt);
	iApp->setSize(Size().iWidth, Size().iHeight);

	iPeriodic = CPeriodic::NewL(CActive::EPriorityIdle);
	iPeriodic->Start(100, 100, TCallBack(CMcpeContainer::DrawCallBack, this));

	Window().PointerFilter(EPointerFilterDrag, 0);

	iWsEventReceiver = CWsEventReceiver::NewL(*this, &CCoeEnv::Static()->WsSession());
}

bool CMcpeContainer::PromptTextL(std::string &out, TInt maxLength) {
	if (iImeShown) { return false; }

	TBuf<128> data;

	TPtrC8 strPtr(reinterpret_cast<const TUint8 *>(out.c_str()));
	auto bufIn = EscapeUtils::ConvertToUnicodeFromUtf8L(strPtr);
	CleanupStack::PushL(bufIn);

	data.Copy(*bufIn);

	CleanupStack::PopAndDestroy();

	auto dlg = CAknTextQueryDialog::NewL(data);
	dlg->SetPromptL(_L("Enter text"));
	if (maxLength > 0) { dlg->SetMaxLength(maxLength); }

	iImeShown = true;
	bool ok = dlg->ExecuteLD(R_MCPE_TEXT_QUERY);
	if (ok) {
		auto mbcsTxt = EscapeUtils::ConvertFromUnicodeToUtf8L(data);
		CleanupStack::PushL(mbcsTxt);

		out = std::string(reinterpret_cast<const char *>(mbcsTxt->Ptr()), mbcsTxt->Length());

		CleanupStack::PopAndDestroy();
	}
	iImeShown = false;

	return ok;
}

TInt CMcpeContainer::DrawCallBack(TAny *aInstance) {
	CMcpeContainer *instance = static_cast<CMcpeContainer *>(aInstance);

	instance->iApp->update();

	User::ResetInactivityTime();

	User::After(100);

	return 0;
}


bool CMcpeContainer::IsScanCodeNonModifier(TInt aScanCode) {
	return !(
		aScanCode == EStdKeyLeftFunc ||
		aScanCode == EStdKeyRightFunc ||
		aScanCode == EStdKeyLeftCtrl ||
		aScanCode == EStdKeyRightCtrl ||
		aScanCode == EStdKeyLeftShift ||
		aScanCode == EStdKeyRightShift
	);
}

bool CMcpeContainer::HandleWsEvent(TWsEvent &aEvent) {
	TInt scanCode;
	switch (aEvent.Type()) {
	case EEventKeyDown:
		scanCode = aEvent.Key()->iScanCode;
		switch (aEvent.Key()->iCode) {
		case EStdKeyIncVolume:
			// TODO: Volume control broken?
			iVolume += iVolumeStep;
			iOutputStatus = ESetVolume;
			return true;
		case EStdKeyDecVolume:
			iVolume -= iVolumeStep;
			iOutputStatus = ESetVolume;
			return true;
		default:
			if (!std::count(iPressedKeys.begin(), iPressedKeys.end(), scanCode)) {
				iPressedKeys.push_back(scanCode);
				if (CMcpeContainer::IsScanCodeNonModifier(scanCode)) {
					iModifierUsed = true;
				}
			}
			return true;
		}
		break;
	case EEventKeyUp:
		scanCode = aEvent.Key()->iScanCode;
		if (CMcpeContainer::IsScanCodeNonModifier(scanCode)) {
			iPressedKeys.remove(scanCode);
			if (!std::count_if(iPressedKeys.begin(), iPressedKeys.end(), CMcpeContainer::IsScanCodeNonModifier)) {
				for (auto modifier : iDepressedModifiers) {
					iPressedKeys.remove(modifier);
				}
				iDepressedModifiers.clear();
			}
		} else if (iModifierUsed) {
			if (!std::count_if(iPressedKeys.begin(), iPressedKeys.end(), CMcpeContainer::IsScanCodeNonModifier)) {
				iPressedKeys.remove(scanCode);
				iDepressedModifiers.erase(scanCode);
			} else {
				iPressedKeys.clear();
			}
		} else {
			iDepressedModifiers.insert(scanCode);
		}
		if (!std::count_if(iPressedKeys.begin(), iPressedKeys.end(), [](TInt scanCode) {
				return !CMcpeContainer::IsScanCodeNonModifier(scanCode);
			})) {
			iModifierUsed = false;
		}
		if (iPressedKeys.empty()) {
			iDepressedModifiers.clear();
		}
		return true;
	case EEventPointer: {
		const auto pos = aEvent.Pointer()->iPosition;
		const auto ident = aEvent.Pointer()->PointerNumber();

		switch (aEvent.Pointer()->iType) {
		case TPointerEvent::EButton1Down:
			if (0 == ident) { Mouse::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_DOWN, pos.iX, pos.iY); }
			Multitouch::feed(1, MouseAction::DATA_DOWN, pos.iX, pos.iY, ident);
			break;
		case TPointerEvent::EButton1Up:
			if (0 == ident) { Mouse::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_UP, pos.iX, pos.iY); }
			Multitouch::feed(1, MouseAction::DATA_UP, pos.iX, pos.iY, ident);
			break;
		//case TPointerEvent::EMove:
		case TPointerEvent::EDrag:
			if (0 == ident) { Mouse::feed(MouseAction::ACTION_MOVE, MouseAction::DATA_DOWN, pos.iX, pos.iY); }
			Multitouch::feed(1, MouseAction::DATA_DOWN, pos.iX, pos.iY, ident);
			break;
		}
	} return true;
	default:
		break;
	}
	return false;
}

void CMcpeContainer::SizeChanged() {
	if (iApp) { iApp->setSize(Size().iWidth, Size().iHeight); }
}

void CMcpeContainer::HandleResourceChange(TInt aType) {
	switch (aType) {
	case KEikDynamicLayoutVariantSwitch:
		SetExtentToWholeScreen();
		break;
	}
}

CMcpeContainer::~CMcpeContainer() {
	delete iPeriodic;

	delete iApp;

	eglMakeCurrent(iEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroySurface(iEglDisplay, iEglSurface);
	eglDestroyContext(iEglDisplay, iEglContext);
	eglTerminate(iEglDisplay);
}

#pragma mark - CWsEventReceiver impl

CWsEventReceiver::CWsEventReceiver()
	: CActive(CActive::EPriorityHigh), iParent(NULL) {}

CWsEventReceiver::~CWsEventReceiver() { Cancel(); }

CWsEventReceiver *CWsEventReceiver::NewL(CMcpeContainer &aParent, RWsSession *aWsSession) {
	CWsEventReceiver *self = new (ELeave) CWsEventReceiver;

	CleanupStack::PushL(self);

	self->ConstructL(aParent, aWsSession);

	CleanupStack::Pop(self);

	return self;
}

void CWsEventReceiver::ConstructL(CMcpeContainer &aParent, RWsSession *aWsSession) {
	iParent = &aParent;
	iWsSession = aWsSession;
	iWsSession->EventReady(&iStatus);

	CActiveScheduler::Add(this);

	SetActive();
}

void CWsEventReceiver::RunL() {
	TWsEvent wsEvent;
	iWsSession->GetEvent(wsEvent);

	if (iParent->iImeShown || !iParent->HandleWsEvent(wsEvent)) {
		static_cast<CMcpeAppUi *>(iParent->iAppUi)->HandleEventL(wsEvent);
	}

	iWsSession->EventReady(&iStatus);

	SetActive();
}

void CWsEventReceiver::DoCancel() {
	iWsSession->EventReadyCancel();
}
