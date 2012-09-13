//========================================================================
// GLFW - An OpenGL library
// Platform:    Cocoa
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

#include <IOKit/graphics/IOGraphicsLib.h>


//========================================================================
// Get the name of the specified display
//========================================================================

const char* getDisplayName(CGDirectDisplayID displayID)
{
    char* name;
    CFDictionaryRef info, names;
    CFStringRef value;
    CFIndex size;

    info = IODisplayCreateInfoDictionary(CGDisplayIOServicePort(displayID),
                                         kIODisplayOnlyPreferredName);
    names = CFDictionaryGetValue(info, CFSTR(kDisplayProductName));

    if (!CFDictionaryGetValueIfPresent(names, CFSTR("en_US"),
                                       (const void**) &value))
    {
        CFRelease(info);
        return strdup("Unknown");
    }

    size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(value),
                                             kCFStringEncodingUTF8);
    name = (char*) malloc(size + 1);
    CFStringGetCString(value, name, size, kCFStringEncodingUTF8);

    CFRelease(info);

    return name;
}


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

    _glfwLibrary.NS.previousMode = CGDisplayCopyDisplayMode(CGMainDisplayID());

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
                            _glfwLibrary.NS.previousMode,
                            NULL);

    CGDisplayRelease(CGMainDisplayID());
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Return a list of available monitors
//========================================================================

_GLFWmonitor** _glfwPlatformGetMonitors(int* count)
{
    uint32_t i, found = 0, monitorCount;
    _GLFWmonitor** monitors;
    CGDirectDisplayID* displays;

    *count = 0;

    CGGetActiveDisplayList(0, NULL, &monitorCount);

    displays = (CGDirectDisplayID*) calloc(monitorCount, sizeof(CGDirectDisplayID));
    if (!displays)
    {
        _glfwSetError(GLFW_OUT_OF_MEMORY, NULL);
        return NULL;
    }

    monitors = (_GLFWmonitor**) calloc(monitorCount, sizeof(_GLFWmonitor*));
    if (!monitors)
    {
        _glfwSetError(GLFW_OUT_OF_MEMORY, NULL);
        return NULL;
    }

    for (i = 0;  i < monitorCount;  i++)
    {
        const CGSize size = CGDisplayScreenSize(displays[i]);
        const CGRect bounds = CGDisplayBounds(displays[i]);
        const char* name = getDisplayName(displays[i]);

        monitors[found] = _glfwCreateMonitor(name,
                                             size.width, size.height,
                                             bounds.origin.x, bounds.origin.y);

        monitors[found]->NS.displayID = displays[i];
        found++;
    }

    free(displays);

    *count = monitorCount;
    return monitors;
}


//========================================================================
// Destroy a monitor struct
//========================================================================

void _glfwPlatformDestroyMonitor(_GLFWmonitor* monitor)
{
}


//========================================================================
// Get a list of available video modes
//========================================================================

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
    CFArrayRef modes;
    CFIndex count, i;
    GLFWvidmode* result;

    modes = CGDisplayCopyAllDisplayModes(monitor->NS.displayID, NULL);
    count = CFArrayGetCount(modes);

    result = (GLFWvidmode*) malloc(sizeof(GLFWvidmode) * count);
    *found = 0;

    for (i = 0;  i < count;  i++)
    {
        CGDisplayModeRef mode;

        mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
        if (modeIsGood(mode))
        {
            result[*found] = vidmodeFromCGDisplayMode(mode);
            (*found)++;
        }
    }

    CFRelease(modes);
    return result;
}


//========================================================================
// Get the current video mode for the specified monitor
//========================================================================

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode *mode)
{
    CGDisplayModeRef displayMode;

    displayMode = CGDisplayCopyDisplayMode(monitor->NS.displayID);
    *mode = vidmodeFromCGDisplayMode(displayMode);
    CGDisplayModeRelease(displayMode);
}

