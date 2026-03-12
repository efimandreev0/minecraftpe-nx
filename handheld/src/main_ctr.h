//
// Created by Notebook on 10.03.2026. // Привет из будущего! :)
//

#ifndef MINECRAFTCPP_MAIN_CTR_H
#define MINECRAFTCPP_MAIN_CTR_H

#include <cassert>
#include <vector>
#include <cmath>
#include <malloc.h>
#include <algorithm>
#include <unistd.h>

#if defined(__3DS__)
#include <3ds.h>
#define RIP_BACKEND_KYGX 0x01
#include <GLASS.h>
#include <GLES/gl.h>
#include <GLES/gl2.h>
#endif

#include <sys/stat.h>

#include "App.h"
#include "AppPlatform_ctr.h"
#include "platform/log.h"
#include "platform/input/Mouse.h"
#include "platform/input/Multitouch.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Controller.h"

static bool _app_inited = false;

static u32* soc_sharedmem = NULL;

void networkInit() {
    if (soc_sharedmem != NULL) return;

    soc_sharedmem = (u32*)memalign(0x1000, 0x100000);
    if(soc_sharedmem == NULL) {
        printf("Failed to allocate SOC memory!\n");
        return;
    }

    Result ret = socInit(soc_sharedmem, 0x100000);
    if(R_FAILED(ret)) {
        printf("socInit failed: 0x%08lX\n", ret);
    } else {
        printf("Network initialized perfectly!\n");
    }
}

void networkExit() {
    if (soc_sharedmem) {
        socExit();
        free(soc_sharedmem);
        soc_sharedmem = NULL;
    }
}
static GLASSCtx g_glassCtx = NULL;
static GLuint g_mainFB = 0;
static GLuint g_colorRB = 0;
static GLuint g_depthRB = 0;
static void initGraphics(App* app, AppContext* state)
{
    osSetSpeedupEnable(true);

    gfxInitDefault();
    kygxInit();

    g_glassCtx = glassCreateDefaultContext(GLASS_VERSION_ES_1_1);
    glassBindContext(g_glassCtx);

    glGenFramebuffers(1, &g_mainFB);
    glGenRenderbuffers(1, &g_colorRB);
    glGenRenderbuffers(1, &g_depthRB);

    glBindFramebuffer(GL_FRAMEBUFFER, g_mainFB);

    glBindRenderbuffer(GL_RENDERBUFFER, g_colorRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, 400, 240);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, g_colorRB);

    glBindRenderbuffer(GL_RENDERBUFFER, g_depthRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, 400, 240);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_depthRB);

    if (!_app_inited) {
        _app_inited = true;
        app->init(*state);
    } else {
        app->onGraphicsReset(*state);
    }

    app->setSize(400, 240);
}
static void deinitGraphics() {
    glassDestroyContext(g_glassCtx);
    kygxExit();
    gfxExit();
}
void handleTouch() {
    static bool wasTouching = false;
    static int16_t lastX = 0;
    static int16_t lastY = 0;

    touchPosition touch;
    hidTouchRead(&touch);

    bool isTouching = (hidKeysHeld() & KEY_TOUCH) != 0;

    if (isTouching) {
        int16_t x = (touch.px * 400) / 320;
        int16_t y = touch.py;

        if (!wasTouching) {
            Mouse::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_DOWN, x, y);
            Multitouch::feed(1, MouseAction::DATA_DOWN, x, y, 0);
            wasTouching = true;
        } else if (x != lastX || y != lastY) {
            Mouse::feed(MouseAction::ACTION_MOVE, MouseAction::DATA_DOWN, x, y);
            Multitouch::feed(1, MouseAction::DATA_DOWN, x, y, 0);
        }

        lastX = x;
        lastY = y;
    }
    else if (wasTouching) {
        Mouse::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_UP, lastX, lastY);
        Multitouch::feed(1, MouseAction::DATA_UP, lastX, lastY, 0);
        wasTouching = false;
    }
}

static void trackpadFeed(int stick, float x, float y) {
    float limitX = (std::abs(x) > 0.15f) ? x : 0.0f;
    float limitY = (std::abs(y) > 0.15f) ? y : 0.0f;

    limitX = std::max(-1.0f, std::min(1.0f, limitX));
    limitY = std::max(-1.0f, std::min(1.0f, limitY));

    Controller::feed(stick, Controller::STATE_TOUCH, limitX, limitY);
}

void handleController() {
    u32 kDown = hidKeysDown();
    u32 kUp = hidKeysUp();
    u32 kHeld = hidKeysHeld();
    u32 changed = kDown | kUp;

    circlePosition cp;
    hidCircleRead(&cp);

    trackpadFeed(1, (float)cp.dx / 156.0f, -(float)cp.dy / 156.0f);

    circlePosition cs;
    irrstCstickRead(&cs);
    trackpadFeed(2, (float)cs.dx / 156.0f, -(float)cs.dy / 156.0f);

    if(changed & KEY_DUP) Keyboard::feed(Keyboard::KEY_F5, (kHeld & KEY_DUP) ? 1 : 0);
    if(changed & KEY_R) Keyboard::feed(Keyboard::KEY_RIGHT, (kHeld & KEY_R) ? 1 : 0);
    if(changed & KEY_L) Keyboard::feed(Keyboard::KEY_LEFT, (kHeld & KEY_L) ? 1 : 0);
    if(changed & KEY_DDOWN) Keyboard::feed(Keyboard::KEY_LSHIFT, (kHeld & KEY_DDOWN) ? 1 : 0);
    if(changed & KEY_A) Keyboard::feed(Keyboard::KEY_SPACE, (kHeld & KEY_A) ? 1 : 0);
    if(changed & KEY_X) Keyboard::feed(Keyboard::KEY_C, (kHeld & KEY_X) ? 1 : 0);
    if(changed & KEY_B) Keyboard::feed(Keyboard::KEY_ESCAPE, (kHeld & KEY_B) ? 1 : 0);
    if(changed & KEY_Y) Keyboard::feed(Keyboard::KEY_E, (kHeld & KEY_Y) ? 1 : 0);
    if(changed & KEY_START) Keyboard::feed(Keyboard::KEY_P, (kHeld & KEY_START) ? 1 : 0);
    if(changed & KEY_ZR) Mouse::feed(MouseAction::ACTION_LEFT, (kHeld & KEY_ZR) ? 1 : 0, 0, 0);
    if(changed & KEY_ZL) Mouse::feed(MouseAction::ACTION_RIGHT, (kHeld & KEY_ZL) ? 1 : 0, 0, 0);
}

int main(int argc, char** argv) {
    romfsInit();

    mkdir("sdmc:/3ds", 0777);
    mkdir("sdmc:/3ds/minecraftpe", 0777);

    FILE* log_file = fopen("sdmc:/3ds/minecraftpe/debug_log.txt", "w");
    if (log_file) {
        setvbuf(log_file, NULL, _IONBF, 0);
        dup2(fileno(log_file), STDOUT_FILENO);
        dup2(fileno(log_file), STDERR_FILENO);
    }

    networkInit();
    irrstInit();

    MAIN_CLASS* app = new MAIN_CLASS();

    app->externalStoragePath = "sdmc:/3ds/minecraftpe";
    app->externalCacheStoragePath = "sdmc:/3ds/minecraftpe";

    int commandPort = 0;
    if (argc > 1) commandPort = atoi(argv[1]);
    if (commandPort != 0) app->commandPort = commandPort;

    AppContext context;
    AppPlatform_3ds platform;
    context.doRender = true;
    context.platform = &platform;

    initGraphics(app, &context);

    while (aptMainLoop()) {
        hidScanInput();

        if ((hidKeysHeld() & KEY_START) && (hidKeysHeld() & KEY_SELECT)) break;

        handleTouch();
        handleController();

        glBindFramebuffer(GL_FRAMEBUFFER, g_mainFB);
        glassSetTargetSide(g_glassCtx, GLASS_SIDE_LEFT);

        app->update();

        glassSwapBuffers();
    }

    deinitGraphics();

    irrstExit();
    networkExit();
    romfsExit();

    if (log_file) fclose(log_file);

    return 0;
}

#endif //MINECRAFTCPP_MAIN_CTR_H