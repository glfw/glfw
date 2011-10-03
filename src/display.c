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


//========================================================================
// Get a list of connected monitors
//========================================================================

GLFWAPI GLFWmonitor glfwGetNextMonitor(GLFWmonitor iterator)
{
    GLFWmonitor result = GLFW_MONITOR_INVALID_HANDLE;
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return result;
    }

    if (iterator == GLFW_MONITOR_INVALID_HANDLE)
    {
        result = _glfwLibrary.monitorListHead;
    }
    else
    {
        result = iterator->next;
    }
    return result;
}

GLFWAPI int glfwGetMonitorIntegerParam(GLFWmonitor monitor, int param)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return 0;
    }

    if (monitor == GLFW_MONITOR_INVALID_HANDLE)
    {
        _glfwSetError(GLFW_INVALID_VALUE, "Monitor handle is invalid.");
        return 0;
    }

    switch(param)
    {
        case GLFW_MONITOR_PARAM_I_PHYS_WIDTH:
            return monitor->physicalWidth;
        case GLFW_MONITOR_PARAM_I_PHYS_HEIGHT:
            return monitor->physicalHeight;
        case GLFW_MONITOR_PARAM_I_SCREEN_X_POS:
            return monitor->screenXPosition;
        case GLFW_MONITOR_PARAM_I_SCREEN_Y_POS:
            return monitor->screenYPosition;
        default:
            _glfwSetError(GLFW_INVALID_ENUM, "Param represents not a valid integer monitor attribute.");
            return 0;
    }
}

GLFWAPI const char* glfwGetMonitorStringParam(GLFWmonitor monitor, int param)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    if (monitor == GLFW_MONITOR_INVALID_HANDLE)
    {
        _glfwSetError(GLFW_INVALID_VALUE, "monitor handle is invalid.");
        return NULL;
    }

    switch(param)
    {
        case GLFW_MONITOR_PARAM_S_NAME:
            return monitor->deviceName;
        default:
            _glfwSetError(GLFW_INVALID_ENUM, "Param represents not a valid string monitor attribute.");
            return NULL;
    }
}

