//========================================================================
// GLFW - An OpenGL framework
// Platform:    Carbon/AGL/CGL
// API Version: 2.7
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2003      Keith Bauer
// Copyright (c) 2003-2010 Camilla Berglund <elmindreda@elmindreda.org>
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
// _glfwVideoModesEqual() - Compares two video modes
//========================================================================

static int _glfwVideoModesEqual( GLFWvidmode* first,
                                 GLFWvidmode* second )
{
    if( first->Width != second->Width )
	return 0;
		
    if( first->Height != second->Height )
	return 0;
		
    if( first->RedBits + first->GreenBits + first->BlueBits !=
      second->RedBits + second->GreenBits + second->BlueBits )
	return 0;
	
    return 1;
}
                            
//========================================================================
// _glfwCGToGLFWVideoMode() - Converts a CG mode to a GLFW mode
//========================================================================

static void _glfwCGToGLFWVideoMode( CFDictionaryRef cgMode,
                                    GLFWvidmode* glfwMode )
{
    int bitsPerSample;

    CFNumberGetValue( CFDictionaryGetValue( cgMode, kCGDisplayWidth ),
                     kCFNumberIntType,
                     &(glfwMode->Width) );
    CFNumberGetValue( CFDictionaryGetValue( cgMode, kCGDisplayHeight ),
                     kCFNumberIntType,
                     &(glfwMode->Height) );

    CFNumberGetValue( CFDictionaryGetValue( cgMode, kCGDisplayBitsPerSample ),
                     kCFNumberIntType,
                     &bitsPerSample );

    glfwMode->RedBits = bitsPerSample;
    glfwMode->GreenBits = bitsPerSample;
    glfwMode->BlueBits = bitsPerSample;
}

//========================================================================
// _glfwPlatformGetVideoModes() - Get a list of available video modes
//========================================================================

int _glfwPlatformGetVideoModes( GLFWvidmode *list, int maxcount )
{
    int i, j, maxModes, numModes;
    GLFWvidmode mode;
    CFArrayRef availableModes = CGDisplayAvailableModes( kCGDirectMainDisplay );
    CFIndex numberOfAvailableModes = CFArrayGetCount( availableModes );

    numModes = 0;
    maxModes = ( numberOfAvailableModes < maxcount ?
                 numberOfAvailableModes :
                 maxcount );

    for( i = 0; i < maxModes; ++i )
    {
        _glfwCGToGLFWVideoMode( CFArrayGetValueAtIndex( availableModes, i ),
                                &mode );

        // Is it a valid mode? (only list depths >= 15 bpp)
	if( mode.RedBits + mode.GreenBits + mode.BlueBits < 15 )
	    continue;
			
        // Check for duplicate of current mode in target list
      	for( j = 0; j < numModes; ++j )
      	{
      	    if( _glfwVideoModesEqual( &mode, &(list[j]) ) )
      		break;
      	}
      	
      	// If empty list or no match found
      	if( numModes == 0 || j == numModes )
      	    list[numModes++] = mode;
    }

    return numModes;
}

//========================================================================
// glfwGetDesktopMode() - Get the desktop video mode
//========================================================================

void _glfwPlatformGetDesktopMode( GLFWvidmode *mode )
{
    _glfwCGToGLFWVideoMode( _glfwDesktopVideoMode, mode );
}

