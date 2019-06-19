//========================================================================
// GLFW 3.3 - www.glfw.org
//------------------------------------------------------------------------
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

#include <linux/input.h>
#include <linux/limits.h>
#include <regex.h>

#define _GLFW_PLATFORM_LIBRARY_EVDEV_STATE _GLFWlibraryEvdev  evdev

#define GLFW_EVENT_DEVICES_MAX 16

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)

// Event device data
//
typedef struct GLFWeventDevice
{
    GLFWbool        present;
    char*           name;
    int             fd;
    GLFWbool        dropped;
    int             scancode;
    int             mods;
    unsigned long   keyState[NBITS(KEY_CNT)];
    unsigned long   ledState[NBITS(LED_MAX)];
    char            path[PATH_MAX];
} _GLFWeventDevice;

// Evdev API data
//
typedef struct _GLFWlibraryEvdev
{
    int                 inotify;
    int                 watch;
    regex_t             regex;
    int                 mods;
    _GLFWeventDevice    devices[GLFW_EVENT_DEVICES_MAX];
} _GLFWlibraryEvdev;

#undef BITS_PER_LONG
#undef NBITS

GLFWbool _glfwInitEvdev();
void _glfwTerminateEvdev();
void _glfwDetectEvdevConnection();
int _glfwPollEvdevDevice(_GLFWeventDevice* ed);
void _glfwPollEvdevDevices();

// Implement it to receive evdev input callbacks
//
void _glfwEvdevInputKey(int key, int scancode, int action, int mods);
void _glfwEvdevInputChar(unsigned int codepoint, int mods, GLFWbool plain);
void _glfwEvdevInputScroll(double xoffset, double yoffset);
void _glfwEvdevInputMouseClick(int button, int action, int mods);
void _glfwEvdevInputCursorPos(double xoffset, double yoffset);
