//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2017 Camilla Löwy <elmindreda@glfw.org>
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

#pragma once

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/timer.h>
#include <proto/keymap.h>
#include <proto/utility.h>
#include <proto/icon.h>
#include <proto/wb.h>
#include <proto/requester.h>
#include <proto/textclip.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>

#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>

#include <graphics/displayinfo.h>
#include <graphics/rastport.h>

#include <devices/timer.h>
#include <devices/keymap.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/gameport.h>

#include <classes/requester.h>

#include <amigainput/amigainput.h>
#include <proto/amigainput.h>

#include "os4_joystick.h"

#define MIN_MINIGLVERSION 2
#define MIN_OGLES2_VERSION 0

#define GLFW_OS4_WINDOW_STATE             _GLFWwindowOS4   os4;
#define GLFW_OS4_LIBRARY_WINDOW_STATE     _GLFWlibraryOS4  os4;
#define GLFW_OS4_MONITOR_STATE            _GLFWmonitorOS4  os4;
#define GLFW_OS4_CONTEXT_STATE            _GLFWcontextGL   gl;
#define GLFW_OS4_JOYSTICK_STATE           _GLFWjoystickOS4 os4js;
#define GLFW_OS4_LIBRARY_JOYSTICK_STATE   _GLFWjoystickOS4 os4js;
#define GLFW_OS4_CURSOR_STATE             _GLFWcursorOS4   os4;
#define GLFW_OS4_LIBRARY_CONTEXT_STATE

#define GID_ICONIFY 123

struct MyIntuiMessage
{
    uint32 Class;
    uint16 Code;
    uint16 Qualifier;

    struct Gadget *Gadget;
    struct Window *IDCMPWindow;

    int16  RelativeMouseX;
    int16  RelativeMouseY;

    int16  WindowMouseX; // Absolute pointer position, relative to
    int16  WindowMouseY; // top-left corner of inner window

    int16  ScreenMouseX;
    int16  ScreenMouseY;

    int16  Width; // Inner window dimensions
    int16  Height;
};

typedef struct {
    ULONG					modeid;
    long                    depth;
    LONG					x;
    LONG					y;
} GLFW_DisplayModeData;

// OS4-specific per-window data
//
typedef struct _GLFWwindowOS4
{
    struct Window    *handle;
    int               xpos;
    int               ypos;
    int               oldxpos;
    int               oldypos;
    int               lastCursorPosX;
    int               lastCursorPosY;
    _GLFWcursor      *currentCursor;
    int               width;
    int               height;
    char             *title;
    GLFWbool          visible;
    GLFWbool          iconified;
    GLFWbool          maximized;
    GLFWbool          resizable;
    GLFWbool          decorated;
    GLFWbool          floating;
    GLFWbool          transparent;
    GLFWbool          fullscreen;
    float             opacity;
    int               windowType; // NORMAL - GL - GLES

    struct AppIcon   *appIcon;
    struct AppWindow *appWin;
    struct Screen    *screen;

    struct Gadget    *gadget;
    struct Image     *image;

} _GLFWwindowOS4;

typedef struct _GLFWcontextGL {
    struct BitMap  *bm;
    void*           glContext;
    BOOL            vsyncEnabled;
} _GLFWcontextGL;

// OS4-specific per-monitor data
//
typedef struct _GLFWmonitorOS4
{
    GLFWgammaramp   ramp;
} _GLFWmonitorOS4;

// OS4-specific per-cursor data
//
typedef struct _GLFWcursorOS4
{
    Object *handle;
    Object *currentHandle;
    uint32_t *imageData;
    int id;
} _GLFWcursorOS4;

// OS4-specific global data
//
typedef struct _GLFWlibraryOS4 {
    STRPTR           appName;

    int              xcursor;
    int              ycursor;

    char             keynames[GLFW_KEY_LAST + 1][17];
    short int        keycodes[512];
    short int        scancodes[GLFW_KEY_LAST + 1];

    char            *clipboardString;
    struct MsgPort  *appMsgPort;
    struct MsgPort  *userPort;

    struct Screen   *publicScreen;

    _GLFWwindow     *focusedWindow;
    // The window whose disabled cursor mode is active
    _GLFWwindow     *disabledCursorWindow;
} _GLFWlibraryOS4;

// OS4-specific joystick data
//
typedef struct _GLFWjoystickOS4 {
    struct joystick_hwdata *hwdata;
    uint32          joystickCount;
    struct joystick joystickList [MAX_JOYSTICKS];
    APTR            joystickContext;
    uint32          instance_id;
} _GLFWjoystickOS4;

void _glfwPollMonitorsOS4(void);

GLFWbool _glfwConnectOS4(int platformID, _GLFWplatform* platform);
int _glfwInitOS4(void);
void _glfwTerminateOS4(void);

void _glfwFreeMonitorOS4(_GLFWmonitor* monitor);
void _glfwGetMonitorPosOS4(_GLFWmonitor* monitor, int* xpos, int* ypos);
void _glfwGetMonitorContentScaleOS4(_GLFWmonitor* monitor, float* xscale, float* yscale);
void _glfwGetMonitorWorkareaOS4(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
GLFWvidmode* _glfwGetVideoModesOS4(_GLFWmonitor* monitor, int* found);
void _glfwGetVideoModeOS4(_GLFWmonitor* monitor, GLFWvidmode* mode);
GLFWbool _glfwGetGammaRampOS4(_GLFWmonitor* monitor, GLFWgammaramp* ramp);
void _glfwSetGammaRampOS4(_GLFWmonitor* monitor, const GLFWgammaramp* ramp);

int _glfwCreateWindowOS4(_GLFWwindow* window, const _GLFWwndconfig* wndconfig, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig);
void _glfwDestroyWindowOS4(_GLFWwindow* window);
void _glfwSetWindowTitleOS4(_GLFWwindow* window, const char* title);
void _glfwSetWindowIconOS4(_GLFWwindow* window, int count, const GLFWimage* images);
void _glfwSetWindowMonitorOS4(_GLFWwindow* window, _GLFWmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
void _glfwGetWindowPosOS4(_GLFWwindow* window, int* xpos, int* ypos);
void _glfwSetWindowPosOS4(_GLFWwindow* window, int xpos, int ypos);
void _glfwGetWindowSizeOS4(_GLFWwindow* window, int* width, int* height);
void _glfwSetWindowSizeOS4(_GLFWwindow* window, int width, int height);
void _glfwSetWindowSizeLimitsOS4(_GLFWwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _glfwSetWindowAspectRatioOS4(_GLFWwindow* window, int n, int d);
void _glfwGetFramebufferSizeOS4(_GLFWwindow* window, int* width, int* height);
void _glfwGetWindowFrameSizeOS4(_GLFWwindow* window, int* left, int* top, int* right, int* bottom);
void _glfwGetWindowContentScaleOS4(_GLFWwindow* window, float* xscale, float* yscale);
void _glfwIconifyWindowOS4(_GLFWwindow* window);
void _glfwRestoreWindowOS4(_GLFWwindow* window);
void _glfwMaximizeWindowOS4(_GLFWwindow* window);
int _glfwWindowMaximizedOS4(_GLFWwindow* window);
int _glfwWindowHoveredOS4(_GLFWwindow* window);
int _glfwFramebufferTransparentOS4(_GLFWwindow* window);
void _glfwSetWindowResizableOS4(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowDecoratedOS4(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowFloatingOS4(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowMousePassthroughOS4(_GLFWwindow* window, GLFWbool enabled);
float _glfwGetWindowOpacityOS4(_GLFWwindow* window);
void _glfwSetWindowOpacityOS4(_GLFWwindow* window, float opacity);
void _glfwSetRawMouseMotionOS4(_GLFWwindow *window, GLFWbool enabled);
GLFWbool _glfwRawMouseMotionSupportedOS4(void);
void _glfwShowWindowOS4(_GLFWwindow* window);
void _glfwRequestWindowAttentionOS4(_GLFWwindow* window);
void _glfwRequestWindowAttentionOS4(_GLFWwindow* window);
void _glfwHideWindowOS4(_GLFWwindow* window);
void _glfwFocusWindowOS4(_GLFWwindow* window);
int _glfwWindowFocusedOS4(_GLFWwindow* window);
int _glfwWindowIconifiedOS4(_GLFWwindow* window);
int _glfwWindowVisibleOS4(_GLFWwindow* window);
void _glfwPollEventsOS4(void);
void _glfwWaitEventsOS4(void);
void _glfwWaitEventsTimeoutOS4(double timeout);
void _glfwPostEmptyEventOS4(void);
void _glfwGetCursorPosOS4(_GLFWwindow* window, double* xpos, double* ypos);
void _glfwSetCursorPosOS4(_GLFWwindow* window, double x, double y);
void _glfwSetCursorModeOS4(_GLFWwindow* window, int mode);
int _glfwCreateCursorOS4(_GLFWcursor* cursor, const GLFWimage* image, int xhot, int yhot);
int _glfwCreateStandardCursorOS4(_GLFWcursor* cursor, int shape);
void _glfwDestroyCursorOS4(_GLFWcursor* cursor);
void _glfwSetCursorOS4(_GLFWwindow* window, _GLFWcursor* cursor);
void _glfwSetClipboardStringOS4(const char* string);
const char* _glfwGetClipboardStringOS4(void);
const char* _glfwGetScancodeNameOS4(int scancode);
int _glfwGetKeyScancodeOS4(int key);

void _glfwGetRequiredInstanceExtensionsOS4(char** extensions);
int _glfwGetPhysicalDevicePresentationSupportOS4(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
VkResult _glfwCreateWindowSurfaceOS4(VkInstance instance, _GLFWwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);
GLFWbool _glfwCreateContextGL(_GLFWwindow* window, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig);

EGLenum _glfwGetEGLPlatformOS4(EGLint** attribs);
EGLNativeDisplayType _glfwGetEGLNativeDisplayOS4(void);
EGLNativeWindowType _glfwGetEGLNativeWindowOS4(_GLFWwindow* window);

void _glfwPollMonitorsOS4(void);

GLFWbool _glfwInitJoysticksOS4(void);
void _glfwTerminateJoysticksOS4(void);
int _glfwPollJoystickOS4(_GLFWjoystick* js, int mode);
const char* _glfwGetMappingNameOS4(void);
void _glfwUpdateGamepadGUIDOS4(char* guid);
/************************************************************************************/
/********************************* AmigaOS4 METHODS *********************************/
/************************************************************************************/
//#define DEBUG

#ifndef DEBUG
# define dprintf(format, args...)
# define kprintf(format, args...)
#else /* DEBUG */
# define dprintf(format, args...) IExec->DebugPrintF("[%s] " format, __PRETTY_FUNCTION__ , ## args)
//# define dprintf(format, args...)((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF("[%s] " format, __PRETTY_FUNCTION__ , ## args)
# define kprintf(format, args...)((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(format, ## args)
#endif /* DEBUG */

BOOL OS4_LockPubScreen();
void OS4_UnlockPubScreen();
void OS4_IconifyWindow(_GLFWwindow *window);
void OS4_UniconifyWindow(_GLFWwindow* window);
