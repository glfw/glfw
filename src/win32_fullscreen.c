//========================================================================
// GLFW - An OpenGL library
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

#include <stdlib.h>
#include <limits.h>
#include <malloc.h>


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
// Get a list of available video modes
//========================================================================

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
    int deviceModeIndex = 0, count = 0;
    GLFWvidmode* result = NULL;
    WCHAR* deviceName;

    deviceName = _glfwCreateWideStringFromUTF8(monitor->Win32.name);
    if (!deviceName)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR, "Win32: Failed to convert device name");
        return NULL;
    }

    *found = 0;

    for (;;)
    {
        int i;
        GLFWvidmode mode;
        DEVMODE dm;

        ZeroMemory(&dm, sizeof(DEVMODE));
        dm.dmSize = sizeof(DEVMODE);

        if (!EnumDisplaySettings(deviceName, deviceModeIndex, &dm))
            break;

        deviceModeIndex++;

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

    free(deviceName);
    return result;
}


//========================================================================
// Get the desktop video mode
//========================================================================

void _glfwPlatformGetDesktopMode(GLFWvidmode* mode)
{
    DEVMODE dm;

    // Get desktop display mode
    dm.dmSize = sizeof(DEVMODE);
    EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &dm);

    // Return desktop mode parameters
    mode->width  = dm.dmPelsWidth;
    mode->height = dm.dmPelsHeight;
    _glfwSplitBPP(dm.dmBitsPerPel,
                  &mode->redBits,
                  &mode->greenBits,
                  &mode->blueBits);
}

