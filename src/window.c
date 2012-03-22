//========================================================================
// GLFW - An OpenGL library
// Platform:    Any
// API version: 3.0
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
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


//========================================================================
// Return the maxiumum of the specified values
//========================================================================

static int Max(int a, int b)
{
    return (a > b) ? a : b;
}


//========================================================================
// Close all GLFW windows with the closed flag set
//========================================================================

static void closeFlaggedWindows(void)
{
    _GLFWwindow* window;

    for (window = _glfwLibrary.windowListHead;  window; )
    {
        if (window->closeRequested && _glfwLibrary.windowCloseCallback)
            window->closeRequested = _glfwLibrary.windowCloseCallback(window);

        if (window->closeRequested)
        {
            _GLFWwindow* next = window->next;
            glfwCloseWindow(window);
            window = next;
        }
        else
            window = window->next;
    }
}


//========================================================================
// Clear scroll offsets for all windows
//========================================================================

static void clearScrollOffsets(void)
{
    _GLFWwindow* window;

    for (window = _glfwLibrary.windowListHead;  window;  window = window->next)
    {
        window->scrollX = 0;
        window->scrollY = 0;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Reset all window hints to their default values
//========================================================================

void _glfwSetDefaultWindowHints(void)
{
    memset(&_glfwLibrary.hints, 0, sizeof(_glfwLibrary.hints));

    // The default minimum OpenGL version is 1.0
    _glfwLibrary.hints.glMajor = 1;
    _glfwLibrary.hints.glMinor = 0;

    // The default is to allow window resizing
    _glfwLibrary.hints.resizable = GL_TRUE;
}


//========================================================================
// Register window focus events
//========================================================================

void _glfwInputWindowFocus(_GLFWwindow* window, GLboolean activated)
{
    if (activated)
    {
        if (_glfwLibrary.activeWindow != window)
        {
            _glfwLibrary.activeWindow = window;

            if (_glfwLibrary.windowFocusCallback)
                _glfwLibrary.windowFocusCallback(window, activated);
        }
    }
    else
    {
        if (_glfwLibrary.activeWindow == window)
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

            _glfwLibrary.activeWindow = NULL;

            if (_glfwLibrary.windowFocusCallback)
                _glfwLibrary.windowFocusCallback(window, activated);
        }
    }
}


//========================================================================
// Register window position events
//========================================================================

void _glfwInputWindowPos(_GLFWwindow* window, int x, int y)
{
    window->positionX = x;
    window->positionY = y;
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

    if (_glfwLibrary.windowSizeCallback)
        _glfwLibrary.windowSizeCallback(window, width, height);
}


//========================================================================
// Register window size events
//========================================================================

void _glfwInputWindowIconify(_GLFWwindow* window, int iconified)
{
    if (window->iconified == iconified)
        return;

    window->iconified = iconified;

    if (_glfwLibrary.windowIconifyCallback)
        _glfwLibrary.windowIconifyCallback(window, iconified);
}


//========================================================================
// Register window damage events
//========================================================================

void _glfwInputWindowDamage(_GLFWwindow* window)
{
    if (_glfwLibrary.windowRefreshCallback)
        _glfwLibrary.windowRefreshCallback(window);
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Create the GLFW window and its associated context
//========================================================================

GLFWAPI GLFWwindow glfwOpenWindow(int width, int height,
                                  int mode, const char* title,
                                  GLFWwindow share)
{
    _GLFWfbconfig fbconfig;
    _GLFWwndconfig wndconfig;
    _GLFWwindow* window;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    // We need to copy these values before doing anything that can fail, as the
    // window hints should be cleared after each call even if it fails

    // Set up desired framebuffer config
    fbconfig.redBits        = Max(_glfwLibrary.hints.redBits, 0);
    fbconfig.greenBits      = Max(_glfwLibrary.hints.greenBits, 0);
    fbconfig.blueBits       = Max(_glfwLibrary.hints.blueBits, 0);
    fbconfig.alphaBits      = Max(_glfwLibrary.hints.alphaBits, 0);
    fbconfig.depthBits      = Max(_glfwLibrary.hints.depthBits, 0);
    fbconfig.stencilBits    = Max(_glfwLibrary.hints.stencilBits, 0);
    fbconfig.accumRedBits   = Max(_glfwLibrary.hints.accumRedBits, 0);
    fbconfig.accumGreenBits = Max(_glfwLibrary.hints.accumGreenBits, 0);
    fbconfig.accumBlueBits  = Max(_glfwLibrary.hints.accumBlueBits, 0);
    fbconfig.accumAlphaBits = Max(_glfwLibrary.hints.accumAlphaBits, 0);
    fbconfig.auxBuffers     = Max(_glfwLibrary.hints.auxBuffers, 0);
    fbconfig.stereo         = _glfwLibrary.hints.stereo ? GL_TRUE : GL_FALSE;
    fbconfig.samples        = Max(_glfwLibrary.hints.samples, 0);

    // Set up desired window config
    wndconfig.mode           = mode;
    wndconfig.title          = title;
    wndconfig.refreshRate    = Max(_glfwLibrary.hints.refreshRate, 0);
    wndconfig.resizable      = _glfwLibrary.hints.resizable ? GL_TRUE : GL_FALSE;
    wndconfig.glMajor        = _glfwLibrary.hints.glMajor;
    wndconfig.glMinor        = _glfwLibrary.hints.glMinor;
    wndconfig.glForward      = _glfwLibrary.hints.glForward ? GL_TRUE : GL_FALSE;
    wndconfig.glDebug        = _glfwLibrary.hints.glDebug ? GL_TRUE : GL_FALSE;
    wndconfig.glProfile      = _glfwLibrary.hints.glProfile;
    wndconfig.glRobustness   = _glfwLibrary.hints.glRobustness ? GL_TRUE : GL_FALSE;
    wndconfig.share          = share;

    // Reset to default values for the next call
    _glfwSetDefaultWindowHints();

    // Check the OpenGL bits of the window config
    if (!_glfwIsValidContextConfig(&wndconfig))
        return GL_FALSE;

    if (mode != GLFW_WINDOWED && mode != GLFW_FULLSCREEN)
    {
        _glfwSetError(GLFW_INVALID_ENUM,
                      "glfwOpenWindow: Invalid enum for 'mode' parameter");
        return GL_FALSE;
    }

    // Check width & height
    if (width > 0 && height <= 0)
    {
        // Set the window aspect ratio to 4:3
        height = (width * 3) / 4;
    }
    else if (width <= 0 && height > 0)
    {
        // Set the window aspect ratio to 4:3
        width = (height * 4) / 3;
    }
    else if (width <= 0 && height <= 0)
    {
        // Default window size
        width  = 640;
        height = 480;
    }

    window = (_GLFWwindow*) malloc(sizeof(_GLFWwindow));
    if (!window)
    {
        _glfwSetError(GLFW_OUT_OF_MEMORY,
                      "glfwOpenWindow: Failed to allocate window structure");
        return NULL;
    }

    memset(window, 0, sizeof(_GLFWwindow));

    window->next = _glfwLibrary.windowListHead;
    _glfwLibrary.windowListHead = window;

    // Remember window settings
    window->width      = width;
    window->height     = height;
    window->mode       = mode;
    window->cursorMode = GLFW_CURSOR_NORMAL;
    window->systemKeys = GL_TRUE;

    // Open the actual window and create its context
    if (!_glfwPlatformOpenWindow(window, &wndconfig, &fbconfig))
    {
        glfwCloseWindow(window);
        return GL_FALSE;
    }

    // Cache the actual (as opposed to desired) window parameters
    glfwMakeContextCurrent(window);
    _glfwPlatformRefreshWindowParams();

    if (!_glfwIsValidContext(window, &wndconfig))
    {
        glfwCloseWindow(window);
        return GL_FALSE;
    }

    // The GLFW specification states that fullscreen windows have the cursor
    // captured by default
    if (mode == GLFW_FULLSCREEN)
        glfwSetInputMode(window, GLFW_CURSOR_MODE, GLFW_CURSOR_CAPTURED);

    // Clearing the front buffer to black to avoid garbage pixels left over
    // from previous uses of our bit of VRAM
    glClear(GL_COLOR_BUFFER_BIT);
    _glfwPlatformSwapBuffers();

    return window;
}


//========================================================================
// Returns GL_TRUE if the specified window handle is an actual window
//========================================================================

GLFWAPI int glfwIsWindow(GLFWwindow handle)
{
    _GLFWwindow* entry;
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return GL_FALSE;
    }

    if (window == NULL)
        return GL_FALSE;

    for (entry = _glfwLibrary.windowListHead;  entry;  entry = entry->next)
    {
        if (entry == window)
            return GL_TRUE;
    }

    return GL_FALSE;
}


//========================================================================
// Set hints for opening the window
//========================================================================

GLFWAPI void glfwOpenWindowHint(int target, int hint)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    switch (target)
    {
        case GLFW_RED_BITS:
            _glfwLibrary.hints.redBits = hint;
            break;
        case GLFW_GREEN_BITS:
            _glfwLibrary.hints.greenBits = hint;
            break;
        case GLFW_BLUE_BITS:
            _glfwLibrary.hints.blueBits = hint;
            break;
        case GLFW_ALPHA_BITS:
            _glfwLibrary.hints.alphaBits = hint;
            break;
        case GLFW_DEPTH_BITS:
            _glfwLibrary.hints.depthBits = hint;
            break;
        case GLFW_STENCIL_BITS:
            _glfwLibrary.hints.stencilBits = hint;
            break;
        case GLFW_REFRESH_RATE:
            _glfwLibrary.hints.refreshRate = hint;
            break;
        case GLFW_ACCUM_RED_BITS:
            _glfwLibrary.hints.accumRedBits = hint;
            break;
        case GLFW_ACCUM_GREEN_BITS:
            _glfwLibrary.hints.accumGreenBits = hint;
            break;
        case GLFW_ACCUM_BLUE_BITS:
            _glfwLibrary.hints.accumBlueBits = hint;
            break;
        case GLFW_ACCUM_ALPHA_BITS:
            _glfwLibrary.hints.accumAlphaBits = hint;
            break;
        case GLFW_AUX_BUFFERS:
            _glfwLibrary.hints.auxBuffers = hint;
            break;
        case GLFW_STEREO:
            _glfwLibrary.hints.stereo = hint;
            break;
        case GLFW_WINDOW_RESIZABLE:
            _glfwLibrary.hints.resizable = hint;
            break;
        case GLFW_FSAA_SAMPLES:
            _glfwLibrary.hints.samples = hint;
            break;
        case GLFW_OPENGL_VERSION_MAJOR:
            _glfwLibrary.hints.glMajor = hint;
            break;
        case GLFW_OPENGL_VERSION_MINOR:
            _glfwLibrary.hints.glMinor = hint;
            break;
        case GLFW_OPENGL_FORWARD_COMPAT:
            _glfwLibrary.hints.glForward = hint;
            break;
        case GLFW_OPENGL_DEBUG_CONTEXT:
            _glfwLibrary.hints.glDebug = hint;
            break;
        case GLFW_OPENGL_PROFILE:
            _glfwLibrary.hints.glProfile = hint;
            break;
        case GLFW_OPENGL_ROBUSTNESS:
            _glfwLibrary.hints.glRobustness = hint;
            break;
        default:
            _glfwSetError(GLFW_INVALID_ENUM, NULL);
            break;
    }
}


//========================================================================
// Properly kill the window / video display
//========================================================================

GLFWAPI void glfwCloseWindow(GLFWwindow handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    // Allow closing of NULL (to match the behavior of free)
    if (window == NULL)
        return;

    // Clear the current context if this window's context is current
    if (window == _glfwLibrary.currentWindow)
        glfwMakeContextCurrent(NULL);

    // Clear the active window pointer if this is the active window
    if (window == _glfwLibrary.activeWindow)
        _glfwLibrary.activeWindow = NULL;

    _glfwPlatformCloseWindow(window);

    // Unlink window from global linked list
    {
        _GLFWwindow** prev = &_glfwLibrary.windowListHead;

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
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
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
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
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
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
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

    if (window->mode == GLFW_FULLSCREEN)
    {
        // Refresh window parameters (may have changed due to changed video
        // modes)
        _glfwPlatformRefreshWindowParams();
    }
}


//========================================================================
// Get the window position
//========================================================================

GLFWAPI void glfwGetWindowPos(GLFWwindow handle, int* xpos, int* ypos)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (xpos != NULL)
        *xpos = window->positionX;

    if (ypos != NULL)
        *ypos = window->positionY;
}


//========================================================================
// Set the window position
//========================================================================

GLFWAPI void glfwSetWindowPos(GLFWwindow handle, int xpos, int ypos)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (window->mode == GLFW_FULLSCREEN || window->iconified)
    {
        // TODO: Figure out if this is an error
        return;
    }

    _glfwPlatformSetWindowPos(window, xpos, ypos);
}


//========================================================================
// Window iconification
//========================================================================

GLFWAPI void glfwIconifyWindow(GLFWwindow handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
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
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (!window->iconified)
        return;

    _glfwPlatformRestoreWindow(window);

    if (window->mode == GLFW_FULLSCREEN)
        _glfwPlatformRefreshWindowParams();
}


//========================================================================
// Get window parameter
//========================================================================

GLFWAPI int glfwGetWindowParam(GLFWwindow handle, int param)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return 0;
    }

    switch (param)
    {
        case GLFW_ACTIVE:
            return window == _glfwLibrary.activeWindow;
        case GLFW_ICONIFIED:
            return window->iconified;
        case GLFW_ACCELERATED:
            return window->accelerated;
        case GLFW_RED_BITS:
            return window->redBits;
        case GLFW_GREEN_BITS:
            return window->greenBits;
        case GLFW_BLUE_BITS:
            return window->blueBits;
        case GLFW_ALPHA_BITS:
            return window->alphaBits;
        case GLFW_DEPTH_BITS:
            return window->depthBits;
        case GLFW_STENCIL_BITS:
            return window->stencilBits;
        case GLFW_ACCUM_RED_BITS:
            return window->accumRedBits;
        case GLFW_ACCUM_GREEN_BITS:
            return window->accumGreenBits;
        case GLFW_ACCUM_BLUE_BITS:
            return window->accumBlueBits;
        case GLFW_ACCUM_ALPHA_BITS:
            return window->accumAlphaBits;
        case GLFW_AUX_BUFFERS:
            return window->auxBuffers;
        case GLFW_STEREO:
            return window->stereo;
        case GLFW_REFRESH_RATE:
            return window->refreshRate;
        case GLFW_WINDOW_RESIZABLE:
            return window->resizable;
        case GLFW_FSAA_SAMPLES:
            return window->samples;
        case GLFW_OPENGL_VERSION_MAJOR:
            return window->glMajor;
        case GLFW_OPENGL_VERSION_MINOR:
            return window->glMinor;
        case GLFW_OPENGL_REVISION:
            return window->glRevision;
        case GLFW_OPENGL_FORWARD_COMPAT:
            return window->glForward;
        case GLFW_OPENGL_DEBUG_CONTEXT:
            return window->glDebug;
        case GLFW_OPENGL_PROFILE:
            return window->glProfile;
        case GLFW_OPENGL_ROBUSTNESS:
            return window->glRobustness;
    }

    _glfwSetError(GLFW_INVALID_ENUM, NULL);
    return 0;
}


//========================================================================
// Set the user pointer for the specified window
//========================================================================

GLFWAPI void glfwSetWindowUserPointer(GLFWwindow handle, void* pointer)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
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
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    return window->userPointer;
}


//========================================================================
// Set callback function for window size changes
//========================================================================

GLFWAPI void glfwSetWindowSizeCallback(GLFWwindowsizefun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    _glfwLibrary.windowSizeCallback = cbfun;

    // Call the callback function to let the application know the current
    // window size
    if (cbfun)
    {
        _GLFWwindow* window;

        for (window = _glfwLibrary.windowListHead;  window;  window = window->next)
            cbfun(window, window->width, window->height);
    }
}


//========================================================================
// Set callback function for window close events
//========================================================================

GLFWAPI void glfwSetWindowCloseCallback(GLFWwindowclosefun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    _glfwLibrary.windowCloseCallback = cbfun;
}


//========================================================================
// Set callback function for window refresh events
//========================================================================

GLFWAPI void glfwSetWindowRefreshCallback(GLFWwindowrefreshfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    _glfwLibrary.windowRefreshCallback = cbfun;
}


//========================================================================
// Set callback function for window focus events
//========================================================================

GLFWAPI void glfwSetWindowFocusCallback(GLFWwindowfocusfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    _glfwLibrary.windowFocusCallback = cbfun;
}


//========================================================================
// Set callback function for window iconification events
//========================================================================

GLFWAPI void glfwSetWindowIconifyCallback(GLFWwindowiconifyfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    _glfwLibrary.windowIconifyCallback = cbfun;
}


//========================================================================
// Poll for new window and input events and close any flagged windows
//========================================================================

GLFWAPI void glfwPollEvents(void)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    clearScrollOffsets();

    _glfwPlatformPollEvents();

    closeFlaggedWindows();
}


//========================================================================
// Wait for new window and input events
//========================================================================

GLFWAPI void glfwWaitEvents(void)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    clearScrollOffsets();

    _glfwPlatformWaitEvents();

    closeFlaggedWindows();
}

