//========================================================================
// GLFW 3.1 - www.glfw.org
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

GLFWAPI int glfwJoystickPresent(int joy)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(0);

    if (joy < 0 || joy > GLFW_JOYSTICK_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM, NULL);
        return 0;
    }

    return _glfwPlatformJoystickPresent(joy);
}

GLFWAPI const float* glfwGetJoystickAxes(int joy, int* count)
{
    *count = 0;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    if (joy < 0 || joy > GLFW_JOYSTICK_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM, NULL);
        return NULL;
    }

    return _glfwPlatformGetJoystickAxes(joy, count);
}

GLFWAPI const unsigned char* glfwGetJoystickButtons(int joy, int* count)
{
    *count = 0;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    if (joy < 0 || joy > GLFW_JOYSTICK_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM, NULL);
        return NULL;
    }

    return _glfwPlatformGetJoystickButtons(joy, count);
}

GLFWAPI const char* glfwGetJoystickName(int joy)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    if (joy < 0 || joy > GLFW_JOYSTICK_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM, NULL);
        return NULL;
    }

    return _glfwPlatformGetJoystickName(joy);
}

