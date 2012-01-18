//========================================================================
// GLFW - An OpenGL library
// Platform:    Win32/WGL
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

#include <stdlib.h>

#ifdef __BORLANDC__
// With the Borland C++ compiler, we want to disable FPU exceptions
#include <float.h>
#endif // __BORLANDC__


//========================================================================
// Load necessary libraries (DLLs)
//========================================================================

static GLboolean initLibraries(void)
{
#ifndef _GLFW_NO_DLOAD_GDI32
    // gdi32.dll (OpenGL pixel format functions & SwapBuffers)

    _glfwLibrary.Win32.gdi.instance = LoadLibrary("gdi32.dll");
    if (!_glfwLibrary.Win32.gdi.instance)
        return GL_FALSE;

    _glfwLibrary.Win32.gdi.ChoosePixelFormat = (CHOOSEPIXELFORMAT_T)
        GetProcAddress(_glfwLibrary.Win32.gdi.instance, "ChoosePixelFormat");
    _glfwLibrary.Win32.gdi.DescribePixelFormat = (DESCRIBEPIXELFORMAT_T)
        GetProcAddress(_glfwLibrary.Win32.gdi.instance, "DescribePixelFormat");
    _glfwLibrary.Win32.gdi.GetPixelFormat = (GETPIXELFORMAT_T)
        GetProcAddress(_glfwLibrary.Win32.gdi.instance, "GetPixelFormat");
    _glfwLibrary.Win32.gdi.SetPixelFormat = (SETPIXELFORMAT_T)
        GetProcAddress(_glfwLibrary.Win32.gdi.instance, "SetPixelFormat");
    _glfwLibrary.Win32.gdi.SwapBuffers = (SWAPBUFFERS_T)
        GetProcAddress(_glfwLibrary.Win32.gdi.instance, "SwapBuffers");
    _glfwLibrary.Win32.gdi.GetDeviceGammaRamp  = (GETDEVICEGAMMARAMP_T)
        GetProcAddress(_glfwLibrary.Win32.gdi.instance, "GetDeviceGammaRamp");
    _glfwLibrary.Win32.gdi.SetDeviceGammaRamp  = (SETDEVICEGAMMARAMP_T)
        GetProcAddress(_glfwLibrary.Win32.gdi.instance, "SetDeviceGammaRamp");

    if (!_glfwLibrary.Win32.gdi.ChoosePixelFormat ||
        !_glfwLibrary.Win32.gdi.DescribePixelFormat ||
        !_glfwLibrary.Win32.gdi.GetPixelFormat ||
        !_glfwLibrary.Win32.gdi.SetPixelFormat ||
        !_glfwLibrary.Win32.gdi.SwapBuffers ||
        !_glfwLibrary.Win32.gdi.GetDeviceGammaRamp ||
        !_glfwLibrary.Win32.gdi.SetDeviceGammaRamp)
    {
        return GL_FALSE;
    }
#endif // _GLFW_NO_DLOAD_GDI32

#ifndef _GLFW_NO_DLOAD_WINMM
    // winmm.dll (for joystick and timer support)

    _glfwLibrary.Win32.winmm.instance = LoadLibrary("winmm.dll");
    if (!_glfwLibrary.Win32.winmm.instance)
        return GL_FALSE;

    _glfwLibrary.Win32.winmm.joyGetDevCapsA = (JOYGETDEVCAPSA_T)
        GetProcAddress(_glfwLibrary.Win32.winmm.instance, "joyGetDevCapsA");
    _glfwLibrary.Win32.winmm.joyGetPos = (JOYGETPOS_T)
        GetProcAddress(_glfwLibrary.Win32.winmm.instance, "joyGetPos");
    _glfwLibrary.Win32.winmm.joyGetPosEx = (JOYGETPOSEX_T)
        GetProcAddress(_glfwLibrary.Win32.winmm.instance, "joyGetPosEx");
    _glfwLibrary.Win32.winmm.timeGetTime = (TIMEGETTIME_T)
        GetProcAddress(_glfwLibrary.Win32.winmm.instance, "timeGetTime");

    if (!_glfwLibrary.Win32.winmm.joyGetDevCapsA ||
        !_glfwLibrary.Win32.winmm.joyGetPos ||
        !_glfwLibrary.Win32.winmm.joyGetPosEx ||
        !_glfwLibrary.Win32.winmm.timeGetTime)
    {
        return GL_FALSE;
    }
#endif // _GLFW_NO_DLOAD_WINMM

    return GL_TRUE;
}


//========================================================================
// Unload used libraries (DLLs)
//========================================================================

static void freeLibraries(void)
{
#ifndef _GLFW_NO_DLOAD_GDI32
    if (_glfwLibrary.Win32.gdi.instance != NULL)
    {
        FreeLibrary(_glfwLibrary.Win32.gdi.instance);
        _glfwLibrary.Win32.gdi.instance = NULL;
    }
#endif // _GLFW_NO_DLOAD_GDI32

#ifndef _GLFW_NO_DLOAD_WINMM
    if (_glfwLibrary.Win32.winmm.instance != NULL)
    {
        FreeLibrary(_glfwLibrary.Win32.winmm.instance);
        _glfwLibrary.Win32.winmm.instance = NULL;
    }
#endif // _GLFW_NO_DLOAD_WINMM
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Initialize various GLFW state
//========================================================================

int _glfwPlatformInit(void)
{
    // To make SetForegroundWindow work as we want, we need to fiddle
    // with the FOREGROUNDLOCKTIMEOUT system setting (we do this as early
    // as possible in the hope of still being the foreground process)
    SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0,
                         &_glfwLibrary.Win32.foregroundLockTimeout, 0);
    SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID) 0,
                         SPIF_SENDCHANGE);

    if (!initLibraries())
        return GL_FALSE;

#ifdef __BORLANDC__
    // With the Borland C++ compiler, we want to disable FPU exceptions
    // (this is recommended for OpenGL applications under Windows)
    _control87(MCW_EM, MCW_EM);
#endif

    _glfwLibrary.Win32.instance = GetModuleHandle(NULL);

    // Save the original gamma ramp
    _glfwLibrary.originalRampSize = 256;
    _glfwPlatformGetGammaRamp(&_glfwLibrary.originalRamp);
    _glfwLibrary.currentRamp = _glfwLibrary.originalRamp;

    _glfwInitTimer();

    return GL_TRUE;
}


//========================================================================
// Close window and shut down library
//========================================================================

int _glfwPlatformTerminate(void)
{
    // Restore the original gamma ramp
    _glfwPlatformSetGammaRamp(&_glfwLibrary.originalRamp);

    if (_glfwLibrary.Win32.classAtom)
    {
        UnregisterClass(_GLFW_WNDCLASSNAME, _glfwLibrary.Win32.instance);
        _glfwLibrary.Win32.classAtom = 0;
    }

    // TODO: Remove keyboard hook

    freeLibraries();

    // Restore previous FOREGROUNDLOCKTIMEOUT system setting
    SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0,
                         (LPVOID) _glfwLibrary.Win32.foregroundLockTimeout,
                         SPIF_SENDCHANGE);

    return GL_TRUE;
}


//========================================================================
// Get the GLFW version string
//========================================================================

const char* _glfwPlatformGetVersionString(void)
{
    const char* version = _GLFW_VERSION_FULL
#if defined(__MINGW32__)
        " MinGW"
#elif defined(__CYGWIN__)
        " Cygwin"
#elif defined(_MSC_VER)
        " Visual C++ "
#elif defined(__BORLANDC__)
        " Borland C"
#else
        " (unknown compiler)"
#endif
#if defined(GLFW_BUILD_DLL)
        " DLL"
#endif
#if !defined(_GLFW_NO_DLOAD_GDI32)
        " load(gdi32)"
#endif
#if !defined(_GLFW_NO_DLOAD_WINMM)
        " load(winmm)"
#endif
        ;

    return version;
}

