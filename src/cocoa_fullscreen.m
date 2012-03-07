//========================================================================
// GLFW - An OpenGL library
// Platform:    Cocoa/NSOpenGL
// API Version: 3.0
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2009-2010 Camilla Berglund <elmindreda@elmindreda.org>
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
// Check whether the display mode should be included in enumeration
//========================================================================

static GLboolean modeIsGood(CGDisplayModeRef mode)
{
    uint32_t flags = CGDisplayModeGetIOFlags(mode);
    if (!(flags & kDisplayModeValidFlag) || !(flags & kDisplayModeSafeFlag))
        return GL_FALSE;

    if (flags & kDisplayModeInterlacedFlag)
        return GL_FALSE;

    if (flags & kDisplayModeTelevisionFlag)
        return GL_FALSE;

    if (flags & kDisplayModeStretchedFlag)
        return GL_FALSE;

    CFStringRef format = CGDisplayModeCopyPixelEncoding(mode);
    if (CFStringCompare(format, CFSTR(IO16BitDirectPixels), 0) &&
        CFStringCompare(format, CFSTR(IO32BitDirectPixels), 0))
    {
        CFRelease(format);
        return GL_FALSE;
    }

    CFRelease(format);
    return GL_TRUE;
}


//========================================================================
// Convert Core Graphics display mode to GLFW video mode
//========================================================================

static GLFWvidmode vidmodeFromCGDisplayMode(CGDisplayModeRef mode)
{
    GLFWvidmode result;
    result.width = CGDisplayModeGetWidth(mode);
    result.height = CGDisplayModeGetHeight(mode);

    CFStringRef format = CGDisplayModeCopyPixelEncoding(mode);

    if (CFStringCompare(format, CFSTR(IO16BitDirectPixels), 0) == 0)
    {
        result.redBits = 5;
        result.greenBits = 5;
        result.blueBits = 5;
    }
    else
    {
        result.redBits = 8;
        result.greenBits = 8;
        result.blueBits = 8;
    }

    CFRelease(format);
    return result;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Change the current video mode
//========================================================================

GLboolean _glfwSetVideoMode(int* width, int* height, int* bpp, int* refreshRate)
{
    CGDisplayModeRef bestMode = NULL;
    CFArrayRef modes;
    CFIndex count, i;
    unsigned int leastSizeDiff = UINT_MAX;
    double leastRateDiff = DBL_MAX;

    modes = CGDisplayCopyAllDisplayModes(CGMainDisplayID(), NULL);
    count = CFArrayGetCount(modes);

    for (i = 0;  i < count;  i++)
    {
        CGDisplayModeRef mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
        if (!modeIsGood(mode))
            continue;

        int modeBPP;

        // Identify display mode pixel encoding
        {
            CFStringRef format = CGDisplayModeCopyPixelEncoding(mode);

            if (CFStringCompare(format, CFSTR(IO16BitDirectPixels), 0) == 0)
                modeBPP = 16;
            else
                modeBPP = 32;

            CFRelease(format);
        }

        int modeWidth = (int) CGDisplayModeGetWidth(mode);
        int modeHeight = (int) CGDisplayModeGetHeight(mode);

        unsigned int sizeDiff = (abs(modeBPP - *bpp) << 25) |
                                ((modeWidth - *width) * (modeWidth - *width) +
                                 (modeHeight - *height) * (modeHeight - *height));

        double rateDiff;

        if (*refreshRate > 0)
            rateDiff = fabs(CGDisplayModeGetRefreshRate(mode) - *refreshRate);
        else
        {
            // If no refresh rate was specified, then they're all the same
            rateDiff = 0;
        }

        if ((sizeDiff < leastSizeDiff) ||
            (sizeDiff == leastSizeDiff && (rateDiff < leastRateDiff)))
        {
            bestMode = mode;

            leastSizeDiff = sizeDiff;
            leastRateDiff = rateDiff;
        }
    }

    if (!bestMode)
    {
        CFRelease(modes);
        return GL_FALSE;
    }

    CGDisplayCapture(CGMainDisplayID());
    CGDisplaySetDisplayMode(CGMainDisplayID(), bestMode, NULL);

    CFRelease(modes);
    return GL_TRUE;
}


//========================================================================
// Restore the previously saved (original) video mode
//========================================================================

void _glfwRestoreVideoMode(void)
{
    CGDisplaySetDisplayMode(CGMainDisplayID(),
                            _glfwLibrary.NS.desktopMode,
                            NULL);

    CGDisplayRelease(CGMainDisplayID());
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Get a list of available video modes
//========================================================================

int _glfwPlatformGetVideoModes(GLFWvidmode* list, int maxcount)
{
    CGDisplayModeRef mode;
    CFArrayRef modes;
    CFIndex count, i;
    int stored = 0;

    modes = CGDisplayCopyAllDisplayModes(CGMainDisplayID(), NULL);
    count = CFArrayGetCount(modes);

    for (i = 0;  i < count && stored < maxcount;  i++)
    {
        mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
        if (modeIsGood(mode))
            list[stored++] = vidmodeFromCGDisplayMode(mode);
    }

    CFRelease(modes);
    return stored;
}


//========================================================================
// Get the desktop video mode
//========================================================================

void _glfwPlatformGetDesktopMode(GLFWvidmode *mode)
{
    *mode = vidmodeFromCGDisplayMode(_glfwLibrary.NS.desktopMode);
}

