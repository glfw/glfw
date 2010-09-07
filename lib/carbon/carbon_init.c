//========================================================================
// GLFW - An OpenGL framework
// Platform:    Carbon/AGL/CGL
// API Version: 3.0
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2003      Keith Bauer
// Copyright (c) 2003-2010 Camilla Berglund <elmindreda@elmindreda.org>
// Copyright (c) 2006-2007 Robin Leffmann
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

#include <unistd.h>

//========================================================================
// Global variables
//========================================================================

// KCHR resource pointer for keycode translation
void *KCHRPtr;


//========================================================================
// Terminate GLFW when exiting application
//========================================================================

static void glfw_atexit( void )
{
    glfwTerminate();
}


#define NO_BUNDLE_MESSAGE \
    "Working in unbundled mode.  " \
    "You should build a .app wrapper for your Mac OS X applications.\n"

#define UNBUNDLED \
    fprintf(stderr, NO_BUNDLE_MESSAGE); \
    _glfwLibrary.Unbundled = 1; \
    return

void _glfwChangeToResourcesDirectory( void )
{
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if( mainBundle == NULL )
    {
        UNBUNDLED;
    }

    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL( mainBundle );
    char resourcesPath[ _GLFW_MAX_PATH_LENGTH ];

    CFStringRef lastComponent = CFURLCopyLastPathComponent( resourcesURL );
    if( kCFCompareEqualTo != CFStringCompare(
            CFSTR( "Resources" ),
            lastComponent,
            0 ) )
    {
        UNBUNDLED;
    }

    CFRelease( lastComponent );

    if( !CFURLGetFileSystemRepresentation( resourcesURL,
                                           TRUE,
                                           (UInt8*)resourcesPath,
                                           _GLFW_MAX_PATH_LENGTH ) )
    {
        CFRelease( resourcesURL );
        UNBUNDLED;
    }

    CFRelease( resourcesURL );

    if( chdir( resourcesPath ) != 0 )
    {
        UNBUNDLED;
    }
}

int _glfwPlatformInit( void )
{
    struct timeval tv;
    UInt32 nullDummy = 0;

    _glfwWin.window = NULL;
    _glfwWin.aglContext = NULL;
    _glfwWin.cglContext = NULL;
    _glfwWin.windowUPP = NULL;

    _glfwInput.Modifiers = 0;

    _glfwLibrary.Unbundled = 0;

    _glfwLibrary.Libs.OpenGLFramework =
        CFBundleGetBundleWithIdentifier( CFSTR( "com.apple.opengl" ) );
    if( _glfwLibrary.Libs.OpenGLFramework == NULL )
    {
        fprintf( stderr, "glfwInit failing because you aren't linked to OpenGL\n" );
        return GL_FALSE;
    }

    _glfwDesktopVideoMode = CGDisplayCurrentMode( kCGDirectMainDisplay );
    if( _glfwDesktopVideoMode == NULL )
    {
        fprintf( stderr, "glfwInit failing because it kind find the desktop display mode\n" );
        return GL_FALSE;
    }

    // Install atexit routine
    atexit( glfw_atexit );

    _glfwChangeToResourcesDirectory();

    // Ugly hack to reduce the nasty jump that occurs at the first non-
    // sys keypress, caused by OS X loading certain meta scripts used
    // for lexical- and raw keycode translation - instead of letting
    // this happen while our application is running, we do some blunt
    // function calls in advance just to get the script caching out of
    // the way BEFORE our window/screen is opened. These calls might
    // generate err return codes, but we don't care in this case.
    // NOTE: KCHRPtr is declared globally, because we need it later on.
    KCHRPtr = (void *)GetScriptVariable( smCurrentScript, smKCHRCache );
    KeyTranslate( KCHRPtr, 0, &nullDummy );
    UppercaseText( (char *)&nullDummy, 0, smSystemScript );

    gettimeofday( &tv, NULL );
    _glfwLibrary.Timer.t0 = tv.tv_sec + (double) tv.tv_usec / 1000000.0;

    return GL_TRUE;
}

int _glfwPlatformTerminate( void )
{
    return GL_TRUE;
}

