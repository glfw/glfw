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
#ifndef _GLFW_NO_DLOAD_WINMM
    // winmm.dll (for joystick and timer support)

    _glfwLibrary.Win32.winmm.instance = LoadLibrary(L"winmm.dll");
    if (!_glfwLibrary.Win32.winmm.instance)
        return GL_FALSE;

    _glfwLibrary.Win32.winmm.joyGetDevCaps = (JOYGETDEVCAPS_T)
        GetProcAddress(_glfwLibrary.Win32.winmm.instance, "joyGetDevCapsW");
    _glfwLibrary.Win32.winmm.joyGetPos = (JOYGETPOS_T)
        GetProcAddress(_glfwLibrary.Win32.winmm.instance, "joyGetPos");
    _glfwLibrary.Win32.winmm.joyGetPosEx = (JOYGETPOSEX_T)
        GetProcAddress(_glfwLibrary.Win32.winmm.instance, "joyGetPosEx");
    _glfwLibrary.Win32.winmm.timeGetTime = (TIMEGETTIME_T)
        GetProcAddress(_glfwLibrary.Win32.winmm.instance, "timeGetTime");

    if (!_glfwLibrary.Win32.winmm.joyGetDevCaps ||
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
#ifndef _GLFW_NO_DLOAD_WINMM
    if (_glfwLibrary.Win32.winmm.instance != NULL)
    {
        FreeLibrary(_glfwLibrary.Win32.winmm.instance);
        _glfwLibrary.Win32.winmm.instance = NULL;
    }
#endif // _GLFW_NO_DLOAD_WINMM
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Returns a wide string version of the specified UTF-8 string
//========================================================================

WCHAR* _glfwCreateWideStringFromUTF8(const char* source)
{
    WCHAR* target;
    int length;

    length = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
    if (!length)
        return NULL;

    target = (WCHAR*) malloc(sizeof(WCHAR) * (length + 1));

    if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, length + 1))
    {
        free(target);
        return NULL;
    }

    return target;
}


//========================================================================
// Returns a UTF-8 string version of the specified wide string
//========================================================================

char* _glfwCreateUTF8FromWideString(const WCHAR* source)
{
    char* target;
    int length;

    length = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
    if (!length)
        return NULL;

    target = (char*) malloc(length + 1);

    if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, length + 1, NULL, NULL))
    {
        free(target);
        return NULL;
    }

    return target;
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
    SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, UIntToPtr(0),
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
    if (_glfwLibrary.rampChanged)
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
                         UIntToPtr(_glfwLibrary.Win32.foregroundLockTimeout),
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
#elif defined(_MSC_VER)
        " Visual C++ "
#elif defined(__BORLANDC__)
        " Borland C"
#else
        " (unknown compiler)"
#endif
#if defined(_GLFW_BUILD_DLL)
        " DLL"
#endif
#if !defined(_GLFW_NO_DLOAD_WINMM)
        " load(winmm)"
#endif
        ;

    return version;
}

