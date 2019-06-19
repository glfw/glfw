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

#include <dlfcn.h>
#include <stdio.h>

#define _GLFW_PLATFORM_CONTEXT_STATE
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE
#define _GLFW_OSMESA_CONTEXT_STATE
#define _GLFW_OSMESA_LIBRARY_CONTEXT_STATE

#include "posix_time.h"
#include "posix_thread.h"
#include "xkb_unicode.h"
#include "egl_context.h"
#include "linux_joystick.h"
#include "evdev.h"

#define _glfw_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define _glfw_dlclose(handle) dlclose(handle)
#define _glfw_dlsym(handle, name) dlsym(handle, name)

#define _GLFW_EGL_NATIVE_WINDOW  (window->vivante.native_window)
#define _GLFW_EGL_NATIVE_DISPLAY (_glfw.vivante.display)

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowVivante  vivante
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryVivante vivante
#define _GLFW_PLATFORM_MONITOR_STATE        //_GLFWmonitorVivante vivante
#define _GLFW_PLATFORM_CURSOR_STATE         //_GLFWcursorVivante  vivante

// EGL function pointer typedefs
typedef void * EGLNativeDisplayType;
typedef void * EGLNativeWindowType;

typedef EGLNativeDisplayType (EGLAPIENTRY * PFN_fbGetDisplay)(void *context);
typedef EGLNativeDisplayType (EGLAPIENTRY * PFN_fbGetDisplayByIndex)(int DisplayIndex);
typedef void (EGLAPIENTRY * PFN_fbGetDisplayGeometry)(EGLNativeDisplayType Display, int *Width, int *Height);
typedef void (EGLAPIENTRY * PFN_fbGetDisplayInfo)(EGLNativeDisplayType Display, int *Width, int *Height, unsigned long *Physical, int *Stride, int *BitsPerPixel);
typedef void (EGLAPIENTRY * PFN_fbDestroyDisplay)(EGLNativeDisplayType Display);
typedef EGLNativeWindowType (EGLAPIENTRY * PFN_fbCreateWindow)(EGLNativeDisplayType Display, int X, int Y, int Width, int Height);
typedef void (EGLAPIENTRY * PFN_fbGetWindowGeometry)(EGLNativeWindowType Window, int *X, int *Y, int *Width, int *Height);
typedef void (EGLAPIENTRY * PFN_fbGetWindowInfo)(EGLNativeWindowType Window, int *X, int *Y, int *Width, int *Height, int *BitsPerPixel, unsigned int *Offset);
typedef void (EGLAPIENTRY * PFN_fbDestroyWindow)(EGLNativeWindowType Window);

#define fbGetDisplay _glfw.vivante.GetDisplay
#define fbGetDisplayByIndex _glfw.vivante.GetDisplayByIndex
#define fbGetDisplayGeometry _glfw.vivante.GetDisplayGeometry
#define fbGetDisplayInfo _glfw.vivante.GetDisplayInfo
#define fbDestroyDisplay _glfw.vivante.DestroyDisplay
#define fbCreateWindow _glfw.vivante.CreateWindow
#define fbGetWindowGeometry _glfw.vivante.GetWindowGeometry
#define fbGetWindowInfo _glfw.vivante.GetWindowInfo
#define fbDestroyWindow _glfw.vivante.DestroyWindow


// Vivante-specific per-window data
//
typedef struct _GLFWwindowVivante
{
    EGLNativeWindowType native_window;
    
    // Cached position and size used to filter out duplicate events
    int             width, height;
    int             xpos, ypos;

} _GLFWwindowVivante;

// Vivante-specific global data
//
typedef struct _GLFWlibraryVivante
{
    EGLNativeDisplayType display;

    void*                handle;

    PFN_fbGetDisplay            GetDisplay;
    PFN_fbGetDisplayByIndex     GetDisplayByIndex;
    PFN_fbGetDisplayGeometry    GetDisplayGeometry;
    PFN_fbGetDisplayInfo        GetDisplayInfo;
    PFN_fbDestroyDisplay        DestroyDisplay;
    PFN_fbCreateWindow          CreateWindow;
    PFN_fbGetWindowGeometry     GetWindowGeometry;
    PFN_fbGetWindowInfo         GetWindowInfo;
    PFN_fbDestroyWindow         DestroyWindow;
    
} _GLFWlibraryVivante;
