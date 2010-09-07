//========================================================================
// GLFW - An OpenGL framework
// Platform:    Cocoa/NSOpenGL
// API Version: 2.7
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

//========================================================================
// Check whether the display mode should be included in enumeration
//========================================================================

static BOOL modeIsGood( NSDictionary *mode )
{
    // This is a bit controversial, if you've got something other than an
    // LCD computer monitor as an output device you might not want these
    // checks.  You might also want to reject modes which are interlaced,
    // or TV out.  There is no one-size-fits-all policy that can work here.
    // This seems like a decent compromise, but certain applications may
    // wish to patch this...
    return [[mode objectForKey:(id)kCGDisplayBitsPerPixel] intValue] >= 15 &&
           [mode objectForKey:(id)kCGDisplayModeIsSafeForHardware] != nil &&
           [mode objectForKey:(id)kCGDisplayModeIsStretched] == nil;
}

//========================================================================
// Convert Core Graphics display mode to GLFW video mode
//========================================================================

static GLFWvidmode vidmodeFromCGDisplayMode( NSDictionary *mode )
{
    unsigned int width = [[mode objectForKey:(id)kCGDisplayWidth] unsignedIntValue];
    unsigned int height = [[mode objectForKey:(id)kCGDisplayHeight] unsignedIntValue];
    unsigned int bps = [[mode objectForKey:(id)kCGDisplayBitsPerSample] unsignedIntValue];

    GLFWvidmode result;
    result.Width = width;
    result.Height = height;
    result.RedBits = bps;
    result.GreenBits = bps;
    result.BlueBits = bps;
    return result;
}


//************************************************************************
//****               Platform implementation functions                ****
//************************************************************************

//========================================================================
// Get a list of available video modes
//========================================================================

int _glfwPlatformGetVideoModes( GLFWvidmode *list, int maxcount )
{
    NSArray *modes = (NSArray *)CGDisplayAvailableModes( CGMainDisplayID() );

    unsigned int i, j = 0, n = [modes count];
    for( i = 0; i < n && i < (unsigned)maxcount; i++ )
    {
        NSDictionary *mode = [modes objectAtIndex:i];
        if( modeIsGood( mode ) )
        {
            list[j++] = vidmodeFromCGDisplayMode( mode );
        }
    }

    return j;
}

//========================================================================
// Get the desktop video mode
//========================================================================

void _glfwPlatformGetDesktopMode( GLFWvidmode *mode )
{
    *mode = vidmodeFromCGDisplayMode( _glfwLibrary.DesktopMode );
}

