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
#include <limits.h>
#include <malloc.h>

// These constants are missing on MinGW
#ifndef EDS_ROTATEDMODE
 #define EDS_ROTATEDMODE 0x00000004
#endif
#ifndef DISPLAY_DEVICE_ACTIVE
 #define DISPLAY_DEVICE_ACTIVE 0x00000001
#endif


//========================================================================
// Return closest video mode by dimensions, refresh rate and bits per pixel
//========================================================================

static GLboolean getClosestVideoMode(int* width, int* height,
                                     int* bpp, int* refreshRate,
                                     GLboolean exactBPP)
{
    int mode, bestWidth, bestHeight, bestBPP, bestRate;
    unsigned int sizeDiff, rateDiff, leastSizeDiff, leastRateDiff;
    GLboolean foundMode = GL_FALSE;
    DEVMODE dm;

    leastSizeDiff = leastRateDiff = UINT_MAX;

    for (mode = 0;  ;  mode++)
    {
        dm.dmSize = sizeof(DEVMODE);
        if (!EnumDisplaySettings(NULL, mode, &dm))
            break;

        if (exactBPP && dm.dmBitsPerPel != *bpp)
            continue;

        sizeDiff = (abs(dm.dmBitsPerPel - *bpp) << 25) |
                   ((dm.dmPelsWidth - *width) *
                    (dm.dmPelsWidth - *width) +
                    (dm.dmPelsHeight - *height) *
                    (dm.dmPelsHeight - *height));

        if (*refreshRate > 0)
        {
            rateDiff = (dm.dmDisplayFrequency - *refreshRate) *
                       (dm.dmDisplayFrequency - *refreshRate);
        }
        else
        {
            // If no refresh rate was specified, then they're all the same
            rateDiff = 0;
        }

        // We match first BPP, then screen area and last refresh rate

        if ((sizeDiff < leastSizeDiff) ||
            (sizeDiff == leastSizeDiff && (rateDiff < leastRateDiff)))
        {
            bestWidth  = dm.dmPelsWidth;
            bestHeight = dm.dmPelsHeight;
            bestBPP    = dm.dmBitsPerPel;
            bestRate   = dm.dmDisplayFrequency;

            leastSizeDiff = sizeDiff;
            leastRateDiff = rateDiff;

            foundMode = GL_TRUE;
        }
    }

    if (!foundMode)
        return GL_FALSE;

    *width  = bestWidth;
    *height = bestHeight;
    *bpp    = bestBPP;

    // Only save the found refresh rate if the client requested a specific
    // rate; otherwise keep it at zero to let Windows select the best rate
    if (*refreshRate > 0)
        *refreshRate = bestRate;

    return GL_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Change the current video mode
//========================================================================

void _glfwSetVideoMode(int* width, int* height,
                       int* bpp, int* refreshRate,
                       GLboolean exactBPP)
{
    DEVMODE dm;
    int closestWidth, closestHeight, closestBPP, closestRate;

    closestWidth  = *width;
    closestHeight = *height;
    closestBPP    = *bpp;
    closestRate   = *refreshRate;

    if (getClosestVideoMode(&closestWidth, &closestHeight,
                            &closestBPP, &closestRate, GL_FALSE))
    {
        dm.dmSize = sizeof(DEVMODE);
        dm.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
        dm.dmPelsWidth  = closestWidth;
        dm.dmPelsHeight = closestHeight;
        dm.dmBitsPerPel = closestBPP;

        if (*refreshRate > 0)
        {
            dm.dmFields |= DM_DISPLAYFREQUENCY;
            dm.dmDisplayFrequency = closestRate;
        }

        if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
        {
            *width       = closestWidth;
            *height      = closestHeight;
            *bpp         = closestBPP;
            *refreshRate = closestRate;
        }
    }
    else
    {
        dm.dmSize = sizeof(DEVMODE);
        EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &dm);

        *width       = dm.dmPelsWidth;
        *height      = dm.dmPelsHeight;
        *bpp         = dm.dmBitsPerPel;
        *refreshRate = dm.dmDisplayFrequency;
    }
}


//========================================================================
// Restore the previously saved (original) video mode
//========================================================================

void _glfwRestoreVideoMode(void)
{
    ChangeDisplaySettings(NULL, CDS_FULLSCREEN);
}


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

        DISPLAY_DEVICE adapter, monitor;
        DEVMODE settings;
        const char* name;
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

        ZeroMemory(&settings, sizeof(DEVMODE));
        settings.dmSize = sizeof(DEVMODE);

        EnumDisplaySettingsEx(adapter.DeviceName,
                              ENUM_CURRENT_SETTINGS,
                              &settings,
                              EDS_ROTATEDMODE);

        name = _glfwCreateUTF8FromWideString(adapter.DeviceName);
        if (!name)
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
            if (!monitors)
            {
                // TODO: wat
                return NULL;
            }
        }

        ZeroMemory(&monitor, sizeof(DISPLAY_DEVICE));
        monitor.cb = sizeof(DISPLAY_DEVICE);

        EnumDisplayDevices(adapter.DeviceName, 0, &monitor, 0);
        dc = CreateDC(L"DISPLAY", monitor.DeviceString, NULL, NULL);

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

        monitors[found]->Win32.name = wcsdup(adapter.DeviceName);
        found++;
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


//========================================================================
// Get a list of available video modes
//========================================================================

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

        if (!EnumDisplaySettings(monitor->Win32.name, modeIndex, &dm))
            break;

        modeIndex++;

        if (dm.dmBitsPerPel < 15)
        {
            // Skip modes with less than 15 BPP
            continue;
        }

        mode.width = dm.dmPelsWidth;
        mode.height = dm.dmPelsHeight;
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
            void* larger;

            if (count)
                count *= 2;
            else
                count = 128;

            larger = realloc(result, count * sizeof(GLFWvidmode));
            if (!larger)
            {
                free(result);

                _glfwSetError(GLFW_OUT_OF_MEMORY, NULL);
                return NULL;
            }

            result = (GLFWvidmode*) larger;
        }

        result[*found] = mode;
        (*found)++;
    }

    return result;
}


//========================================================================
// Get the current video mode for the specified monitor
//========================================================================

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
    DEVMODE dm;

    ZeroMemory(&dm, sizeof(DEVMODE));
    dm.dmSize = sizeof(DEVMODE);

    EnumDisplaySettings(monitor->Win32.name, ENUM_REGISTRY_SETTINGS, &dm);

    mode->width  = dm.dmPelsWidth;
    mode->height = dm.dmPelsHeight;
    _glfwSplitBPP(dm.dmBitsPerPel,
                  &mode->redBits,
                  &mode->greenBits,
                  &mode->blueBits);
}

