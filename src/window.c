//========================================================================
// GLFW - An OpenGL library
// Platform:    Any
// API version: 3.0
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
// Copyright (c) 2012 Torsten Walluhn <tw@mad-cad.net>
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

#include <string.h>
#include <stdlib.h>
#if defined(_MSC_VER)
 #include <malloc.h>
#endif


//========================================================================
// Return the maxiumum of the specified values
//========================================================================

static int Max(int a, int b)
{
    return (a > b) ? a : b;
}


//========================================================================
// Clear scroll offsets for all windows
//========================================================================

static void clearScrollOffsets(void)
{
    _GLFWwindow* window;

    for (window = _glfw.windowListHead;  window;  window = window->next)
    {
        window->scrollX = 0;
        window->scrollY = 0;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Register window focus events
//========================================================================

void _glfwInputWindowFocus(_GLFWwindow* window, GLboolean focused)
{
    if (focused)
    {
        if (_glfw.focusedWindow != window)
        {
            _glfw.focusedWindow = window;

            if (window->windowFocusCallback)
                window->windowFocusCallback(window, focused);
        }
    }
    else
    {
        if (_glfw.focusedWindow == window)
        {
            int i;

            // Release all pressed keyboard keys
            for (i = 0;  i <= GLFW_KEY_LAST;  i++)
            {
                if (window->key[i] == GLFW_PRESS)
                    _glfwInputKey(window, i, GLFW_RELEASE);
            }

            // Release all pressed mouse buttons
            for (i = 0;  i <= GLFW_MOUSE_BUTTON_LAST;  i++)
            {
                if (window->mouseButton[i] == GLFW_PRESS)
                    _glfwInputMouseClick(window, i, GLFW_RELEASE);
            }

            _glfw.focusedWindow = NULL;

            if (window->windowFocusCallback)
                window->windowFocusCallback(window, focused);
        }
    }
}


//========================================================================
// Register window position events
//========================================================================

void _glfwInputWindowPos(_GLFWwindow* window, int x, int y)
{
    if (window->positionX == x && window->positionY == y)
        return;

    window->positionX = x;
    window->positionY = y;

    if (window->windowPosCallback)
        window->windowPosCallback(window, x, y);
}


//========================================================================
// Register window size events
//========================================================================

void _glfwInputWindowSize(_GLFWwindow* window, int width, int height)
{
    if (window->width == width && window->height == height)
        return;

    window->width = width;
    window->height = height;

    if (window->windowSizeCallback)
        window->windowSizeCallback(window, width, height);
}


//========================================================================
// Register window size events
//========================================================================

void _glfwInputWindowIconify(_GLFWwindow* window, int iconified)
{
    if (window->iconified == iconified)
        return;

    window->iconified = iconified;

    if (window->windowIconifyCallback)
        window->windowIconifyCallback(window, iconified);
}


//========================================================================
// Register window visibility events
//========================================================================

void _glfwInputWindowVisibility(_GLFWwindow* window, int visible)
{
    window->visible = visible;
}


//========================================================================
// Register window damage events
//========================================================================

void _glfwInputWindowDamage(_GLFWwindow* window)
{
    if (window->windowRefreshCallback)
        window->windowRefreshCallback(window);
}


//========================================================================
// Register window close request events
//========================================================================

void _glfwInputWindowCloseRequest(_GLFWwindow* window)
{
    if (window->windowCloseCallback)
        window->closeRequested = window->windowCloseCallback(window);
    else
        window->closeRequested = GL_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Create the GLFW window and its associated context
//========================================================================

GLFWAPI GLFWwindow glfwCreateWindow(int width, int height,
                                    const char* title,
                                    GLFWmonitor monitor,
                                    GLFWwindow share)
{
    _GLFWfbconfig fbconfig;
    _GLFWwndconfig wndconfig;
    _GLFWwindow* window;
    _GLFWwindow* previous;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    // We need to copy these values before doing anything that can fail, as the
    // window hints should be cleared after each call even if it fails

    // Set up desired framebuffer config
    fbconfig.redBits        = Max(_glfw.hints.redBits, 0);
    fbconfig.greenBits      = Max(_glfw.hints.greenBits, 0);
    fbconfig.blueBits       = Max(_glfw.hints.blueBits, 0);
    fbconfig.alphaBits      = Max(_glfw.hints.alphaBits, 0);
    fbconfig.depthBits      = Max(_glfw.hints.depthBits, 0);
    fbconfig.stencilBits    = Max(_glfw.hints.stencilBits, 0);
    fbconfig.accumRedBits   = Max(_glfw.hints.accumRedBits, 0);
    fbconfig.accumGreenBits = Max(_glfw.hints.accumGreenBits, 0);
    fbconfig.accumBlueBits  = Max(_glfw.hints.accumBlueBits, 0);
    fbconfig.accumAlphaBits = Max(_glfw.hints.accumAlphaBits, 0);
    fbconfig.auxBuffers     = Max(_glfw.hints.auxBuffers, 0);
    fbconfig.stereo         = _glfw.hints.stereo ? GL_TRUE : GL_FALSE;
    fbconfig.samples        = Max(_glfw.hints.samples, 0);
    fbconfig.sRGB           = _glfw.hints.sRGB ? GL_TRUE : GL_FALSE;

    // Set up desired window config
    wndconfig.title         = title;
    wndconfig.resizable     = _glfw.hints.resizable ? GL_TRUE : GL_FALSE;
    wndconfig.visible       = _glfw.hints.visible ? GL_TRUE : GL_FALSE;
    wndconfig.positionX     = _glfw.hints.positionX;
    wndconfig.positionY     = _glfw.hints.positionY;
    wndconfig.clientAPI     = _glfw.hints.clientAPI;
    wndconfig.glMajor       = _glfw.hints.glMajor;
    wndconfig.glMinor       = _glfw.hints.glMinor;
    wndconfig.glForward     = _glfw.hints.glForward ? GL_TRUE : GL_FALSE;
    wndconfig.glDebug       = _glfw.hints.glDebug ? GL_TRUE : GL_FALSE;
    wndconfig.glProfile     = _glfw.hints.glProfile;
    wndconfig.glRobustness  = _glfw.hints.glRobustness;
    wndconfig.monitor       = (_GLFWmonitor*) monitor;
    wndconfig.share         = (_GLFWwindow*) share;

    // Check the OpenGL bits of the window config
    if (!_glfwIsValidContextConfig(&wndconfig))
        return GL_FALSE;

    // Save the currently current context so it can be restored later
    previous = glfwGetCurrentContext();

    if (width <= 0 || height <= 0)
    {
        _glfwInputError(GLFW_INVALID_VALUE, "Invalid window size");
        return GL_FALSE;
    }

    window = (_GLFWwindow*) calloc(1, sizeof(_GLFWwindow));
    if (!window)
    {
        _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
        return NULL;
    }

    window->next = _glfw.windowListHead;
    _glfw.windowListHead = window;

    // Remember window settings
    window->width      = width;
    window->height     = height;
    window->resizable  = wndconfig.resizable;
    window->cursorMode = GLFW_CURSOR_NORMAL;

    window->monitor = wndconfig.monitor;
    if (window->monitor)
    {
        window->videoMode.width     = width;
        window->videoMode.height    = height;
        window->videoMode.redBits   = fbconfig.redBits;
        window->videoMode.greenBits = fbconfig.greenBits;
        window->videoMode.blueBits  = fbconfig.blueBits;
    }

    // Open the actual window and create its context
    if (!_glfwPlatformCreateWindow(window, &wndconfig, &fbconfig))
    {
        glfwDestroyWindow(window);
        glfwMakeContextCurrent(previous);
        return GL_FALSE;
    }

    glfwMakeContextCurrent(window);

    // Cache the actual (as opposed to requested) context parameters
    if (!_glfwRefreshContextParams())
    {
        glfwDestroyWindow(window);
        glfwMakeContextCurrent(previous);
        return GL_FALSE;
    }

    // Verify the context against the requested parameters
    if (!_glfwIsValidContext(&wndconfig))
    {
        glfwDestroyWindow(window);
        glfwMakeContextCurrent(previous);
        return GL_FALSE;
    }

    // Clearing the front buffer to black to avoid garbage pixels left over
    // from previous uses of our bit of VRAM
    glClear(GL_COLOR_BUFFER_BIT);
    _glfwPlatformSwapBuffers(window);

    // Restore the previously current context (or NULL)
    glfwMakeContextCurrent(previous);

    // The GLFW specification states that fullscreen windows have the cursor
    // captured by default
    if (wndconfig.monitor)
        glfwSetInputMode(window, GLFW_CURSOR_MODE, GLFW_CURSOR_CAPTURED);

    if (wndconfig.monitor == NULL && wndconfig.visible)
        glfwShowWindow(window);

    return window;
}


//========================================================================
// Reset all window hints to their default values
//========================================================================

void glfwDefaultWindowHints(void)
{
    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    memset(&_glfw.hints, 0, sizeof(_glfw.hints));

    // The default is OpenGL with minimum version 1.0
    _glfw.hints.clientAPI = GLFW_OPENGL_API;
    _glfw.hints.glMajor = 1;
    _glfw.hints.glMinor = 0;

    // The default is to show the window and allow window resizing
    _glfw.hints.resizable = GL_TRUE;
    _glfw.hints.visible   = GL_TRUE;

    // The default window position is the upper left corner of the screen
    _glfw.hints.positionX = 0;
    _glfw.hints.positionY = 0;

    // The default is 24 bits of color, 24 bits of depth and 8 bits of stencil
    _glfw.hints.redBits     = 8;
    _glfw.hints.greenBits   = 8;
    _glfw.hints.blueBits    = 8;
    _glfw.hints.depthBits   = 24;
    _glfw.hints.stencilBits = 8;
}


//========================================================================
// Set hints for creating the window
//========================================================================

GLFWAPI void glfwWindowHint(int target, int hint)
{
    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    switch (target)
    {
        case GLFW_RED_BITS:
            _glfw.hints.redBits = hint;
            break;
        case GLFW_GREEN_BITS:
            _glfw.hints.greenBits = hint;
            break;
        case GLFW_BLUE_BITS:
            _glfw.hints.blueBits = hint;
            break;
        case GLFW_ALPHA_BITS:
            _glfw.hints.alphaBits = hint;
            break;
        case GLFW_DEPTH_BITS:
            _glfw.hints.depthBits = hint;
            break;
        case GLFW_STENCIL_BITS:
            _glfw.hints.stencilBits = hint;
            break;
        case GLFW_ACCUM_RED_BITS:
            _glfw.hints.accumRedBits = hint;
            break;
        case GLFW_ACCUM_GREEN_BITS:
            _glfw.hints.accumGreenBits = hint;
            break;
        case GLFW_ACCUM_BLUE_BITS:
            _glfw.hints.accumBlueBits = hint;
            break;
        case GLFW_ACCUM_ALPHA_BITS:
            _glfw.hints.accumAlphaBits = hint;
            break;
        case GLFW_AUX_BUFFERS:
            _glfw.hints.auxBuffers = hint;
            break;
        case GLFW_STEREO:
            _glfw.hints.stereo = hint;
            break;
        case GLFW_RESIZABLE:
            _glfw.hints.resizable = hint;
            break;
        case GLFW_VISIBLE:
            _glfw.hints.visible = hint;
            break;
        case GLFW_POSITION_X:
            _glfw.hints.positionX = hint;
            break;
        case GLFW_POSITION_Y:
            _glfw.hints.positionY = hint;
            break;
        case GLFW_SAMPLES:
            _glfw.hints.samples = hint;
            break;
        case GLFW_SRGB_CAPABLE:
            _glfw.hints.sRGB = hint;
            break;
        case GLFW_CLIENT_API:
            _glfw.hints.clientAPI = hint;
            break;
        case GLFW_CONTEXT_VERSION_MAJOR:
            _glfw.hints.glMajor = hint;
            break;
        case GLFW_CONTEXT_VERSION_MINOR:
            _glfw.hints.glMinor = hint;
            break;
        case GLFW_CONTEXT_ROBUSTNESS:
            _glfw.hints.glRobustness = hint;
            break;
        case GLFW_OPENGL_FORWARD_COMPAT:
            _glfw.hints.glForward = hint;
            break;
        case GLFW_OPENGL_DEBUG_CONTEXT:
            _glfw.hints.glDebug = hint;
            break;
        case GLFW_OPENGL_PROFILE:
            _glfw.hints.glProfile = hint;
            break;
        default:
            _glfwInputError(GLFW_INVALID_ENUM, NULL);
            break;
    }
}


//========================================================================
// Properly kill the window / video display
//========================================================================

GLFWAPI void glfwDestroyWindow(GLFWwindow handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    // Allow closing of NULL (to match the behavior of free)
    if (window == NULL)
        return;

    // Clear all callbacks to avoid exposing a half torn-down window object
    window->windowPosCallback = NULL;
    window->windowSizeCallback = NULL;
    window->windowCloseCallback = NULL;
    window->windowRefreshCallback = NULL;
    window->windowFocusCallback = NULL;
    window->windowIconifyCallback = NULL;
    window->mouseButtonCallback = NULL;
    window->cursorPosCallback = NULL;
    window->cursorEnterCallback = NULL;
    window->scrollCallback = NULL;
    window->keyCallback = NULL;
    window->charCallback = NULL;

    // The window's context must not be current on another thread when the
    // window is destroyed
    if (window == _glfwPlatformGetCurrentContext())
        _glfwPlatformMakeContextCurrent(NULL);

    // Clear the focused window pointer if this is the focused window
    if (window == _glfw.focusedWindow)
        _glfw.focusedWindow = NULL;

    _glfwPlatformDestroyWindow(window);

    // Unlink window from global linked list
    {
        _GLFWwindow** prev = &_glfw.windowListHead;

        while (*prev != window)
            prev = &((*prev)->next);

        *prev = window->next;
    }

    free(window);
}


//========================================================================
// Set the window title
//========================================================================

GLFWAPI void glfwSetWindowTitle(GLFWwindow handle, const char* title)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    _glfwPlatformSetWindowTitle(window, title);
}


//========================================================================
// Get the window size
//========================================================================

GLFWAPI void glfwGetWindowSize(GLFWwindow handle, int* width, int* height)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (width != NULL)
        *width = window->width;

    if (height != NULL)
        *height = window->height;
}


//========================================================================
// Set the window size
//========================================================================

GLFWAPI void glfwSetWindowSize(GLFWwindow handle, int width, int height)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (window->iconified)
    {
        // TODO: Figure out if this is an error
        return;
    }

    // Don't do anything if the window size did not change
    if (width == window->width && height == window->height)
        return;

    _glfwPlatformSetWindowSize(window, width, height);
}


//========================================================================
// Window iconification
//========================================================================

GLFWAPI void glfwIconifyWindow(GLFWwindow handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (window->iconified)
        return;

    _glfwPlatformIconifyWindow(window);
}


//========================================================================
// Window un-iconification
//========================================================================

GLFWAPI void glfwRestoreWindow(GLFWwindow handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (!window->iconified)
        return;

    _glfwPlatformRestoreWindow(window);
}


//========================================================================
// Window show
//========================================================================

GLFWAPI void glfwShowWindow(GLFWwindow handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (window->monitor)
        return;

    _glfwPlatformShowWindow(window);
}


//========================================================================
// Window hide
//========================================================================

GLFWAPI void glfwHideWindow(GLFWwindow handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (window->monitor)
        return;

    _glfwPlatformHideWindow(window);
}


//========================================================================
// Get window parameter
//========================================================================

GLFWAPI int glfwGetWindowParam(GLFWwindow handle, int param)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return 0;
    }

    switch (param)
    {
        case GLFW_FOCUSED:
            return window == _glfw.focusedWindow;
        case GLFW_ICONIFIED:
            return window->iconified;
        case GLFW_SHOULD_CLOSE:
            return window->closeRequested;
        case GLFW_RESIZABLE:
            return window->resizable;
        case GLFW_VISIBLE:
            return window->visible;
        case GLFW_POSITION_X:
            return window->positionX;
        case GLFW_POSITION_Y:
            return window->positionY;
        case GLFW_CLIENT_API:
            return window->clientAPI;
        case GLFW_CONTEXT_VERSION_MAJOR:
            return window->glMajor;
        case GLFW_CONTEXT_VERSION_MINOR:
            return window->glMinor;
        case GLFW_CONTEXT_REVISION:
            return window->glRevision;
        case GLFW_CONTEXT_ROBUSTNESS:
            return window->glRobustness;
        case GLFW_OPENGL_FORWARD_COMPAT:
            return window->glForward;
        case GLFW_OPENGL_DEBUG_CONTEXT:
            return window->glDebug;
        case GLFW_OPENGL_PROFILE:
            return window->glProfile;
    }

    _glfwInputError(GLFW_INVALID_ENUM, NULL);
    return 0;
}


//========================================================================
// Get window monitor
//========================================================================

GLFWAPI GLFWmonitor glfwGetWindowMonitor(GLFWwindow handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    return (GLFWmonitor) window->monitor;
}


//========================================================================
// Set the user pointer for the specified window
//========================================================================

GLFWAPI void glfwSetWindowUserPointer(GLFWwindow handle, void* pointer)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    window->userPointer = pointer;
}


//========================================================================
// Get the user pointer for the specified window
//========================================================================

GLFWAPI void* glfwGetWindowUserPointer(GLFWwindow handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    return window->userPointer;
}


//========================================================================
// Set callback function for window position changes
//========================================================================

GLFWAPI void glfwSetWindowPosCallback(GLFWwindow handle, GLFWwindowposfun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    window->windowPosCallback = cbfun;
}


//========================================================================
// Set callback function for window size changes
//========================================================================

GLFWAPI void glfwSetWindowSizeCallback(GLFWwindow handle, GLFWwindowsizefun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    window->windowSizeCallback = cbfun;
}


//========================================================================
// Set callback function for window close events
//========================================================================

GLFWAPI void glfwSetWindowCloseCallback(GLFWwindow handle, GLFWwindowclosefun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    window->windowCloseCallback = cbfun;
}


//========================================================================
// Set callback function for window refresh events
//========================================================================

GLFWAPI void glfwSetWindowRefreshCallback(GLFWwindow handle, GLFWwindowrefreshfun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    window->windowRefreshCallback = cbfun;
}


//========================================================================
// Set callback function for window focus events
//========================================================================

GLFWAPI void glfwSetWindowFocusCallback(GLFWwindow handle, GLFWwindowfocusfun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    window->windowFocusCallback = cbfun;
}


//========================================================================
// Set callback function for window iconification events
//========================================================================

GLFWAPI void glfwSetWindowIconifyCallback(GLFWwindow handle, GLFWwindowiconifyfun cbfun)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    window->windowIconifyCallback = cbfun;
}


//========================================================================
// Poll for new window and input events
//========================================================================

GLFWAPI void glfwPollEvents(void)
{
    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    clearScrollOffsets();

    _glfwPlatformPollEvents();
}


//========================================================================
// Wait for new window and input events
//========================================================================

GLFWAPI void glfwWaitEvents(void)
{
    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    clearScrollOffsets();

    _glfwPlatformWaitEvents();
}

