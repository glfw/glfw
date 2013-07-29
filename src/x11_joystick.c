//========================================================================
// GLFW 3.0 X11 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
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

#include "internal.h"

#ifdef __linux__
#include <linux/joystick.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif // __linux__


// Attempt to open the specified joystick device
//
static int openJoystickDevice(int joy, const char* path)
{
#ifdef __linux__
    char axisCount, buttonCount;
    char name[256];
    int fd, version;

    fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd == -1)
        return GL_FALSE;

    _glfw.x11.joystick[joy].fd = fd;

    // Verify that the joystick driver version is at least 1.0
    ioctl(fd, JSIOCGVERSION, &version);
    if (version < 0x010000)
    {
        // It's an old 0.x interface (we don't support it)
        close(fd);
        return GL_FALSE;
    }

    if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0)
        strncpy(name, "Unknown", sizeof(name));

    _glfw.x11.joystick[joy].name = strdup(name);

    ioctl(fd, JSIOCGAXES, &axisCount);
    _glfw.x11.joystick[joy].axisCount = (int) axisCount;

    ioctl(fd, JSIOCGBUTTONS, &buttonCount);
    _glfw.x11.joystick[joy].buttonCount = (int) buttonCount;

    _glfw.x11.joystick[joy].axes = calloc(axisCount, sizeof(float));
    _glfw.x11.joystick[joy].buttons = calloc(buttonCount, 1);

    _glfw.x11.joystick[joy].present = GL_TRUE;
#endif // __linux__

    return GL_TRUE;
}

// Polls for and processes events for all present joysticks
//
static void pollJoystickEvents(void)
{
#ifdef __linux__
    int i;
    ssize_t result;
    struct js_event e;

    for (i = 0;  i <= GLFW_JOYSTICK_LAST;  i++)
    {
        if (!_glfw.x11.joystick[i].present)
            continue;

        // Read all queued events (non-blocking)
        for (;;)
        {
            errno = 0;
            result = read(_glfw.x11.joystick[i].fd, &e, sizeof(e));

            if (errno == ENODEV)
            {
                free(_glfw.x11.joystick[i].axes);
                free(_glfw.x11.joystick[i].buttons);
                free(_glfw.x11.joystick[i].name);
                _glfw.x11.joystick[i].present = GL_FALSE;
            }

            if (result == -1)
                break;

            // We don't care if it's an init event or not
            e.type &= ~JS_EVENT_INIT;

            switch (e.type)
            {
                case JS_EVENT_AXIS:
                    _glfw.x11.joystick[i].axes[e.number] =
                        (float) e.value / 32767.0f;

                    // We need to change the sign for the Y axes, so that
                    // positive = up/forward, according to the GLFW spec.
                    if (e.number & 1)
                    {
                        _glfw.x11.joystick[i].axes[e.number] =
                            -_glfw.x11.joystick[i].axes[e.number];
                    }

                    break;

                case JS_EVENT_BUTTON:
                    _glfw.x11.joystick[i].buttons[e.number] =
                        e.value ? GLFW_PRESS : GLFW_RELEASE;
                    break;

                default:
                    break;
            }
        }
    }
#endif // __linux__
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize joystick interface
//
void _glfwInitJoysticks(void)
{
#ifdef __linux__
    int joy = 0;
    size_t i;
    regex_t regex;
    DIR* dir;
    const char* dirs[] =
    {
        "/dev/input",
        "/dev"
    };

    if (regcomp(&regex, "^js[0-9]\\+$", 0) != 0)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to compile regex");
        return;
    }

    for (i = 0;  i < sizeof(dirs) / sizeof(dirs[0]);  i++)
    {
        struct dirent* entry;

        dir = opendir(dirs[i]);
        if (!dir)
            continue;

        while ((entry = readdir(dir)))
        {
            char path[20];
            regmatch_t match;

            if (regexec(&regex, entry->d_name, 1, &match, 0) != 0)
                continue;

            snprintf(path, sizeof(path), "%s/%s", dirs[i], entry->d_name);
            if (openJoystickDevice(joy, path))
                joy++;
        }

        closedir(dir);
    }

    regfree(&regex);
#endif // __linux__
}

// Close all opened joystick handles
//
void _glfwTerminateJoysticks(void)
{
#ifdef __linux__
    int i;

    for (i = 0;  i <= GLFW_JOYSTICK_LAST;  i++)
    {
        if (_glfw.x11.joystick[i].present)
        {
            close(_glfw.x11.joystick[i].fd);
            free(_glfw.x11.joystick[i].axes);
            free(_glfw.x11.joystick[i].buttons);
            free(_glfw.x11.joystick[i].name);

            _glfw.x11.joystick[i].present = GL_FALSE;
        }
    }
#endif // __linux__
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformJoystickPresent(int joy)
{
    pollJoystickEvents();

    return _glfw.x11.joystick[joy].present;
}

const float* _glfwPlatformGetJoystickAxes(int joy, int* count)
{
    pollJoystickEvents();

    if (!_glfw.x11.joystick[joy].present)
        return NULL;

    *count = _glfw.x11.joystick[joy].axisCount;
    return _glfw.x11.joystick[joy].axes;
}

const unsigned char* _glfwPlatformGetJoystickButtons(int joy, int* count)
{
    pollJoystickEvents();

    if (!_glfw.x11.joystick[joy].present)
        return NULL;

    *count = _glfw.x11.joystick[joy].buttonCount;
    return _glfw.x11.joystick[joy].buttons;
}

const char* _glfwPlatformGetJoystickName(int joy)
{
    pollJoystickEvents();

    return _glfw.x11.joystick[joy].name;
}

