/*************************************************************************
 * GLFW 3.4 - www.glfw.org
 * A library for OpenGL, window and input
 *------------------------------------------------------------------------
 * Copyright (c) 2002-2006 Marcus Geelnard
 * Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 *************************************************************************/

#ifndef _glfw3impl_h_
#define _glfw3impl_h_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)

    #define _GLFW_WIN32
    // -D_CRT_SECURE_NO_WARNINGS
    // -lgdi32
    // -lshell32
    // -luser32

#elif defined(__APPLE__)

    #define _GLFW_COCOA
    // -framework Cocoa
    // -framework IOKit
    // -framework QuartzCore
    // -x objective-c
    //    or
    // -x objective-c++

#endif

#include "../../src/cocoa_init.m"
#include "../../src/cocoa_joystick.m"
#include "../../src/cocoa_monitor.m"
#include "../../src/cocoa_time.c"
#include "../../src/cocoa_window.m"
#include "../../src/context.c"
#include "../../src/egl_context.c"
#include "../../src/glx_context.c"
#include "../../src/init.c"
#include "../../src/input.c"
#include "../../src/linux_joystick.c"
#include "../../src/monitor.c"
#include "../../src/nsgl_context.m"
#include "../../src/null_init.c"
#include "../../src/null_joystick.c"
#include "../../src/null_monitor.c"
#include "../../src/null_window.c"
#include "../../src/osmesa_context.c"
#include "../../src/platform.c"
#include "../../src/posix_module.c"
#include "../../src/posix_poll.c"
#include "../../src/posix_thread.c"
#include "../../src/posix_time.c"
#include "../../src/vulkan.c"
#include "../../src/wgl_context.c"
#include "../../src/win32_init.c"
#include "../../src/win32_joystick.c"
#include "../../src/win32_module.c"
#include "../../src/win32_monitor.c"
#include "../../src/win32_thread.c"
#include "../../src/win32_time.c"
#include "../../src/win32_window.c"
#include "../../src/window.c"
#include "../../src/wl_init.c"
#include "../../src/wl_monitor.c"
#include "../../src/wl_window.c"
#include "../../src/x11_init.c"
#include "../../src/x11_monitor.c"
#include "../../src/x11_window.c"
#include "../../src/xkb_unicode.c"

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _glfw3impl_h_ */
