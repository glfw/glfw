//========================================================================
// GLFW - An OpenGL library
// Platform:    Any
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

#ifndef _internal_h_
#define _internal_h_

//========================================================================
// GLFWGLOBAL is a macro that places all global variables in the init.c
// module (all other modules reference global variables as 'extern')
//========================================================================

#if defined(_init_c_)
 #define GLFWGLOBAL
#else
 #define GLFWGLOBAL extern
#endif


//========================================================================
// Input handling definitions
//========================================================================

// Internal key and button state/action definitions
#define GLFW_STICK 2


//========================================================================
// Internal type declarations
//========================================================================

typedef struct _GLFWhints       _GLFWhints;
typedef struct _GLFWwndconfig   _GLFWwndconfig;
typedef struct _GLFWfbconfig    _GLFWfbconfig;
typedef struct _GLFWwindow      _GLFWwindow;
typedef struct _GLFWlibrary     _GLFWlibrary;


//------------------------------------------------------------------------
// Platform specific definitions goes in platform.h (which also includes
// glfw.h)
//------------------------------------------------------------------------

#include "config.h"

#include "../include/GL/glfw3.h"

// This path may need to be changed if you build GLFW using your own setup
// We ship and use our own copy of glext.h since GLFW uses fairly new
// extensions and not all operating systems come with an up-to-date version
#include "../support/GL/glext.h"

#if defined(_GLFW_COCOA_NSGL)
 #include "cocoa_platform.h"
#elif defined(_GLFW_WIN32_WGL)
 #include "win32_platform.h"
#elif defined(_GLFW_X11_GLX)
 #include "x11_platform.h"
#else
 #error "No supported platform selected"
#endif


//------------------------------------------------------------------------
// Window hints, set by glfwOpenWindowHint and consumed by glfwOpenWindow
// A bucket of semi-random stuff lumped together for historical reasons
// This is used only by the platform independent code and only to store
// parameters passed to us by glfwOpenWindowHint
//------------------------------------------------------------------------
struct _GLFWhints
{
    int         redBits;
    int         greenBits;
    int         blueBits;
    int         alphaBits;
    int         depthBits;
    int         stencilBits;
    int         refreshRate;
    int         accumRedBits;
    int         accumGreenBits;
    int         accumBlueBits;
    int         accumAlphaBits;
    int         auxBuffers;
    GLboolean   stereo;
    GLboolean   resizable;
    int         samples;
    int         glMajor;
    int         glMinor;
    GLboolean   glForward;
    GLboolean   glDebug;
    int         glProfile;
    int         glRobustness;
};


//------------------------------------------------------------------------
// Parameters relating to the creation of the context and window but not
// directly related to the properties of the framebuffer
// This is used to pass window and context creation parameters from the
// platform independent code to the platform specific code
//------------------------------------------------------------------------
struct _GLFWwndconfig
{
    int           mode;
    const char*   title;
    int           refreshRate;
    GLboolean     resizable;
    int           glMajor;
    int           glMinor;
    GLboolean     glForward;
    GLboolean     glDebug;
    int           glProfile;
    int           glRobustness;
    _GLFWwindow*  share;
};


//------------------------------------------------------------------------
// Framebuffer configuration descriptor, i.e. buffers and their sizes
// Also a platform specific ID used to map back to the actual backend APIs
// This is used to pass framebuffer parameters from the platform independent
// code to the platform specific code, and also to enumerate and select
// available framebuffer configurations
//------------------------------------------------------------------------
struct _GLFWfbconfig
{
    int         redBits;
    int         greenBits;
    int         blueBits;
    int         alphaBits;
    int         depthBits;
    int         stencilBits;
    int         accumRedBits;
    int         accumGreenBits;
    int         accumBlueBits;
    int         accumAlphaBits;
    int         auxBuffers;
    GLboolean   stereo;
    int         samples;
    GLFWintptr  platformID;
};


//------------------------------------------------------------------------
// Window structure
//------------------------------------------------------------------------
struct _GLFWwindow
{
    struct _GLFWwindow* next;

    // Window settings and state
    GLboolean iconified;       // GL_TRUE if this window is iconified
    GLboolean closeRequested;  // GL_TRUE if this window should be closed
    int       width, height;
    int       positionX, positionY;
    int       mode;            // GLFW_WINDOW or GLFW_FULLSCREEN
    GLboolean resizable;       // GL_TRUE if user may resize this window
    int       refreshRate;     // monitor refresh rate
    void*     userPointer;

    // Window input state
    GLboolean stickyKeys;
    GLboolean stickyMouseButtons;
    GLboolean keyRepeat;
    GLboolean systemKeys;      // system keys enabled flag
    int       cursorPosX, cursorPosY;
    int       cursorMode;
    double    scrollX, scrollY;
    char      mouseButton[GLFW_MOUSE_BUTTON_LAST + 1];
    char      key[GLFW_KEY_LAST + 1];

    // Framebuffer attributes
    int       redBits;
    int       greenBits;
    int       blueBits;
    int       alphaBits;
    int       depthBits;
    int       stencilBits;
    int       accumRedBits;
    int       accumGreenBits;
    int       accumBlueBits;
    int       accumAlphaBits;
    int       auxBuffers;
    GLboolean stereo;
    int       samples;

    // OpenGL extensions and context attributes
    GLboolean accelerated;     // GL_TRUE if OpenGL context is "accelerated"
    int       glMajor, glMinor, glRevision;
    GLboolean glForward, glDebug;
    int       glProfile;
    int       glRobustness;
    PFNGLGETSTRINGIPROC GetStringi;

    // These are defined in the current port's platform.h
    _GLFW_PLATFORM_WINDOW_STATE;
    _GLFW_PLATFORM_CONTEXT_STATE;
};


//------------------------------------------------------------------------
// Library global data
//------------------------------------------------------------------------
struct _GLFWlibrary
{
    _GLFWhints    hints;

    _GLFWwindow*  windowListHead;
    _GLFWwindow*  currentWindow;
    _GLFWwindow*  activeWindow;

    GLFWwindowsizefun    windowSizeCallback;
    GLFWwindowclosefun   windowCloseCallback;
    GLFWwindowrefreshfun windowRefreshCallback;
    GLFWwindowfocusfun   windowFocusCallback;
    GLFWwindowiconifyfun windowIconifyCallback;
    GLFWmousebuttonfun   mouseButtonCallback;
    GLFWmouseposfun      mousePosCallback;
    GLFWcursorenterfun   cursorEnterCallback;
    GLFWscrollfun        scrollCallback;
    GLFWkeyfun           keyCallback;
    GLFWcharfun          charCallback;

    GLFWgammaramp currentRamp;
    GLFWgammaramp originalRamp;
    int           originalRampSize;
    GLboolean     rampChanged;

    // This is defined in the current port's platform.h
    _GLFW_PLATFORM_LIBRARY_WINDOW_STATE;
    _GLFW_PLATFORM_LIBRARY_OPENGL_STATE;
};


//========================================================================
// System independent global variables (GLFW internals)
//========================================================================

// Flag indicating if GLFW has been initialized
#if defined(_init_c_)
GLboolean _glfwInitialized = GL_FALSE;
#else
GLFWGLOBAL GLboolean _glfwInitialized;
#endif

GLFWGLOBAL _GLFWlibrary _glfwLibrary;


//========================================================================
// Prototypes for platform specific implementation functions
//========================================================================

// Init/terminate
int _glfwPlatformInit(void);
int _glfwPlatformTerminate(void);
const char* _glfwPlatformGetVersionString(void);

// Input
void _glfwPlatformEnableSystemKeys(_GLFWwindow* window);
void _glfwPlatformDisableSystemKeys(_GLFWwindow* window);
void _glfwPlatformSetMouseCursorPos(_GLFWwindow* window, int x, int y);
void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode);

// Fullscreen
int  _glfwPlatformGetVideoModes(GLFWvidmode* list, int maxcount);
void _glfwPlatformGetDesktopMode(GLFWvidmode* mode);

// Gamma ramp
void _glfwPlatformGetGammaRamp(GLFWgammaramp* ramp);
void _glfwPlatformSetGammaRamp(const GLFWgammaramp* ramp);

// Clipboard
void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string);
const char* _glfwPlatformGetClipboardString(_GLFWwindow* window);

// Joystick
int _glfwPlatformGetJoystickParam(int joy, int param);
int _glfwPlatformGetJoystickPos(int joy, float* pos, int numaxes);
int _glfwPlatformGetJoystickButtons(int joy, unsigned char* buttons, int numbuttons);

// Time
double _glfwPlatformGetTime(void);
void _glfwPlatformSetTime(double time);

// Window management
int  _glfwPlatformOpenWindow(_GLFWwindow* window, const _GLFWwndconfig* wndconfig, const _GLFWfbconfig* fbconfig);
void _glfwPlatformCloseWindow(_GLFWwindow* window);
void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title);
void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height);
void _glfwPlatformSetWindowPos(_GLFWwindow* window, int x, int y);
void _glfwPlatformIconifyWindow(_GLFWwindow* window);
void _glfwPlatformRestoreWindow(_GLFWwindow* window);

// Event management
void _glfwPlatformPollEvents(void);
void _glfwPlatformWaitEvents(void);

// OpenGL context management
void _glfwPlatformMakeContextCurrent(_GLFWwindow* window);
void _glfwPlatformSwapBuffers(void);
void _glfwPlatformSwapInterval(int interval);
void _glfwPlatformRefreshWindowParams(void);
int  _glfwPlatformExtensionSupported(const char* extension);
void* _glfwPlatformGetProcAddress(const char* procname);
void _glfwPlatformCopyContext(_GLFWwindow* src, _GLFWwindow* dst, unsigned long mask);


//========================================================================
// Prototypes for platform independent internal functions
//========================================================================

// Fullscren management (fullscreen.c)
void _glfwSplitBPP(int bpp, int* red, int* green, int* blue);

// Error handling (error.c)
void _glfwSetError(int error, const char* description);

// Window management (window.c)
void _glfwSetDefaultWindowHints(void);

// Window event notification (window.c)
void _glfwInputWindowFocus(_GLFWwindow* window, GLboolean activated);
void _glfwInputWindowPos(_GLFWwindow* window, int x, int y);
void _glfwInputWindowSize(_GLFWwindow* window, int width, int height);
void _glfwInputWindowIconify(_GLFWwindow* window, int iconified);
void _glfwInputWindowDamage(_GLFWwindow* window);

// Input event notification (input.c)
void _glfwInputKey(_GLFWwindow* window, int key, int action);
void _glfwInputChar(_GLFWwindow* window, int character);
void _glfwInputScroll(_GLFWwindow* window, double x, double y);
void _glfwInputMouseClick(_GLFWwindow* window, int button, int action);
void _glfwInputCursorMotion(_GLFWwindow* window, int x, int y);
void _glfwInputCursorEnter(_GLFWwindow* window, int entered);

// OpenGL context helpers (opengl.c)
int _glfwStringInExtensionString(const char* string, const GLubyte* extensions);
const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* desired,
                                         const _GLFWfbconfig* alternatives,
                                         unsigned int count);
GLboolean _glfwIsValidContextConfig(_GLFWwndconfig* wndconfig);
GLboolean _glfwIsValidContext(_GLFWwindow* window, _GLFWwndconfig* wndconfig);


#endif // _internal_h_
