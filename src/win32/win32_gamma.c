//========================================================================
// GLFW - An OpenGL framework
// Platform:    Win32/WGL
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

#include <limits.h>


//************************************************************************
//****                  GLFW internal functions                       ****
//************************************************************************

//========================================================================
// Save the gamma ramp to our internal copy
//========================================================================

void _glfwPlatformGetGammaRamp(GLFWgammaramp* ramp)
{
    _glfw_GetDeviceGammaRamp(GetDC(GetDesktopWindow()), (WORD*) ramp);
}


//========================================================================
// Restore the gamma ramp to our internal copy of the gamma ramp
//========================================================================

void _glfwPlatformSetGammaRamp(const GLFWgammaramp* ramp)
{
    _glfw_SetDeviceGammaRamp(GetDC(GetDesktopWindow()), (WORD*) ramp);
}

