//========================================================================
// GLFW - An OpenGL library
// Platform:    X11
// API version: 3.0
// WWW:         http://www.glfw.org/
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

// The Xf86VidMode extension provides fallback gamma control
#include <X11/extensions/xf86vmode.h>

// The XRandR extension provides mode setting and gamma control
#include <X11/extensions/Xrandr.h>

// The Xkb extension provides improved keyboard support
#include <X11/XKBlib.h>

#if defined(_GLFW_GLX)
 #define _GLFW_X11_CONTEXT_VISUAL window->glx.visual
 #include "glx_platform.h"
#elif defined(_GLFW_EGL)
 #define _GLFW_X11_CONTEXT_VISUAL window->egl.visual
 #define _GLFW_EGL_NATIVE_WINDOW  window->x11.handle
 #define _GLFW_EGL_NATIVE_DISPLAY _glfw.x11.display
 #include "egl_platform.h"
#else
 #error "No supported context creation API selected"
#endif

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowX11  x11
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryX11 x11
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorX11 x11

// Clipboard format atom indices (in order of preference)
#define _GLFW_CLIPBOARD_FORMAT_UTF8     0
#define _GLFW_CLIPBOARD_FORMAT_COMPOUND 1
#define _GLFW_CLIPBOARD_FORMAT_STRING   2
#define _GLFW_CLIPBOARD_FORMAT_COUNT    3


//========================================================================
// GLFW platform specific types
//========================================================================

//------------------------------------------------------------------------
// Pointer length integer
//------------------------------------------------------------------------
typedef intptr_t GLFWintptr;


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
    GLboolean       cursorGrabbed;    // True if cursor is currently grabbed
    GLboolean       cursorHidden;     // True if cursor is currently hidden
    GLboolean       cursorCentered;   // True if cursor was moved since last poll
    int             cursorPosX, cursorPosY;

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

    // Window manager atoms
    Atom            WM_STATE;
    Atom            WM_DELETE_WINDOW;
    Atom            NET_WM_NAME;
    Atom            NET_WM_ICON_NAME;
    Atom            NET_WM_PING;
    Atom            NET_WM_STATE;
    Atom            NET_WM_STATE_FULLSCREEN;
    Atom            NET_ACTIVE_WINDOW;

    // Selection atoms
    Atom            TARGETS;
    Atom            CLIPBOARD;
    Atom            UTF8_STRING;
    Atom            COMPOUND_STRING;

    // True if window manager supports EWMH
    GLboolean       hasEWMH;

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
    } randr;

    struct {
        int         majorOpcode;
        int         eventBase;
        int         errorBase;
        int         versionMajor;
        int         versionMinor;
    } xkb;

    // LUT for mapping X11 key codes to GLFW key codes
    int             keyCodeLUT[256];

    struct {
        GLboolean   changed;
        int         timeout;
        int         interval;
        int         blanking;
        int         exposure;
    } saver;

    struct {
        GLboolean   monotonic;
        double      resolution;
        uint64_t    base;
    } timer;

    struct {
        Atom        formats[_GLFW_CLIPBOARD_FORMAT_COUNT];
        char*       string;
        Atom        property;
    } selection;

    struct {
        int         present;
        int         fd;
        int         numAxes;
        int         numButtons;
        float*      axis;
        unsigned char* button;
        char*       name;
    } joystick[GLFW_JOYSTICK_LAST + 1];

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


//========================================================================
// Prototypes for platform specific internal functions
//========================================================================

// Time
void _glfwInitTimer(void);

// Gamma
void _glfwInitGammaRamp(void);

// OpenGL support
int _glfwInitContextAPI(void);
void _glfwTerminateContextAPI(void);
int _glfwCreateContext(_GLFWwindow* window,
                       const _GLFWwndconfig* wndconfig,
                       const _GLFWfbconfig* fbconfig);
void _glfwDestroyContext(_GLFWwindow* window);

// Fullscreen support
void _glfwSetVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* mode);
void _glfwRestoreVideoMode(_GLFWmonitor* monitor);

// Joystick input
int  _glfwInitJoysticks(void);
void _glfwTerminateJoysticks(void);

// Unicode support
long _glfwKeySym2Unicode(KeySym keysym);

// Clipboard handling
Atom _glfwWriteSelection(XSelectionRequestEvent* request);

// Window support
_GLFWwindow* _glfwFindWindowByHandle(Window handle);
unsigned long _glfwGetWindowProperty(Window window,
                                     Atom property,
                                     Atom type,
                                     unsigned char** value);

#endif // _x11_platform_h_
