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
#if defined(_MSC_VER)
 #include <malloc.h>
#endif


//========================================================================
// Lexical comparison function for GLFW video modes, used by qsort
//========================================================================

int compareVideoModes(const void* firstPtr, const void* secondPtr)
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


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Create a monitor struct from the specified information
//========================================================================

_GLFWmonitor* _glfwCreateMonitor(const char* name,
                                 int physicalWidth, int physicalHeight,
                                 int screenX, int screenY)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) calloc(1, sizeof(_GLFWmonitor));
    if (!monitor)
    {
        _glfwSetError(GLFW_OUT_OF_MEMORY, NULL);
        return NULL;
    }

    monitor->name = strdup(name);
    monitor->physicalWidth = physicalWidth;
    monitor->physicalHeight = physicalHeight;
    monitor->screenX = screenX;
    monitor->screenY = screenY;

    return monitor;
}


//========================================================================
// Destroy the specified monitor
//========================================================================

void _glfwDestroyMonitor(_GLFWmonitor* monitor)
{
    if (monitor == NULL)
        return;

    _glfwPlatformDestroyMonitor(monitor);

    free(monitor->modes);
    free(monitor->name);
    free(monitor);
}


//========================================================================
// Enumerate monitors and notify user of changes
//========================================================================

void _glfwInputMonitorChange(void)
{
    int i, j, monitorCount;
    _GLFWmonitor** monitors;

    monitors = _glfwPlatformGetMonitors(&monitorCount);

    for (i = 0;  i < monitorCount;  i++)
    {
        for (j = 0;  j < _glfwLibrary.monitorCount;  j++)
        {
            if (_glfwLibrary.monitors[j] == NULL)
                continue;

            if (strcmp(monitors[i]->name, _glfwLibrary.monitors[j]->name) == 0)
            {
                // This monitor was connected before, so re-use the existing
                // monitor object to preserve its address and user pointer

                _glfwDestroyMonitor(monitors[i]);
                monitors[i] = _glfwLibrary.monitors[j];
                _glfwLibrary.monitors[j] = NULL;
                break;
            }
        }

        if (j == _glfwLibrary.monitorCount)
        {
            // This monitor was not connected before
            _glfwLibrary.monitorCallback(monitors[i], GLFW_MONITOR_CONNECTED);
        }
    }

    for (i = 0;  i < _glfwLibrary.monitorCount;  i++)
    {
        if (_glfwLibrary.monitors[i] == NULL)
            continue;

        // This monitor is no longer connected
        _glfwLibrary.monitorCallback(_glfwLibrary.monitors[i],
                                     GLFW_MONITOR_DISCONNECTED);
    }

    _glfwDestroyMonitors();

    _glfwLibrary.monitors = monitors;
    _glfwLibrary.monitorCount = monitorCount;
}


//========================================================================
// Destroy all monitors
//========================================================================

void _glfwDestroyMonitors(void)
{
    int i;

    for (i = 0;  i < _glfwLibrary.monitorCount;  i++)
        _glfwDestroyMonitor(_glfwLibrary.monitors[i]);

    free(_glfwLibrary.monitors);
    _glfwLibrary.monitors = NULL;
    _glfwLibrary.monitorCount = 0;
}


//========================================================================
// Lexical comparison of GLFW video modes
//========================================================================

int _glfwCompareVideoModes(const GLFWvidmode* first, const GLFWvidmode* second)
{
    return compareVideoModes(first, second);
}


//========================================================================
// Convert BPP to RGB bits based on "best guess"
//========================================================================

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

//========================================================================
// Return the currently connected monitors
//========================================================================

GLFWAPI GLFWmonitor* glfwGetMonitors(int* count)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    if (count == NULL)
    {
        _glfwSetError(GLFW_INVALID_VALUE, NULL);
        return NULL;
    }

    *count = _glfwLibrary.monitorCount;
    return (GLFWmonitor*) _glfwLibrary.monitors;
}


//========================================================================
// Get the primary monitor
//========================================================================

GLFWAPI GLFWmonitor glfwGetPrimaryMonitor(void)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    return _glfwLibrary.monitors[0];
}


//========================================================================
// Get monitor parameter
//========================================================================

GLFWAPI int glfwGetMonitorParam(GLFWmonitor handle, int param)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return 0;
    }

    if (monitor == NULL)
    {
        _glfwSetError(GLFW_INVALID_VALUE,
                      "glfwGetMonitorParam: Invalid monitor handle");
        return 0;
    }

    switch (param)
    {
        case GLFW_MONITOR_PHYSICAL_WIDTH:
            return monitor->physicalWidth;
        case GLFW_MONITOR_PHYSICAL_HEIGHT:
            return monitor->physicalHeight;
        case GLFW_MONITOR_SCREEN_POS_X:
            return monitor->screenX;
        case GLFW_MONITOR_SCREEN_POS_Y:
            return monitor->screenY;
    }

    _glfwSetError(GLFW_INVALID_ENUM,
                  "glfwGetMonitorParam: Invalid enum value for 'param' parameter");
    return 0;
}


//========================================================================
// Get monitor string
//========================================================================

GLFWAPI const char* glfwGetMonitorString(GLFWmonitor handle, int param)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    if (monitor == NULL)
    {
        _glfwSetError(GLFW_INVALID_VALUE,
                      "glfwGetMonitorString: Invalid monitor handle");
        return NULL;
    }

    switch (param)
    {
        case GLFW_MONITOR_NAME:
            return monitor->name;
    }

    _glfwSetError(GLFW_INVALID_ENUM,
                  "glfwGetMonitorString: Invalid enum value for 'param' parameter");
    return NULL;
}


//========================================================================
// Set a callback function for monitor events
//========================================================================

GLFWAPI void glfwSetMonitorCallback(GLFWmonitorfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    _glfwLibrary.monitorCallback= cbfun;
}


//========================================================================
// Get a list of available video modes
//========================================================================

GLFWAPI GLFWvidmode* glfwGetVideoModes(GLFWmonitor handle, int* count)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    if (monitor == NULL)
    {
        _glfwSetError(GLFW_INVALID_VALUE,
                      "glfwGetVideoModes: Invalid monitor handle");
        return 0;
    }

    if (count == NULL)
    {
        _glfwSetError(GLFW_INVALID_VALUE, NULL);
        return NULL;
    }

    free(monitor->modes);

    monitor->modes = _glfwPlatformGetVideoModes(monitor, count);
    if (monitor->modes)
        qsort(monitor->modes, *count, sizeof(GLFWvidmode), compareVideoModes);

    return monitor->modes;
}


//========================================================================
// Get the current video mode for the specified monitor
//========================================================================

GLFWAPI void glfwGetVideoMode(GLFWmonitor handle, GLFWvidmode* mode)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (mode == NULL)
    {
        _glfwSetError(GLFW_INVALID_VALUE, NULL);
        return;
    }

    _glfwPlatformGetVideoMode(monitor, mode);
}

