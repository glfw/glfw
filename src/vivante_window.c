//========================================================================
// GLFW 3.3 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2019 Camilla Löwy <elmindreda@glfw.org>
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
#include <errno.h>
#include <stdlib.h>

// Wait for data to arrive using select
// TODO: because we get keyboard and mouse events via polling,
// we have to rewrite it using active waiting.
// Else we will wait only for device connected/disconnected events
//
static GLFWbool waitForEvent(double* timeout)
{
    fd_set fds;
    int maxFd = -1;
    int count = 0;
    
    if (_glfw.evdev.inotify > maxFd){
        count = _glfw.evdev.inotify + 1;
        maxFd = _glfw.evdev.inotify;
    }
    
    if (_glfw.linjs.inotify > maxFd){
        count = _glfw.linjs.inotify + 1;
        maxFd = _glfw.linjs.inotify;
    }
    // TODO: code above is based on x11_window.c:waitForEvent
    // I don't fully understand how it works so it can be done wrong way

    for (;;)
    {
        FD_ZERO(&fds);
        if (_glfw.evdev.inotify > 0)
            FD_SET(_glfw.evdev.inotify, &fds);
        if (_glfw.linjs.inotify > 0)
            FD_SET(_glfw.linjs.inotify, &fds);

        if (timeout)
        {
            const long seconds = (long) *timeout;
            const long microseconds = (long) ((*timeout - seconds) * 1e6);
            struct timeval tv = { seconds, microseconds };
            const uint64_t base = _glfwPlatformGetTimerValue();

            const int result = select(count, &fds, NULL, NULL, &tv);
            const int error = errno;

            *timeout -= (_glfwPlatformGetTimerValue() - base) /
                (double) _glfwPlatformGetTimerFrequency();

            if (result > 0)
                return GLFW_TRUE;
            if ((result == -1 && error == EINTR) || *timeout <= 0.0)
                return GLFW_FALSE;
        }
        else if (select(count, &fds, NULL, NULL, NULL) != -1 || errno != EINTR)
            return GLFW_TRUE;
    }
}

static int queryWindowGeometry(_GLFWwindow* window )
{
    if (!window->vivante.native_window)
        return GLFW_FALSE;
    
    fbGetWindowGeometry(window->vivante.native_window
                      , &window->vivante.xpos, &window->vivante.ypos
                      , &window->vivante.width, &window->vivante.height);
    
    return GLFW_TRUE;
}

static int createNativeWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig)
{
    int width = wndconfig->width;
    int height = wndconfig->height;
    int xpos = wndconfig->xpos;
    int ypos = wndconfig->ypos;

    if (window->monitor)
    {
        xpos = 0;
        ypos = 0;
        width = window->monitor->widthMM;
        height = window->monitor->heightMM;
    }
    else
    {
        if (xpos == GLFW_DONT_CARE)
            xpos = 0;
        if (ypos == GLFW_DONT_CARE)
            ypos = 0;
    }

    window->vivante.native_window = fbCreateWindow(_GLFW_EGL_NATIVE_DISPLAY, xpos, ypos, width, height);
    if (!window->vivante.native_window)
        return GLFW_FALSE;

    if (!queryWindowGeometry(window))
        return GLFW_FALSE;
    
    return GLFW_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWctxconfig* ctxconfig,
                              const _GLFWfbconfig* fbconfig)
{
    if (!createNativeWindow(window, wndconfig))
        return GLFW_FALSE;

    if (ctxconfig->client != GLFW_NO_API)
    {
        if (ctxconfig->source == GLFW_NATIVE_CONTEXT_API ||
            ctxconfig->source == GLFW_EGL_CONTEXT_API)
        {
            if (!_glfwInitEGL())
                return GLFW_FALSE;
            if (!_glfwCreateContextEGL(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }
        else
        {
            _glfwInputError(GLFW_API_UNAVAILABLE, "Vivante: EGL not available");
            return GLFW_FALSE;
        }
    }

    return GLFW_TRUE;
}

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
    if (_glfwPlatformWindowFocused(window))
        _glfw.vivante.focusedWindow = GLFW_FALSE;
    
    if (window->context.destroy)
        window->context.destroy(window);
    
    if (window->vivante.native_window){
        fbDestroyWindow(window->vivante.native_window);
        window->vivante.native_window = NULL;
    }
}

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
}

void _glfwPlatformSetWindowIcon(_GLFWwindow* window, int count,
                                const GLFWimage* images)
{
}

void _glfwPlatformSetWindowMonitor(_GLFWwindow* window,
                                   _GLFWmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate)
{
}

void _glfwPlatformGetWindowPos(_GLFWwindow* window, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = window->vivante.xpos;
    if (ypos)
        *ypos = window->vivante.ypos;
}

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int xpos, int ypos)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Vivante: Window position setting not supported");
}

void _glfwPlatformGetWindowSize(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->vivante.width;
    if (height)
        *height = window->vivante.height;
}

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Vivante: Window resizing not supported");
}

void _glfwPlatformSetWindowSizeLimits(_GLFWwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{
}

void _glfwPlatformSetWindowAspectRatio(_GLFWwindow* window, int n, int d)
{
}

void _glfwPlatformGetFramebufferSize(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->vivante.width;
    if (height)
        *height = window->vivante.height;
}

void _glfwPlatformGetWindowFrameSize(_GLFWwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
    if (left)
        *left = 0;
    if (top)
        *top = 0;
    if (right)
        *right = 0;
    if (bottom)
        *bottom = 0;
}

void _glfwPlatformGetWindowContentScale(_GLFWwindow* window,
                                        float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
}

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
}

void _glfwPlatformMaximizeWindow(_GLFWwindow* window)
{
}

int _glfwPlatformWindowMaximized(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

int _glfwPlatformWindowHovered(_GLFWwindow* window)
{
    double xpos = _glfw.vivante.cursorXpos;
    double ypos = _glfw.vivante.cursorYpos;
    xpos -= window->vivante.xpos;
    ypos -= window->vivante.ypos;
    
    if( xpos < 0.0 )
        return GLFW_FALSE;
    if( xpos > window->vivante.width )
        return GLFW_FALSE;
    if( ypos < 0.0 )
        return GLFW_FALSE;
    if( ypos > window->vivante.height )
        return GLFW_FALSE;
    
    return GLFW_TRUE;
}

int _glfwPlatformFramebufferTransparent(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

void _glfwPlatformSetWindowResizable(_GLFWwindow* window, GLFWbool enabled)
{
}

void _glfwPlatformSetWindowDecorated(_GLFWwindow* window, GLFWbool enabled)
{
}

void _glfwPlatformSetWindowFloating(_GLFWwindow* window, GLFWbool enabled)
{
}

float _glfwPlatformGetWindowOpacity(_GLFWwindow* window)
{
    return 1.f;
}

void _glfwPlatformSetWindowOpacity(_GLFWwindow* window, float opacity)
{
}

void _glfwPlatformSetRawMouseMotion(_GLFWwindow *window, GLFWbool enabled)
{
}

GLFWbool _glfwPlatformRawMouseMotionSupported(void)
{
    return GLFW_FALSE;
}

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
}


void _glfwPlatformRequestWindowAttention(_GLFWwindow* window)
{
}

void _glfwPlatformUnhideWindow(_GLFWwindow* window)
{
}

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
}

void _glfwPlatformFocusWindow(_GLFWwindow* window)
{
    _glfw.vivante.focusedWindow = window;
}

int _glfwPlatformWindowFocused(_GLFWwindow* window)
{
    return _glfw.vivante.focusedWindow == window;
}

int _glfwPlatformWindowIconified(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

int _glfwPlatformWindowVisible(_GLFWwindow* window)
{
    return GLFW_TRUE;
}

void _glfwPlatformPollEvents(void)
{
    _glfwDetectEvdevConnection();
    _glfwPollEvdevDevices();
}

void _glfwPlatformWaitEvents(void)
{
    waitForEvent(NULL);
    _glfwPlatformPollEvents();
}

void _glfwPlatformWaitEventsTimeout(double timeout)
{
    waitForEvent(&timeout);
    _glfwPlatformPollEvents();
}

void _glfwPlatformPostEmptyEvent(void)
{
}

void _glfwPlatformGetCursorPos(_GLFWwindow* window, double* xpos, double* ypos)
{
    double cursorXpos = _glfw.vivante.cursorXpos;
    double cursorYpos = _glfw.vivante.cursorYpos;
    cursorXpos -= window->vivante.xpos;
    cursorYpos -= window->vivante.ypos;
    
    if (xpos)
        *xpos = cursorXpos;
    if (ypos)
        *ypos = cursorYpos;
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double x, double y)
{
    if( x < 0.0 )
        return;
    if( x > window->vivante.width )
        return;
    if( y < 0.0 )
        return;
    if( y > window->vivante.height )
        return;
    
    _glfwVivanteSetCursorPos(x + window->vivante.xpos, y + window->vivante.ypos);
}

void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode)
{
}

int _glfwPlatformCreateCursor(_GLFWcursor* cursor,
                              const GLFWimage* image,
                              int xhot, int yhot)
{
    return GLFW_TRUE;
}

int _glfwPlatformCreateStandardCursor(_GLFWcursor* cursor, int shape)
{
    return GLFW_TRUE;
}

void _glfwPlatformDestroyCursor(_GLFWcursor* cursor)
{
}

void _glfwPlatformSetCursor(_GLFWwindow* window, _GLFWcursor* cursor)
{
}

void _glfwPlatformSetClipboardString(const char* string)
{
    if (_glfw.vivante.clipboardString)
        free(_glfw.vivante.clipboardString);
    _glfw.vivante.clipboardString = _glfw_strdup(string);
}

const char* _glfwPlatformGetClipboardString(void)
{
    return _glfw.vivante.clipboardString;
}

const char* _glfwPlatformGetScancodeName(int scancode)
{
    return "";
}

int _glfwPlatformGetKeyScancode(int key)
{
    return -1;
}

void _glfwPlatformGetRequiredInstanceExtensions(char** extensions)
{
}

int _glfwPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    return GLFW_FALSE;
}

VkResult _glfwPlatformCreateWindowSurface(VkInstance instance,
                                          _GLFWwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface)
{
    // This seems like the most appropriate error to return here
    return VK_ERROR_INITIALIZATION_FAILED;
}

