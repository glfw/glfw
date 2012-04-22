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
#ifndef NOMINMAX
 #define NOMINMAX
#endif

#ifndef VC_EXTRALEAN
 #define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN
#endif

// This is a workaround for the fact that glfw3.h needs to export APIENTRY (to
// correctly declare a GL_ARB_debug_output callback, for example) but windows.h
// thinks it is the only one that gets to do so
#undef APIENTRY

// GLFW on Windows is Unicode only and does not work in MBCS mode
#ifndef UNICODE
 #define UNICODE
#endif

// GLFW requires Windows XP
#ifndef WINVER
 #define WINVER 0x0501
#endif

#include <windows.h>
#include <mmsystem.h>

// This path may need to be changed if you build GLFW using your own setup
// We ship and use our own copy of wglext.h since GLFW uses fairly new
// extensions and not all operating systems come with an up-to-date version
#include "../support/GL/wglext.h"


//========================================================================
// Hack: Define things that some windows.h variants don't
//========================================================================

#ifndef WM_MOUSEHWHEEL
 #define WM_MOUSEHWHEEL 0x020E
#endif


//========================================================================
// DLLs that are loaded at glfwInit()
//========================================================================

// winmm.dll function pointer typedefs
#ifndef _GLFW_NO_DLOAD_WINMM
typedef MMRESULT (WINAPI * JOYGETDEVCAPS_T) (UINT,LPJOYCAPS,UINT);
typedef MMRESULT (WINAPI * JOYGETPOS_T) (UINT,LPJOYINFO);
typedef MMRESULT (WINAPI * JOYGETPOSEX_T) (UINT,LPJOYINFOEX);
typedef DWORD (WINAPI * TIMEGETTIME_T) (void);
#endif // _GLFW_NO_DLOAD_WINMM


// winmm.dll shortcuts
#ifndef _GLFW_NO_DLOAD_WINMM
 #define _glfw_joyGetDevCaps _glfwLibrary.Win32.winmm.joyGetDevCaps
 #define _glfw_joyGetPos     _glfwLibrary.Win32.winmm.joyGetPos
 #define _glfw_joyGetPosEx   _glfwLibrary.Win32.winmm.joyGetPosEx
 #define _glfw_timeGetTime   _glfwLibrary.Win32.winmm.timeGetTime
#else
 #define _glfw_joyGetDevCaps joyGetDevCaps
 #define _glfw_joyGetPos     joyGetPos
 #define _glfw_joyGetPosEx   joyGetPosEx
 #define _glfw_timeGetTime   timeGetTime
#endif // _GLFW_NO_DLOAD_WINMM


// We use versioned window class names in order not to cause conflicts
// between applications using different versions of GLFW
#define _GLFW_WNDCLASSNAME L"GLFW30"


#define _GLFW_PLATFORM_WINDOW_STATE  _GLFWwindowWin32 Win32
#define _GLFW_PLATFORM_CONTEXT_STATE _GLFWcontextWGL WGL
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryWin32 Win32
#define _GLFW_PLATFORM_LIBRARY_OPENGL_STATE _GLFWlibraryWGL WGL


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
    GLboolean                           EXT_swap_control;
    GLboolean                           ARB_multisample;
    GLboolean                           ARB_pixel_format;
    GLboolean                           ARB_create_context;
    GLboolean                           ARB_create_context_profile;
    GLboolean                           EXT_create_context_es2_profile;
    GLboolean                           ARB_create_context_robustness;
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
    GLboolean cursorCentered;
    GLboolean cursorInside;
    int       oldMouseX, oldMouseY;
} _GLFWwindowWin32;


//------------------------------------------------------------------------
// Platform-specific library global data for Win32
//------------------------------------------------------------------------
typedef struct _GLFWlibraryWin32
{
    HINSTANCE                 instance;     // Instance of the application
    ATOM                      classAtom;    // Window class atom
    HHOOK                     keyboardHook; // Keyboard hook handle
    DWORD                     foregroundLockTimeout;
    char*                     clipboardString;

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

#ifndef _GLFW_NO_DLOAD_WINMM
    // winmm.dll
    struct {
        HINSTANCE             instance;
        JOYGETDEVCAPS_T       joyGetDevCaps;
        JOYGETPOS_T           joyGetPos;
        JOYGETPOSEX_T         joyGetPosEx;
        TIMEGETTIME_T         timeGetTime;
    } winmm;
#endif // _GLFW_NO_DLOAD_WINMM

} _GLFWlibraryWin32;


//------------------------------------------------------------------------
// Platform-specific library global data for WGL
//------------------------------------------------------------------------
typedef struct _GLFWlibraryWGL
{
    int dummy;

} _GLFWlibraryWGL;


//========================================================================
// Prototypes for platform specific internal functions
//========================================================================

// Wide strings
WCHAR* _glfwCreateWideStringFromUTF8(const char* source);
char* _glfwCreateUTF8FromWideString(const WCHAR* source);

// Time
void _glfwInitTimer(void);

// Fullscreen support
void _glfwSetVideoMode(int* width, int* height,
                       int* bpp, int* refreshRate,
                       GLboolean exactBPP);
void _glfwRestoreVideoMode(void);


#endif // _platform_h_
