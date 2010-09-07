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

// With the Borland C++ compiler, we want to disable FPU exceptions
#ifdef __BORLANDC__
#include <float.h>
#endif // __BORLANDC__



//************************************************************************
//****                  GLFW internal functions                       ****
//************************************************************************

//========================================================================
// Load necessary libraries (DLLs)
//========================================================================

static int _glfwInitLibraries( void )
{
    // gdi32.dll (OpenGL pixel format functions & SwapBuffers)
#ifndef _GLFW_NO_DLOAD_GDI32
    _glfwLibrary.Libs.gdi32 = LoadLibrary( "gdi32.dll" );
    if( _glfwLibrary.Libs.gdi32 != NULL )
    {
        _glfwLibrary.Libs.ChoosePixelFormat   = (CHOOSEPIXELFORMAT_T)
            GetProcAddress( _glfwLibrary.Libs.gdi32, "ChoosePixelFormat" );
        _glfwLibrary.Libs.DescribePixelFormat = (DESCRIBEPIXELFORMAT_T)
            GetProcAddress( _glfwLibrary.Libs.gdi32, "DescribePixelFormat" );
        _glfwLibrary.Libs.GetPixelFormat      = (GETPIXELFORMAT_T)
            GetProcAddress( _glfwLibrary.Libs.gdi32, "GetPixelFormat" );
        _glfwLibrary.Libs.SetPixelFormat      = (SETPIXELFORMAT_T)
            GetProcAddress( _glfwLibrary.Libs.gdi32, "SetPixelFormat" );
        _glfwLibrary.Libs.SwapBuffers         = (SWAPBUFFERS_T)
            GetProcAddress( _glfwLibrary.Libs.gdi32, "SwapBuffers" );
        if( _glfwLibrary.Libs.ChoosePixelFormat   == NULL ||
            _glfwLibrary.Libs.DescribePixelFormat == NULL ||
            _glfwLibrary.Libs.GetPixelFormat      == NULL ||
            _glfwLibrary.Libs.SetPixelFormat      == NULL ||
            _glfwLibrary.Libs.SwapBuffers         == NULL )
        {
            FreeLibrary( _glfwLibrary.Libs.gdi32 );
            _glfwLibrary.Libs.gdi32 = NULL;
            return GL_FALSE;
        }
    }
    else
    {
        return GL_FALSE;
    }
#endif // _GLFW_NO_DLOAD_GDI32

    // winmm.dll (for joystick and timer support)
#ifndef _GLFW_NO_DLOAD_WINMM
    _glfwLibrary.Libs.winmm = LoadLibrary( "winmm.dll" );
    if( _glfwLibrary.Libs.winmm != NULL )
    {
        _glfwLibrary.Libs.joyGetDevCapsA = (JOYGETDEVCAPSA_T)
            GetProcAddress( _glfwLibrary.Libs.winmm, "joyGetDevCapsA" );
        _glfwLibrary.Libs.joyGetPos      = (JOYGETPOS_T)
            GetProcAddress( _glfwLibrary.Libs.winmm, "joyGetPos" );
        _glfwLibrary.Libs.joyGetPosEx    = (JOYGETPOSEX_T)
            GetProcAddress( _glfwLibrary.Libs.winmm, "joyGetPosEx" );
        _glfwLibrary.Libs.timeGetTime    = (TIMEGETTIME_T)
            GetProcAddress( _glfwLibrary.Libs.winmm, "timeGetTime" );
        if( _glfwLibrary.Libs.joyGetDevCapsA == NULL ||
            _glfwLibrary.Libs.joyGetPos      == NULL ||
            _glfwLibrary.Libs.joyGetPosEx    == NULL ||
            _glfwLibrary.Libs.timeGetTime    == NULL )
        {
            FreeLibrary( _glfwLibrary.Libs.winmm );
            _glfwLibrary.Libs.winmm = NULL;
            return GL_FALSE;
        }
    }
    else
    {
        return GL_FALSE;
    }
#endif // _GLFW_NO_DLOAD_WINMM

    return GL_TRUE;
}


//========================================================================
// Unload used libraries (DLLs)
//========================================================================

static void _glfwFreeLibraries( void )
{
    // gdi32.dll
#ifndef _GLFW_NO_DLOAD_GDI32
    if( _glfwLibrary.Libs.gdi32 != NULL )
    {
        FreeLibrary( _glfwLibrary.Libs.gdi32 );
        _glfwLibrary.Libs.gdi32 = NULL;
    }
#endif // _GLFW_NO_DLOAD_GDI32

    // winmm.dll
#ifndef _GLFW_NO_DLOAD_WINMM
    if( _glfwLibrary.Libs.winmm != NULL )
    {
        FreeLibrary( _glfwLibrary.Libs.winmm );
        _glfwLibrary.Libs.winmm = NULL;
    }
#endif // _GLFW_NO_DLOAD_WINMM
}


//========================================================================
// Terminate GLFW when exiting application
//========================================================================

void _glfwTerminate_atexit( void )
{
    glfwTerminate();
}



//************************************************************************
//****               Platform implementation functions                ****
//************************************************************************

//========================================================================
// Initialize various GLFW state
//========================================================================

int _glfwPlatformInit( void )
{
    OSVERSIONINFO osi;

    // To make SetForegroundWindow() work as we want, we need to fiddle
    // with the FOREGROUNDLOCKTIMEOUT system setting (we do this as early
    // as possible in the hope of still being the foreground process)
    SystemParametersInfo( SPI_GETFOREGROUNDLOCKTIMEOUT, 0,
                          &_glfwLibrary.Sys.foregroundLockTimeout, 0 );
    SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0,
                          SPIF_SENDCHANGE );

    // Check which OS version we are running
    osi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionEx( &osi );
    _glfwLibrary.Sys.winVer = _GLFW_WIN_UNKNOWN;
    if( osi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
    {
        if( osi.dwMajorVersion == 4 && osi.dwMinorVersion < 10 )
        {
            _glfwLibrary.Sys.winVer = _GLFW_WIN_95;
        }
        else if( osi.dwMajorVersion == 4 && osi.dwMinorVersion < 90 )
        {
            _glfwLibrary.Sys.winVer = _GLFW_WIN_98;
        }
        else if( osi.dwMajorVersion == 4 && osi.dwMinorVersion == 90 )
        {
            _glfwLibrary.Sys.winVer = _GLFW_WIN_ME;
        }
        else if( osi.dwMajorVersion >= 4 )
        {
            _glfwLibrary.Sys.winVer = _GLFW_WIN_UNKNOWN_9x;
        }
    }
    else if( osi.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
        if( osi.dwMajorVersion == 4 && osi.dwMinorVersion == 0 )
        {
            _glfwLibrary.Sys.winVer = _GLFW_WIN_NT4;
        }
        else if( osi.dwMajorVersion == 5 && osi.dwMinorVersion == 0 )
        {
            _glfwLibrary.Sys.winVer = _GLFW_WIN_2K;
        }
        else if( osi.dwMajorVersion == 5 && osi.dwMinorVersion == 1 )
        {
            _glfwLibrary.Sys.winVer = _GLFW_WIN_XP;
        }
        else if( osi.dwMajorVersion == 5 && osi.dwMinorVersion == 2 )
        {
            _glfwLibrary.Sys.winVer = _GLFW_WIN_NET_SERVER;
        }
        else if( osi.dwMajorVersion >= 5 )
        {
            _glfwLibrary.Sys.winVer = _GLFW_WIN_UNKNOWN_NT;
        }
    }

    // Do we have Unicode support?
    if( _glfwLibrary.Sys.winVer >= _GLFW_WIN_NT4 )
    {
        // Windows NT/2000/XP/.NET has Unicode support
        _glfwLibrary.Sys.hasUnicode = GL_TRUE;
    }
    else
    {
        // Windows 9x/ME does not have Unicode support
        _glfwLibrary.Sys.hasUnicode = GL_FALSE;
    }

    // Load libraries (DLLs)
    if( !_glfwInitLibraries() )
    {
        return GL_FALSE;
    }

    // With the Borland C++ compiler, we want to disable FPU exceptions
    // (this is recommended for OpenGL applications under Windows)
#ifdef __BORLANDC__
    _control87( MCW_EM, MCW_EM );
#endif

    // Retrieve GLFW instance handle
    _glfwLibrary.instance = GetModuleHandle( NULL );

    // System keys are not disabled
    _glfwWin.keyboardHook = NULL;

    // Install atexit() routine
    atexit( _glfwTerminate_atexit );

    // Start the timer
    _glfwInitTimer();

    return GL_TRUE;
}


//========================================================================
// Close window and shut down library
//========================================================================

int _glfwPlatformTerminate( void )
{
    // Close OpenGL window
    glfwCloseWindow();

    // Enable system keys again (if they were disabled)
    glfwEnable( GLFW_SYSTEM_KEYS );

    // Unload libraries (DLLs)
    _glfwFreeLibraries();

    // Restore FOREGROUNDLOCKTIMEOUT system setting
    SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT, 0,
                          (LPVOID)_glfwLibrary.Sys.foregroundLockTimeout,
                          SPIF_SENDCHANGE );

    return GL_TRUE;
}

