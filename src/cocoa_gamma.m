//========================================================================
// GLFW - An OpenGL library
// Platform:    Cocoa/NSOpenGL
// API version: 3.0
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2010 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <string.h>


//************************************************************************
//****                  GLFW internal functions                       ****
//************************************************************************

//========================================================================
// Save the original gamma ramp so that we can restore it later
//========================================================================

void _glfwPlatformGetGammaRamp(GLFWgammaramp* ramp)
{
    uint32_t sampleCount;
    int i;
    CGGammaValue red[GLFW_GAMMA_RAMP_SIZE];
    CGGammaValue green[GLFW_GAMMA_RAMP_SIZE];
    CGGammaValue blue[GLFW_GAMMA_RAMP_SIZE];
    CGGetDisplayTransferByTable(0, GLFW_GAMMA_RAMP_SIZE, red, green, blue, &sampleCount);
    for (i = 0; i < GLFW_GAMMA_RAMP_SIZE; i++)
    {
        _glfwLibrary.currentRamp.red[i] = red[i] * 65535;
        _glfwLibrary.currentRamp.green[i] = green[i] * 65535;
        _glfwLibrary.currentRamp.blue[i] = blue[i] * 65535;
    }
}


//========================================================================
// Make the specified gamma ramp current
//========================================================================

void _glfwPlatformSetGammaRamp(const GLFWgammaramp* ramp)
{
    int i;
    CGGammaValue red[GLFW_GAMMA_RAMP_SIZE];
    CGGammaValue green[GLFW_GAMMA_RAMP_SIZE];
    CGGammaValue blue[GLFW_GAMMA_RAMP_SIZE];
    for (i = 0; i < GLFW_GAMMA_RAMP_SIZE; i++)
    {
        red[i] = ((float)_glfwLibrary.currentRamp.red[i]) / 65535.0f;
        blue[i] = ((float)_glfwLibrary.currentRamp.green[i]) / 65535.0f;
        green[i] = ((float)_glfwLibrary.currentRamp.blue[i]) / 65535.0f;
    }
    CGSetDisplayTransferByTable(0, GLFW_GAMMA_RAMP_SIZE, red, green, blue);
}

