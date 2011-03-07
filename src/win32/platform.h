//========================================================================
// GLFW - An OpenGL library
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


// We don't need all the fancy stuff
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windows.h>
#include <mmsystem.h>

#include "../../include/GL/wglext.h"


//========================================================================
// Hack: Define things that some windows.h variants don't
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
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
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
typedef BOOL (WINAPI * GETDEVICEGAMMARAMP_T) (HDC,PVOID);
typedef BOOL (WINAPI * SETDEVICEGAMMARAMP_T) (HDC,PVOID);
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
#define _glfw_ChoosePixelFormat   _glfwLibrary.Win32.gdi.ChoosePixelFormat
#define _glfw_DescribePixelFormat _glfwLibrary.Win32.gdi.DescribePixelFormat
#define _glfw_GetPixelFormat      _glfwLibrary.Win32.gdi.GetPixelFormat
#define _glfw_SetPixelFormat      _glfwLibrary.Win32.gdi.SetPixelFormat
#define _glfw_SwapBuffers         _glfwLibrary.Win32.gdi.SwapBuffers
#define _glfw_GetDeviceGammaRamp  _glfwLibrary.Win32.gdi.GetDeviceGammaRamp
#define _glfw_SetDeviceGammaRamp  _glfwLibrary.Win32.gdi.SetDeviceGammaRamp
#else
#define _glfw_ChoosePixelFormat   ChoosePixelFormat
#define _glfw_DescribePixelFormat DescribePixelFormat
#define _glfw_GetPixelFormat      GetPixelFormat
#define _glfw_SetPixelFormat      SetPixelFormat
#define _glfw_SwapBuffers         SwapBuffers
#define _glfw_GetDeviceGammaRamp  GetDeviceGammaRamp
#define _glfw_SetDeviceGammaRamp  SetDeviceGammaRamp
#endif // _GLFW_NO_DLOAD_GDI32

// winmm.dll shortcuts
#ifndef _GLFW_NO_DLOAD_WINMM
#define _glfw_joyGetDevCaps _glfwLibrary.Win32.winmm.joyGetDevCapsA
#define _glfw_joyGetPos     _glfwLibrary.Win32.winmm.joyGetPos
#define _glfw_joyGetPosEx   _glfwLibrary.Win32.winmm.joyGetPosEx
#define _glfw_timeGetTime   _glfwLibrary.Win32.winmm.timeGetTime
#else
#define _glfw_joyGetDevCaps joyGetDevCapsA
#define _glfw_joyGetPos     joyGetPos
#define _glfw_joyGetPosEx   joyGetPosEx
#define _glfw_timeGetTime   timeGetTime
#endif // _GLFW_NO_DLOAD_WINMM


// We use versioned window class names in order not to cause conflicts
// between applications using different versions of GLFW
#define _GLFW_WNDCLASSNAME "GLFW30"


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
    PFNWGLSWAPINTERVALEXTPROC           SwapIntervalEXT;
    PFNWGLGETPIXELFORMATATTRIBIVARBPROC GetPixelFormatAttribivARB;
    PFNWGLGETEXTENSIONSSTRINGEXTPROC    GetExtensionsStringEXT;
    PFNWGLGETEXTENSIONSSTRINGARBPROC    GetExtensionsStringARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC   CreateContextAttribsARB;
    GLboolean                           has_WGL_EXT_swap_control;
    GLboolean                           has_WGL_ARB_multisample;
    GLboolean                           has_WGL_ARB_pixel_format;
    GLboolean                           has_WGL_ARB_create_context;
    GLboolean                           has_WGL_ARB_create_context_profile;
    GLboolean                           has_WGL_EXT_create_context_es2_profile;
    GLboolean                           has_WGL_ARB_create_context_robustness;
} _GLFWcontextWGL;


//------------------------------------------------------------------------
// Platform-specific window structure
//------------------------------------------------------------------------
typedef struct _GLFWwindowWin32
{
    // Platform specific window resources
    HWND      handle;          // Window handle
    DWORD     dwStyle;         // Window styles used for window creation
    DWORD     dwExStyle;       // --"--

    // Various platform specific internal variables
    int       desiredRefreshRate; // Desired vertical monitor refresh rate
    GLboolean mouseMoved;
    int       oldMouseX, oldMouseY;
} _GLFWwindowWin32;


//------------------------------------------------------------------------
// Platform-specific library global data
//------------------------------------------------------------------------
typedef struct _GLFWlibraryWin32
{
    HINSTANCE                 instance;     // Instance of the application
    ATOM                      classAtom;    // Window class atom
    HHOOK                     keyboardHook; // Keyboard hook handle
    DWORD                     foregroundLockTimeout;

    // Default monitor
    struct {
        GLboolean             modeChanged;
        int                   width;
        int                   height;
        int                   bitsPerPixel;
        int                   refreshRate;
    } monitor;

    // Timer data
    struct {
        GLboolean             hasPerformanceCounter;
        double                resolution;
        unsigned int          t0_32;
        __int64               t0_64;
    } timer;

#ifndef _GLFW_NO_DLOAD_GDI32
    // gdi32.dll
    struct {
        HINSTANCE             instance;
        CHOOSEPIXELFORMAT_T   ChoosePixelFormat;
        DESCRIBEPIXELFORMAT_T DescribePixelFormat;
        GETPIXELFORMAT_T      GetPixelFormat;
        SETPIXELFORMAT_T      SetPixelFormat;
        SWAPBUFFERS_T         SwapBuffers;
        GETDEVICEGAMMARAMP_T  GetDeviceGammaRamp;
        SETDEVICEGAMMARAMP_T  SetDeviceGammaRamp;
    } gdi;
#endif // _GLFW_NO_DLOAD_GDI32

#ifndef _GLFW_NO_DLOAD_WINMM
    // winmm.dll
    struct {
        HINSTANCE             instance;
        JOYGETDEVCAPSA_T      joyGetDevCapsA;
        JOYGETPOS_T           joyGetPos;
        JOYGETPOSEX_T         joyGetPosEx;
        TIMEGETTIME_T         timeGetTime;
    } winmm;
#endif // _GLFW_NO_DLOAD_WINMM

} _GLFWlibraryWin32;


//========================================================================
// Prototypes for platform specific internal functions
//========================================================================

// Time
void _glfwInitTimer(void);

// Fullscreen support
void _glfwSetVideoMode(int* width, int* height,
                       int* bpp, int* refreshRate,
                       GLboolean exactBPP);
void _glfwRestoreVideoMode(void);


#endif // _platform_h_
