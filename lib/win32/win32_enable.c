//========================================================================
// GLFW - An OpenGL framework
// Platform:    Win32/WGL
// API version: 2.7
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


//************************************************************************
//****                  GLFW internal functions                       ****
//************************************************************************

//========================================================================
// Low level keyboard hook (system callback) function
// Used to disable system keys under Windows NT
//========================================================================

static LRESULT CALLBACK keyboardHook( int nCode, WPARAM wParam, LPARAM lParam )
{
    BOOL syskeys = FALSE;
    PKBDLLHOOKSTRUCT p;

    // We are only looking for keyboard events - interpret lParam as a
    // pointer to a KBDLLHOOKSTRUCT
    p = (PKBDLLHOOKSTRUCT) lParam;

    if( nCode == HC_ACTION )
    {
        // We have a keyboard event

        switch( wParam )
        {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
                // Detect: ALT+TAB, ALT+ESC, ALT+F4, CTRL+ESC,
                // LWIN, RWIN, APPS (mysterious menu key)
                syskeys = ( p->vkCode == VK_TAB &&
                            p->flags & LLKHF_ALTDOWN ) ||
                          ( p->vkCode == VK_ESCAPE &&
                            p->flags & LLKHF_ALTDOWN ) ||
                          ( p->vkCode == VK_F4 &&
                            p->flags & LLKHF_ALTDOWN ) ||
                          ( p->vkCode == VK_ESCAPE &&
                            (GetKeyState(VK_CONTROL) & 0x8000)) ||
                          p->vkCode == VK_LWIN ||
                          p->vkCode == VK_RWIN ||
                          p->vkCode == VK_APPS;
                break;

            default:
                break;
        }
    }

    // Was it a system key combination (e.g. ALT+TAB)?
    if( syskeys )
    {
        // Pass the key event to our window message loop
        if( _glfwWin.opened )
        {
            PostMessage( _glfwWin.window, (UINT) wParam, p->vkCode, 0 );
        }

        // We've taken care of it - don't let the system know about this
        // key event
        return 1;
    }
    else
    {
        // It's a harmless key press, let the system deal with it
        return CallNextHookEx( _glfwWin.keyboardHook, nCode, wParam, lParam );
    }
}



//************************************************************************
//****               Platform implementation functions                ****
//************************************************************************

//========================================================================
// Enable system keys
//========================================================================

void _glfwPlatformEnableSystemKeys( void )
{
    BOOL dummy;

    // Use different methods depending on operating system version
    if( _glfwLibrary.Sys.winVer >= _GLFW_WIN_NT4 )
    {
        if( _glfwWin.keyboardHook != NULL )
        {
            UnhookWindowsHookEx( _glfwWin.keyboardHook );
            _glfwWin.keyboardHook = NULL;
        }
    }
    else
    {
        (void) SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, FALSE, &dummy, 0 );
    }
}

//========================================================================
// Disable system keys
//========================================================================

void _glfwPlatformDisableSystemKeys( void )
{
    BOOL dummy;

    // Use different methods depending on operating system version
    if( _glfwLibrary.Sys.winVer >= _GLFW_WIN_NT4 )
    {
        // Under Windows NT, install a low level keyboard hook
        _glfwWin.keyboardHook = SetWindowsHookEx( WH_KEYBOARD_LL,
                                                  keyboardHook,
                                                  _glfwLibrary.instance,
                                                  0 );
    }
    else
    {
        // Under Windows 95/98/ME, fool Windows that a screensaver
        // is running => prevents ALT+TAB, CTRL+ESC and CTRL+ALT+DEL
        (void) SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, TRUE, &dummy, 0 );
    }
}

