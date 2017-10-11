//========================================================================
// GLFW 3.3 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2006-2016 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include <android/log.h>
#include "internal.h"

extern int main();
void handle_cmd(struct android_app* _app, int32_t cmd) {
    switch (cmd) {
    case APP_CMD_INIT_WINDOW: {
        app = _app; // The window is being shown so the initialization is finished.
        break;
    }
    case APP_CMD_LOST_FOCUS: {
        break;
    }
    case APP_CMD_GAINED_FOCUS: {
        break;
    }
    case  APP_CMD_TERM_WINDOW: {
        glfwDestroyWindow((GLFWwindow *) _glfw.windowListHead);
    }
}
}

// Android Entry Point
void android_main(struct android_app *app) {
    app->onAppCmd = handle_cmd;
    pthread_create(&(pthread_t){0}, NULL, (void*)&main, NULL); // Call the main entry point

    while (1) {
        int ident;
        int events;
        struct android_poll_source* source;

        // Process events
        while ((ident=ALooper_pollAll(0, NULL, &events,(void**)&source)) >= 0)
            if (source != NULL)
                source->process(app, source);
    }

}
//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformInit(void)
{
    _glfwInitTimerPOSIX();
    while (app == NULL); // Wait for the app to be initialized or the app will crash occasionally
    return GLFW_TRUE;
}

void _glfwPlatformTerminate(void)
{
    _glfwTerminateOSMesa();
}

const char* _glfwPlatformGetVersionString(void)
{
    return _GLFW_VERSION_NUMBER " Android";
}

