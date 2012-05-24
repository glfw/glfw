//========================================================================
// GLFW - An OpenGL library
// Platform:    Any
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

#include <math.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Calculate a gamma ramp from the specified value and set it
//========================================================================

GLFWAPI void glfwSetGamma(float gamma)
{
    int i, size = GLFW_GAMMA_RAMP_SIZE;
    GLFWgammaramp ramp;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (gamma <= 0.f)
    {
        _glfwSetError(GLFW_INVALID_VALUE,
                      "glfwSetGamma: Gamma value must be greater than zero");
        return;
    }

    for (i = 0;  i < size;  i++)
    {
        float value = (float) i / ((float) (size - 1));

        // Apply gamma
        value = (float) pow(value, 1.f / gamma) * 65535.f + 0.5f;

        // Clamp values
        if (value < 0.f)
            value = 0.f;
        else if (value > 65535.f)
            value = 65535.f;

        // Set the gamma ramp values
        ramp.red[i]   = (unsigned short) value;
        ramp.green[i] = (unsigned short) value;
        ramp.blue[i]  = (unsigned short) value;
    }

    glfwSetGammaRamp(&ramp);
}


//========================================================================
// Return the cached currently set gamma ramp
//========================================================================

GLFWAPI void glfwGetGammaRamp(GLFWgammaramp* ramp)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    *ramp = _glfwLibrary.currentRamp;
}


//========================================================================
// Make the specified gamma ramp current
//========================================================================

GLFWAPI void glfwSetGammaRamp(const GLFWgammaramp* ramp)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    _glfwPlatformSetGammaRamp(ramp);
    _glfwLibrary.currentRamp = *ramp;
    _glfwLibrary.rampChanged = GL_TRUE;
}

