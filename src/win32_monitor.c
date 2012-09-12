//========================================================================
// GLFW - An OpenGL library
// Platform:    X11 (Unix)
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

#include <stdlib.h>
#include <string.h>

// These constants are missing on MinGW
#ifndef EDS_ROTATEDMODE
 #define EDS_ROTATEDMODE 0x00000004
#endif
#ifndef DISPLAY_DEVICE_ACTIVE
 #define DISPLAY_DEVICE_ACTIVE 0x00000001
#endif


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Return a list of available monitors
//========================================================================

_GLFWmonitor** _glfwPlatformGetMonitors(int* count)
{
    int size = 0, found = 0;
    _GLFWmonitor** monitors = NULL;
    DWORD adapterIndex = 0;

    for (;;)
    {
        // Enumerate display adapters

        DISPLAY_DEVICE adapter;
        DWORD monitorIndex = 0;

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

        for (;;)
        {
            // Enumerate monitors for the display adapter

            DISPLAY_DEVICE monitor;
            DEVMODE settings;
            const char* name;
            HDC dc;

            ZeroMemory(&monitor, sizeof(DISPLAY_DEVICE));
            monitor.cb = sizeof(DISPLAY_DEVICE);

            if (!EnumDisplayDevices(adapter.DeviceName, monitorIndex, &monitor, 0))
                break;

            monitorIndex++;

            ZeroMemory(&settings, sizeof(DEVMODE));
            settings.dmSize = sizeof(DEVMODE);

            EnumDisplaySettingsEx(adapter.DeviceName,
                                  ENUM_CURRENT_SETTINGS,
                                  &settings,
                                  EDS_ROTATEDMODE);

            name = _glfwCreateUTF8FromWideString(monitor.DeviceName);
            if (!name)
            {
                // TODO: wat
                return NULL;
            }

            dc = CreateDC(L"DISPLAY", monitor.DeviceString, NULL, NULL);
            if (!dc)
            {
                // TODO: wat
                return NULL;
            }

            if (found == size)
            {
                if (size)
                    size *= 2;
                else
                    size = 4;

                monitors = (_GLFWmonitor**) realloc(monitors, sizeof(_GLFWmonitor*) * size);
            }

            monitors[found] = _glfwCreateMonitor(name,
                                                 GetDeviceCaps(dc, HORZSIZE),
                                                 GetDeviceCaps(dc, VERTSIZE),
                                                 settings.dmPosition.x,
                                                 settings.dmPosition.y);

            DeleteDC(dc);

            if (!monitors[found])
            {
                // TODO: wat
                return NULL;
            }

            monitors[found]->Win32.name = wcsdup(monitor.DeviceName);
            found++;
        }
    }

    *count = found;
    return monitors;
}


//========================================================================
// Destroy a monitor struct
//========================================================================

void _glfwPlatformDestroyMonitor(_GLFWmonitor* monitor)
{
    free(monitor->Win32.name);
}

