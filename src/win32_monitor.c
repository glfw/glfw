//========================================================================
// GLFW 3.0 Win32 - www.glfw.org
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

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>

// These constants are missing on MinGW
#ifndef EDS_ROTATEDMODE
 #define EDS_ROTATEDMODE 0x00000004
#endif
#ifndef DISPLAY_DEVICE_ACTIVE
 #define DISPLAY_DEVICE_ACTIVE 0x00000001
#endif


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Change the current video mode
//
GLboolean _glfwSetVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* desired)
{
    GLFWvidmode current;
    const GLFWvidmode* best;
    DEVMODE dm;

    best = _glfwChooseVideoMode(monitor, desired);

    _glfwPlatformGetVideoMode(monitor, &current);
    if (_glfwCompareVideoModes(&current, best) == 0)
        return GL_TRUE;

    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(DEVMODE);
    dm.dmFields           = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL |
                            DM_DISPLAYFREQUENCY;
    dm.dmPelsWidth        = best->width;
    dm.dmPelsHeight       = best->height;
    dm.dmBitsPerPel       = best->redBits + best->greenBits + best->blueBits;
    dm.dmDisplayFrequency = best->refreshRate;

    if (dm.dmBitsPerPel < 15 || dm.dmBitsPerPel >= 24)
        dm.dmBitsPerPel = 32;

    if (ChangeDisplaySettingsEx(monitor->win32.name,
                                &dm,
                                NULL,
                                CDS_FULLSCREEN,
                                NULL) != DISP_CHANGE_SUCCESSFUL)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to set video mode");
        return GL_FALSE;
    }

    return GL_TRUE;
}

// Restore the previously saved (original) video mode
//
void _glfwRestoreVideoMode(_GLFWmonitor* monitor)
{
    ChangeDisplaySettingsEx(monitor->win32.name,
                            NULL, NULL, CDS_FULLSCREEN, NULL);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

_GLFWmonitor** _glfwPlatformGetMonitors(int* count)
{
    int size = 0, found = 0;
    _GLFWmonitor** monitors = NULL;
    DWORD adapterIndex = 0;
    int primaryIndex = 0;

    *count = 0;

    for (;;)
    {
        DISPLAY_DEVICE adapter, display;
        char* name;
        HDC dc;

        ZeroMemory(&adapter, sizeof(DISPLAY_DEVICE));
        adapter.cb = sizeof(DISPLAY_DEVICE);

        if (!EnumDisplayDevices(NULL, adapterIndex, &adapter, 0))
            break;

        adapterIndex++;

        if ((adapter.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) ||
            !(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
        {
            continue;
        }

        if (found == size)
        {
            if (size)
                size *= 2;
            else
                size = 4;

            monitors = (_GLFWmonitor**) realloc(monitors, sizeof(_GLFWmonitor*) * size);
        }

        ZeroMemory(&display, sizeof(DISPLAY_DEVICE));
        display.cb = sizeof(DISPLAY_DEVICE);

        EnumDisplayDevices(adapter.DeviceName, 0, &display, 0);
        dc = CreateDC(L"DISPLAY", display.DeviceString, NULL, NULL);

        if (adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
            primaryIndex = found;

        name = _glfwCreateUTF8FromWideString(display.DeviceString);
        if (!name)
        {
            _glfwDestroyMonitors(monitors, found);
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "Failed to convert string to UTF-8");

            free(monitors);
            return NULL;
        }

        monitors[found] = _glfwCreateMonitor(name,
                                             GetDeviceCaps(dc, HORZSIZE),
                                             GetDeviceCaps(dc, VERTSIZE));

        free(name);
        DeleteDC(dc);

        wcscpy(monitors[found]->win32.name, adapter.DeviceName);
        found++;
    }

    if (primaryIndex > 0)
    {
        _GLFWmonitor* temp = monitors[0];
        monitors[0] = monitors[primaryIndex];
        monitors[primaryIndex] = temp;
    }

    *count = found;
    return monitors;
}

GLboolean _glfwPlatformIsSameMonitor(_GLFWmonitor* first, _GLFWmonitor* second)
{
    return wcscmp(first->win32.name, second->win32.name) == 0;
}

void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    DEVMODE settings;
    ZeroMemory(&settings, sizeof(DEVMODE));
    settings.dmSize = sizeof(DEVMODE);

    EnumDisplaySettingsEx(monitor->win32.name,
                          ENUM_CURRENT_SETTINGS,
                          &settings,
                          EDS_ROTATEDMODE);

    if (xpos)
        *xpos = settings.dmPosition.x;
    if (ypos)
        *ypos = settings.dmPosition.y;
}

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
    int modeIndex = 0, count = 0;
    GLFWvidmode* result = NULL;

    *found = 0;

    for (;;)
    {
        int i;
        GLFWvidmode mode;
        DEVMODE dm;

        ZeroMemory(&dm, sizeof(DEVMODE));
        dm.dmSize = sizeof(DEVMODE);

        if (!EnumDisplaySettings(monitor->win32.name, modeIndex, &dm))
            break;

        modeIndex++;

        if (dm.dmBitsPerPel < 15)
        {
            // Skip modes with less than 15 BPP
            continue;
        }

        mode.width  = dm.dmPelsWidth;
        mode.height = dm.dmPelsHeight;
        mode.refreshRate = dm.dmDisplayFrequency;
        _glfwSplitBPP(dm.dmBitsPerPel,
                      &mode.redBits,
                      &mode.greenBits,
                      &mode.blueBits);

        for (i = 0;  i < *found;  i++)
        {
            if (_glfwCompareVideoModes(result + i, &mode) == 0)
                break;
        }

        if (i < *found)
        {
            // This is a duplicate, so skip it
            continue;
        }

        if (*found == count)
        {
            if (count)
                count *= 2;
            else
                count = 128;

            result = (GLFWvidmode*) realloc(result, count * sizeof(GLFWvidmode));
        }

        result[*found] = mode;
        (*found)++;
    }

    return result;
}

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
    DEVMODE dm;

    ZeroMemory(&dm, sizeof(DEVMODE));
    dm.dmSize = sizeof(DEVMODE);

    EnumDisplaySettings(monitor->win32.name, ENUM_CURRENT_SETTINGS, &dm);

    mode->width  = dm.dmPelsWidth;
    mode->height = dm.dmPelsHeight;
    mode->refreshRate = dm.dmDisplayFrequency;
    _glfwSplitBPP(dm.dmBitsPerPel,
                  &mode->redBits,
                  &mode->greenBits,
                  &mode->blueBits);
}

