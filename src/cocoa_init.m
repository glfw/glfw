//========================================================================
// GLFW 3.1 OS X - www.glfw.org
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


#if defined(_GLFW_USE_CHDIR)

// Change to our application bundle's resources directory, if present
//
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

#endif /* _GLFW_USE_CHDIR */


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformInit(void)
{
    _glfw.ns.autoreleasePool = [[NSAutoreleasePool alloc] init];

#if defined(_GLFW_USE_CHDIR)
    changeToResourcesDirectory();
#endif

    _glfw.ns.eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    if (!_glfw.ns.eventSource)
        return GL_FALSE;

    CGEventSourceSetLocalEventsSuppressionInterval(_glfw.ns.eventSource, 0.0);

    if (!_glfwInitContextAPI())
        return GL_FALSE;

    _glfwInitTimer();
    _glfwInitJoysticks();

    return GL_TRUE;
}

void _glfwPlatformTerminate(void)
{
    if (_glfw.ns.eventSource)
    {
        CFRelease(_glfw.ns.eventSource);
        _glfw.ns.eventSource = NULL;
    }

    [NSApp setDelegate:nil];
    [_glfw.ns.delegate release];
    _glfw.ns.delegate = nil;

    [_glfw.ns.autoreleasePool release];
    _glfw.ns.autoreleasePool = nil;

    [_glfw.ns.cursor release];
    _glfw.ns.cursor = nil;

    free(_glfw.ns.clipboardString);

    _glfwTerminateJoysticks();
    _glfwTerminateContextAPI();
}

const char* _glfwPlatformGetVersionString(void)
{
    const char* version = _GLFW_VERSION_NUMBER " Cocoa"
#if defined(_GLFW_NSGL)
        " NSGL"
#endif
#if defined(_GLFW_USE_CHDIR)
        " chdir"
#endif
#if defined(_GLFW_USE_MENUBAR)
        " menubar"
#endif
#if defined(_GLFW_USE_RETINA)
        " retina"
#endif
#if defined(_GLFW_BUILD_DLL)
        " dynamic"
#endif
        ;

    return version;
}

