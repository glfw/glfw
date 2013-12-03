//========================================================================
// GLFW 3.0 OS X - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2010 Camilla Berglund <elmindreda@elmindreda.org>
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

#ifndef _cocoa_platform_h_
#define _cocoa_platform_h_


#include <stdint.h>

#if defined(__OBJC__)
#import <Cocoa/Cocoa.h>
#else
#include <ApplicationServices/ApplicationServices.h>
typedef void* id;
#endif

#if defined(_GLFW_NSGL)
 #include "nsgl_platform.h"
#else
 #error "No supported context creation API selected"
#endif

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDKeys.h>

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowNS  ns
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryNS ns
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorNS ns


//========================================================================
// GLFW platform specific types
//========================================================================


//------------------------------------------------------------------------
// Platform-specific window structure
//------------------------------------------------------------------------
typedef struct _GLFWwindowNS
{
    id              object;
    id	            delegate;
    id              view;
    unsigned int    modifierFlags;
} _GLFWwindowNS;


//------------------------------------------------------------------------
// Joystick information & state
//------------------------------------------------------------------------
typedef struct
{
    int             present;
    char            name[256];

    IOHIDDeviceInterface** interface;

    CFMutableArrayRef axisElements;
    CFMutableArrayRef buttonElements;
    CFMutableArrayRef hatElements;

    float*          axes;
    unsigned char*  buttons;

} _GLFWjoy;


//------------------------------------------------------------------------
// Platform-specific library global data for Cocoa
//------------------------------------------------------------------------
typedef struct _GLFWlibraryNS
{
    struct {
        double      base;
        double      resolution;
    } timer;

    CGEventSourceRef eventSource;
    id              delegate;
    id              autoreleasePool;
    id              cursor;

    char*           clipboardString;

    _GLFWjoy        joysticks[GLFW_JOYSTICK_LAST + 1];
} _GLFWlibraryNS;


//------------------------------------------------------------------------
// Platform-specific monitor structure
//------------------------------------------------------------------------
typedef struct _GLFWmonitorNS
{
    CGDirectDisplayID   displayID;
    CGDisplayModeRef    previousMode;
    id                  screen;

} _GLFWmonitorNS;


//========================================================================
// Prototypes for platform specific internal functions
//========================================================================

// Time
void _glfwInitTimer(void);

// Joystick input
void _glfwInitJoysticks(void);
void _glfwTerminateJoysticks(void);

// Fullscreen
GLboolean _glfwSetVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* desired);
void _glfwRestoreVideoMode(_GLFWmonitor* monitor);

// OpenGL support
int _glfwInitContextAPI(void);
void _glfwTerminateContextAPI(void);
int _glfwCreateContext(_GLFWwindow* window,
                       const _GLFWwndconfig* wndconfig,
                       const _GLFWfbconfig* fbconfig);
void _glfwDestroyContext(_GLFWwindow* window);

#endif // _cocoa_platform_h_
