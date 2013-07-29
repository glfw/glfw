//========================================================================
// GLFW 3.0 Win32 - www.glfw.org
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


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformGetGammaRamp(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    HDC dc;
    WORD values[768];
    DISPLAY_DEVICE display;

    ZeroMemory(&display, sizeof(DISPLAY_DEVICE));
    display.cb = sizeof(DISPLAY_DEVICE);
    EnumDisplayDevices(monitor->win32.name, 0, &display, 0);

    dc = CreateDC(L"DISPLAY", display.DeviceString, NULL, NULL);
    GetDeviceGammaRamp(dc, values);
    DeleteDC(dc);

    _glfwAllocGammaArrays(ramp, 256);

    memcpy(ramp->red,   values +   0, 256 * sizeof(unsigned short));
    memcpy(ramp->green, values + 256, 256 * sizeof(unsigned short));
    memcpy(ramp->blue,  values + 512, 256 * sizeof(unsigned short));
}

void _glfwPlatformSetGammaRamp(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    HDC dc;
    WORD values[768];
    DISPLAY_DEVICE display;

    if (ramp->size != 256)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Gamma ramp size must be 256");
        return;
    }

    memcpy(values +   0, ramp->red,   256 * sizeof(unsigned short));
    memcpy(values + 256, ramp->green, 256 * sizeof(unsigned short));
    memcpy(values + 512, ramp->blue,  256 * sizeof(unsigned short));

    ZeroMemory(&display, sizeof(DISPLAY_DEVICE));
    display.cb = sizeof(DISPLAY_DEVICE);
    EnumDisplayDevices(monitor->win32.name, 0, &display, 0);

    dc = CreateDC(L"DISPLAY", display.DeviceString, NULL, NULL);
    SetDeviceGammaRamp(dc, values);
    DeleteDC(dc);
}

