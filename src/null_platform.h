//========================================================================
// GLFW 3.4 - www.glfw.org
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

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowNull null
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryNull null
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorNull null

#define _GLFW_PLATFORM_CONTEXT_STATE         struct { int dummyContext; }
#define _GLFW_PLATFORM_CURSOR_STATE          struct { int dummyCursor; }
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE struct { int dummyLibraryContext; }

#include "posix_time.h"
#include "posix_thread.h"
#include "null_joystick.h"

// Null-specific per-window data
//
typedef struct _GLFWwindowNull
{
    int             xpos;
    int             ypos;
    int             width;
    int             height;
    char*           title;
    GLFWbool        visible;
    GLFWbool        iconified;
    GLFWbool        maximized;
    GLFWbool        resizable;
    GLFWbool        decorated;
    GLFWbool        floating;
    GLFWbool        transparent;
    float           opacity;
} _GLFWwindowNull;

// Null-specific per-monitor data
//
typedef struct _GLFWmonitorNull
{
    GLFWgammaramp   ramp;
} _GLFWmonitorNull;

// Null-specific global data
//
typedef struct _GLFWlibraryNull
{
    int             xcursor;
    int             ycursor;
    char*           clipboardString;
    _GLFWwindow*    focusedWindow;
} _GLFWlibraryNull;

void _glfwPollMonitorsNull(void);

