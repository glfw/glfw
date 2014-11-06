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
} _GLFWmonitorMir;

typedef struct _GLFWlibraryMir
{
  MirConnection* connection;

  MirEGLNativeDisplayType native_display;


} _GLFWlibraryMir;

typedef struct _GLFWcursorMir
{
} _GLFWcursorMir;

#endif // _mir_platform_h_
