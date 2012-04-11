//========================================================================
// GLFW - An OpenGL library
// Platform:    Cocoa/NSOpenGL
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

#include <limits.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Set the clipboard contents
//========================================================================

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string)
{
    NSArray* types = [NSArray arrayWithObjects:NSStringPboardType, nil];

    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard declareTypes:types owner:nil];
    [pasteboard setString:[NSString stringWithUTF8String:string]
                  forType:NSStringPboardType];
}


//========================================================================
// Return the current clipboard contents
//========================================================================

size_t _glfwPlatformGetClipboardString(_GLFWwindow* window, char* string, size_t size)
{
    const char* source;
    size_t targetSize;
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];

    if (![[pasteboard types] containsObject:NSStringPboardType])
    {
        _glfwSetError(GLFW_FORMAT_UNAVAILABLE, NULL);
        return 0;
    }

    NSString* object = [pasteboard stringForType:NSStringPboardType];
    if (!object)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Cocoa/NSGL: Failed to retrieve object from pasteboard");
        return 0;
    }

    source = [object UTF8String];
    targetSize = strlen(source) + 1;
    if (targetSize > size)
        targetSize = size;

    strlcpy(string, source, targetSize);
    return 0;
}

