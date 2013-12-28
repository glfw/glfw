//========================================================================
// GLFW 3.0 Win32 - www.glfw.org
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

#ifndef _win32_platform_h_
#define _win32_platform_h_


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

// GLFW requires Windows XP or later
#if WINVER < 0x0501
 #undef WINVER
 #define WINVER 0x0501
#endif
#if _WIN32_WINNT < 0x0501
 #undef _WIN32_WINNT
 #define _WIN32_WINNT 0x0501
#endif

#include <windows.h>
#include <mmsystem.h>
#include <dbt.h>


//========================================================================
// Hack: Define things that some windows.h variants don't
//========================================================================

#ifndef WM_MOUSEHWHEEL
 #define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef WM_DWMCOMPOSITIONCHANGED
 #define WM_DWMCOMPOSITIONCHANGED 0x031E
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
 #define _glfw_joyGetDevCaps _glfw.win32.winmm.joyGetDevCaps
 #define _glfw_joyGetPos     _glfw.win32.winmm.joyGetPos
 #define _glfw_joyGetPosEx   _glfw.win32.winmm.joyGetPosEx
 #define _glfw_timeGetTime   _glfw.win32.winmm.timeGetTime
#else
 #define _glfw_joyGetDevCaps joyGetDevCaps
 #define _glfw_joyGetPos     joyGetPos
 #define _glfw_joyGetPosEx   joyGetPosEx
 #define _glfw_timeGetTime   timeGetTime
#endif // _GLFW_NO_DLOAD_WINMM

// user32.dll function pointer typedefs
typedef BOOL (WINAPI * SETPROCESSDPIAWARE_T)(void);
#define _glfw_SetProcessDPIAware _glfw.win32.user32.SetProcessDPIAware

// dwmapi.dll function pointer typedefs
typedef HRESULT (WINAPI * DWMISCOMPOSITIONENABLED_T)(BOOL*);
#define _glfw_DwmIsCompositionEnabled _glfw.win32.dwmapi.DwmIsCompositionEnabled


// We use versioned window class names in order not to cause conflicts
// between applications using different versions of GLFW
#define _GLFW_WNDCLASSNAME L"GLFW30"

#define _GLFW_RECREATION_NOT_NEEDED 0
#define _GLFW_RECREATION_REQUIRED   1
#define _GLFW_RECREATION_IMPOSSIBLE 2


#if defined(_GLFW_WGL)
 #include "wgl_platform.h"
#elif defined(_GLFW_EGL)
 #define _GLFW_EGL_NATIVE_WINDOW  window->win32.handle
 #define _GLFW_EGL_NATIVE_DISPLAY EGL_DEFAULT_DISPLAY
 #include "egl_platform.h"
#else
 #error "No supported context creation API selected"
#endif

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowWin32  win32
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryWin32 win32
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorWin32 win32


//========================================================================
// GLFW platform specific types
//========================================================================


//------------------------------------------------------------------------
// Platform-specific window structure
//------------------------------------------------------------------------
typedef struct _GLFWwindowWin32
{
    // Platform specific window resources
    HWND                handle;    // Window handle
    DWORD               dwStyle;   // Window styles used for window creation
    DWORD               dwExStyle; // --"--

    // Various platform specific internal variables
    GLboolean           cursorCentered;
    GLboolean           cursorInside;
    GLboolean           cursorHidden;
    int                 oldCursorX, oldCursorY;
} _GLFWwindowWin32;


//------------------------------------------------------------------------
// Platform-specific library global data for Win32
//------------------------------------------------------------------------
typedef struct _GLFWlibraryWin32
{
    ATOM                classAtom;
    DWORD               foregroundLockTimeout;
    char*               clipboardString;

    // Timer data
    struct {
        GLboolean       hasPC;
        double          resolution;
        unsigned __int64 base;
    } timer;

#ifndef _GLFW_NO_DLOAD_WINMM
    // winmm.dll
    struct {
        HINSTANCE       instance;
        JOYGETDEVCAPS_T joyGetDevCaps;
        JOYGETPOS_T     joyGetPos;
        JOYGETPOSEX_T   joyGetPosEx;
        TIMEGETTIME_T   timeGetTime;
    } winmm;
#endif // _GLFW_NO_DLOAD_WINMM

    // user32.dll
    struct {
        HINSTANCE       instance;
        SETPROCESSDPIAWARE_T SetProcessDPIAware;
    } user32;

    // dwmapi.dll
    struct {
        HINSTANCE       instance;
        DWMISCOMPOSITIONENABLED_T DwmIsCompositionEnabled;
    } dwmapi;

    struct {
        float           axes[6];
        unsigned char   buttons[36]; // 32 buttons plus one hat
        char*           name;
    } joystick[GLFW_JOYSTICK_LAST + 1];

} _GLFWlibraryWin32;


//------------------------------------------------------------------------
// Platform-specific monitor structure
//------------------------------------------------------------------------
typedef struct _GLFWmonitorWin32
{
    // This size matches the static size of DISPLAY_DEVICE.DeviceName
    WCHAR               name[32];

} _GLFWmonitorWin32;


//========================================================================
// Prototypes for platform specific internal functions
//========================================================================

// Desktop compositing
BOOL _glfwIsCompositionEnabled(void);

// Wide strings
WCHAR* _glfwCreateWideStringFromUTF8(const char* source);
char* _glfwCreateUTF8FromWideString(const WCHAR* source);

// Time
void _glfwInitTimer(void);

// Joystick input
void _glfwInitJoysticks(void);
void _glfwTerminateJoysticks(void);

// OpenGL support
int _glfwInitContextAPI(void);
void _glfwTerminateContextAPI(void);
int _glfwCreateContext(_GLFWwindow* window,
                       const _GLFWwndconfig* wndconfig,
                       const _GLFWfbconfig* fbconfig);
void _glfwDestroyContext(_GLFWwindow* window);
int _glfwAnalyzeContext(const _GLFWwindow* window,
                        const _GLFWwndconfig* wndconfig,
                        const _GLFWfbconfig* fbconfig);

// Fullscreen support
GLboolean _glfwSetVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* desired);
void _glfwRestoreVideoMode(_GLFWmonitor* monitor);


#endif // _win32_platform_h_
