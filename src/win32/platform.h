//========================================================================
// GLFW - An OpenGL framework
// Platform:    Win32/WGL
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


// This is the Windows version of GLFW
#define _GLFW_WIN32

// We don't need all the fancy stuff
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

// Include files
#include <windows.h>
#include <mmsystem.h>
#include "../../include/GL/glfw3.h"


//========================================================================
// Hack: Define things that some <windows.h>'s do not define
//========================================================================

// Some old versions of w32api (used by MinGW and Cygwin) define
// WH_KEYBOARD_LL without typedef:ing KBDLLHOOKSTRUCT (!)
#if defined(__MINGW32__) || defined(__CYGWIN__)
#include <w32api.h>
#if defined(WH_KEYBOARD_LL) && (__W32API_MAJOR_VERSION == 1) && (__W32API_MINOR_VERSION <= 2)
#undef WH_KEYBOARD_LL
#endif
#endif

//------------------------------------------------------------------------
// ** NOTE **  If this gives you compiler errors and you are using MinGW
// (or Dev-C++), update to w32api version 1.3 or later:
// http://sourceforge.net/project/showfiles.php?group_id=2435
//------------------------------------------------------------------------
#ifndef WH_KEYBOARD_LL
#define WH_KEYBOARD_LL 13
typedef struct tagKBDLLHOOKSTRUCT {
  DWORD   vkCode;
  DWORD   scanCode;
  DWORD   flags;
  DWORD   time;
  DWORD   dwExtraInfo;
} KBDLLHOOKSTRUCT, FAR *LPKBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;
#endif // WH_KEYBOARD_LL

#ifndef LLKHF_ALTDOWN
#define LLKHF_ALTDOWN  0x00000020
#endif

#ifndef SPI_SETSCREENSAVERRUNNING
#define SPI_SETSCREENSAVERRUNNING 97
#endif
#ifndef SPI_GETANIMATION
#define SPI_GETANIMATION 72
#endif
#ifndef SPI_SETANIMATION
#define SPI_SETANIMATION 73
#endif
#ifndef SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT 0x2000
#endif
#ifndef SPI_SETFOREGROUNDLOCKTIMEOUT
#define SPI_SETFOREGROUNDLOCKTIMEOUT 0x2001
#endif

#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN 4
#endif

#ifndef PFD_GENERIC_ACCELERATED
#define PFD_GENERIC_ACCELERATED 0x00001000
#endif
#ifndef PFD_DEPTH_DONTCARE
#define PFD_DEPTH_DONTCARE 0x20000000
#endif

#ifndef ENUM_CURRENT_SETTINGS
#define ENUM_CURRENT_SETTINGS -1
#endif
#ifndef ENUM_REGISTRY_SETTINGS
#define ENUM_REGISTRY_SETTINGS -2
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x020B
#endif
#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP 0x020C
#endif
#ifndef XBUTTON1
#define XBUTTON1 1
#endif
#ifndef XBUTTON2
#define XBUTTON2 2
#endif

#ifndef WGL_ARB_pixel_format

// wglSwapIntervalEXT typedef (Win32 buffer-swap interval control)
typedef int (APIENTRY * WGLSWAPINTERVALEXT_T) (int);
// wglGetPixelFormatAttribivARB typedef
typedef BOOL (WINAPI * WGLGETPIXELFORMATATTRIBIVARB_T) (HDC, int, int, UINT, const int* , int* );
// wglGetExtensionStringEXT typedef
typedef const char* (APIENTRY * WGLGETEXTENSIONSSTRINGEXT_T)(void);
// wglGetExtensionStringARB typedef
typedef const char* (APIENTRY * WGLGETEXTENSIONSSTRINGARB_T)(HDC);

/* Constants for wglGetPixelFormatAttribivARB */
#define WGL_NUMBER_PIXEL_FORMATS_ARB    0x2000
#define WGL_DRAW_TO_WINDOW_ARB          0x2001
#define WGL_SUPPORT_OPENGL_ARB          0x2010
#define WGL_ACCELERATION_ARB            0x2003
#define WGL_DOUBLE_BUFFER_ARB           0x2011
#define WGL_STEREO_ARB                  0x2012
#define WGL_PIXEL_TYPE_ARB              0x2013
#define WGL_COLOR_BITS_ARB              0x2014
#define WGL_RED_BITS_ARB                0x2015
#define WGL_GREEN_BITS_ARB              0x2017
#define WGL_BLUE_BITS_ARB               0x2019
#define WGL_ALPHA_BITS_ARB              0x201B
#define WGL_ACCUM_BITS_ARB              0x201D
#define WGL_ACCUM_RED_BITS_ARB          0x201E
#define WGL_ACCUM_GREEN_BITS_ARB        0x201F
#define WGL_ACCUM_BLUE_BITS_ARB         0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB        0x2021
#define WGL_DEPTH_BITS_ARB              0x2022
#define WGL_STENCIL_BITS_ARB            0x2023
#define WGL_AUX_BUFFERS_ARB             0x2024
#define WGL_SAMPLE_BUFFERS_ARB          0x2041
#define WGL_SAMPLES_ARB                 0x2042

/* Constants for WGL_ACCELERATION_ARB */
#define WGL_NO_ACCELERATION_ARB         0x2025
#define WGL_GENERIC_ACCELERATION_ARB    0x2026
#define WGL_FULL_ACCELERATION_ARB       0x2027

/* Constants for WGL_PIXEL_TYPE_ARB */
#define WGL_TYPE_RGBA_ARB               0x202B
#define WGL_TYPE_COLORINDEX_ARB         0x202C

#endif /*WGL_ARB_pixel_format*/


#ifndef WGL_ARB_create_context

/* wglCreateContextAttribsARB */
typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC, HGLRC, const int* );

/* Tokens for wglCreateContextAttribsARB attributes */
#define WGL_CONTEXT_MAJOR_VERSION_ARB          0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB          0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB            0x2093
#define WGL_CONTEXT_FLAGS_ARB                  0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB           0x9126

/* Bits for WGL_CONTEXT_FLAGS_ARB */
#define WGL_CONTEXT_DEBUG_BIT_ARB              0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002

/* Bits for WGL_CONTEXT_PROFILE_MASK_ARB */
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#endif /*WGL_ARB_create_context*/


#ifndef GL_VERSION_3_0

typedef const GLubyte*  (APIENTRY *PFNGLGETSTRINGIPROC) (GLenum, GLuint);

#endif /*GL_VERSION_3_0*/


//========================================================================
// DLLs that are loaded at glfwInit()
//========================================================================

// gdi32.dll function pointer typedefs
#ifndef _GLFW_NO_DLOAD_GDI32
typedef int  (WINAPI * CHOOSEPIXELFORMAT_T) (HDC,CONST PIXELFORMATDESCRIPTOR*);
typedef int  (WINAPI * DESCRIBEPIXELFORMAT_T) (HDC,int,UINT,LPPIXELFORMATDESCRIPTOR);
typedef int  (WINAPI * GETPIXELFORMAT_T) (HDC);
typedef BOOL (WINAPI * SETPIXELFORMAT_T) (HDC,int,const PIXELFORMATDESCRIPTOR*);
typedef BOOL (WINAPI * SWAPBUFFERS_T) (HDC);
#endif // _GLFW_NO_DLOAD_GDI32

// winmm.dll function pointer typedefs
#ifndef _GLFW_NO_DLOAD_WINMM
typedef MMRESULT (WINAPI * JOYGETDEVCAPSA_T) (UINT,LPJOYCAPSA,UINT);
typedef MMRESULT (WINAPI * JOYGETPOS_T) (UINT,LPJOYINFO);
typedef MMRESULT (WINAPI * JOYGETPOSEX_T) (UINT,LPJOYINFOEX);
typedef DWORD (WINAPI * TIMEGETTIME_T) (void);
#endif // _GLFW_NO_DLOAD_WINMM


// gdi32.dll shortcuts
#ifndef _GLFW_NO_DLOAD_GDI32
#define _glfw_ChoosePixelFormat   _glfwLibrary.Libs.ChoosePixelFormat
#define _glfw_DescribePixelFormat _glfwLibrary.Libs.DescribePixelFormat
#define _glfw_GetPixelFormat      _glfwLibrary.Libs.GetPixelFormat
#define _glfw_SetPixelFormat      _glfwLibrary.Libs.SetPixelFormat
#define _glfw_SwapBuffers         _glfwLibrary.Libs.SwapBuffers
#else
#define _glfw_ChoosePixelFormat   ChoosePixelFormat
#define _glfw_DescribePixelFormat DescribePixelFormat
#define _glfw_GetPixelFormat      GetPixelFormat
#define _glfw_SetPixelFormat      SetPixelFormat
#define _glfw_SwapBuffers         SwapBuffers
#endif // _GLFW_NO_DLOAD_GDI32

// winmm.dll shortcuts
#ifndef _GLFW_NO_DLOAD_WINMM
#define _glfw_joyGetDevCaps _glfwLibrary.Libs.joyGetDevCapsA
#define _glfw_joyGetPos     _glfwLibrary.Libs.joyGetPos
#define _glfw_joyGetPosEx   _glfwLibrary.Libs.joyGetPosEx
#define _glfw_timeGetTime   _glfwLibrary.Libs.timeGetTime
#else
#define _glfw_joyGetDevCaps joyGetDevCapsA
#define _glfw_joyGetPos     joyGetPos
#define _glfw_joyGetPosEx   joyGetPosEx
#define _glfw_timeGetTime   timeGetTime
#endif // _GLFW_NO_DLOAD_WINMM


#define _GLFW_PLATFORM_WINDOW_STATE  _GLFWwindowWin32 Win32
#define _GLFW_PLATFORM_LIBRARY_STATE _GLFWlibraryWin32 Win32
#define _GLFW_PLATFORM_CONTEXT_STATE _GLFWcontextWGL WGL


//========================================================================
// GLFW platform specific types
//========================================================================

//------------------------------------------------------------------------
// Pointer length integer
//------------------------------------------------------------------------
typedef INT_PTR GLFWintptr;


//------------------------------------------------------------------------
// Platform-specific OpenGL context structure
//------------------------------------------------------------------------
typedef struct _GLFWcontextWGL
{
    // Platform specific window resources
    HDC       DC;              // Private GDI device context
    HGLRC     context;         // Permanent rendering context

    // Platform specific extensions (context specific)
    WGLSWAPINTERVALEXT_T           SwapIntervalEXT;
    WGLGETPIXELFORMATATTRIBIVARB_T GetPixelFormatAttribivARB;
    WGLGETEXTENSIONSSTRINGEXT_T    GetExtensionsStringEXT;
    WGLGETEXTENSIONSSTRINGARB_T    GetExtensionsStringARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB;
    GLboolean                      has_WGL_EXT_swap_control;
    GLboolean                      has_WGL_ARB_multisample;
    GLboolean                      has_WGL_ARB_pixel_format;
    GLboolean                      has_WGL_ARB_create_context;
} _GLFWcontextWGL;


//------------------------------------------------------------------------
// Platform-specific window structure
//------------------------------------------------------------------------
typedef struct _GLFWwindowWin32
{
    // Platform specific window resources
    HWND      handle;          // Window handle
    ATOM      classAtom;       // Window class atom
    int       modeID;          // Mode ID for fullscreen mode
    DWORD     dwStyle;         // Window styles used for window creation
    DWORD     dwExStyle;       // --"--

    // Various platform specific internal variables
    int       oldMouseLock;    // Old mouse-lock flag (used for remembering
                               // mouse-lock state when iconifying)
    int       oldMouseLockValid;
    int       desiredRefreshRate; // Desired vertical monitor refresh rate
    int       mouseMoved;
    int       oldMouseX, oldMouseY;
} _GLFWwindowWin32;


//------------------------------------------------------------------------
// Platform-specific library global data
//------------------------------------------------------------------------
typedef struct _GLFWlibraryWin32
{
    HINSTANCE instance;        // Instance of the application
    HHOOK     keyboardHook;    // Keyboard hook handle
    DWORD     foregroundLockTimeout;

    // Timer data
    struct {
        int          HasPerformanceCounter;
        double       Resolution;
        unsigned int t0_32;
        __int64      t0_64;
    } timer;

    // System information
    struct {
        int     winVer;
    } sys;

#if !defined(_GLFW_NO_DLOAD_WINMM) || !defined(_GLFW_NO_DLOAD_GDI32)
    // Library handles and function pointers
    struct {
#ifndef _GLFW_NO_DLOAD_GDI32
        // gdi32.dll
        HINSTANCE             gdi32;
        CHOOSEPIXELFORMAT_T   ChoosePixelFormat;
        DESCRIBEPIXELFORMAT_T DescribePixelFormat;
        GETPIXELFORMAT_T      GetPixelFormat;
        SETPIXELFORMAT_T      SetPixelFormat;
        SWAPBUFFERS_T         SwapBuffers;
#endif // _GLFW_NO_DLOAD_GDI32

      // winmm.dll
#ifndef _GLFW_NO_DLOAD_WINMM
        HINSTANCE             winmm;
        JOYGETDEVCAPSA_T      joyGetDevCapsA;
        JOYGETPOS_T           joyGetPos;
        JOYGETPOSEX_T         joyGetPosEx;
        TIMEGETTIME_T         timeGetTime;
#endif // _GLFW_NO_DLOAD_WINMM
    } libs;
#endif

} _GLFWlibraryWin32;


//========================================================================
// Various Windows version constants
//========================================================================

#define _GLFW_WIN_UNKNOWN    0x0000  // Earlier than 95 or NT4
#define _GLFW_WIN_95         0x0001
#define _GLFW_WIN_98         0x0002
#define _GLFW_WIN_ME         0x0003
#define _GLFW_WIN_UNKNOWN_9x 0x0004  // Later than ME
#define _GLFW_WIN_NT4        0x0101
#define _GLFW_WIN_2K         0x0102
#define _GLFW_WIN_XP         0x0103
#define _GLFW_WIN_NET_SERVER 0x0104
#define _GLFW_WIN_UNKNOWN_NT 0x0105  // Later than .NET Server


//========================================================================
// Prototypes for platform specific internal functions
//========================================================================

// Time
void _glfwInitTimer(void);

// Fullscreen support
int _glfwGetClosestVideoModeBPP(int* w, int* h, int* bpp, int* refresh);
int _glfwGetClosestVideoMode(int* w, int* h, int* r, int* g, int* b, int* refresh);
void _glfwSetVideoModeMODE(int mode);
void _glfwSetVideoMode(int* w, int* h, int r, int g, int b, int refresh);


#endif // _platform_h_
