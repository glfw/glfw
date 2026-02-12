//========================================================================
// GLFW 3.5 OGC - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2024 Alberto Mardegan <info@mardy.it>
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

#include "internal.h"

#if defined(_GLFW_OGC)

#include <assert.h>
#include <fat.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiikeyboard/keyboard.h>
#include <wiiuse/wpad.h>

#define FIFO_SIZE (256*1024)

char _glfwUnimplementedFmt[] = "Ogc: the platform does not support %s";

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwConnectOgc(int platformID, _GLFWplatform* platform)
{
    const _GLFWplatform ogc =
    {
        .platformID = GLFW_PLATFORM_OGC,
        .init = _glfwInitOgc,
        .terminate = _glfwTerminateOgc,
        .getCursorPos = _glfwGetCursorPosOgc,
        .setCursorPos = _glfwSetCursorPosOgc,
        .setCursorMode = _glfwSetCursorModeOgc,
        .setRawMouseMotion = _glfwSetRawMouseMotionOgc,
        .rawMouseMotionSupported = _glfwRawMouseMotionSupportedOgc,
        .createCursor = _glfwCreateCursorOgc,
        .createStandardCursor = _glfwCreateStandardCursorOgc,
        .destroyCursor = _glfwDestroyCursorOgc,
        .setCursor = _glfwSetCursorOgc,
        .getScancodeName = _glfwGetScancodeNameOgc,
        .getKeyScancode = _glfwGetKeyScancodeOgc,
        .setClipboardString = _glfwSetClipboardStringOgc,
        .getClipboardString = _glfwGetClipboardStringOgc,
        .initJoysticks = _glfwInitJoysticksOgc,
        .terminateJoysticks = _glfwTerminateJoysticksOgc,
        .pollJoystick = _glfwPollJoystickOgc,
        .getMappingName = _glfwGetMappingNameOgc,
        .updateGamepadGUID = _glfwUpdateGamepadGUIDOgc,
        .freeMonitor = _glfwFreeMonitorOgc,
        .getMonitorPos = _glfwGetMonitorPosOgc,
        .getMonitorContentScale = _glfwGetMonitorContentScaleOgc,
        .getMonitorWorkarea = _glfwGetMonitorWorkareaOgc,
        .getVideoModes = _glfwGetVideoModesOgc,
        .getVideoMode = _glfwGetVideoModeOgc,
        .getGammaRamp = _glfwGetGammaRampOgc,
        .setGammaRamp = _glfwSetGammaRampOgc,
        .createWindow = _glfwCreateWindowOgc,
        .destroyWindow = _glfwDestroyWindowOgc,
        .setWindowTitle = _glfwSetWindowTitleOgc,
        .setWindowIcon = _glfwSetWindowIconOgc,
        .getWindowPos = _glfwGetWindowPosOgc,
        .setWindowPos = _glfwSetWindowPosOgc,
        .getWindowSize = _glfwGetWindowSizeOgc,
        .setWindowSize = _glfwSetWindowSizeOgc,
        .setWindowSizeLimits = _glfwSetWindowSizeLimitsOgc,
        .setWindowAspectRatio = _glfwSetWindowAspectRatioOgc,
        .getFramebufferSize = _glfwGetFramebufferSizeOgc,
        .getWindowFrameSize = _glfwGetWindowFrameSizeOgc,
        .getWindowContentScale = _glfwGetWindowContentScaleOgc,
        .iconifyWindow = _glfwIconifyWindowOgc,
        .restoreWindow = _glfwRestoreWindowOgc,
        .maximizeWindow = _glfwMaximizeWindowOgc,
        .showWindow = _glfwShowWindowOgc,
        .hideWindow = _glfwHideWindowOgc,
        .requestWindowAttention = _glfwRequestWindowAttentionOgc,
        .focusWindow = _glfwFocusWindowOgc,
        .setWindowMonitor = _glfwSetWindowMonitorOgc,
        .windowFocused = _glfwWindowFocusedOgc,
        .windowIconified = _glfwWindowIconifiedOgc,
        .windowVisible = _glfwWindowVisibleOgc,
        .windowMaximized = _glfwWindowMaximizedOgc,
        .windowHovered = _glfwWindowHoveredOgc,
        .framebufferTransparent = _glfwFramebufferTransparentOgc,
        .getWindowOpacity = _glfwGetWindowOpacityOgc,
        .setWindowResizable = _glfwSetWindowResizableOgc,
        .setWindowDecorated = _glfwSetWindowDecoratedOgc,
        .setWindowFloating = _glfwSetWindowFloatingOgc,
        .setWindowOpacity = _glfwSetWindowOpacityOgc,
        .setWindowMousePassthrough = _glfwSetWindowMousePassthroughOgc,
        .pollEvents = _glfwPollEventsOgc,
        .waitEvents = _glfwWaitEventsOgc,
        .waitEventsTimeout = _glfwWaitEventsTimeoutOgc,
        .postEmptyEvent = _glfwPostEmptyEventOgc,
        .getEGLPlatform = _glfwGetEGLPlatformOgc,
        .getEGLNativeDisplay = _glfwGetEGLNativeDisplayOgc,
        .getEGLNativeWindow = _glfwGetEGLNativeWindowOgc,
        .getRequiredInstanceExtensions = _glfwGetRequiredInstanceExtensionsOgc,
        .getPhysicalDevicePresentationSupport = _glfwGetPhysicalDevicePresentationSupportOgc,
        .createWindowSurface = _glfwCreateWindowSurfaceOgc
    };

    *platform = ogc;
    return GLFW_TRUE;
}

int _glfwInitOgc(void)
{
    fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__);

    VIDEO_Init();

    void *fifoBuffer = MEM_K0_TO_K1(memalign(32, FIFO_SIZE));
    memset(fifoBuffer, 0, FIFO_SIZE);
    GX_Init(fifoBuffer, FIFO_SIZE);

    fatInitDefault();

    PAD_Init();
    KEYBOARD_Init(NULL);
#ifdef __wii__
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(WPAD_CHAN_ALL, 640, 480);
#endif

    _glfwCreateMonitorOgc();
    return GLFW_TRUE;
}

void _glfwTerminateOgc(void)
{
    _glfwTerminateEGL();
    _glfwTerminateOSMesa();
}

#endif // _GLFW_OGC

