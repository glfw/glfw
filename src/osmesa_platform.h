
#ifndef _osmesa_platform_h_
#define _osmesa_platform_h_

#include <dlfcn.h>

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowOSMesa     osmesa
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorOSMesa    osmesa
#define _GLFW_PLATFORM_CURSOR_STATE         _GLFWcursorOSMesa     osmesa

#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWwinlibraryOSMesa osmesawin

#define _GLFW_PLATFORM_LIBRARY_JOYSTICK_STATE
#define _GLFW_EGL_CONTEXT_STATE
#define _GLFW_EGL_LIBRARY_CONTEXT_STATE

#include "osmesa_context.h"
#include "posix_time.h"
#include "posix_tls.h"

#define _glfw_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define _glfw_dlclose(handle) dlclose(handle)
#define _glfw_dlsym(handle, name) dlsym(handle, name)

// TODO(dalyj) "PLACEHOLDER" variables are there to silence "empty struct"
// warnings

// OSMesa-specific per-window data
//
typedef struct _GLFWwindowOSMesa
{
  int width;
  int height;
} _GLFWwindowOSMesa;


// OSMesa-specific global data
//
typedef struct _GLFWwinlibraryOSMesa
{
  int PLACEHOLDER;
} _GLFWwinlibraryOSMesa;


// OSMesa-specific per-monitor data
//
typedef struct _GLFWmonitorOSMesa
{
  int PLACEHOLDER;
} _GLFWmonitorOSMesa;


// OSMesa-specific per-cursor data
//
typedef struct _GLFWcursorOSMesa
{
  int PLACEHOLDER;
} _GLFWcursorOSMesa;


#endif // _osmesa_platform_h_
