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
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
 #include <malloc.h>
#endif


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwAllocGammaRamp(GLFWgammaramp* ramp, unsigned int size)
{
    ramp->red = (unsigned short*) malloc(size * sizeof(unsigned short));
    ramp->green = (unsigned short*) malloc(size * sizeof(unsigned short));
    ramp->blue = (unsigned short*) malloc(size * sizeof(unsigned short));
    ramp->size = size;
}

void _glfwFreeGammaRamp(GLFWgammaramp* ramp)
{
    free(ramp->red);
    free(ramp->green);
    free(ramp->blue);

    memset(ramp, 0, sizeof(GLFWgammaramp));
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI void glfwSetGamma(GLFWmonitor* handle, float gamma)
{
    int i;
    unsigned short values[256];
    GLFWgammaramp ramp;

    _GLFW_REQUIRE_INIT();

    if (gamma <= 0.f)
    {
        _glfwInputError(GLFW_INVALID_VALUE,
                        "Gamma value must be greater than zero");
        return;
    }

    for (i = 0;  i < 256;  i++)
    {
        float value;

        // Calculate intensity
        value = i / 255.f;
        // Apply gamma curve
        value = (float) pow(value, 1.f / gamma) * 65535.f + 0.5f;

        // Clamp to value range
        if (value < 0.f)
            value = 0.f;
        else if (value > 65535.f)
            value = 65535.f;

        values[i] = (unsigned short) value;
    }

    ramp.red = values;
    ramp.green = values;
    ramp.blue = values;
    ramp.size = 256;

    glfwSetGammaRamp(handle, &ramp);
}

GLFWAPI const GLFWgammaramp* glfwGetGammaRamp(GLFWmonitor* handle)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    _glfwFreeGammaRamp(&monitor->currentRamp);
    _glfwPlatformGetGammaRamp(monitor, &monitor->currentRamp);

    return &monitor->currentRamp;
}

GLFWAPI void glfwSetGammaRamp(GLFWmonitor* handle, const GLFWgammaramp* ramp)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;

    _GLFW_REQUIRE_INIT();

    if (!monitor->originalRamp.size)
        _glfwPlatformGetGammaRamp(monitor, &monitor->originalRamp);

    _glfwPlatformSetGammaRamp(monitor, ramp);
}

