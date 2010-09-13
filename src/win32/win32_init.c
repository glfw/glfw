//========================================================================
// GLFW - An OpenGL framework
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

    _glfwLibrary.Win32.libs.gdi32 = LoadLibrary("gdi32.dll");
    if (_glfwLibrary.Win32.libs.gdi32 != NULL)
    {
        _glfwLibrary.Win32.libs.ChoosePixelFormat = (CHOOSEPIXELFORMAT_T)
            GetProcAddress(_glfwLibrary.Win32.libs.gdi32, "ChoosePixelFormat");
        _glfwLibrary.Win32.libs.DescribePixelFormat = (DESCRIBEPIXELFORMAT_T)
            GetProcAddress(_glfwLibrary.Win32.libs.gdi32, "DescribePixelFormat");
        _glfwLibrary.Win32.libs.GetPixelFormat = (GETPIXELFORMAT_T)
            GetProcAddress(_glfwLibrary.Win32.libs.gdi32, "GetPixelFormat");
        _glfwLibrary.Win32.libs.SetPixelFormat = (SETPIXELFORMAT_T)
            GetProcAddress(_glfwLibrary.Win32.libs.gdi32, "SetPixelFormat");
        _glfwLibrary.Win32.libs.SwapBuffers = (SWAPBUFFERS_T)
            GetProcAddress(_glfwLibrary.Win32.libs.gdi32, "SwapBuffers");

        if (!_glfwLibrary.Win32.libs.ChoosePixelFormat ||
            !_glfwLibrary.Win32.libs.DescribePixelFormat ||
            !_glfwLibrary.Win32.libs.GetPixelFormat ||
            !_glfwLibrary.Win32.libs.SetPixelFormat ||
            !_glfwLibrary.Win32.libs.SwapBuffers)
        {
            FreeLibrary(_glfwLibrary.Win32.libs.gdi32);
            _glfwLibrary.Win32.libs.gdi32 = NULL;
            return GL_FALSE;
        }
    }
    else
        return GL_FALSE;
#endif // _GLFW_NO_DLOAD_GDI32

#ifndef _GLFW_NO_DLOAD_WINMM
    // winmm.dll (for joystick and timer support)

    _glfwLibrary.Win32.libs.winmm = LoadLibrary("winmm.dll");
    if (_glfwLibrary.Win32.libs.winmm != NULL)
    {
        _glfwLibrary.Win32.libs.joyGetDevCapsA = (JOYGETDEVCAPSA_T)
            GetProcAddress(_glfwLibrary.Win32.libs.winmm, "joyGetDevCapsA");
        _glfwLibrary.Win32.libs.joyGetPos = (JOYGETPOS_T)
            GetProcAddress(_glfwLibrary.Win32.libs.winmm, "joyGetPos");
        _glfwLibrary.Win32.libs.joyGetPosEx = (JOYGETPOSEX_T)
            GetProcAddress(_glfwLibrary.Win32.libs.winmm, "joyGetPosEx");
        _glfwLibrary.Win32.libs.timeGetTime = (TIMEGETTIME_T)
            GetProcAddress(_glfwLibrary.Win32.libs.winmm, "timeGetTime");

        if (!_glfwLibrary.Win32.libs.joyGetDevCapsA ||
            !_glfwLibrary.Win32.libs.joyGetPos ||
            !_glfwLibrary.Win32.libs.joyGetPosEx ||
            !_glfwLibrary.Win32.libs.timeGetTime)
        {
            FreeLibrary(_glfwLibrary.Win32.libs.winmm);
            _glfwLibrary.Win32.libs.winmm = NULL;
            return GL_FALSE;
        }
    }
    else
        return GL_FALSE;
#endif // _GLFW_NO_DLOAD_WINMM

    return GL_TRUE;
}


//========================================================================
// Unload used libraries (DLLs)
//========================================================================

static void freeLibraries(void)
{
    // gdi32.dll
#ifndef _GLFW_NO_DLOAD_GDI32
    if (_glfwLibrary.Win32.libs.gdi32 != NULL)
    {
        FreeLibrary(_glfwLibrary.Win32.libs.gdi32);
        _glfwLibrary.Win32.libs.gdi32 = NULL;
    }
#endif // _GLFW_NO_DLOAD_GDI32

    // winmm.dll
#ifndef _GLFW_NO_DLOAD_WINMM
    if (_glfwLibrary.Win32.libs.winmm != NULL)
    {
        FreeLibrary(_glfwLibrary.Win32.libs.winmm);
        _glfwLibrary.Win32.libs.winmm = NULL;
    }
#endif // _GLFW_NO_DLOAD_WINMM
}


//========================================================================
// Terminate GLFW when exiting application
//========================================================================

static void glfw_atexit(void)
{
    glfwTerminate();
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

    atexit(glfw_atexit);

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
    const char* version = "GLFW "
#if defined(__MINGW32__)
        " MinGW"
#elif defined(__CYGWIN__)
        " Cygwin"
#elif defined(_MSC_VER)
        " Visual C++"
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

