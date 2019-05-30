//========================================================================
// GLFW 3.3 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2017 Camilla Löwy <elmindreda@glfw.org>
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

void _glfwInitFbMonitor(void)
{
    int width, height;
    fbGetDisplayGeometry(_glfw.vivante.display, &width, &height);
    
    _glfwInputMonitor(_glfwAllocMonitor("Display", width, height),
                      GLFW_CONNECTED,
                      _GLFW_INSERT_FIRST);
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformInit(void)
{
    _glfw.vivante.handle = _glfw_dlopen("libEGL.so.1");
    if (!_glfw.vivante.handle) {
        _glfw.vivante.handle = _glfw_dlopen("libEGL.so");
        if (!_glfw.vivante.handle) {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Vivante: EGL Library not found");

            return GLFW_FALSE;
        }
    }
    
    _glfw.vivante.GetDisplay = (PFN_fbGetDisplay)
        _glfw_dlsym(_glfw.vivante.handle, "fbGetDisplay");
    _glfw.vivante.GetDisplayByIndex = (PFN_fbGetDisplayByIndex)
        _glfw_dlsym(_glfw.vivante.handle, "fbGetDisplayByIndex");
    _glfw.vivante.GetDisplayGeometry = (PFN_fbGetDisplayGeometry)
        _glfw_dlsym(_glfw.vivante.handle, "fbGetDisplayGeometry");
    _glfw.vivante.GetDisplayInfo = (PFN_fbGetDisplayInfo)
        _glfw_dlsym(_glfw.vivante.handle, "fbGetDisplayInfo");
    _glfw.vivante.DestroyDisplay = (PFN_fbDestroyDisplay)
        _glfw_dlsym(_glfw.vivante.handle, "fbDestroyDisplay");
    _glfw.vivante.CreateWindow = (PFN_fbCreateWindow)
        _glfw_dlsym(_glfw.vivante.handle, "fbCreateWindow");
    _glfw.vivante.GetWindowGeometry = (PFN_fbGetWindowGeometry)
        _glfw_dlsym(_glfw.vivante.handle, "fbGetWindowGeometry");
    _glfw.vivante.GetWindowInfo = (PFN_fbGetWindowInfo)
        _glfw_dlsym(_glfw.vivante.handle, "fbGetWindowInfo");
    _glfw.vivante.DestroyWindow = (PFN_fbDestroyWindow)
        _glfw_dlsym(_glfw.vivante.handle, "fbDestroyWindow");
    
    if (!_glfw.vivante.GetDisplay ||
        !_glfw.vivante.GetDisplayByIndex ||
        !_glfw.vivante.GetDisplayGeometry ||
        !_glfw.vivante.GetDisplayInfo ||
        !_glfw.vivante.DestroyDisplay ||
        !_glfw.vivante.CreateWindow ||
        !_glfw.vivante.GetWindowGeometry ||
        !_glfw.vivante.GetWindowInfo ||
        !_glfw.vivante.DestroyWindow)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Vivante: Failed to load required entry points");

        _glfwTerminateEGL();
        return GLFW_FALSE;
    }
    
    
    _glfw.vivante.display = fbGetDisplay(NULL);
    if (!_glfw.vivante.display)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Vivante: Failed to open display");

        return GLFW_FALSE;
    }
    
    if (!_glfwInitJoysticksLinux())
        return GLFW_FALSE;
    
    _glfwInitTimerPOSIX();
    
    _glfwInitFbMonitor();
    
    return GLFW_TRUE;
}

void _glfwPlatformTerminate(void)
{
    _glfwTerminateJoysticksLinux();
    
    if (_glfw.vivante.display)
    {
        fbDestroyDisplay(_glfw.vivante.display);
        _glfw.vivante.display = NULL;
    }
    
    if (_glfw.vivante.handle)
    {
        _glfw_dlclose(_glfw.vivante.handle);
        _glfw.vivante.handle = NULL;
    }
}

const char* _glfwPlatformGetVersionString(void)
{
    return _GLFW_VERSION_NUMBER " vivante EGL"
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
        " clock_gettime"
#else
        " gettimeofday"
#endif
#if defined(__linux__)
        " evdev"
#endif
#if defined(_GLFW_BUILD_DLL)
        " shared"
#endif
        ;
}

