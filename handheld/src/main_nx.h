#ifndef MAIN_SWITCH_H__
#define MAIN_SWITCH_H__

#include <cassert>
#include <vector>
#include <cmath>

#if defined(__SWITCH__)
#include <switch.h>
#endif

#include "EGL/egl.h"
#include "EGL/eglext.h"

#include "App.h"
// Если у тебя остался AppPlatform_Vita, можешь пока использовать его,
// но в идеале его тоже стоит переименовать в AppPlatform_Switch
#include "AppPlatform_nx.h"
#include "platform/log.h"
#include "platform/input/Mouse.h"
#include "platform/input/Multitouch.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Controller.h"

#define checkGl() assert(glGetError() == 0)

static bool _inited_egl = false;
static bool _app_inited = false;
static bool sneaking = false;

PadState pad;

static void initEgl(App* app, AppContext* state, uint32_t w, uint32_t h)
{
	EGLBoolean result;

	static const EGLint attribute_list[] =
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 16, // Или 24
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};
	static const EGLint context_attributes[] = 
	{
		EGL_CONTEXT_CLIENT_VERSION, 1, // OpenGL ES 1.1
		EGL_NONE
	};
	EGLConfig config;
	EGLint num_config;

	state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(state->display!=EGL_NO_DISPLAY);

	result = eglInitialize(state->display, NULL, NULL);
	assert(EGL_FALSE != result);

	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);

	result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);

	NWindow* win = nwindowGetDefault();
	nwindowSetDimensions(win, 1280, 720);

	state->surface = eglCreateWindowSurface(state->display, config, win, NULL);
	assert(state->surface != EGL_NO_SURFACE);

	state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, context_attributes);
	assert(state->context!=EGL_NO_CONTEXT);

   	result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
	assert(EGL_FALSE != result);

	_inited_egl = true;
	if (!_app_inited) {
		_app_inited = true;
		app->init(*state);
	} else {
		app->onGraphicsReset(*state);
	}
	app->setSize(1280, 720);
}

static void deinitEgl(AppContext* state) {
	if (!_inited_egl) return;

	eglSwapBuffers(state->display, state->surface);
	eglMakeCurrent(state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(state->display, state->surface);
	eglDestroyContext(state->display, state->context);
	eglTerminate(state->display);

	_inited_egl = false;
}
void handleTouch() {
	static HidTouchScreenState prevTouch = {0};
	HidTouchScreenState currTouch = {0};

	if (hidGetTouchScreenStates(&currTouch, 1) < 1) return;

	static int fingerSlots[12] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	auto allocSlot = [&](int id) -> int { for (int i=0; i<12; i++) if(fingerSlots[i]==-1){fingerSlots[i]=id; return i;} return -1; };
	auto findSlot = [&](int id) -> int { for (int i=0; i<12; i++) if(fingerSlots[i]==id) return i; return -1; };
	auto freeSlot = [&](int id) { for (int i=0; i<12; i++) if(fingerSlots[i]==id){fingerSlots[i]=-1; return;} };

	// touchDown (Касание экрана)
	for (int i = 0; i < currTouch.count; i++) {
		bool found = false;
		for (int j = 0; j < prevTouch.count; j++) {
			if (prevTouch.touches[j].finger_id == currTouch.touches[i].finger_id) {
				found = true;
				break;
			}
		}
		if (!found) {
			int slot = allocSlot(currTouch.touches[i].finger_id);
			if (slot == -1) continue;

			// Свитч отдает готовые 1280x720, НИКАКОГО СКЕЙЛА БОЛЬШЕ НЕ НУЖНО!
			int16_t x = currTouch.touches[i].x;
			int16_t y = currTouch.touches[i].y;

			if (slot == 0) Mouse::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_DOWN, x, y);
			Multitouch::feed(1, MouseAction::DATA_DOWN, x, y, slot);
		}
	}

	// touchMove (Движение пальцем)
	for (int i = 0; i < currTouch.count; i++) {
		for (int j = 0; j < prevTouch.count; j++) {
			if (prevTouch.touches[j].finger_id == currTouch.touches[i].finger_id) {
				int slot = findSlot(currTouch.touches[i].finger_id);
				if (slot == -1) break;

				int16_t x = currTouch.touches[i].x;
				int16_t y = currTouch.touches[i].y;
				int16_t prevX = prevTouch.touches[j].x;
				int16_t prevY = prevTouch.touches[j].y;

				// ФИКС ДЕРГАНИЯ: Отправляем эвент движения ТОЛЬКО если палец реально сдвинулся.
				// Иначе майнкрафт сходит с ума от спама нулевыми сдвигами каждый кадр.
				if (x != prevX || y != prevY) {
					if (slot == 0) Mouse::feed(MouseAction::ACTION_MOVE, MouseAction::DATA_DOWN, x, y);
					Multitouch::feed(1, MouseAction::DATA_DOWN, x, y, slot);
				}
				break;
			}
		}
	}

	for (int i = 0; i < prevTouch.count; i++) {
		bool found = false;
		for (int j = 0; j < currTouch.count; j++) {
			if (currTouch.touches[j].finger_id == prevTouch.touches[i].finger_id) {
				found = true;
				break;
			}
		}
		if (!found) {
			int slot = findSlot(prevTouch.touches[i].finger_id);
			if (slot == -1) continue;

			int16_t x = prevTouch.touches[i].x;
			int16_t y = prevTouch.touches[i].y;

			if (slot == 0) Mouse::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_UP, x, y);
			Multitouch::feed(1, MouseAction::DATA_UP, x, y, slot);

			freeSlot(prevTouch.touches[i].finger_id);
		}
	}

	prevTouch = currTouch;
}
static void trackpadFeed(int stick, float x, float y) {
	float limitX = (std::abs(x) > 0.15f) ? x : 0.0f;
	float limitY = (std::abs(y) > 0.15f) ? y : 0.0f;

	Controller::feed(stick, Controller::STATE_TOUCH, limitX, limitY);
}


void handleController() {
	u64 kDown = padGetButtonsDown(&pad);
	u64 kUp = padGetButtonsUp(&pad);
	u64 kHeld = padGetButtons(&pad);
	u64 changed = kDown | kUp;

	HidAnalogStickState ls = padGetStickPos(&pad, 0);
	HidAnalogStickState rs = padGetStickPos(&pad, 1);

	trackpadFeed(1, (float)ls.x / 32767.0f, -(float)ls.y / 32767.0f);
	trackpadFeed(2, (float)rs.x / 32767.0f, -(float)rs.y / 32767.0f);

	// Cam change
	if(changed & HidNpadButton_Up) Keyboard::feed(Keyboard::KEY_F5, (kHeld & HidNpadButton_Up) ? 1 : 0);

	// Choosing slots
	if(changed & (HidNpadButton_Right | HidNpadButton_R)) Keyboard::feed(Keyboard::KEY_RIGHT, (kHeld & (HidNpadButton_Right | HidNpadButton_R)) ? 1 : 0);
	if(changed & (HidNpadButton_Left | HidNpadButton_L)) Keyboard::feed(Keyboard::KEY_LEFT, (kHeld & (HidNpadButton_Left  | HidNpadButton_L)) ? 1 : 0);

	// Sneak
	if(changed & HidNpadButton_Down) {
		sneaking = !sneaking;
		Keyboard::feed(Keyboard::KEY_LSHIFT, sneaking);
	}
	//if(changed & (HidNpadButton_Down | HidNpadButton_StickL))
	//	Keyboard::feed(Keyboard::KEY_LSHIFT, (kHeld & (HidNpadButton_Down | HidNpadButton_StickL)) ? 1 : 0);

	// Jump
	if(changed & HidNpadButton_A) Keyboard::feed(Keyboard::KEY_SPACE, (kHeld & HidNpadButton_A) ? 1 : 0);

	// Inventory
	if(changed & HidNpadButton_X) Keyboard::feed(Keyboard::KEY_C, (kHeld & HidNpadButton_X) ? 1 : 0);

	// Throw away
	if(changed & HidNpadButton_B) Keyboard::feed(Keyboard::KEY_ESCAPE, (kHeld & HidNpadButton_B) ? 1 : 0);

	// Crafting
	if(changed & HidNpadButton_Y) Keyboard::feed(Keyboard::KEY_E, (kHeld & HidNpadButton_Y) ? 1 : 0);

	// Pause
	if(changed & HidNpadButton_Plus) Keyboard::feed(Keyboard::KEY_P, (kHeld & HidNpadButton_Plus) ? 1 : 0);

	// Break blocks
	if(changed & HidNpadButton_ZR)
		Mouse::feed(MouseAction::ACTION_LEFT, (kHeld & HidNpadButton_ZR) ? 1 : 0, 0, 0);

	// Place blocks
	if(changed & HidNpadButton_ZL)
		Mouse::feed(MouseAction::ACTION_RIGHT, (kHeld & (HidNpadButton_ZL)) ? 1 : 0, 0, 0);
}

int main(int argc, char** argv) {
	//FILE* log_file = fopen("sdmc:/switch/minecraftpe/debug_log.txt", "w");
	//if (log_file) {
	//	setvbuf(log_file, NULL, _IONBF, 0);
//
	//	dup2(fileno(log_file), STDOUT_FILENO);
	//	dup2(fileno(log_file), STDERR_FILENO);
	//}

	SocketInitConfig config = {
		.tcp_tx_buf_size = 0x8000,
		.tcp_rx_buf_size = 0x8000,
		.tcp_tx_buf_max_size = 0x20000,
		.tcp_rx_buf_max_size = 0x20000,
		.udp_tx_buf_size = 0x2400,
		.udp_rx_buf_size = 0xA500,
		.sb_efficiency = 4
	};

	Result rc = socketInitialize(&config);
	if (!rc)
		printf("[DEBUG] Cannot initialize socket with result %x.\n", rc);

	nifmInitialize(NifmServiceType_User);

	//socketInitializeDefault();
	romfsInit();

	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	padInitializeDefault(&pad);

	MAIN_CLASS* app = new MAIN_CLASS();

	app->externalStoragePath = "sdmc:/switch/minecraftpe";
	app->externalCacheStoragePath = "sdmc:/switch/minecraftpe";

	int commandPort = 0;
	if (argc > 1) commandPort = atoi(argv[1]);
	if (commandPort != 0) app->commandPort = commandPort;

	AppContext context;
	AppPlatform_NX platform;
	context.doRender = true;
	context.platform = &platform;

	initEgl(app, &context, 1280, 720);
	
	while (appletMainLoop()) {
		padUpdate(&pad);
		if ((padGetButtons(&pad) & HidNpadButton_Plus) && (padGetButtons(&pad) & HidNpadButton_Minus)) break;

		handleTouch();
		handleController();
		
		app->update();

		eglSwapBuffers(context.display, context.surface);
	}

	deinitEgl(&context);

	romfsExit();
	nifmExit();
	socketExit();

	return 0;
}

#endif