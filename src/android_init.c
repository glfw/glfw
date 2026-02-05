//========================================================================
// GLFW 3.5 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2024 kunitoki <kunitoki@gmail.com>
// Copyright (c) 2017 Curi0 <curi0minecraft@gmail.com>
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

#include <android_native_app_glue.h>
#include <android/native_window.h>
#include <android/log.h>

#include "internal.h"

struct android_app* _globalAndroidApp = NULL;

extern int main();

void handleAppCmd(struct android_app* app, int32_t cmd)
{
    switch (cmd)
    {
    case APP_CMD_START:
        break;

    case APP_CMD_RESUME:
        //_glfwInputWindowIconify(_glfw.windowListHead, GLFW_FALSE);
        break;

    case APP_CMD_PAUSE:
        //_glfwInputWindowIconify(_glfw.windowListHead, GLFW_TRUE);
        break;

    case APP_CMD_STOP:
        break;

    case APP_CMD_DESTROY:
        break;

    case APP_CMD_INIT_WINDOW:
        break;

    case APP_CMD_TERM_WINDOW:
        //_glfwInputWindowCloseRequest(_glfw.windowListHead);
        break;

    case APP_CMD_LOST_FOCUS:
        //_glfwInputWindowFocus(_glfw.windowListHead, GLFW_FALSE);
        break;

    case APP_CMD_GAINED_FOCUS:
        //_glfwInputWindowFocus(_glfw.windowListHead, GLFW_TRUE);
        break;

    case APP_CMD_WINDOW_RESIZED:
        _glfwInputWindowSize(_glfw.windowListHead, ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window));
        break;

    case APP_CMD_WINDOW_REDRAW_NEEDED:
        _glfwInputWindowDamage(_glfw.windowListHead);
        break;

    case APP_CMD_CONTENT_RECT_CHANGED:
        _glfwInputFramebufferSize(_glfw.windowListHead, ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window));
        break;

    case APP_CMD_CONFIG_CHANGED:
        break;

    case APP_CMD_INPUT_CHANGED:
        break;
    }
}

// Android Entry Point
void android_main(struct android_app* app)
{
    app_dummy();

    app->onAppCmd = handleAppCmd;

    _globalAndroidApp = app;

    main();
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwConnectAndroid(int platformID, _GLFWplatform* platform)
{
    const _GLFWplatform android =
    {
        .platformID = GLFW_PLATFORM_ANDROID,
        .init = _glfwInitAndroid,
        .terminate = _glfwTerminateAndroid,
        .getCursorPos = _glfwGetCursorPosAndroid,
        .setCursorPos = _glfwSetCursorPosAndroid,
        .setCursorMode = _glfwSetCursorModeAndroid,
        .setRawMouseMotion = _glfwSetRawMouseMotionAndroid,
        .rawMouseMotionSupported = _glfwRawMouseMotionSupportedAndroid,
        .createCursor = _glfwCreateCursorAndroid,
        .createStandardCursor = _glfwCreateStandardCursorAndroid,
        .destroyCursor = _glfwDestroyCursorAndroid,
        .setCursor = _glfwSetCursorAndroid,
        .getScancodeName = _glfwGetScancodeNameAndroid,
        .getKeyScancode = _glfwGetKeyScancodeAndroid,
        .setClipboardString = _glfwSetClipboardStringAndroid,
        .getClipboardString = _glfwGetClipboardStringAndroid,
        .initJoysticks = _glfwInitJoysticksAndroid,
        .terminateJoysticks = _glfwTerminateJoysticksAndroid,
        .pollJoystick = _glfwPollJoystickAndroid,
        .getMappingName = _glfwGetMappingNameAndroid,
        .updateGamepadGUID = _glfwUpdateGamepadGUIDAndroid,
        .freeMonitor = _glfwFreeMonitorAndroid,
        .getMonitorPos = _glfwGetMonitorPosAndroid,
        .getMonitorContentScale = _glfwGetMonitorContentScaleAndroid,
        .getMonitorWorkarea = _glfwGetMonitorWorkareaAndroid,
        .getVideoModes = _glfwGetVideoModesAndroid,
        .getVideoMode = _glfwGetVideoModeAndroid,
        .getGammaRamp = _glfwGetGammaRampAndroid,
        .setGammaRamp = _glfwSetGammaRampAndroid,
        .createWindow = _glfwCreateWindowAndroid,
        .destroyWindow = _glfwDestroyWindowAndroid,
        .setWindowTitle = _glfwSetWindowTitleAndroid,
        .setWindowIcon = _glfwSetWindowIconAndroid,
        .getWindowPos = _glfwGetWindowPosAndroid,
        .setWindowPos = _glfwSetWindowPosAndroid,
        .getWindowSize = _glfwGetWindowSizeAndroid,
        .setWindowSize = _glfwSetWindowSizeAndroid,
        .setWindowSizeLimits = _glfwSetWindowSizeLimitsAndroid,
        .setWindowAspectRatio = _glfwSetWindowAspectRatioAndroid,
        .getFramebufferSize = _glfwGetFramebufferSizeAndroid,
        .getWindowFrameSize = _glfwGetWindowFrameSizeAndroid,
        .getWindowContentScale = _glfwGetWindowContentScaleAndroid,
        .iconifyWindow = _glfwIconifyWindowAndroid,
        .restoreWindow = _glfwRestoreWindowAndroid,
        .maximizeWindow = _glfwMaximizeWindowAndroid,
        .showWindow = _glfwShowWindowAndroid,
        .hideWindow = _glfwHideWindowAndroid,
        .requestWindowAttention = _glfwRequestWindowAttentionAndroid,
        .focusWindow = _glfwFocusWindowAndroid,
        .setWindowMonitor = _glfwSetWindowMonitorAndroid,
        .windowFocused = _glfwWindowFocusedAndroid,
        .windowIconified = _glfwWindowIconifiedAndroid,
        .windowVisible = _glfwWindowVisibleAndroid,
        .windowMaximized = _glfwWindowMaximizedAndroid,
        .windowHovered = _glfwWindowHoveredAndroid,
        .framebufferTransparent = _glfwFramebufferTransparentAndroid,
        .getWindowOpacity = _glfwGetWindowOpacityAndroid,
        .setWindowResizable = _glfwSetWindowResizableAndroid,
        .setWindowDecorated = _glfwSetWindowDecoratedAndroid,
        .setWindowFloating = _glfwSetWindowFloatingAndroid,
        .setWindowOpacity = _glfwSetWindowOpacityAndroid,
        .setWindowMousePassthrough = _glfwSetWindowMousePassthroughAndroid,
        .pollEvents = _glfwPollEventsAndroid,
        .waitEvents = _glfwWaitEventsAndroid,
        .waitEventsTimeout = _glfwWaitEventsTimeoutAndroid,
        .postEmptyEvent = _glfwPostEmptyEventAndroid,
        .getEGLPlatform = _glfwGetEGLPlatformAndroid,
        .getEGLNativeDisplay = _glfwGetEGLNativeDisplayAndroid,
        .getEGLNativeWindow = _glfwGetEGLNativeWindowAndroid,
        .getRequiredInstanceExtensions = _glfwGetRequiredInstanceExtensionsAndroid,
        .getPhysicalDevicePresentationSupport = _glfwGetPhysicalDevicePresentationSupportAndroid,
        .createWindowSurface = _glfwCreateWindowSurfaceAndroid
    };

    *platform = android;
    return GLFW_TRUE;
}

int _glfwInitAndroid(void)
{
    _glfw.gstate.app = _globalAndroidApp;
    _glfw.gstate.source = NULL;

    return GLFW_TRUE;
}

void _glfwTerminateAndroid(void)
{
    _glfw.gstate.app = NULL;
}
