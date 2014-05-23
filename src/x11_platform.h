//========================================================================
// GLFW 3.1 X11 - www.glfw.org
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

#ifndef _x11_platform_h_
#define _x11_platform_h_

#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>

// The Xf86VidMode extension provides fallback gamma control
#include <X11/extensions/xf86vmode.h>

// The XRandR extension provides mode setting and gamma control
#include <X11/extensions/Xrandr.h>

// The XInput2 extension provides improved input events
#include <X11/extensions/XInput2.h>

// The Xkb extension provides improved keyboard support
#include <X11/XKBlib.h>

#include "posix_tls.h"

#if defined(_GLFW_GLX)
 #define _GLFW_X11_CONTEXT_VISUAL window->glx.visual
 #include "glx_context.h"
#elif defined(_GLFW_EGL)
 #define _GLFW_X11_CONTEXT_VISUAL window->egl.visual
 #define _GLFW_EGL_NATIVE_WINDOW  window->x11.handle
 #define _GLFW_EGL_NATIVE_DISPLAY _glfw.x11.display
 #include "egl_context.h"
#else
 #error "No supported context creation API selected"
#endif

#include "posix_time.h"
#include "linux_joystick.h"

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowX11  x11
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryX11 x11
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorX11 x11
#define _GLFW_PLATFORM_CURSOR_STATE         _GLFWcursorX11  x11


//========================================================================
// GLFW platform specific types
//========================================================================


//------------------------------------------------------------------------
// Platform-specific window structure
//------------------------------------------------------------------------
typedef struct _GLFWwindowX11
{
    // Platform specific window resources
    Colormap        colormap;          // Window colormap
    Window          handle;            // Window handle

    // Various platform specific internal variables
    GLboolean       overrideRedirect; // True if window is OverrideRedirect

    // Cached position and size used to filter out duplicate events
    int             width, height;
    int             xpos, ypos;

    // The last received cursor position, regardless of source
    double          cursorPosX, cursorPosY;
    // The last position the cursor was warped to by GLFW
    int             warpPosX, warpPosY;

} _GLFWwindowX11;


//------------------------------------------------------------------------
// Platform-specific library global data for X11
//------------------------------------------------------------------------
typedef struct _GLFWlibraryX11
{
    Display*        display;
    int             screen;
    Window          root;

    // Invisible cursor for hidden cursor mode
    Cursor          cursor;
    XContext        context;

    // Window manager atoms
    Atom            WM_PROTOCOLS;
    Atom            WM_STATE;
    Atom            WM_DELETE_WINDOW;
    Atom            NET_WM_NAME;
    Atom            NET_WM_ICON_NAME;
    Atom            NET_WM_PID;
    Atom            NET_WM_PING;
    Atom            NET_WM_STATE;
    Atom            NET_WM_STATE_ABOVE;
    Atom            NET_WM_STATE_FULLSCREEN;
    Atom            NET_WM_BYPASS_COMPOSITOR;
    Atom            NET_ACTIVE_WINDOW;
    Atom            NET_FRAME_EXTENTS;
    Atom            NET_REQUEST_FRAME_EXTENTS;
    Atom            MOTIF_WM_HINTS;

    // Xdnd (drag and drop) atoms
    Atom            XdndAware;
    Atom            XdndEnter;
    Atom            XdndPosition;
    Atom            XdndStatus;
    Atom            XdndActionCopy;
    Atom            XdndDrop;
    Atom            XdndLeave;
    Atom            XdndFinished;
    Atom            XdndSelection;

    // Selection (clipboard) atoms
    Atom            TARGETS;
    Atom            MULTIPLE;
    Atom            CLIPBOARD;
    Atom            CLIPBOARD_MANAGER;
    Atom            SAVE_TARGETS;
    Atom            _NULL;
    Atom            UTF8_STRING;
    Atom            COMPOUND_STRING;
    Atom            ATOM_PAIR;
    Atom            GLFW_SELECTION;

    // True if window manager supports EWMH
    GLboolean       hasEWMH;

    // Error code received by the X error handler
    int             errorCode;

    struct {
        GLboolean   available;
        int         eventBase;
        int         errorBase;
    } vidmode;

    struct {
        GLboolean   available;
        int         eventBase;
        int         errorBase;
        int         versionMajor;
        int         versionMinor;
        GLboolean   gammaBroken;
        GLboolean   monitorBroken;
    } randr;

    struct {
        GLboolean   available;
        GLboolean   detectable;
        int         majorOpcode;
        int         eventBase;
        int         errorBase;
        int         versionMajor;
        int         versionMinor;
    } xkb;

    struct {
        GLboolean   available;
        int         majorOpcode;
        int         eventBase;
        int         errorBase;
        int         versionMajor;
        int         versionMinor;
    } xi;

    // LUT for mapping X11 key codes to GLFW key codes
    int             keyCodeLUT[256];

    struct {
        int         count;
        int         timeout;
        int         interval;
        int         blanking;
        int         exposure;
    } saver;

    struct {
        char*       string;
    } selection;

    struct {
        Window      source;
    } xdnd;

} _GLFWlibraryX11;


//------------------------------------------------------------------------
// Platform-specific monitor structure
//------------------------------------------------------------------------
typedef struct _GLFWmonitorX11
{
    RROutput        output;
    RRCrtc          crtc;
    RRMode          oldMode;

} _GLFWmonitorX11;


//------------------------------------------------------------------------
// Platform-specific cursor structure
//------------------------------------------------------------------------
typedef struct _GLFWcursorX11
{
    Cursor handle;
} _GLFWcursorX11;


//========================================================================
// Prototypes for platform specific internal functions
//========================================================================

// Gamma
void _glfwInitGammaRamp(void);

// Fullscreen support
GLboolean _glfwSetVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* desired);
void _glfwRestoreVideoMode(_GLFWmonitor* monitor);

// Unicode support
long _glfwKeySym2Unicode(KeySym keysym);

// Clipboard handling
void _glfwHandleSelectionClear(XEvent* event);
void _glfwHandleSelectionRequest(XEvent* event);
void _glfwPushSelectionToManager(_GLFWwindow* window);

// Window support
_GLFWwindow* _glfwFindWindowByHandle(Window handle);
unsigned long _glfwGetWindowProperty(Window window,
                                     Atom property,
                                     Atom type,
                                     unsigned char** value);

// X11 error handler
void _glfwGrabXErrorHandler(void);
void _glfwReleaseXErrorHandler(void);
void _glfwInputXError(int error, const char* message);

#endif // _x11_platform_h_
