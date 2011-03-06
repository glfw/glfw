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

//========================================================================
// Determine joystick capabilities
//========================================================================

GLFWAPI int glfwGetJoystickParam(int joy, int param)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return 0;
    }

    return _glfwPlatformGetJoystickParam(joy, param);
}


//========================================================================
// Get joystick axis positions
//========================================================================

GLFWAPI int glfwGetJoystickPos(int joy, float* pos, int numaxes)
{
    int i;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return 0;
    }

    // Clear positions
    for (i = 0;  i < numaxes;  i++)
        pos[i] = 0.0f;

    return _glfwPlatformGetJoystickPos(joy, pos, numaxes);
}


//========================================================================
// Get joystick button states
//========================================================================

GLFWAPI int glfwGetJoystickButtons(int joy,
                                   unsigned char* buttons,
                                   int numbuttons)
{
    int i;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return 0;
    }

    // Clear button states
    for (i = 0;  i < numbuttons;  i++)
        buttons[i] = GLFW_RELEASE;

    return _glfwPlatformGetJoystickButtons(joy, buttons, numbuttons);
}

