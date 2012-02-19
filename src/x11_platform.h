//========================================================================
// GLFW - An OpenGL library
// Platform:    X11/GLX
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

#ifndef _platform_h_
#define _platform_h_

#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#define GLX_GLXEXT_LEGACY
#include <GL/glx.h>

// This path may need to be changed if you build GLFW using your own setup
// We ship and use our own copy of glxext.h since GLFW uses fairly new
// extensions and not all operating systems come with an up-to-date version
#include "../support/GL/glxext.h"


// We need declarations for GLX version 1.3 or above even if the server doesn't
// support version 1.3
#ifndef GLX_VERSION_1_3
 #error "GLX header version 1.3 or above is required"
#endif

// With XFree86, we can use the XF86VidMode extension
#if defined(_GLFW_HAS_XF86VIDMODE)
 #include <X11/extensions/xf86vmode.h>
#endif

#if defined(_GLFW_HAS_XRANDR)
 #include <X11/extensions/Xrandr.h>
#endif

// Do we have support for dlopen/dlsym?
#if defined(_GLFW_HAS_DLOPEN)
 #include <dlfcn.h>
#endif

// The Xkb extension provides improved keyboard support
#if defined(_GLFW_HAS_XKB)
 #include <X11/XKBlib.h>
#endif

// We support four different ways for getting addresses for GL/GLX
// extension functions: glXGetProcAddress, glXGetProcAddressARB,
// glXGetProcAddressEXT, and dlsym
#if defined(_GLFW_HAS_GLXGETPROCADDRESSARB)
 #define _glfw_glXGetProcAddress(x) glXGetProcAddressARB(x)
#elif defined(_GLFW_HAS_GLXGETPROCADDRESS)
 #define _glfw_glXGetProcAddress(x) glXGetProcAddress(x)
#elif defined(_GLFW_HAS_GLXGETPROCADDRESSEXT)
 #define _glfw_glXGetProcAddress(x) glXGetProcAddressEXT(x)
#elif defined(_GLFW_HAS_DLOPEN)
 #define _glfw_glXGetProcAddress(x) dlsym(_glfwLibrary.X11.libGL, x)
 #define _GLFW_DLOPEN_LIBGL
#else
 #error "No OpenGL entry point retrieval mechanism was enabled"
#endif

#define _GLFW_PLATFORM_WINDOW_STATE  _GLFWwindowX11 X11
#define _GLFW_PLATFORM_LIBRARY_STATE _GLFWlibraryX11 X11
#define _GLFW_PLATFORM_CONTEXT_STATE _GLFWcontextGLX GLX

// Clipboard atoms
#define _GLFW_CLIPBOARD_ATOM_PRIMARY 0
#define _GLFW_CLIPBOARD_ATOM_CLIPBOARD 1
#define _GLFW_CLIPBOARD_ATOM_SECONDARY 2
#define _GLFW_CLIPBOARD_ATOM_COUNT 3

// String atoms
#define _GLFW_STRING_ATOM_UTF8 0
#define _GLFW_STRING_ATOM_COMPOUND 1
#define _GLFW_STRING_ATOM_STRING 2
#define _GLFW_STRING_ATOM_COUNT 3

//========================================================================
// GLFW platform specific types
//========================================================================

//------------------------------------------------------------------------
// Pointer length integer
//------------------------------------------------------------------------
typedef intptr_t GLFWintptr;


//------------------------------------------------------------------------
// Platform-specific OpenGL context structure
//------------------------------------------------------------------------
typedef struct _GLFWcontextGLX
{
    GLXFBConfigID fbconfigID;        // ID of selected GLXFBConfig
    GLXContext    context;           // OpenGL rendering context
    XVisualInfo*  visual;            // Visual for selected GLXFBConfig

    // GLX extensions
    PFNGLXSWAPINTERVALSGIPROC             SwapIntervalSGI;
    PFNGLXSWAPINTERVALEXTPROC             SwapIntervalEXT;
    PFNGLXGETFBCONFIGATTRIBSGIXPROC       GetFBConfigAttribSGIX;
    PFNGLXCHOOSEFBCONFIGSGIXPROC          ChooseFBConfigSGIX;
    PFNGLXCREATECONTEXTWITHCONFIGSGIXPROC CreateContextWithConfigSGIX;
    PFNGLXGETVISUALFROMFBCONFIGSGIXPROC   GetVisualFromFBConfigSGIX;
    PFNGLXCREATECONTEXTATTRIBSARBPROC     CreateContextAttribsARB;
    GLboolean   SGIX_fbconfig;
    GLboolean   SGI_swap_control;
    GLboolean   EXT_swap_control;
    GLboolean   ARB_multisample;
    GLboolean   ARB_create_context;
    GLboolean   ARB_create_context_profile;
    GLboolean   ARB_create_context_robustness;
    GLboolean   EXT_create_context_es2_profile;

} _GLFWcontextGLX;


//------------------------------------------------------------------------
// Platform-specific window structure
//------------------------------------------------------------------------
typedef struct _GLFWwindowX11
{
    // Platform specific window resources
    Colormap      colormap;          // Window colormap
    Window        handle;            // Window handle
    Atom          wmDeleteWindow;    // WM_DELETE_WINDOW atom
    Atom          wmPing;            // _NET_WM_PING atom
    Atom          wmState;           // _NET_WM_STATE atom
    Atom          wmStateFullscreen; // _NET_WM_STATE_FULLSCREEN atom
    Atom          wmActiveWindow;    // _NET_ACTIVE_WINDOW atom

    // Various platform specific internal variables
    GLboolean     hasEWMH;          // True if window manager supports EWMH
    GLboolean     overrideRedirect; // True if window is OverrideRedirect
    GLboolean     keyboardGrabbed;  // True if keyboard is currently grabbed
    GLboolean     cursorGrabbed;    // True if cursor is currently grabbed
    GLboolean     cursorHidden;     // True if cursor is currently hidden
    GLboolean     cursorCentered;   // True if cursor was moved since last poll
    int           cursorPosX, cursorPosY;

} _GLFWwindowX11;


//------------------------------------------------------------------------
// Platform-specific library global data
//------------------------------------------------------------------------
typedef struct _GLFWlibraryX11
{
    Display*        display;
    int             screen;
    Window          root;
    Cursor          cursor;   // Invisible cursor for hidden cursor

    // Server-side GLX version
    int             glxMajor, glxMinor;

    struct {
        GLboolean   available;
        int         eventBase;
        int         errorBase;
    } VidMode;

    struct {
        GLboolean   available;
        int         eventBase;
        int         errorBase;
        int         majorVersion;
        int         minorVersion;
        GLboolean   gammaBroken;
    } RandR;

    struct {
        GLboolean   available;
        int         majorOpcode;
        int         eventBase;
        int         errorBase;
        int         majorVersion;
        int         minorVersion;
    } Xkb;

    // Key code LUT (mapping X11 key codes to GLFW key codes)
    int             keyCodeLUT[256];

    // Screensaver data
    struct {
        GLboolean   changed;
        int         timeout;
        int         interval;
        int         blanking;
        int         exposure;
    } saver;

    // Fullscreen data
    struct {
        GLboolean   modeChanged;
#if defined(_GLFW_HAS_XRANDR)
        SizeID      oldSizeID;
        int         oldWidth;
        int         oldHeight;
        Rotation    oldRotation;
#endif /*_GLFW_HAS_XRANDR*/
#if defined(_GLFW_HAS_XF86VIDMODE)
        XF86VidModeModeInfo oldMode;
#endif /*_GLFW_HAS_XF86VIDMODE*/
    } FS;

    // Timer data
    struct {
        GLboolean   monotonic;
        double      resolution;
        uint64_t    base;
    } timer;

    // Selection data
    struct {
	struct {
		Atom clipboard[_GLFW_CLIPBOARD_ATOM_COUNT];
		Atom string[_GLFW_STRING_ATOM_COUNT];
	} atoms;
	struct {
		size_t stringlen;
		char *string;
	} clipboard;
	Atom request;
        int converted;
    } selection;

#if defined(_GLFW_DLOPEN_LIBGL)
    void*           libGL;  // dlopen handle for libGL.so
#endif
} _GLFWlibraryX11;


//------------------------------------------------------------------------
// Joystick information & state
//------------------------------------------------------------------------
GLFWGLOBAL struct {
    int           Present;
    int           fd;
    int           NumAxes;
    int           NumButtons;
    float*        Axis;
    unsigned char* Button;
} _glfwJoy[GLFW_JOYSTICK_LAST + 1];


//========================================================================
// Prototypes for platform specific internal functions
//========================================================================

// Time
void _glfwInitTimer(void);

// Fullscreen support
int  _glfwGetClosestVideoMode(int screen, int* width, int* height, int* rate);
void _glfwSetVideoModeMODE(int screen, int mode, int rate);
void _glfwSetVideoMode(int screen, int* width, int* height, int* rate);
void _glfwRestoreVideoMode(int screen);

// Joystick input
void _glfwInitJoysticks(void);
void _glfwTerminateJoysticks(void);

// Unicode support
long _glfwKeySym2Unicode(KeySym keysym);

// Clipboard handling
Atom _glfwSelectionRequest(XSelectionRequestEvent *request);

#endif // _platform_h_
