//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2019 Camilla Löwy <elmindreda@glfw.org>
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// The the sole (fake) video mode of our (sole) fake monitor
//
static GLFWvidmode getVideoMode(void)
{
    struct Screen* currentScreen = IIntuition->LockPubScreen(NULL);
    
    GLFWvidmode mode;
    mode.width = currentScreen->Width;
    mode.height = currentScreen->Height;
    // TODO - Change this
    mode.redBits = 8;
    mode.greenBits = 8;
    mode.blueBits = 8;
    mode.refreshRate = 60;
    
    IIntuition->UnlockPubScreen(NULL, currentScreen);
    return mode;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPollMonitorsOS4(void)
{
    const float dpi = 141.f;
    const GLFWvidmode mode = getVideoMode();
    _GLFWmonitor* monitor = _glfwAllocMonitor("OS4 Monitor 0",
                                              (int) (mode.width * 25.4f / dpi),
                                              (int) (mode.height * 25.4f / dpi));
    _glfwInputMonitor(monitor, GLFW_CONNECTED, _GLFW_INSERT_FIRST);
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwFreeMonitorOS4(_GLFWmonitor* monitor)
{
    _glfwFreeGammaArrays(&monitor->os4.ramp);
}

void _glfwGetMonitorPosOS4(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;
}

void _glfwGetMonitorContentScaleOS4(_GLFWmonitor* monitor,
                                     float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _glfwGetMonitorWorkareaOS4(_GLFWmonitor* monitor,
                                 int* xpos, int* ypos,
                                 int* width, int* height)
{
    const GLFWvidmode mode = getVideoMode();

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 10;
    if (width)
        *width = mode.width;
    if (height)
        *height = mode.height - 10;
}

GLFWvidmode* _glfwGetVideoModesOS4(_GLFWmonitor* monitor, int* found)
{
    GLFWvidmode* mode = _glfw_calloc(1, sizeof(GLFWvidmode));
    *mode = getVideoMode();
    *found = 1;
    return mode;
}

void _glfwGetVideoModeOS4(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
    *mode = getVideoMode();
}

GLFWbool _glfwGetGammaRampOS4(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    if (!monitor->os4.ramp.size)
    {
        unsigned int i;

        _glfwAllocGammaArrays(&monitor->os4.ramp, 256);

        for (i = 0;  i < monitor->os4.ramp.size;  i++)
        {
            const float gamma = 2.2f;
            float value;
            value = i / (float) (monitor->os4.ramp.size - 1);
            value = powf(value, 1.f / gamma) * 65535.f + 0.5f;
            value = _glfw_fminf(value, 65535.f);

            monitor->os4.ramp.red[i]   = (unsigned short) value;
            monitor->os4.ramp.green[i] = (unsigned short) value;
            monitor->os4.ramp.blue[i]  = (unsigned short) value;
        }
    }

    _glfwAllocGammaArrays(ramp, monitor->os4.ramp.size);
    memcpy(ramp->red,   monitor->os4.ramp.red,   sizeof(short) * ramp->size);
    memcpy(ramp->green, monitor->os4.ramp.green, sizeof(short) * ramp->size);
    memcpy(ramp->blue,  monitor->os4.ramp.blue,  sizeof(short) * ramp->size);
    return GLFW_TRUE;
}

void _glfwSetGammaRampOS4(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    if (monitor->os4.ramp.size != ramp->size)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "OS4: Gamma ramp size must match current ramp size");
        return;
    }

    memcpy(monitor->os4.ramp.red,   ramp->red,   sizeof(short) * ramp->size);
    memcpy(monitor->os4.ramp.green, ramp->green, sizeof(short) * ramp->size);
    memcpy(monitor->os4.ramp.blue,  ramp->blue,  sizeof(short) * ramp->size);
}

