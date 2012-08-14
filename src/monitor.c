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


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Iterate through connected monitors
//========================================================================

GLFWAPI GLFWmonitor glfwGetNextMonitor(GLFWmonitor handle)
{
    _GLFWmonitor* iterator = (_GLFWmonitor*) handle;
    _GLFWmonitor* result;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    if (iterator == NULL)
        result = _glfwLibrary.monitorListHead;
    else
        result = iterator->next;

    return result;
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
// Initialize the monitor list.
//========================================================================

void _glfwInitMonitors(void)
{
    _glfwLibrary.monitorListHead = _glfwCreateMonitors();
}


//========================================================================
// Refresh monitor list and notify callback.
//========================================================================

void _glfwRefreshMonitors(void)
{
    _GLFWmonitor* newMonitorList;
    _GLFWmonitor* curNewMonitor;
    _GLFWmonitor* curOldMonitor;

    newMonitorList = _glfwCreateMonitors();
    curNewMonitor = newMonitorList;
    curOldMonitor = _glfwLibrary.monitorListHead;

    while (_glfwLibrary.monitorCallback && (curNewMonitor || curOldMonitor))
    {
        _GLFWmonitor* lookAheadOldMonitor;
        _GLFWmonitor* lookAheadNewMonitor;

        if (curOldMonitor && curNewMonitor && !strcmp(curOldMonitor->name, curOldMonitor->name))
        {
            curNewMonitor = curNewMonitor->next;
            curOldMonitor = curOldMonitor->next;
            continue;
        }

        if (curNewMonitor && !curOldMonitor)
        {
            _glfwLibrary.monitorCallback(curNewMonitor, GLFW_MONITOR_CONNECTED);
            curNewMonitor = curNewMonitor->next;
            continue;
        }

        if (!curNewMonitor && curOldMonitor)
        {
            _glfwLibrary.monitorCallback(curOldMonitor, GLFW_MONITOR_DISCONNECTED);
            curOldMonitor = curOldMonitor->next;
            continue;
        }

        lookAheadOldMonitor = curOldMonitor->next;
        lookAheadNewMonitor = curNewMonitor->next;

        while (lookAheadOldMonitor && !strcmp(curNewMonitor->name, lookAheadOldMonitor->name))
            lookAheadOldMonitor = lookAheadOldMonitor->next;

        while (lookAheadNewMonitor && !strcmp(curOldMonitor->name, lookAheadNewMonitor->name))
            lookAheadNewMonitor = lookAheadNewMonitor->next;

        if (!lookAheadOldMonitor)
        {
            // nothing found in the old monitor list, that matches the current new monitor.
            _glfwLibrary.monitorCallback(curNewMonitor, GLFW_MONITOR_CONNECTED);
            curNewMonitor = curNewMonitor->next;
        }
        else
        {
            while (strcmp(curOldMonitor->name, lookAheadOldMonitor->name))
            {
                _glfwLibrary.monitorCallback(curOldMonitor, GLFW_MONITOR_DISCONNECTED);
                curOldMonitor = curOldMonitor->next;
            }
        }

        if (!lookAheadNewMonitor)
        {
            // nothing found in the new monitor list, that matches the current old monitor.
            _glfwLibrary.monitorCallback(curOldMonitor, GLFW_MONITOR_DISCONNECTED);
            curOldMonitor = curOldMonitor->next;
        }
        else
        {
            while (strcmp(curNewMonitor->name, lookAheadNewMonitor->name))
            {
                _glfwLibrary.monitorCallback(curNewMonitor, GLFW_MONITOR_CONNECTED);
                curNewMonitor = curNewMonitor->next;
            }
        }
    }

    _glfwTerminateMonitors();
    _glfwLibrary.monitorListHead = newMonitorList;
}


//========================================================================
// Delete the monitor list.
//========================================================================

void _glfwTerminateMonitors(void)
{
    while (_glfwLibrary.monitorListHead)
        _glfwLibrary.monitorListHead = _glfwDestroyMonitor(_glfwLibrary.monitorListHead);
}

