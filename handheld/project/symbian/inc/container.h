#pragma once

#ifndef _PROJECT_SYMBIAN_CONTAINER_H_0839D35E_88DD_52B8_BAB2_3B3290752949
#define _PROJECT_SYMBIAN_CONTAINER_H_0839D35E_88DD_52B8_BAB2_3B3290752949

#include <GLES/egl.h>

#include <aknappui.h>
#include <coecntrl.h>
#include <w32std.h>

#include <list>
#include <set>

#include "../../../src/NinecraftApp.h"

#include "../../../src/AppPlatform_Symbian.h"
#include "../../../src/App.h"

#include "../../../src/platform/input/Mouse.h"
#include "../../../src/platform/input/Multitouch.h"
#include "../../../src/platform/input/Keyboard.h"

struct SoundHandlerSymbian;

struct CMcpeAppUi;
struct CWsEventReceiver;

struct CMcpeContainer : CCoeControl {
	friend struct CMcpeAppUi;
	friend struct CWsEventReceiver;

	friend struct SoundHandlerSymbian;

	void ConstructL(const TRect &aRect, CAknAppUi *aAppUi);

	CMcpeContainer();

	~CMcpeContainer() override;

	bool PromptTextL(std::string &out, TInt maxLength = 0);

	bool IsImeShown() const { return iImeShown; }

	static TInt DrawCallBack(TAny *aInstance);

	bool HandleWsEvent(TWsEvent &aEvent);

	inline static CMcpeContainer *instance() { return gInstance; }

private:
	void SizeChanged() override;

	void HandleResourceChange(TInt aType) override;

	inline TInt CountComponentControls() const override { return 0; }

	inline CCoeControl *ComponentControl(TInt aIndex) const override { return NULL; }

	inline void Draw(const TRect &aRect) const override {}

	static bool IsScanCodeNonModifier(TInt iScanCode);

private:
	enum TStatus {
		ENotReady,
		EOpen,
		ESetVolume
	};

	enum { KBufferMaxFrames = 10 };

	static CMcpeContainer *gInstance;

	CWsEventReceiver *iWsEventReceiver;

	EGLDisplay iEglDisplay;
	EGLSurface iEglSurface;
	EGLContext iEglContext;

	CPeriodic *iPeriodic;
	CAknAppUi *iAppUi;

	std::list<TInt> iPressedKeys;
	std::set<TInt> iDepressedModifiers;
	bool iModifierUsed;

	bool iImeShown;

	NinecraftApp *iApp;
	AppContext iAppCxt;
	AppPlatform_Symbian iAppPlat;

	TInt iVolume;
	TUint iVolumeStep;
	TStatus iOutputStatus;
};

struct CWsEventReceiver : CActive {
	void RunL() override;

	void DoCancel() override;

	static CWsEventReceiver *NewL(CMcpeContainer &aParent, RWsSession *aWsSession);

	~CWsEventReceiver();

private:
	CWsEventReceiver();

	void ConstructL(CMcpeContainer &aParent, RWsSession *aWsSession);

private:
	RWsSession *iWsSession;

	CMcpeContainer *iParent;
};

#endif

