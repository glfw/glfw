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

// Needed for _NSGetProgname
#include <crt_externs.h>

#include "internal.h"


//========================================================================
// GLFW application class
//========================================================================

@interface GLFWApplication : NSApplication
@end

@implementation GLFWApplication

// From http://cocoadev.com/index.pl?GameKeyboardHandlingAlmost
// This works around an AppKit bug, where key up events while holding
// down the command key don't get sent to the key window.
- (void)sendEvent:(NSEvent *)event
{
    if ([event type] == NSKeyUp && ([event modifierFlags] & NSCommandKeyMask))
        [[self keyWindow] sendEvent:event];
    else
        [super sendEvent:event];
}

@end


// Prior to Snow Leopard, we need to use this oddly-named semi-private API
// to get the application menu working properly.  Need to be careful in
// case it goes away in a future OS update.
@interface NSApplication (NSAppleMenu)
- (void)setAppleMenu:(NSMenu*)m;
@end

// Keys to search for as potential application names
NSString* GLFWNameKeys[] =
{
    @"CFBundleDisplayName",
    @"CFBundleName",
    @"CFBundleExecutable",
};


//========================================================================
// Try to figure out what the calling application is called
//========================================================================
static NSString* findAppName(void)
{
    unsigned int i;
    NSDictionary* infoDictionary = [[NSBundle mainBundle] infoDictionary];

    for (i = 0;  i < sizeof(GLFWNameKeys) / sizeof(GLFWNameKeys[0]);  i++)
    {
        id name = [infoDictionary objectForKey:GLFWNameKeys[i]];
        if (name &&
            [name isKindOfClass:[NSString class]] &&
            ![@"" isEqualToString:name])
        {
            return name;
        }
    }

    // If we get here, we're unbundled
    if (!_glfwLibrary.NS.unbundled)
    {
        // Could do this only if we discover we're unbundled, but it should
        // do no harm...
        ProcessSerialNumber psn = { 0, kCurrentProcess };
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);

        // Having the app in front of the terminal window is also generally
        // handy.  There is an NSApplication API to do this, but...
        SetFrontProcess(&psn);

        _glfwLibrary.NS.unbundled = GL_TRUE;
    }

    char** progname = _NSGetProgname();
    if (progname && *progname)
    {
        // TODO: UTF-8?
        return [NSString stringWithUTF8String:*progname];
    }

    // Really shouldn't get here
    return @"GLFW Application";
}

//========================================================================
// Set up the menu bar (manually)
// This is nasty, nasty stuff -- calls to undocumented semi-private APIs that
// could go away at any moment, lots of stuff that really should be
// localize(d|able), etc.  Loading a nib would save us this horror, but that
// doesn't seem like a good thing to require of GLFW's clients.
//========================================================================
static void setUpMenuBar(void)
{
    NSString* appName = findAppName();

    NSMenu* bar = [[NSMenu alloc] init];
    [NSApp setMainMenu:bar];

    NSMenuItem* appMenuItem =
        [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    NSMenu* appMenu = [[NSMenu alloc] init];
    [appMenuItem setSubmenu:appMenu];

    [appMenu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName]
                       action:@selector(orderFrontStandardAboutPanel:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    NSMenu* servicesMenu = [[NSMenu alloc] init];
    [NSApp setServicesMenu:servicesMenu];
    [[appMenu addItemWithTitle:@"Services"
                       action:NULL
                keyEquivalent:@""] setSubmenu:servicesMenu];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Hide %@", appName]
                       action:@selector(hide:)
                keyEquivalent:@"h"];
    [[appMenu addItemWithTitle:@"Hide Others"
                       action:@selector(hideOtherApplications:)
                keyEquivalent:@"h"]
        setKeyEquivalentModifierMask:NSAlternateKeyMask | NSCommandKeyMask];
    [appMenu addItemWithTitle:@"Show All"
                       action:@selector(unhideAllApplications:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName]
                       action:@selector(terminate:)
                keyEquivalent:@"q"];

    NSMenuItem* windowMenuItem =
        [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    NSMenu* windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    [NSApp setWindowsMenu:windowMenu];
    [windowMenuItem setSubmenu:windowMenu];

    [windowMenu addItemWithTitle:@"Miniaturize"
                          action:@selector(performMiniaturize:)
                   keyEquivalent:@"m"];
    [windowMenu addItemWithTitle:@"Zoom"
                          action:@selector(performZoom:)
                   keyEquivalent:@""];
    [windowMenu addItem:[NSMenuItem separatorItem]];
    [windowMenu addItemWithTitle:@"Bring All to Front"
                          action:@selector(arrangeInFront:)
                   keyEquivalent:@""];

    // At least guard the call to private API to avoid an exception if it
    // goes away.  Hopefully that means the worst we'll break in future is to
    // look ugly...
    if ([NSApp respondsToSelector:@selector(setAppleMenu:)])
        [NSApp setAppleMenu:appMenu];
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

    // Implicitly create shared NSApplication instance
    [GLFWApplication sharedApplication];

    _glfwLibrary.NS.OpenGLFramework =
        CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
    if (_glfwLibrary.NS.OpenGLFramework == NULL)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "glfwInit: Failed to locate OpenGL framework");
        return GL_FALSE;
    }

    NSString* resourcePath = [[NSBundle mainBundle] resourcePath];

    if (access([resourcePath cStringUsingEncoding:NSUTF8StringEncoding], R_OK) == 0)
        chdir([resourcePath cStringUsingEncoding:NSUTF8StringEncoding]);

    // Setting up menu bar must go exactly here else weirdness ensues
    setUpMenuBar();

    [NSApp finishLaunching];

    _glfwPlatformSetTime(0.0);

    _glfwLibrary.NS.desktopMode =
	    (NSDictionary*) CGDisplayCurrentMode(CGMainDisplayID());

    // Save the original gamma ramp
    _glfwLibrary.originalRampSize = CGDisplayGammaTableCapacity(CGMainDisplayID());
    _glfwPlatformGetGammaRamp(&_glfwLibrary.originalRamp);
    _glfwLibrary.currentRamp = _glfwLibrary.originalRamp;

    return GL_TRUE;
}

//========================================================================
// Close window, if open, and shut down GLFW
//========================================================================

int _glfwPlatformTerminate(void)
{
    // TODO: Probably other cleanup

    // Restore the original gamma ramp
    _glfwPlatformSetGammaRamp(&_glfwLibrary.originalRamp);

    [NSApp setDelegate:nil];
    [_glfwLibrary.NS.delegate release];
    _glfwLibrary.NS.delegate = nil;

    [_glfwLibrary.NS.autoreleasePool release];
    _glfwLibrary.NS.autoreleasePool = nil;

    return GL_TRUE;
}


//========================================================================
// Get the GLFW version string
//========================================================================

const char* _glfwPlatformGetVersionString(void)
{
    const char* version = _GLFW_VERSION_FULL " Cocoa";

    return version;
}

