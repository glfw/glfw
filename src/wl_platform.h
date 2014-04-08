//========================================================================
// GLFW 3.1 Wayland - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2014 Jonas Ådahl <jadahl@gmail.com>
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

#ifndef _wayland_platform_h_
#define _wayland_platform_h_


#include <wayland-client.h>

#if defined(_GLFW_EGL)
 #include "egl_context.h"
#else
 #error "The Wayland backend depends on EGL platform support"
#endif

#include "posix_tls.h"
#include "posix_time.h"
#include "linux_joystick.h"

#define _GLFW_EGL_NATIVE_WINDOW         window->wl.native
#define _GLFW_EGL_NATIVE_DISPLAY        _glfw.wl.display

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowWayland  wl
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryWayland wl
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorWayland wl
#define _GLFW_PLATFORM_CURSOR_STATE         _GLFWcursorWayland  wl


typedef struct _GLFWvidmodeWayland _GLFWvidmodeWayland;

typedef struct _GLFWwindowWayland
{
    int                         width, height;
    GLboolean                   visible;
    struct wl_surface*          surface;
    struct wl_egl_window*       native;
    struct wl_shell_surface*    shell_surface;
    EGLSurface                  egl_surface;
    struct wl_callback*         callback;
} _GLFWwindowWayland;

typedef struct _GLFWlibraryWayland
{
    struct wl_display*          display;
    struct wl_registry*         registry;
    struct wl_compositor*       compositor;
    struct wl_shell*            shell;

    _GLFWmonitor**              monitors;
    int                         monitorsCount;
    int                         monitorsSize;

} _GLFWlibraryWayland;

typedef struct _GLFWmonitorWayland
{
    struct wl_output*           output;

    _GLFWvidmodeWayland*        modes;
    int                         modesCount;
    int                         modesSize;
    GLboolean                   done;

    int                         x;
    int                         y;
} _GLFWmonitorWayland;

typedef struct _GLFWcursorWayland
{
    int                         dummy;
} _GLFWcursorWayland;


//========================================================================
// Prototypes for platform specific internal functions
//========================================================================

void _glfwAddOutput(uint32_t name, uint32_t version);

#endif // _wayland_platform_h_
