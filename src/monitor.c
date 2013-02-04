//========================================================================
// GLFW - An OpenGL framework
// Platform:    Any
// API version: 3.0
// WWW:         http://www.glfw.org/
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

#include <string.h>
#include <stdlib.h>
#include <limits.h>

#if defined(_MSC_VER)
 #include <malloc.h>
 #define strdup _strdup
#endif


// Lexical comparison function for GLFW video modes, used by qsort
//
static int compareVideoModes(const void* firstPtr, const void* secondPtr)
{
    int firstBPP, secondBPP, firstSize, secondSize;
    GLFWvidmode* first = (GLFWvidmode*) firstPtr;
    GLFWvidmode* second = (GLFWvidmode*) secondPtr;

    // First sort on color bits per pixel

    firstBPP = first->redBits +
               first->greenBits +
               first->blueBits;
    secondBPP = second->redBits +
                second->greenBits +
                second->blueBits;

    if (firstBPP != secondBPP)
        return firstBPP - secondBPP;

    // Then sort on screen area, in pixels

    firstSize = first->width * first->height;
    secondSize = second->width * second->height;

    return firstSize - secondSize;
}

// Retrieves the available modes for the specified monitor
//
static int refreshVideoModes(_GLFWmonitor* monitor)
{
    int modeCount;

    GLFWvidmode* modes = _glfwPlatformGetVideoModes(monitor, &modeCount);
    if (!modes)
        return GL_FALSE;

    qsort(modes, modeCount, sizeof(GLFWvidmode), compareVideoModes);

    free(monitor->modes);
    monitor->modes = modes;
    monitor->modeCount = modeCount;

    return GL_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                         GLFW event API                       //////
//////////////////////////////////////////////////////////////////////////

void _glfwInputMonitorChange(void)
{
    int i, j, monitorCount;
    _GLFWmonitor** monitors;

    monitors = _glfwPlatformGetMonitors(&monitorCount);

    for (i = 0;  i < monitorCount;  i++)
    {
        for (j = 0;  j < _glfw.monitorCount;  j++)
        {
            if (_glfw.monitors[j] == NULL)
                continue;

            if (strcmp(monitors[i]->name, _glfw.monitors[j]->name) == 0)
            {
                // This monitor was connected before, so re-use the existing
                // monitor object to preserve its address and user pointer

                // TODO: Transfer monitor properties

                _glfwDestroyMonitor(monitors[i]);
                monitors[i] = _glfw.monitors[j];
                _glfw.monitors[j] = NULL;
                break;
            }
        }

        if (j == _glfw.monitorCount)
        {
            // This monitor was not connected before
            _glfw.monitorCallback((GLFWmonitor*) monitors[i], GLFW_CONNECTED);
        }
    }

    for (i = 0;  i < _glfw.monitorCount;  i++)
    {
        _GLFWwindow* window;

        if (_glfw.monitors[i] == NULL)
            continue;

        // This monitor is no longer connected
        _glfw.monitorCallback((GLFWmonitor*) _glfw.monitors[i], GLFW_DISCONNECTED);

        for (window = _glfw.windowListHead;  window;  window = window->next)
        {
            if (window->monitor == _glfw.monitors[i])
                window->monitor = NULL;
        }
    }

    _glfwDestroyMonitors();

    _glfw.monitors = monitors;
    _glfw.monitorCount = monitorCount;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

_GLFWmonitor* _glfwCreateMonitor(const char* name,
                                 GLboolean primary,
                                 int widthMM, int heightMM,
                                 int x, int y)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) calloc(1, sizeof(_GLFWmonitor));
    if (!monitor)
    {
        _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
        return NULL;
    }

    monitor->name = strdup(name);
    monitor->primary = primary;
    monitor->widthMM = widthMM;
    monitor->heightMM = heightMM;
    monitor->positionX = x;
    monitor->positionY = y;

    return monitor;
}

void _glfwDestroyMonitor(_GLFWmonitor* monitor)
{
    if (monitor == NULL)
        return;

    _glfwPlatformDestroyMonitor(monitor);

    free(monitor->modes);
    free(monitor->name);
    free(monitor);
}

void _glfwDestroyMonitors(void)
{
    int i;

    for (i = 0;  i < _glfw.monitorCount;  i++)
        _glfwDestroyMonitor(_glfw.monitors[i]);

    free(_glfw.monitors);
    _glfw.monitors = NULL;
    _glfw.monitorCount = 0;
}

const GLFWvidmode* _glfwChooseVideoMode(_GLFWmonitor* monitor,
                                        const GLFWvidmode* desired)
{
    int i;
    unsigned int sizeDiff, leastSizeDiff = UINT_MAX;
    unsigned int colorDiff, leastColorDiff = UINT_MAX;
    const GLFWvidmode* current;
    const GLFWvidmode* closest = NULL;

    if (!refreshVideoModes(monitor))
        return NULL;

    for (i = 0;  i < monitor->modeCount;  i++)
    {
        current = monitor->modes + i;

        colorDiff = abs((current->redBits + current->greenBits + current->blueBits) -
                        (desired->redBits + desired->greenBits + desired->blueBits));

        sizeDiff = abs((current->width - desired->width) *
                       (current->width - desired->width) +
                       (current->height - desired->height) *
                       (current->height - desired->height));

        if ((colorDiff < leastColorDiff) ||
            (colorDiff == leastColorDiff && sizeDiff < leastSizeDiff))
        {
            closest = current;
            leastSizeDiff = sizeDiff;
            leastColorDiff = colorDiff;
        }
    }

    return closest;
}

int _glfwCompareVideoModes(const GLFWvidmode* first, const GLFWvidmode* second)
{
    return compareVideoModes(first, second);
}

void _glfwSplitBPP(int bpp, int* red, int* green, int* blue)
{
    int delta;

    // We assume that by 32 the user really meant 24
    if (bpp == 32)
        bpp = 24;

    // Convert "bits per pixel" to red, green & blue sizes

    *red = *green = *blue = bpp / 3;
    delta = bpp - (*red * 3);
    if (delta >= 1)
        *green = *green + 1;

    if (delta == 2)
        *red = *red + 1;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI GLFWmonitor** glfwGetMonitors(int* count)
{
    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    *count = _glfw.monitorCount;
    return (GLFWmonitor**) _glfw.monitors;
}

GLFWAPI GLFWmonitor* glfwGetPrimaryMonitor(void)
{
    int i;
    _GLFWmonitor* primary = NULL;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    for (i = 0;  i < _glfw.monitorCount;  i++)
    {
        if (_glfw.monitors[i]->primary)
        {
            primary = _glfw.monitors[i];
            break;
        }
    }

    if (!primary)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, NULL);
        return NULL;
    }

    return (GLFWmonitor*) primary;
}

GLFWAPI void glfwGetMonitorPos(GLFWmonitor* handle, int* xpos, int* ypos)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (xpos)
        *xpos = monitor->positionX;
    if (ypos)
        *ypos = monitor->positionY;
}

GLFWAPI void glfwGetMonitorPhysicalSize(GLFWmonitor* handle, int* width, int* height)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (width)
        *width = monitor->widthMM;
    if (height)
        *height = monitor->heightMM;
}

GLFWAPI const char* glfwGetMonitorName(GLFWmonitor* handle)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    return monitor->name;
}

GLFWAPI void glfwSetMonitorCallback(GLFWmonitorfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    _glfw.monitorCallback = cbfun;
}

GLFWAPI const GLFWvidmode* glfwGetVideoModes(GLFWmonitor* handle, int* count)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    if (!refreshVideoModes(monitor))
        return GL_FALSE;

    *count = monitor->modeCount;
    return monitor->modes;
}

GLFWAPI GLFWvidmode glfwGetVideoMode(GLFWmonitor* handle)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    GLFWvidmode mode = { 0, 0, 0, 0, 0 };

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return mode;
    }

    _glfwPlatformGetVideoMode(monitor, &mode);
    return mode;
}

