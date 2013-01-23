//========================================================================
// GLFW - An OpenGL library
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

GLFWAPI int glfwGetJoystickParam(int joy, int param)
{
    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return 0;
    }

    if (joy < 0 || joy > GLFW_JOYSTICK_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM, NULL);
        return 0;
    }

    return _glfwPlatformGetJoystickParam(joy, param);
}

GLFWAPI int glfwGetJoystickAxes(int joy, float* axes, int numaxes)
{
    int i;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return 0;
    }

    if (joy < 0 || joy > GLFW_JOYSTICK_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM, NULL);
        return 0;
    }

    if (axes == NULL || numaxes < 0)
    {
        _glfwInputError(GLFW_INVALID_VALUE, NULL);
        return 0;
    }

    // Clear positions
    for (i = 0;  i < numaxes;  i++)
        axes[i] = 0.0f;

    return _glfwPlatformGetJoystickAxes(joy, axes, numaxes);
}

GLFWAPI int glfwGetJoystickButtons(int joy,
                                   unsigned char* buttons,
                                   int numbuttons)
{
    int i;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return 0;
    }

    if (joy < 0 || joy > GLFW_JOYSTICK_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM, NULL);
        return 0;
    }

    if (buttons == NULL || numbuttons < 0)
    {
        _glfwInputError(GLFW_INVALID_VALUE, NULL);
        return 0;
    }

    // Clear button states
    for (i = 0;  i < numbuttons;  i++)
        buttons[i] = GLFW_RELEASE;

    return _glfwPlatformGetJoystickButtons(joy, buttons, numbuttons);
}

GLFWAPI const char* glfwGetJoystickName(int joy)
{
    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    if (joy < 0 || joy > GLFW_JOYSTICK_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM, NULL);
        return NULL;
    }

    return _glfwPlatformGetJoystickName(joy);
}

