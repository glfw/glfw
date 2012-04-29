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

// Needed for _NSGetProgname
#include <crt_externs.h>


//========================================================================
// Delegate for window related notifications
//========================================================================

@interface GLFWWindowDelegate : NSObject
{
    _GLFWwindow* window;
}

- (id)initWithGlfwWindow:(_GLFWwindow *)initWndow;

@end

@implementation GLFWWindowDelegate

- (id)initWithGlfwWindow:(_GLFWwindow *)initWindow
{
    self = [super init];
    if (self != nil)
        window = initWindow;

    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    window->closeRequested = GL_TRUE;

    return NO;
}

- (void)windowDidResize:(NSNotification *)notification
{
    [window->NSGL.context update];

    NSRect contentRect =
        [window->NS.object contentRectForFrameRect:[window->NS.object frame]];

    _glfwInputWindowSize(window, contentRect.size.width, contentRect.size.height);
}

- (void)windowDidMove:(NSNotification *)notification
{
    [window->NSGL.context update];

    NSRect contentRect =
        [window->NS.object contentRectForFrameRect:[window->NS.object frame]];

    CGPoint mainScreenOrigin = CGDisplayBounds(CGMainDisplayID()).origin;
    double mainScreenHeight = CGDisplayBounds(CGMainDisplayID()).size.height;
    CGPoint flippedPos = CGPointMake(contentRect.origin.x - mainScreenOrigin.x,
                                      mainScreenHeight - contentRect.origin.y -
                                          mainScreenOrigin.y - window->height);

    _glfwInputWindowPos(window, flippedPos.x, flippedPos.y);
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    _glfwInputWindowIconify(window, GL_TRUE);
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    _glfwInputWindowIconify(window, GL_FALSE);
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    _glfwInputWindowFocus(window, GL_TRUE);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    _glfwInputWindowFocus(window, GL_FALSE);
}

@end


//========================================================================
// Delegate for application related notifications
//========================================================================

@interface GLFWApplicationDelegate : NSObject
@end

@implementation GLFWApplicationDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    _GLFWwindow* window;

    for (window = _glfwLibrary.windowListHead;  window;  window = window->next)
        window->closeRequested = GL_TRUE;

    return NSTerminateCancel;
}

@end


//========================================================================
// Converts a Mac OS X keycode to a GLFW keycode
//========================================================================

static int convertMacKeyCode(unsigned int macKeyCode)
{
    // Keyboard symbol translation table
    // TODO: Need to find mappings for F13-F15, volume down/up/mute, and eject.
    static const unsigned int table[128] =
    {
        /* 00 */ GLFW_KEY_A,
        /* 01 */ GLFW_KEY_S,
        /* 02 */ GLFW_KEY_D,
        /* 03 */ GLFW_KEY_F,
        /* 04 */ GLFW_KEY_H,
        /* 05 */ GLFW_KEY_G,
        /* 06 */ GLFW_KEY_Z,
        /* 07 */ GLFW_KEY_X,
        /* 08 */ GLFW_KEY_C,
        /* 09 */ GLFW_KEY_V,
        /* 0a */ GLFW_KEY_GRAVE_ACCENT,
        /* 0b */ GLFW_KEY_B,
        /* 0c */ GLFW_KEY_Q,
        /* 0d */ GLFW_KEY_W,
        /* 0e */ GLFW_KEY_E,
        /* 0f */ GLFW_KEY_R,
        /* 10 */ GLFW_KEY_Y,
        /* 11 */ GLFW_KEY_T,
        /* 12 */ GLFW_KEY_1,
        /* 13 */ GLFW_KEY_2,
        /* 14 */ GLFW_KEY_3,
        /* 15 */ GLFW_KEY_4,
        /* 16 */ GLFW_KEY_6,
        /* 17 */ GLFW_KEY_5,
        /* 18 */ GLFW_KEY_EQUAL,
        /* 19 */ GLFW_KEY_9,
        /* 1a */ GLFW_KEY_7,
        /* 1b */ GLFW_KEY_MINUS,
        /* 1c */ GLFW_KEY_8,
        /* 1d */ GLFW_KEY_0,
        /* 1e */ GLFW_KEY_RIGHT_BRACKET,
        /* 1f */ GLFW_KEY_O,
        /* 20 */ GLFW_KEY_U,
        /* 21 */ GLFW_KEY_LEFT_BRACKET,
        /* 22 */ GLFW_KEY_I,
        /* 23 */ GLFW_KEY_P,
        /* 24 */ GLFW_KEY_ENTER,
        /* 25 */ GLFW_KEY_L,
        /* 26 */ GLFW_KEY_J,
        /* 27 */ GLFW_KEY_APOSTROPHE,
        /* 28 */ GLFW_KEY_K,
        /* 29 */ GLFW_KEY_SEMICOLON,
        /* 2a */ GLFW_KEY_BACKSLASH,
        /* 2b */ GLFW_KEY_COMMA,
        /* 2c */ GLFW_KEY_SLASH,
        /* 2d */ GLFW_KEY_N,
        /* 2e */ GLFW_KEY_M,
        /* 2f */ GLFW_KEY_PERIOD,
        /* 30 */ GLFW_KEY_TAB,
        /* 31 */ GLFW_KEY_SPACE,
        /* 32 */ GLFW_KEY_WORLD_1,
        /* 33 */ GLFW_KEY_BACKSPACE,
        /* 34 */ -1,
        /* 35 */ GLFW_KEY_ESCAPE,
        /* 36 */ GLFW_KEY_RIGHT_SUPER,
        /* 37 */ GLFW_KEY_LEFT_SUPER,
        /* 38 */ GLFW_KEY_LEFT_SHIFT,
        /* 39 */ GLFW_KEY_CAPS_LOCK,
        /* 3a */ GLFW_KEY_LEFT_ALT,
        /* 3b */ GLFW_KEY_LEFT_CONTROL,
        /* 3c */ GLFW_KEY_RIGHT_SHIFT,
        /* 3d */ GLFW_KEY_RIGHT_ALT,
        /* 3e */ GLFW_KEY_RIGHT_CONTROL,
        /* 3f */ -1, /* Function */
        /* 40 */ GLFW_KEY_F17,
        /* 41 */ GLFW_KEY_KP_DECIMAL,
        /* 42 */ -1,
        /* 43 */ GLFW_KEY_KP_MULTIPLY,
        /* 44 */ -1,
        /* 45 */ GLFW_KEY_KP_ADD,
        /* 46 */ -1,
        /* 47 */ GLFW_KEY_NUM_LOCK, /* Really KeypadClear... */
        /* 48 */ -1, /* VolumeUp */
        /* 49 */ -1, /* VolumeDown */
        /* 4a */ -1, /* Mute */
        /* 4b */ GLFW_KEY_KP_DIVIDE,
        /* 4c */ GLFW_KEY_KP_ENTER,
        /* 4d */ -1,
        /* 4e */ GLFW_KEY_KP_SUBTRACT,
        /* 4f */ GLFW_KEY_F18,
        /* 50 */ GLFW_KEY_F19,
        /* 51 */ GLFW_KEY_KP_EQUAL,
        /* 52 */ GLFW_KEY_KP_0,
        /* 53 */ GLFW_KEY_KP_1,
        /* 54 */ GLFW_KEY_KP_2,
        /* 55 */ GLFW_KEY_KP_3,
        /* 56 */ GLFW_KEY_KP_4,
        /* 57 */ GLFW_KEY_KP_5,
        /* 58 */ GLFW_KEY_KP_6,
        /* 59 */ GLFW_KEY_KP_7,
        /* 5a */ GLFW_KEY_F20,
        /* 5b */ GLFW_KEY_KP_8,
        /* 5c */ GLFW_KEY_KP_9,
        /* 5d */ -1,
        /* 5e */ -1,
        /* 5f */ -1,
        /* 60 */ GLFW_KEY_F5,
        /* 61 */ GLFW_KEY_F6,
        /* 62 */ GLFW_KEY_F7,
        /* 63 */ GLFW_KEY_F3,
        /* 64 */ GLFW_KEY_F8,
        /* 65 */ GLFW_KEY_F9,
        /* 66 */ -1,
        /* 67 */ GLFW_KEY_F11,
        /* 68 */ -1,
        /* 69 */ GLFW_KEY_F13,
        /* 6a */ GLFW_KEY_F16,
        /* 6b */ GLFW_KEY_F14,
        /* 6c */ -1,
        /* 6d */ GLFW_KEY_F10,
        /* 6e */ -1,
        /* 6f */ GLFW_KEY_F12,
        /* 70 */ -1,
        /* 71 */ GLFW_KEY_F15,
        /* 72 */ GLFW_KEY_INSERT, /* Really Help... */
        /* 73 */ GLFW_KEY_HOME,
        /* 74 */ GLFW_KEY_PAGE_UP,
        /* 75 */ GLFW_KEY_DELETE,
        /* 76 */ GLFW_KEY_F4,
        /* 77 */ GLFW_KEY_END,
        /* 78 */ GLFW_KEY_F2,
        /* 79 */ GLFW_KEY_PAGE_DOWN,
        /* 7a */ GLFW_KEY_F1,
        /* 7b */ GLFW_KEY_LEFT,
        /* 7c */ GLFW_KEY_RIGHT,
        /* 7d */ GLFW_KEY_DOWN,
        /* 7e */ GLFW_KEY_UP,
        /* 7f */ -1,
    };

    if (macKeyCode >= 128)
        return -1;

    // This treats keycodes as *positional*; that is, we'll return 'a'
    // for the key left of 's', even on an AZERTY keyboard.  The charInput
    // function should still get 'q' though.
    return table[macKeyCode];
}


//========================================================================
// Content view class for the GLFW window
//========================================================================

@interface GLFWContentView : NSView
{
    _GLFWwindow* window;
    NSTrackingArea* trackingArea;
}

- (id)initWithGlfwWindow:(_GLFWwindow *)initWindow;

@end

@implementation GLFWContentView

- (id)initWithGlfwWindow:(_GLFWwindow *)initWindow
{
    self = [super init];
    if (self != nil)
    {
        window = initWindow;
        trackingArea = nil;

        [self updateTrackingAreas];
    }

    return self;
}

-(void)dealloc
{
    [trackingArea release];
    [super dealloc];
}

- (BOOL)isOpaque
{
    return YES;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseDown:(NSEvent *)event
{
    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
}

- (void)mouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)mouseUp:(NSEvent *)event
{
    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
}

- (void)mouseMoved:(NSEvent *)event
{
    if (window->cursorMode == GLFW_CURSOR_CAPTURED)
        _glfwInputCursorMotion(window, [event deltaX], [event deltaY]);
    else
    {
        NSPoint p = [event locationInWindow];

        // Cocoa coordinate system has origin at lower left
        p.y = [[window->NS.object contentView] bounds].size.height - p.y;

        _glfwInputCursorMotion(window, p.x, p.y);
    }
}

- (void)rightMouseDown:(NSEvent *)event
{
    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);
}

- (void)rightMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)rightMouseUp:(NSEvent *)event
{
    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE);
}

- (void)otherMouseDown:(NSEvent *)event
{
    _glfwInputMouseClick(window, [event buttonNumber], GLFW_PRESS);
}

- (void)otherMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent *)event
{
    _glfwInputMouseClick(window, [event buttonNumber], GLFW_RELEASE);
}

- (void)mouseExited:(NSEvent *)event
{
    _glfwInputCursorEnter(window, GL_FALSE);
}

- (void)mouseEntered:(NSEvent *)event
{
    _glfwInputCursorEnter(window, GL_TRUE);
}

- (void)updateTrackingAreas
{
    if (trackingArea != nil)
    {
        [self removeTrackingArea:trackingArea];
        [trackingArea release];
    }

    NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
                                    NSTrackingActiveAlways |
                                    NSTrackingInVisibleRect;

    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                options:options
                                                  owner:self
                                               userInfo:nil];

    [self addTrackingArea:trackingArea];
}

- (void)keyDown:(NSEvent *)event
{
    NSUInteger i, length;
    NSString* characters;
    int key = convertMacKeyCode([event keyCode]);

    if (key != -1)
    {
        _glfwInputKey(window, key, GLFW_PRESS);

        if ([event modifierFlags] & NSCommandKeyMask)
        {
            if (window->systemKeys)
                [super keyDown:event];
        }
        else
        {
            characters = [event characters];
            length = [characters length];

            for (i = 0;  i < length;  i++)
                _glfwInputChar(window, [characters characterAtIndex:i]);
        }
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    int mode, key;
    unsigned int newModifierFlags =
        [event modifierFlags] | NSDeviceIndependentModifierFlagsMask;

    if (newModifierFlags > window->NS.modifierFlags)
        mode = GLFW_PRESS;
    else
        mode = GLFW_RELEASE;

    window->NS.modifierFlags = newModifierFlags;

    key = convertMacKeyCode([event keyCode]);
    if (key != -1)
      _glfwInputKey(window, key, mode);
}

- (void)keyUp:(NSEvent *)event
{
    int key = convertMacKeyCode([event keyCode]);
    if (key != -1)
        _glfwInputKey(window, key, GLFW_RELEASE);
}

- (void)scrollWheel:(NSEvent *)event
{
    double deltaX = [event deltaX];
    double deltaY = [event deltaY];

    if (fabs(deltaX) > 0.0 || fabs(deltaY) > 0.0)
        _glfwInputScroll(window, deltaX, deltaY);
}

@end


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


//========================================================================
// Try to figure out what the calling application is called
//========================================================================

static NSString* findAppName(void)
{
    unsigned int i;
    NSDictionary* infoDictionary = [[NSBundle mainBundle] infoDictionary];

    // Keys to search for as potential application names
    NSString* GLFWNameKeys[] =
    {
        @"CFBundleDisplayName",
        @"CFBundleName",
        @"CFBundleExecutable",
    };

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
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);

    // Having the app in front of the terminal window is also generally
    // handy.  There is an NSApplication API to do this, but...
    SetFrontProcess(&psn);

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

static void createMenuBar(void)
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

    // Prior to Snow Leopard, we need to use this oddly-named semi-private API
    // to get the application menu working properly.
    [NSApp performSelector:@selector(setAppleMenu:) withObject:appMenu];
}


//========================================================================
// Initialize the Cocoa Application Kit
//========================================================================

static GLboolean initializeAppKit(void)
{
    if (NSApp)
        return GL_TRUE;

    // Implicitly create shared NSApplication instance
    [GLFWApplication sharedApplication];

    // Setting up the menu bar must go between sharedApplication
    // above and finishLaunching below, in order to properly emulate the
    // behavior of NSApplicationMain
    createMenuBar();

    [NSApp finishLaunching];

    return GL_TRUE;
}


//========================================================================
// Create the Cocoa window
//========================================================================

static GLboolean createWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig)
{
    unsigned int styleMask = 0;

    if (wndconfig->mode == GLFW_WINDOWED)
    {
        styleMask = NSTitledWindowMask | NSClosableWindowMask |
                    NSMiniaturizableWindowMask;

        if (wndconfig->resizable)
            styleMask |= NSResizableWindowMask;
    }
    else
        styleMask = NSBorderlessWindowMask;

    window->NS.object = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(0, 0, window->width, window->height)
                  styleMask:styleMask
                    backing:NSBackingStoreBuffered
                      defer:NO];

    if (window->NS.object == nil)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Cocoa/NSOpenGL: Failed to create window");
        return GL_FALSE;
    }

    window->NS.view = [[GLFWContentView alloc] initWithGlfwWindow:window];

    [window->NS.object setTitle:[NSString stringWithUTF8String:wndconfig->title]];
    [window->NS.object setContentView:window->NS.view];
    [window->NS.object setDelegate:window->NS.delegate];
    [window->NS.object setAcceptsMouseMovedEvents:YES];
    [window->NS.object center];

    return GL_TRUE;
}


//========================================================================
// Create the OpenGL context
//========================================================================

static GLboolean createContext(_GLFWwindow* window,
                               const _GLFWwndconfig* wndconfig,
                               const _GLFWfbconfig* fbconfig)
{
    unsigned int attributeCount = 0;

    // Mac OS X needs non-zero color size, so set resonable values
    int colorBits = fbconfig->redBits + fbconfig->greenBits + fbconfig->blueBits;
    if (colorBits == 0)
        colorBits = 24;
    else if (colorBits < 15)
        colorBits = 15;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    // Fail if any OpenGL version above 2.1 other than 3.2 was requested
    if (wndconfig->glMajor > 3 ||
        (wndconfig->glMajor == 3 && wndconfig->glMinor != 2))
    {
        _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                      "Cocoa/NSOpenGL: The targeted version of Mac OS X does "
                      "not support any OpenGL version above 2.1 except 3.2");
        return GL_FALSE;
    }

    if (wndconfig->glMajor > 2)
    {
        if (!wndconfig->glForward)
        {
            _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                          "Cocoa/NSOpenGL: The targeted version of Mac OS X "
                          "only supports OpenGL 3.2 contexts if they are "
                          "forward-compatible");
            return GL_FALSE;
        }

        if (wndconfig->glProfile != GLFW_OPENGL_CORE_PROFILE)
        {
            _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                          "Cocoa/NSOpenGL: The targeted version of Mac OS X "
                          "only supports OpenGL 3.2 contexts if they use the "
                          "core profile");
            return GL_FALSE;
        }
    }
#else
    // Fail if OpenGL 3.0 or above was requested
    if (wndconfig->glMajor > 2)
    {
        _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                      "Cocoa/NSOpenGL: The targeted version of Mac OS X does "
                      "not support OpenGL version 3.0 or above");
        return GL_FALSE;
    }
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

    // Fail if a robustness strategy was requested
    if (wndconfig->glRobustness)
    {
        _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                      "Cocoa/NSOpenGL: Mac OS X does not support OpenGL "
                      "robustness strategies");
        return GL_FALSE;
    }

#define ADD_ATTR(x) { attributes[attributeCount++] = x; }
#define ADD_ATTR2(x, y) { ADD_ATTR(x); ADD_ATTR(y); }

    // Arbitrary array size here
    NSOpenGLPixelFormatAttribute attributes[40];

    ADD_ATTR(NSOpenGLPFADoubleBuffer);

    if (wndconfig->mode == GLFW_FULLSCREEN)
    {
        ADD_ATTR(NSOpenGLPFANoRecovery);
        ADD_ATTR2(NSOpenGLPFAScreenMask,
                  CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID()));
    }

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (wndconfig->glMajor > 2)
        ADD_ATTR2(NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core);
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

    ADD_ATTR2(NSOpenGLPFAColorSize, colorBits);

    if (fbconfig->alphaBits > 0)
        ADD_ATTR2(NSOpenGLPFAAlphaSize, fbconfig->alphaBits);

    if (fbconfig->depthBits > 0)
        ADD_ATTR2(NSOpenGLPFADepthSize, fbconfig->depthBits);

    if (fbconfig->stencilBits > 0)
        ADD_ATTR2(NSOpenGLPFAStencilSize, fbconfig->stencilBits);

    int accumBits = fbconfig->accumRedBits + fbconfig->accumGreenBits +
                    fbconfig->accumBlueBits + fbconfig->accumAlphaBits;

    if (accumBits > 0)
        ADD_ATTR2(NSOpenGLPFAAccumSize, accumBits);

    if (fbconfig->auxBuffers > 0)
        ADD_ATTR2(NSOpenGLPFAAuxBuffers, fbconfig->auxBuffers);

    if (fbconfig->stereo)
        ADD_ATTR(NSOpenGLPFAStereo);

    if (fbconfig->samples > 0)
    {
        ADD_ATTR2(NSOpenGLPFASampleBuffers, 1);
        ADD_ATTR2(NSOpenGLPFASamples, fbconfig->samples);
    }

    ADD_ATTR(0);

#undef ADD_ATTR
#undef ADD_ATTR2

    window->NSGL.pixelFormat =
        [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    if (window->NSGL.pixelFormat == nil)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Cocoa/NSOpenGL: Failed to create OpenGL pixel format");
        return GL_FALSE;
    }

    NSOpenGLContext* share = NULL;

    if (wndconfig->share)
        share = wndconfig->share->NSGL.context;

    window->NSGL.context =
        [[NSOpenGLContext alloc] initWithFormat:window->NSGL.pixelFormat
                                   shareContext:share];
    if (window->NSGL.context == nil)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Cocoa/NSOpenGL: Failed to create OpenGL context");
        return GL_FALSE;
    }

    return GL_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Here is where the window is created, and the OpenGL rendering context is
// created
//========================================================================

int _glfwPlatformOpenWindow(_GLFWwindow* window,
                            const _GLFWwndconfig* wndconfig,
                            const _GLFWfbconfig* fbconfig)
{
    if (!initializeAppKit())
        return GL_FALSE;

    window->resizable = wndconfig->resizable;

    // We can only have one application delegate, but we only allocate it the
    // first time we create a window to keep all window code in this file
    if (_glfwLibrary.NS.delegate == nil)
    {
        _glfwLibrary.NS.delegate = [[GLFWApplicationDelegate alloc] init];
        if (_glfwLibrary.NS.delegate == nil)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "Cocoa/NSOpenGL: Failed to create application "
                          "delegate");
            return GL_FALSE;
        }

        [NSApp setDelegate:_glfwLibrary.NS.delegate];
    }

    window->NS.delegate = [[GLFWWindowDelegate alloc] initWithGlfwWindow:window];
    if (window->NS.delegate == nil)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Cocoa/NSOpenGL: Failed to create window delegate");
        return GL_FALSE;
    }

    // Mac OS X needs non-zero color size, so set resonable values
    int colorBits = fbconfig->redBits + fbconfig->greenBits + fbconfig->blueBits;
    if (colorBits == 0)
        colorBits = 24;
    else if (colorBits < 15)
        colorBits = 15;

    // Don't use accumulation buffer support; it's not accelerated
    // Aux buffers probably aren't accelerated either

    if (!createWindow(window, wndconfig))
        return GL_FALSE;

    if (!createContext(window, wndconfig, fbconfig))
        return GL_FALSE;

    [window->NS.object makeKeyAndOrderFront:nil];
    [window->NSGL.context setView:[window->NS.object contentView]];

    if (wndconfig->mode == GLFW_FULLSCREEN)
    {
        int bpp = colorBits + fbconfig->alphaBits;

        if (!_glfwSetVideoMode(&window->width,
                               &window->height,
                               &bpp,
                               &window->refreshRate))
        {
            return GL_FALSE;
        }

        [[window->NS.object contentView] enterFullScreenMode:[NSScreen mainScreen]
                                                 withOptions:nil];
    }

    glfwMakeContextCurrent(window);

    NSPoint point = [[NSCursor currentCursor] hotSpot];
    window->cursorPosX = point.x;
    window->cursorPosY = point.y;

    window->resizable = wndconfig->resizable;

    return GL_TRUE;
}


//========================================================================
// Properly kill the window / video display
//========================================================================

void _glfwPlatformCloseWindow(_GLFWwindow* window)
{
    [window->NS.object orderOut:nil];

    if (window->mode == GLFW_FULLSCREEN)
    {
        [[window->NS.object contentView] exitFullScreenModeWithOptions:nil];

        _glfwRestoreVideoMode();
    }

    [window->NSGL.pixelFormat release];
    window->NSGL.pixelFormat = nil;

    [NSOpenGLContext clearCurrentContext];
    [window->NSGL.context release];
    window->NSGL.context = nil;

    [window->NS.object setDelegate:nil];
    [window->NS.delegate release];
    window->NS.delegate = nil;

    [window->NS.view release];
    window->NS.view = nil;

    [window->NS.object close];
    window->NS.object = nil;

    // TODO: Probably more cleanup
}


//========================================================================
// Set the window title
//========================================================================

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char *title)
{
    [window->NS.object setTitle:[NSString stringWithUTF8String:title]];
}


//========================================================================
// Set the window size
//========================================================================

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    [window->NS.object setContentSize:NSMakeSize(width, height)];
}


//========================================================================
// Set the window position
//========================================================================

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int x, int y)
{
    NSRect contentRect =
        [window->NS.object contentRectForFrameRect:[window->NS.object frame]];

    // We assume here that the client code wants to position the window within the
    // screen the window currently occupies
    NSRect screenRect = [[window->NS.object screen] visibleFrame];
    contentRect.origin = NSMakePoint(screenRect.origin.x + x,
                                     screenRect.origin.y + screenRect.size.height -
                                         y - contentRect.size.height);

    [window->NS.object setFrame:[window->NS.object frameRectForContentRect:contentRect]
                        display:YES];
}


//========================================================================
// Iconify the window
//========================================================================

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
    [window->NS.object miniaturize:nil];
}


//========================================================================
// Restore (un-iconify) the window
//========================================================================

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
    [window->NS.object deminiaturize:nil];
}


//========================================================================
// Write back window parameters into GLFW window structure
//========================================================================

void _glfwPlatformRefreshWindowParams(void)
{
    GLint value;
    _GLFWwindow* window = _glfwLibrary.currentWindow;

    // Since GLFW doesn't understand screens, we use virtual screen zero

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAAccelerated
                       forVirtualScreen:0];
    window->accelerated = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAAlphaSize
                       forVirtualScreen:0];
    window->alphaBits = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAColorSize
                       forVirtualScreen:0];

    // It seems that the color size includes the size of the alpha channel so
    // we subtract it before splitting
    _glfwSplitBPP(value - window->alphaBits,
                  &window->redBits,
                  &window->greenBits,
                  &window->blueBits);

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFADepthSize
                       forVirtualScreen:0];
    window->depthBits = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAStencilSize
                       forVirtualScreen:0];
    window->stencilBits = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAAccumSize
                       forVirtualScreen:0];

    _glfwSplitBPP(value,
                  &window->accumRedBits,
                  &window->accumGreenBits,
                  &window->accumBlueBits);

    // TODO: Figure out what to set this value to
    window->accumAlphaBits = 0;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAAuxBuffers
                       forVirtualScreen:0];
    window->auxBuffers = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFAStereo
                       forVirtualScreen:0];
    window->stereo = value;

    [window->NSGL.pixelFormat getValues:&value
                           forAttribute:NSOpenGLPFASamples
                       forVirtualScreen:0];
    window->samples = value;

    // These this is forced to false as long as Mac OS X lacks support for
    // requesting debug contexts
    window->glDebug = GL_FALSE;
}


//========================================================================
// Poll for new window and input events
//========================================================================

void _glfwPlatformPollEvents(void)
{
    NSEvent* event;

    do
    {
        event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                   untilDate:[NSDate distantPast]
                                      inMode:NSDefaultRunLoopMode
                                     dequeue:YES];

        if (event)
            [NSApp sendEvent:event];
    }
    while (event);

    [_glfwLibrary.NS.autoreleasePool drain];
    _glfwLibrary.NS.autoreleasePool = [[NSAutoreleasePool alloc] init];
}


//========================================================================
// Wait for new window and input events
//========================================================================

void _glfwPlatformWaitEvents( void )
{
    // I wanted to pass NO to dequeue:, and rely on PollEvents to
    // dequeue and send.  For reasons not at all clear to me, passing
    // NO to dequeue: causes this method never to return.
    NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                        untilDate:[NSDate distantFuture]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    [NSApp sendEvent:event];

    _glfwPlatformPollEvents();
}


//========================================================================
// Set physical mouse cursor position
//========================================================================

void _glfwPlatformSetMouseCursorPos(_GLFWwindow* window, int x, int y)
{
    // The library seems to assume that after calling this the mouse won't move,
    // but obviously it will, and escape the app's window, and activate other apps,
    // and other badness in pain.  I think the API's just silly, but maybe I'm
    // misunderstanding it...

    // Also, (x, y) are window coords...

    // Also, it doesn't seem possible to write this robustly without
    // calculating the maximum y coordinate of all screens, since Cocoa's
    // "global coordinates" are upside down from CG's...

    NSPoint localPoint = NSMakePoint(x, window->height - y);
    NSPoint globalPoint = [window->NS.object convertBaseToScreen:localPoint];
    CGPoint mainScreenOrigin = CGDisplayBounds(CGMainDisplayID()).origin;
    double mainScreenHeight = CGDisplayBounds(CGMainDisplayID()).size.height;
    CGPoint targetPoint = CGPointMake(globalPoint.x - mainScreenOrigin.x,
                                      mainScreenHeight - globalPoint.y -
                                          mainScreenOrigin.y);
    CGDisplayMoveCursorToPoint(CGMainDisplayID(), targetPoint);
}


//========================================================================
// Set physical mouse cursor mode
//========================================================================

void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode)
{
    switch (mode)
    {
        case GLFW_CURSOR_NORMAL:
            [NSCursor unhide];
            CGAssociateMouseAndMouseCursorPosition(true);
            break;
        case GLFW_CURSOR_HIDDEN:
            break;
        case GLFW_CURSOR_CAPTURED:
            [NSCursor hide];
            CGAssociateMouseAndMouseCursorPosition(false);
            break;
    }
}

