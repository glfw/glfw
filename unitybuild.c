
/*
 * Unity build from 
 * https://github.com/glfw/glfw/issues/2561
 * */

/***** GLFW *****/
// Enable extensions, mostly because GLFW uses ppoll
// and doesn't do this before they include poll.h
#define _GNU_SOURCE


#if defined(__TINYC__)
#  define __nmatch
#endif /* __TINYC__ */

/* NOTE: builder must define one of these: 
#define _GLFW_WAYLAND
#define _GLFW_X11
#define _GLFW_WIN32
 * */

#include "src/context.c"
#include "src/egl_context.c"
#include "src/glx_context.c"
#include "src/init.c"
#include "src/input.c"
#include "src/monitor.c"
#include "src/osmesa_context.c"
#include "src/platform.c"
#include "src/vulkan.c"
#include "src/window.c"

#include "src/linux_joystick.c"
#include "src/posix_module.c"
#include "src/posix_poll.c"
#include "src/posix_thread.c"
#include "src/posix_time.c"

#include "src/wl_init.c"
#include "src/wl_monitor.c"
#include "src/wl_window.c"

#include "src/x11_init.c"
#include "src/x11_monitor.c"
#include "src/x11_window.c"
#include "src/xkb_unicode.c"


#include "src/win32_module.c"
#include "src/win32_thread.c"
#include "src/win32_time.c"
#include "src/win32_init.c"
#include "src/win32_monitor.c"
#include "src/win32_window.c"
#include "src/win32_joystick.c"
#include "src/wgl_context.c"

// we need to rename these functions as the "null" platform is
// always linked by platform.c and it defines some static
// functions that conflict with the ones in the other platforms

#define acquireMonitor acquireMonitorNull
#define releaseMonitor releaseMonitorNull
#define createNativeWindow createNativeWindowNull
#define fitToMonitor fitToMonitorNull /* win32 */

#include "src/null_init.c"
#include "src/null_joystick.c"
#include "src/null_monitor.c"
#include "src/null_window.c"

#undef fitToMonitor /* win32 */
#undef acquireMonitor
#undef releaseMonitor
#undef createNativeWindow

