//========================================================================
// GLFW 3.4 Win32 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2017 Camilla Löwy <elmindreda@glfw.org>
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

// We don't need all the fancy stuff
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// This is a workaround for the fact that glfw3.h needs to export APIENTRY (for
// example to allow applications to correctly declare a GL_KHR_debug callback)
// but windows.h assumes no one will define APIENTRY before it does
#undef APIENTRY

// GLFW on Windows is Unicode only and does not work in MBCS mode
#ifndef UNICODE
#define UNICODE
#endif

// GLFW requires Windows XP or later
#if WINVER < 0x0501
#undef WINVER
#define WINVER 0x0501
#endif
#if _WIN32_WINNT < 0x0501
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

// GLFW uses OEM cursor resources
#define OEMRESOURCE

#include <wctype.h>
#include <windows.h>

#define GLFW_WIN32_TLS_STATE            _GLFWtlsWin32     win32;
#define GLFW_WIN32_MUTEX_STATE          _GLFWmutexWin32   win32;
#define GLFW_WIN32_CONDVAR_STATE        _GLFWcondvarWin32 win32;

// Win32-specific thread local storage data
//
typedef struct _GLFWtlsWin32
{
    GLFWbool            allocated;
    DWORD               index;
} _GLFWtlsWin32;

// Win32-specific mutex data
//
typedef struct _GLFWmutexWin32
{
    GLFWbool            allocated;
    CRITICAL_SECTION    section;
} _GLFWmutexWin32;

// Win32-specific conditional variable data
//
typedef struct _GLFWcondvarWin32
{
    GLFWbool            allocated;
    CONDITION_VARIABLE  condvar;
} _GLFWcondvarWin32;
