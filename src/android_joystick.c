//========================================================================
// GLFW 3.5 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2024 kunitoki <kunitoki@gmail.com>
// Copyright (c) 2017 Curi0 <curi0minecraft@gmail.com>
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
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwInitJoysticksAndroid(void)
{
    return GLFW_FALSE;
}

void _glfwTerminateJoysticksAndroid(void)
{
}

GLFWbool _glfwPollJoystickAndroid(_GLFWjoystick* js, int mode)
{
    return GLFW_FALSE;
}

const char* _glfwGetMappingNameAndroid(void)
{
    return "Android";
}

void _glfwUpdateGamepadGUIDAndroid(char* guid)
{
}
