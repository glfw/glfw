//========================================================================
// GLFW - An OpenGL framework
// Platform:    X11/GLX
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

#include <stdio.h>


//************************************************************************
//****                  GLFW internal functions                       ****
//************************************************************************

//========================================================================
// Dynamically load libraries
//========================================================================

static void initLibraries(void)
{
#ifdef _GLFW_DLOPEN_LIBGL
    int i;
    char* libGL_names[ ] =
    {
        "libGL.so",
        "libGL.so.1",
        "/usr/lib/libGL.so",
        "/usr/lib/libGL.so.1",
        NULL
    };

    _glfwLibrary.X11.libGL = NULL;
    for (i = 0;  libGL_names[i] != NULL;  i++)
    {
        _glfwLibrary.X11.libGL = dlopen(libGL_names[i], RTLD_LAZY | RTLD_GLOBAL);
        if (_glfwLibrary.X11.libGL)
            break;
    }
#endif
}


//========================================================================
// Terminate GLFW when exiting application
//========================================================================

static void glfw_atexit(void)
{
    glfwTerminate();
}


//========================================================================
// Initialize X11 display
//========================================================================

static GLboolean initDisplay(void)
{
    // Open display
    _glfwLibrary.X11.display = XOpenDisplay(0);
    if (!_glfwLibrary.X11.display)
    {
        fprintf(stderr, "Failed to open X display\n");
        _glfwSetError(GLFW_OPENGL_NOT_SUPPORTED);
        return GL_FALSE;
    }

    // As the API currently doesn't understand multiple display devices, we hardcode
    // this choice and hope for the best
    _glfwLibrary.X11.screen = DefaultScreen(_glfwLibrary.X11.display);
    _glfwLibrary.X11.root = RootWindow(_glfwLibrary.X11.display,
                                       _glfwLibrary.X11.screen);

    // Check for XF86VidMode extension
#ifdef _GLFW_HAS_XF86VIDMODE
    _glfwLibrary.X11.XF86VidMode.available =
        XF86VidModeQueryExtension(_glfwLibrary.X11.display,
                                  &_glfwLibrary.X11.XF86VidMode.eventBase,
                                  &_glfwLibrary.X11.XF86VidMode.errorBase);
#else
    _glfwLibrary.X11.XF86VidMode.available = 0;
#endif

    // Check for XRandR extension
#ifdef _GLFW_HAS_XRANDR
    _glfwLibrary.X11.XRandR.available =
        XRRQueryExtension(_glfwLibrary.X11.display,
                          &_glfwLibrary.X11.XRandR.eventBase,
                          &_glfwLibrary.X11.XRandR.errorBase);
#else
    _glfwLibrary.X11.XRandR.available = 0;
#endif

    // Fullscreen & screen saver settings
    // Check if GLX is supported on this display
    if (!glXQueryExtension(_glfwLibrary.X11.display, NULL, NULL))
    {
        fprintf(stderr, "GLX not supported\n");
        _glfwSetError(GLFW_OPENGL_NOT_SUPPORTED);
        return GL_FALSE;
    }

    // Retrieve GLX version
    if (!glXQueryVersion(_glfwLibrary.X11.display,
                         &_glfwLibrary.X11.glxMajor,
                         &_glfwLibrary.X11.glxMinor))
    {
        fprintf(stderr, "Unable to query GLX version\n");
        _glfwSetError(GLFW_OPENGL_NOT_SUPPORTED);
        return GL_FALSE;
    }

    return GL_TRUE;
}


//========================================================================
// Terminate X11 display
//========================================================================

static void terminateDisplay(void)
{
    if (_glfwLibrary.X11.display)
    {
        XCloseDisplay(_glfwLibrary.X11.display);
        _glfwLibrary.X11.display = NULL;
    }
}


//************************************************************************
//****               Platform implementation functions                ****
//************************************************************************

//========================================================================
// Initialize various GLFW state
//========================================================================

int _glfwPlatformInit(void)
{
    if (!initDisplay())
        return GL_FALSE;

    // Try to load libGL.so if necessary
    initLibraries();

    // Install atexit() routine
    atexit(glfw_atexit);

    // Initialize joysticks
    _glfwInitJoysticks();

    // Start the timer
    _glfwInitTimer();

    return GL_TRUE;
}


//========================================================================
// Close window and shut down library
//========================================================================

int _glfwPlatformTerminate(void)
{
    while (_glfwLibrary.windowListHead)
        glfwCloseWindow(_glfwLibrary.windowListHead);

    // Terminate display
    terminateDisplay();

    // Terminate joysticks
    _glfwTerminateJoysticks();

    // Unload libGL.so if necessary
#ifdef _GLFW_DLOPEN_LIBGL
    if (_glfwLibrary.X11.libGL != NULL)
    {
        dlclose(_glfwLibrary.X11.libGL);
        _glfwLibrary.X11.libGL = NULL;
    }
#endif

    return GL_TRUE;
}

