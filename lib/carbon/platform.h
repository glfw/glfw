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

#ifndef _platform_h_
#define _platform_h_


// This is the Mac OS X version of GLFW
#define _GLFW_MAC_OS_X

#include <Carbon/Carbon.h>
#include <OpenGL/OpenGL.h>
#include <AGL/agl.h>

#include "../../include/GL/glfw.h"

#if MACOSX_DEPLOYMENT_TARGET < MAC_OS_X_VERSION_10_3

#ifndef kCGLNoError
#define kCGLNoError 0
#endif

#endif


#ifndef GL_VERSION_3_0

typedef const GLubyte * (APIENTRY *PFNGLGETSTRINGIPROC) (GLenum, GLuint);

#endif /*GL_VERSION_3_0*/


//========================================================================
// Defines
//========================================================================

#define _GLFW_MAX_PATH_LENGTH (8192)

#define MAC_KEY_ENTER       0x24
#define MAC_KEY_RETURN      0x34
#define MAC_KEY_ESC         0x35
#define MAC_KEY_F1          0x7A
#define MAC_KEY_F2          0x78
#define MAC_KEY_F3          0x63
#define MAC_KEY_F4          0x76
#define MAC_KEY_F5          0x60
#define MAC_KEY_F6          0x61
#define MAC_KEY_F7          0x62
#define MAC_KEY_F8          0x64
#define MAC_KEY_F9          0x65
#define MAC_KEY_F10         0x6D
#define MAC_KEY_F11         0x67
#define MAC_KEY_F12         0x6F
#define MAC_KEY_F13         0x69
#define MAC_KEY_F14         0x6B
#define MAC_KEY_F15         0x71
#define MAC_KEY_UP          0x7E
#define MAC_KEY_DOWN        0x7D
#define MAC_KEY_LEFT        0x7B
#define MAC_KEY_RIGHT       0x7C
#define MAC_KEY_TAB         0x30
#define MAC_KEY_BACKSPACE   0x33
#define MAC_KEY_HELP        0x72
#define MAC_KEY_DEL         0x75
#define MAC_KEY_PAGEUP      0x74
#define MAC_KEY_PAGEDOWN    0x79
#define MAC_KEY_HOME        0x73
#define MAC_KEY_END         0x77
#define MAC_KEY_KP_0        0x52
#define MAC_KEY_KP_1        0x53
#define MAC_KEY_KP_2        0x54
#define MAC_KEY_KP_3        0x55
#define MAC_KEY_KP_4        0x56
#define MAC_KEY_KP_5        0x57
#define MAC_KEY_KP_6        0x58
#define MAC_KEY_KP_7        0x59
#define MAC_KEY_KP_8        0x5B
#define MAC_KEY_KP_9        0x5C
#define MAC_KEY_KP_DIVIDE   0x4B
#define MAC_KEY_KP_MULTIPLY 0x43
#define MAC_KEY_KP_SUBTRACT 0x4E
#define MAC_KEY_KP_ADD      0x45
#define MAC_KEY_KP_DECIMAL  0x41
#define MAC_KEY_KP_EQUAL    0x51
#define MAC_KEY_KP_ENTER    0x4C
#define MAC_KEY_NUMLOCK     0x47


//========================================================================
// GLFW platform specific types
//========================================================================

//------------------------------------------------------------------------
// Pointer length integer
//------------------------------------------------------------------------
typedef intptr_t GLFWintptr;


GLFWGLOBAL CFDictionaryRef _glfwDesktopVideoMode;

//------------------------------------------------------------------------
// Window structure
//------------------------------------------------------------------------
typedef struct _GLFWwin_struct _GLFWwin;

struct _GLFWwin_struct {

// ========= PLATFORM INDEPENDENT MANDATORY PART =========================

    // User callback functions
    GLFWwindowsizefun    windowSizeCallback;
    GLFWwindowclosefun   windowCloseCallback;
    GLFWwindowrefreshfun windowRefreshCallback;
    GLFWmousebuttonfun   mouseButtonCallback;
    GLFWmouseposfun      mousePosCallback;
    GLFWmousewheelfun    mouseWheelCallback;
    GLFWkeyfun           keyCallback;
    GLFWcharfun          charCallback;

    // User selected window settings
    int       fullscreen;      // Fullscreen flag
    int       mouseLock;       // Mouse-lock flag
    int       autoPollEvents;  // Auto polling flag
    int       sysKeysDisabled; // System keys disabled flag
    int       windowNoResize;  // Resize- and maximize gadgets disabled flag
    int       refreshRate;     // Vertical monitor refresh rate

    // Window status & parameters
    int       opened;          // Flag telling if window is opened or not
    int       active;          // Application active flag
    int       iconified;       // Window iconified flag
    int       width, height;   // Window width and heigth
    int       accelerated;     // GL_TRUE if window is HW accelerated

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
    int       stereo;
    int       samples;

    // OpenGL extensions and context attributes
    int       has_GL_SGIS_generate_mipmap;
    int       has_GL_ARB_texture_non_power_of_two;
    int       glMajor, glMinor, glRevision;
    int       glForward, glDebug, glProfile;

    PFNGLGETSTRINGIPROC GetStringi;

// ========= PLATFORM SPECIFIC PART ======================================

    WindowRef          window;

    AGLContext         aglContext;
    AGLPixelFormat     aglPixelFormat;

    CGLContextObj      cglContext;
    CGLPixelFormatObj  cglPixelFormat;

    EventHandlerUPP    windowUPP;
    EventHandlerUPP    mouseUPP;
    EventHandlerUPP    commandUPP;
    EventHandlerUPP    keyboardUPP;
};

GLFWGLOBAL _GLFWwin _glfwWin;


//------------------------------------------------------------------------
// User input status (some of this should go in _GLFWwin)
//------------------------------------------------------------------------
GLFWGLOBAL struct {

// ========= PLATFORM INDEPENDENT MANDATORY PART =========================

    // Mouse status
    int      MousePosX, MousePosY;
    int      WheelPos;
    char     MouseButton[ GLFW_MOUSE_BUTTON_LAST + 1 ];

    // Keyboard status
    char     Key[ GLFW_KEY_LAST + 1 ];
    int      LastChar;

    // User selected settings
    int      StickyKeys;
    int      StickyMouseButtons;
    int      KeyRepeat;

// ========= PLATFORM SPECIFIC PART ======================================

    UInt32 Modifiers;

} _glfwInput;



//------------------------------------------------------------------------
// Library global data
//------------------------------------------------------------------------
GLFWGLOBAL struct {

// ========= PLATFORM INDEPENDENT MANDATORY PART =========================

    // Window opening hints
    _GLFWhints      hints;

// ========= PLATFORM SPECIFIC PART ======================================

    // Timer data
    struct {
	double       t0;
    } Timer;

    struct {
	    // Bundle for dynamically-loading extension function pointers
        CFBundleRef OpenGLFramework;
    } Libs;

    int Unbundled;

} _glfwLibrary;


//========================================================================
// Prototypes for platform specific internal functions
//========================================================================

void  _glfwChangeToResourcesDirectory( void );

#endif // _platform_h_
