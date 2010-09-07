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

//************************************************************************
//****               Platform implementation functions                ****
//************************************************************************

//========================================================================
// Check if an OpenGL extension is available at runtime
//========================================================================

int _glfwPlatformExtensionSupported( const char *extension )
{
    // There are no AGL, CGL or NSGL extensions.
    return GL_FALSE;
}

//========================================================================
// Get the function pointer to an OpenGL function
//========================================================================

void * _glfwPlatformGetProcAddress( const char *procname )
{
    CFStringRef symbolName = CFStringCreateWithCString( kCFAllocatorDefault,
                                                        procname,
                                                        kCFStringEncodingASCII );

    void *symbol = CFBundleGetFunctionPointerForName( _glfwLibrary.OpenGLFramework,
                                                      symbolName );

    CFRelease( symbolName );

    return symbol;
}

