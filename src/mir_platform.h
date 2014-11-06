//========================================================================
// GLFW 3.1 Mir - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2014 Brandon Schaefer <brandon.schaefer@canonical.com>
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

#ifndef _mir_platform_h_
#define _mir_platform_h_

#include <mir_toolkit/mir_client_library.h>

#include "posix_tls.h"
#include "posix_time.h"
#include "linux_joystick.h"

#if defined(_GLFW_EGL)
 #include "egl_context.h"
#else
 #error "The Mir backend depends on EGL platform support"
#endif

#define _GLFW_EGL_NATIVE_WINDOW  window->mir.native_window
#define _GLFW_EGL_NATIVE_DISPLAY _glfw.mir.native_display

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowMir  mir;
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorMir mir;
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryMir mir;
#define _GLFW_PLATFORM_CURSOR_STATE         _GLFWcursorMir  mir;

typedef struct _GLFWwindowMir
{
  MirSurface* surface;
  int         width;
  int         height;

  EGLSurface egl_surface;
  MirEGLNativeWindowType  native_window;

} _GLFWwindowMir;

typedef struct _GLFWmonitorMir
{
    int num_modes;
    int output_id;
    int x;
    int y;

} _GLFWmonitorMir;

typedef struct _GLFWlibraryMir
{
  MirConnection* connection;
  MirEGLNativeDisplayType native_display;

} _GLFWlibraryMir;

// TODO Only system cursors are implemented in mir atm. Need to wait for support.
typedef struct _GLFWcursorMir
{
} _GLFWcursorMir;

#endif // _mir_platform_h_
