//========================================================================
// GLFW 3.1 Linux - www.glfw.org
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
#include <unistd.h>
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

    _glfw.linux_js[joy].fd = fd;

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

    _glfw.linux_js[joy].name = strdup(name);

    ioctl(fd, JSIOCGAXES, &axisCount);
    _glfw.linux_js[joy].axisCount = (int) axisCount;

    ioctl(fd, JSIOCGBUTTONS, &buttonCount);
    _glfw.linux_js[joy].buttonCount = (int) buttonCount;

    _glfw.linux_js[joy].axes = calloc(axisCount, sizeof(float));
    _glfw.linux_js[joy].buttons = calloc(buttonCount, 1);

    _glfw.linux_js[joy].present = GL_TRUE;
#endif // __linux__

    return GL_TRUE;
}

// Polls for and processes events for all present joysticks
//
static void pollJoystickEvents(void)
{
#ifdef __linux__
    int i;
    struct js_event e;

    for (i = 0;  i <= GLFW_JOYSTICK_LAST;  i++)
    {
        if (!_glfw.linux_js[i].present)
            continue;

        // Read all queued events (non-blocking)
        for (;;)
        {
            errno = 0;
            if (read(_glfw.linux_js[i].fd, &e, sizeof(e)) < 0)
            {
                if (errno == ENODEV)
                {
                    // The joystick was disconnected

                    free(_glfw.linux_js[i].axes);
                    free(_glfw.linux_js[i].buttons);
                    free(_glfw.linux_js[i].name);

                    memset(&_glfw.linux_js[i], 0, sizeof(_glfw.linux_js[i]));
                }

                break;
            }

            // We don't care if it's an init event or not
            e.type &= ~JS_EVENT_INIT;

            switch (e.type)
            {
                case JS_EVENT_AXIS:
                    _glfw.linux_js[i].axes[e.number] =
                        (float) e.value / 32767.0f;
                    break;

                case JS_EVENT_BUTTON:
                    _glfw.linux_js[i].buttons[e.number] =
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
        if (_glfw.linux_js[i].present)
        {
            close(_glfw.linux_js[i].fd);
            free(_glfw.linux_js[i].axes);
            free(_glfw.linux_js[i].buttons);
            free(_glfw.linux_js[i].name);
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

    return _glfw.linux_js[joy].present;
}

const float* _glfwPlatformGetJoystickAxes(int joy, int* count)
{
    pollJoystickEvents();

    *count = _glfw.linux_js[joy].axisCount;
    return _glfw.linux_js[joy].axes;
}

const unsigned char* _glfwPlatformGetJoystickButtons(int joy, int* count)
{
    pollJoystickEvents();

    *count = _glfw.linux_js[joy].buttonCount;
    return _glfw.linux_js[joy].buttons;
}

const char* _glfwPlatformGetJoystickName(int joy)
{
    pollJoystickEvents();

    return _glfw.linux_js[joy].name;
}

