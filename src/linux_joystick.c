//========================================================================
// GLFW 3.3 Linux - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2016 Camilla Löwy <elmindreda@glfw.org>
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

#include <linux/joystick.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


// Attempt to open the specified joystick device
//
static GLFWbool openJoystickDevice(const char* path)
{
    char axisCount, buttonCount;
    char name[256] = "";
    int jid, fd, version;
    _GLFWjoystick* js;

    for (jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++)
    {
        if (!_glfw.joysticks[jid].present)
            continue;
        if (strcmp(_glfw.joysticks[jid].linjs.path, path) == 0)
            return GLFW_FALSE;
    }

    fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd == -1)
        return GLFW_FALSE;

    // Verify that the joystick driver version is at least 1.0
    ioctl(fd, JSIOCGVERSION, &version);
    if (version < 0x010000)
    {
        // It's an old 0.x interface (we don't support it)
        close(fd);
        return GLFW_FALSE;
    }

    if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0)
        strncpy(name, "Unknown", sizeof(name));

    ioctl(fd, JSIOCGAXES, &axisCount);
    ioctl(fd, JSIOCGBUTTONS, &buttonCount);

    js = _glfwAllocJoystick(name, axisCount, buttonCount, 0);
    if (!js)
    {
        close(fd);
        return GLFW_FALSE;
    }

    js->linjs.path = strdup(path);
    js->linjs.fd = fd;

    _glfwInputJoystick(_GLFW_JOYSTICK_ID(js), GLFW_CONNECTED);
    return GLFW_TRUE;
}

// Frees all resources associated with the specified joystick
//
static void closeJoystick(_GLFWjoystick* js)
{
    close(js->linjs.fd);
    free(js->linjs.path);
    _glfwFreeJoystick(js);
    _glfwInputJoystick(_GLFW_JOYSTICK_ID(js), GLFW_DISCONNECTED);
}

// Lexically compare joysticks by name; used by qsort
//
static int compareJoysticks(const void* fp, const void* sp)
{
    const _GLFWjoystick* fj = fp;
    const _GLFWjoystick* sj = sp;
    return strcmp(fj->linjs.path, sj->linjs.path);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize joystick interface
//
GLFWbool _glfwInitJoysticksLinux(void)
{
    DIR* dir;
    int count = 0;
    const char* dirname = "/dev/input";

    _glfw.linjs.inotify = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (_glfw.linjs.inotify == -1)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Linux: Failed to initialize inotify: %s",
                        strerror(errno));
        return GLFW_FALSE;
    }

    // HACK: Register for IN_ATTRIB as well to get notified when udev is done
    //       This works well in practice but the true way is libudev

    _glfw.linjs.watch = inotify_add_watch(_glfw.linjs.inotify,
                                          dirname,
                                          IN_CREATE | IN_ATTRIB | IN_DELETE);
    if (_glfw.linjs.watch == -1)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Linux: Failed to watch for joystick connections in %s: %s",
                        dirname,
                        strerror(errno));
        // Continue without device connection notifications
    }

    if (regcomp(&_glfw.linjs.regex, "^js[0-9]\\+$", 0) != 0)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Linux: Failed to compile regex");
        return GLFW_FALSE;
    }

    dir = opendir(dirname);
    if (dir)
    {
        struct dirent* entry;

        while ((entry = readdir(dir)))
        {
            char path[20];
            regmatch_t match;

            if (regexec(&_glfw.linjs.regex, entry->d_name, 1, &match, 0) != 0)
                continue;

            snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);
            if (openJoystickDevice(path))
                count++;
        }

        closedir(dir);
    }
    else
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Linux: Failed to open joystick device directory %s: %s",
                        dirname,
                        strerror(errno));
        // Continue with no joysticks detected
    }

    qsort(_glfw.joysticks, count, sizeof(_GLFWjoystick), compareJoysticks);
    return GLFW_TRUE;
}

// Close all opened joystick handles
//
void _glfwTerminateJoysticksLinux(void)
{
    int jid;

    for (jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++)
    {
        _GLFWjoystick* js = _glfw.joysticks + jid;
        if (js->present)
            closeJoystick(js);
    }

    regfree(&_glfw.linjs.regex);

    if (_glfw.linjs.inotify > 0)
    {
        if (_glfw.linjs.watch > 0)
            inotify_rm_watch(_glfw.linjs.inotify, _glfw.linjs.watch);

        close(_glfw.linjs.inotify);
    }
}

void _glfwDetectJoystickConnectionLinux(void)
{
    ssize_t offset = 0;
    char buffer[16384];

    const ssize_t size = read(_glfw.linjs.inotify, buffer, sizeof(buffer));

    while (size > offset)
    {
        regmatch_t match;
        const struct inotify_event* e = (struct inotify_event*) (buffer + offset);

        if (regexec(&_glfw.linjs.regex, e->name, 1, &match, 0) == 0)
        {
            char path[20];
            snprintf(path, sizeof(path), "/dev/input/%s", e->name);

            if (e->mask & (IN_CREATE | IN_ATTRIB))
                openJoystickDevice(path);
            else if (e->mask & IN_DELETE)
            {
                int jid;

                for (jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++)
                {
                    if (strcmp(_glfw.joysticks[jid].linjs.path, path) == 0)
                    {
                        closeJoystick(_glfw.joysticks + jid);
                        break;
                    }
                }
            }
        }

        offset += sizeof(struct inotify_event) + e->len;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformPollJoystick(int jid, int mode)
{
    _GLFWjoystick* js = _glfw.joysticks + jid;

    // Read all queued events (non-blocking)
    for (;;)
    {
        struct js_event e;

        errno = 0;
        if (read(js->linjs.fd, &e, sizeof(e)) < 0)
        {
            // Reset the joystick slot if the device was disconnected
            if (errno == ENODEV)
                closeJoystick(js);

            break;
        }

        // Clear the initial-state bit
        e.type &= ~JS_EVENT_INIT;

        if (e.type == JS_EVENT_AXIS)
            _glfwInputJoystickAxis(jid, e.number, e.value / 32767.0f);
        else if (e.type == JS_EVENT_BUTTON)
            _glfwInputJoystickButton(jid, e.number, e.value ? 1 : 0);
    }

    return js->present;
}

