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


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Iterate through connected monitors
//========================================================================

GLFWAPI GLFWmonitor glfwGetNextMonitor(GLFWmonitor handle)
{
    _GLFWmonitor* iterator = (_GLFWmonitor*) handle;
    _GLFWmonitor* result = NULL;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return result;
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

