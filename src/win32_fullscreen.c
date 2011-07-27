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

int _glfwPlatformGetVideoModes(GLFWvidmode* list, int maxcount)
{
    int count, success, mode, i, j;
    int m1, m2, bpp, r, g, b;
    DEVMODE dm;

    // Loop through all video modes and extract all the UNIQUE modes
    count = 0;
    mode  = 0;

    do
    {
        // Get video mode properties
        dm.dmSize = sizeof(DEVMODE);
        success = EnumDisplaySettings(NULL, mode, &dm);

        // Is it a valid mode? (only list depths >= 15 bpp)
        if (success && dm.dmBitsPerPel >= 15)
        {
            // Convert to RGB, and back to bpp ("mask out" alpha bits etc)
            _glfwSplitBPP(dm.dmBitsPerPel, &r, &g, &b);
            bpp = r + g + b;

            // Mode "code" for this mode
            m1 = (bpp << 25) | (dm.dmPelsWidth * dm.dmPelsHeight);

            // Insert mode in list (sorted), and avoid duplicates
            for (i = 0;  i < count;  i++)
            {
                // Mode "code" for already listed mode
                bpp = list[i].redBits + list[i].greenBits + list[i].blueBits;
                m2 = (bpp << 25) | (list[i].width * list[i].height);
                if (m1 <= m2)
                    break;
            }

            // New entry at the end of the list?
            if (i >= count)
            {
                list[count].width     = dm.dmPelsWidth;
                list[count].height    = dm.dmPelsHeight;
                list[count].redBits   = r;
                list[count].greenBits = g;
                list[count].blueBits  = b;
                count ++;
            }
            // Insert new entry in the list?
            else if (m1 < m2)
            {
                for (j = count;  j > i;  j--)
                    list[j] = list[j - 1];

                list[i].width     = dm.dmPelsWidth;
                list[i].height    = dm.dmPelsHeight;
                list[i].redBits   = r;
                list[i].greenBits = g;
                list[i].blueBits  = b;
                count++;
            }
        }
        mode++;
    }
    while (success && (count < maxcount));

    return count;
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

