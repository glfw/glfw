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
#include <sys/param.h> // For MAXPATHLEN

//========================================================================
// Change to our application bundle's resources directory, if present
//========================================================================

static void changeToResourcesDirectory(void)
{
    char resourcesPath[MAXPATHLEN];

    CFBundleRef bundle = CFBundleGetMainBundle();
    if (!bundle)
        return;

    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);

    CFStringRef last = CFURLCopyLastPathComponent(resourcesURL);
    if (CFStringCompare(CFSTR("Resources"), last, 0) != kCFCompareEqualTo)
    {
        CFRelease(last);
        CFRelease(resourcesURL);
        return;
    }

    CFRelease(last);

    if (!CFURLGetFileSystemRepresentation(resourcesURL,
                                          true,
                                          (UInt8*) resourcesPath,
                                          MAXPATHLEN))
    {
        CFRelease(resourcesURL);
        return;
    }

    CFRelease(resourcesURL);

    chdir(resourcesPath);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Initialize the GLFW library
//========================================================================

int _glfwPlatformInit(void)
{
    _glfwLibrary.NS.autoreleasePool = [[NSAutoreleasePool alloc] init];

    _glfwLibrary.NSGL.framework =
        CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
    if (_glfwLibrary.NSGL.framework == NULL)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "glfwInit: Failed to locate OpenGL framework");
        return GL_FALSE;
    }

    changeToResourcesDirectory();

    _glfwLibrary.NS.desktopMode = CGDisplayCopyDisplayMode(CGMainDisplayID());

    // Save the original gamma ramp
    _glfwLibrary.originalRampSize = CGDisplayGammaTableCapacity(CGMainDisplayID());
    _glfwPlatformGetGammaRamp(&_glfwLibrary.originalRamp);
    _glfwLibrary.currentRamp = _glfwLibrary.originalRamp;

    _glfwInitTimer();

    _glfwInitJoysticks();

    _glfwLibrary.NS.eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    if (!_glfwLibrary.NS.eventSource)
        return GL_FALSE;

    CGEventSourceSetLocalEventsSuppressionInterval(_glfwLibrary.NS.eventSource,
                                                   0.0);

    return GL_TRUE;
}


//========================================================================
// Close window, if open, and shut down GLFW
//========================================================================

int _glfwPlatformTerminate(void)
{
    // TODO: Probably other cleanup

    if (_glfwLibrary.NS.eventSource)
    {
        CFRelease(_glfwLibrary.NS.eventSource);
        _glfwLibrary.NS.eventSource = NULL;
    }

    // Restore the original gamma ramp
    if (_glfwLibrary.rampChanged)
        _glfwPlatformSetGammaRamp(&_glfwLibrary.originalRamp);

    CGDisplayModeRelease(_glfwLibrary.NS.desktopMode);

    [NSApp setDelegate:nil];
    [_glfwLibrary.NS.delegate release];
    _glfwLibrary.NS.delegate = nil;

    [_glfwLibrary.NS.autoreleasePool release];
    _glfwLibrary.NS.autoreleasePool = nil;

    _glfwTerminateJoysticks();

    return GL_TRUE;
}


//========================================================================
// Get the GLFW version string
//========================================================================

const char* _glfwPlatformGetVersionString(void)
{
    const char* version = _GLFW_VERSION_FULL
#if defined(_GLFW_BUILD_DLL)
        " dynamic"
#endif
        ;

    return version;
}

