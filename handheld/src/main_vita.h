#ifndef MAIN_VITA_H__
#define MAIN_VITA_H__

#include <cassert>

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "np_mgr.h"

#include <psp2/kernel/modulemgr.h>
#include <psp2/sysmodule.h>
#include <psp2/touch.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/clib.h>
#include <psp2/net/net.h>
#include <psp2/apputil.h>
#include <psp2/net/netctl.h>
#include <gpu_es4/psp2_pvr_hint.h>

#define checkGl() assert(glGetError() == 0)
#define checkSce(x) ret = x; sceClibPrintf(#x ": %08x\n", ret);

#include "App.h"
#include "AppPlatform_Vita.h"
#include "platform/input/Mouse.h"
#include "platform/input/Multitouch.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Controller.h"


int _newlib_heap_size_user   = 64 * 1024 * 1024;
unsigned int sceLibcHeapSize = 3 * 1024 * 1024;

static bool _inited_egl = false;
static bool _app_inited = false;


static void initPvrPSP2() {
	const char* libgpu_es4_ext =	"app0:module/libgpu_es4_ext.suprx";
	const char* libIMGEGL =			"app0:module/libIMGEGL.suprx";
	const char* libGLESv1_CM =		"app0:module/libGLESv1_CM.suprx";
	const char* libpvrPSP2_WSEGL =	"app0:module/libpvrPSP2_WSEGL.suprx";

	int ret;
	checkSce(sceKernelLoadStartModule("app0:sce_module/libfios2.suprx", 0, NULL, 0, NULL, NULL));
	checkSce(sceKernelLoadStartModule("app0:sce_module/libc.suprx", 0, NULL, 0, NULL, NULL));
	checkSce(sceKernelLoadStartModule(libgpu_es4_ext, 0, NULL, 0, NULL, NULL));
  	checkSce(sceKernelLoadStartModule(libIMGEGL, 0, NULL, 0, NULL, NULL));
	
	PVRSRV_PSP2_APPHINT hint;
  	PVRSRVInitializeAppHint(&hint);
	strncpy(hint.szGLES1, libGLESv1_CM, sizeof(hint.szGLES1));
	strncpy(hint.szWindowSystem, libpvrPSP2_WSEGL, sizeof(hint.szWindowSystem));
  	PVRSRVCreateVirtualAppHint(&hint);
}

static void initEgl(App* app, AppContext* state, uint32_t w, uint32_t h)
{
	EGLBoolean result;
	initPvrPSP2();

	static const EGLint attribute_list[] =
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};
	static const EGLint context_attributes[] = 
	{
		EGL_CONTEXT_CLIENT_VERSION, 1,
		EGL_NONE
	};
	EGLConfig config;
	EGLint num_config;

	// get an EGL display connection
	state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(state->display!=EGL_NO_DISPLAY);

	// initialize the EGL display connection
	result = eglInitialize(state->display, NULL, NULL);
	assert(EGL_FALSE != result);

	// bind gles
	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);

	// get an appropriate EGL frame buffer configuration
	result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);

	// create window
	state->surface = eglCreateWindowSurface( state->display, config, (EGLNativeWindowType)0, NULL );
	assert(state->surface != EGL_NO_SURFACE);

	// create an EGL rendering context
	state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, context_attributes);
	assert(state->context!=EGL_NO_CONTEXT);

	// connect the context to the surface
   	result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
	assert(EGL_FALSE != result);

	int surface_width, surface_height;
	eglQuerySurface(state->display, state->surface, EGL_WIDTH, &surface_width);
	eglQuerySurface(state->display, state->surface, EGL_HEIGHT, &surface_height);
	sceClibPrintf("Surface Width: %d, Surface Height: %d\n", surface_width, surface_height);

	_inited_egl = true;
	if (!_app_inited) {
		_app_inited = true;
		app->init(*state);
	} else {
		app->onGraphicsReset(*state);
	}
	app->setSize(w, h);
}

static void deinitEgl(AppContext* state) {
	//printf("deinitEgl: (inited: %d)\n", _inited_egl);
	if (!_inited_egl) {
		return;
	}

	eglSwapBuffers(state->display, state->surface);
	eglMakeCurrent(state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(state->display, state->surface);
	eglDestroySurface(state->display, state->context);
	eglTerminate(state->display);

	//state->doRender = false;
	_inited_egl = false;
}

inline int16_t touch_to_screen_x(int16_t x) {
	return (float)x/2;
}
inline int16_t touch_to_screen_y(int16_t y) {
	return (float)y/2;
}

void handleTouch() {
	int ret;
	static SceTouchData prevTouchData = {};
	static SceTouchData currTouchData = {};
	static int firstFinger = -1;
	static int fingerSlots[12] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

	auto allocSlot = [&](int id) -> int {
        for (int i = 0; i < 12; i++)
            if (fingerSlots[i] == -1) { fingerSlots[i] = id; return i; }
        return -1;
    };
    auto findSlot = [&](int id) -> int {
        for (int i = 0; i < 12; i++)
            if (fingerSlots[i] == id) return i;
        return -1;
    };
    auto freeSlot = [&](int id) {
        for (int i = 0; i < 12; i++)
            if (fingerSlots[i] == id) { fingerSlots[i] = -1; return; }
    };

	prevTouchData = currTouchData;
	ret = sceTouchRead(0, &currTouchData, 1);
	if (ret < 0) {
		sceClibPrintf("sceTouchRead: %08x\n", ret);
		return;
	}

	// touchDown
    for (int i = 0; i < currTouchData.reportNum; i++) {
        SceTouchReport* curr = &currTouchData.report[i];
        bool found = false;
        for (int j = 0; j < prevTouchData.reportNum; j++)
            if (prevTouchData.report[j].id == curr->id) { found = true; break; }
        if (!found) {
            int slot = allocSlot(curr->id);
            if (slot == -1) continue;
            int16_t x = touch_to_screen_x(curr->x);
            int16_t y = touch_to_screen_y(curr->y);
            //sceClibPrintf("touchDown %d %d %d\n", curr->id, x, y);
            if (slot == 0) Mouse::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_DOWN, x, y);
            Multitouch::feed(1, MouseAction::DATA_DOWN, x, y, slot);
        }
    }

    // touchMove
    for (int i = 0; i < currTouchData.reportNum; i++) {
        SceTouchReport* curr = &currTouchData.report[i];
        for (int j = 0; j < prevTouchData.reportNum; j++) {
            if (prevTouchData.report[j].id == curr->id) {
                int slot = findSlot(curr->id);
                if (slot == -1) break;
                int16_t x = touch_to_screen_x(curr->x);
                int16_t y = touch_to_screen_y(curr->y);
                //sceClibPrintf("touchMove %d %d %d\n", curr->id, x, y);
                if (slot == 0) Mouse::feed(MouseAction::ACTION_MOVE, MouseAction::DATA_DOWN, x, y);
                Multitouch::feed(1, MouseAction::DATA_DOWN, x, y, slot);
                break;
            }
        }
    }

    // touchUp
    for (int i = 0; i < prevTouchData.reportNum; i++) {
        SceTouchReport* prev = &prevTouchData.report[i];
        bool found = false;
        for (int j = 0; j < currTouchData.reportNum; j++)
            if (currTouchData.report[j].id == prev->id) { found = true; break; }
        if (!found) {
            int slot = findSlot(prev->id);
            if (slot == -1) continue;
            int16_t x = touch_to_screen_x(prev->x);
            int16_t y = touch_to_screen_y(prev->y);
            //sceClibPrintf("touchUp %d %d %d\n", prev->id, x, y);
            if (slot == 0) Mouse::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_UP, x, y);
            Multitouch::feed(1, MouseAction::DATA_UP, x, y, slot);
            freeSlot(prev->id);
        }
    }
}

#define CONTROLLER_REMAP(x, y) 

static void trackpadPress(int stick, uint8_t x, uint8_t y) {
	float stickX = ((float)x - 128.f) / 128.f;
	float stickY = ((float)y - 128.f) / 128.f;
    Controller::feed(stick, Controller::STATE_TOUCH, stickX, stickY);
}
static void trackpadMove(int stick, uint8_t x, uint8_t y) {
	float stickX = ((float)x - 128.f) / 128.f;
	float stickY = ((float)y - 128.f) / 128.f;
    Controller::feed(stick, Controller::STATE_MOVE, stickX, stickY);
}
static void trackpadRelease(int stick, uint8_t x, uint8_t y) {
	float stickX = ((float)x - 128.f) / 128.f;
	float stickY = ((float)y - 128.f) / 128.f;
    Controller::feed(stick, Controller::STATE_RELEASE, stickX, stickY);
}

#define BTN_STATE(buttons, mask) (((buttons & mask) != 0) ? 1 : 0)

void handleController() {
	SceCtrlData ctrl = {0};
	static SceCtrlData prevCtrl = {0};
	sceCtrlReadBufferPositive(0, &ctrl, 1);

	//sceClibPrintf("ctrl.rx = %d ctrl.ry = %d\n", ctrl.rx, ctrl.ry);
	trackpadPress(2, ctrl.rx, ctrl.ry);
	trackpadPress(1, ctrl.lx, ctrl.ly);

	uint32_t changedButtons = ctrl.buttons ^ prevCtrl.buttons;
	prevCtrl = ctrl;
	if(changedButtons) {
		//sceClibPrintf("changedButtons = %08x\n", changedButtons);
	}

	// f5
	if(changedButtons & SCE_CTRL_UP) {
		Keyboard::feed(Keyboard::KEY_F5, BTN_STATE(ctrl.buttons, SCE_CTRL_UP));
	}

	// change selected slot
	if(changedButtons & SCE_CTRL_RIGHT) {
		Keyboard::feed(Keyboard::KEY_RIGHT, BTN_STATE(ctrl.buttons, SCE_CTRL_RIGHT));
	}
	if(changedButtons & SCE_CTRL_LEFT) {
		Keyboard::feed(Keyboard::KEY_LEFT, BTN_STATE(ctrl.buttons, SCE_CTRL_LEFT));
	}

	// sneak
	if(changedButtons & SCE_CTRL_DOWN) {
		Keyboard::feed(Keyboard::KEY_LSHIFT, BTN_STATE(ctrl.buttons, SCE_CTRL_DOWN));
	}

	// jump
	if(changedButtons & SCE_CTRL_CROSS) {
		Keyboard::feed(Keyboard::KEY_SPACE, BTN_STATE(ctrl.buttons, SCE_CTRL_CROSS));
	}

	// crafting menu
	if(changedButtons & SCE_CTRL_TRIANGLE) {
		Keyboard::feed(Keyboard::KEY_C, BTN_STATE(ctrl.buttons, SCE_CTRL_TRIANGLE));
	}
	// drop item
	if(changedButtons & SCE_CTRL_CIRCLE) {
		Keyboard::feed(Keyboard::KEY_Q, BTN_STATE(ctrl.buttons, SCE_CTRL_CIRCLE));
	}
	// inventory
	if(changedButtons & SCE_CTRL_SQUARE) {
		Keyboard::feed(Keyboard::KEY_E, BTN_STATE(ctrl.buttons, SCE_CTRL_SQUARE));
	}
	// pause
	if(changedButtons & SCE_CTRL_START) {
		Keyboard::feed(Keyboard::KEY_ESCAPE, BTN_STATE(ctrl.buttons, SCE_CTRL_START));
	}

	// psvita: placing and breaking blocks
	if(changedButtons & SCE_CTRL_LTRIGGER) {
		Mouse::feed(MouseAction::ACTION_RIGHT, BTN_STATE(ctrl.buttons, SCE_CTRL_LTRIGGER), 0,0);
	}

	if(changedButtons & SCE_CTRL_RTRIGGER) {
		Mouse::feed(MouseAction::ACTION_LEFT, BTN_STATE(ctrl.buttons, SCE_CTRL_RTRIGGER), 0,0);
	}

	// pstv: placing and breaking blocks
	if(changedButtons & SCE_CTRL_L1) {
		Mouse::feed(MouseAction::ACTION_RIGHT, BTN_STATE(ctrl.buttons, SCE_CTRL_L1), 0,0);
	}
	if(changedButtons & SCE_CTRL_R1) {
		Mouse::feed(MouseAction::ACTION_LEFT, BTN_STATE(ctrl.buttons, SCE_CTRL_R1), 0,0);
	}

}

int main(int argc, char** argv) {
	int ret;
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	sceSysmoduleLoadModule(SCE_SYSMODULE_APPUTIL);
	sceSysmoduleLoadModule(SCE_SYSMODULE_NP);

	static char netMem[0x10000];
	SceNetInitParam netInit = {
		.memory = netMem,
		.size = sizeof(netMem),
		.flags = 0
	};
	checkSce(sceNetInit(&netInit));
	checkSce(sceNetCtlInit());

	SceAppUtilInitParam initParam = {0};
	SceAppUtilBootParam bootParam = {0};

	checkSce(sceAppUtilInit(&initParam, &bootParam));

	SceNpCommunicationConfig cfg = {0};
	checkSce(sceNpInit(&cfg, nullptr));

	// checkSce(sceAppUtilCacheMount());

	MAIN_CLASS* app = new MAIN_CLASS();
	// savedata0 is too slow .. (probably bcs pfs)
	app->externalStoragePath = "ux0:/data/minecraftpe";
	app->externalCacheStoragePath = "ux0:/data/minecraftpe";

	int commandPort = 0;
	if (argc > 1) {
		commandPort = atoi(argv[1]);
	}

	if (commandPort != 0)
		app->commandPort = commandPort;

	AppContext context;
	AppPlatform_Vita platform;
	context.doRender = true;
	context.platform = &platform;

	initEgl(app, &context, width, height);

	sceTouchSetSamplingState(0, SCE_TOUCH_SAMPLING_STATE_START);
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

	bool running = true;
	while (running) {
		handleTouch();
		handleController();
		app->update();
	}

	deinitEgl(&context);

	sceNpTerm();
	sceAppUtilShutdown();

	return 0;
}

#endif
