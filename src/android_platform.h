//========================================================================
// GLFW 3.5 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2024 kunitoki <kunitoki@gmail.com>
// Copyright (c) 2017 Curi0 <curi0minecraft@gmail.com>
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

#include <android_native_app_glue.h>
#include <android/native_window.h>
#include <android/log.h>

#define GLFW_ANDROID_WINDOW_STATE struct android_app* android;
#define GLFW_ANDROID_LIBRARY_WINDOW_STATE android_gstate gstate;
#define GLFW_ANDROID_MONITOR_STATE

#define GLFW_ANDROID_CONTEXT_STATE
#define GLFW_ANDROID_CURSOR_STATE
#define GLFW_ANDROID_LIBRARY_CONTEXT_STATE

#define GLFW_ANDROID_JOYSTICK_STATE
#define GLFW_ANDROID_LIBRARY_JOYSTICK_STATE

typedef struct android_gstate
{
    struct android_app* app;
    struct android_poll_source* source;
} android_gstate;

typedef VkFlags VkAndroidSurfaceCreateFlagsKHR;

typedef struct VkAndroidSurfaceCreateInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    VkAndroidSurfaceCreateFlagsKHR flags;
    ANativeWindow* window;
} VkAndroidSurfaceCreateInfoKHR;

typedef VkResult (APIENTRY *PFN_vkCreateAndroidSurfaceKHR)(VkInstance, const VkAndroidSurfaceCreateInfoKHR*, const VkAllocationCallbacks*, VkSurfaceKHR*);

GLFWbool _glfwConnectAndroid(int platformID, _GLFWplatform* platform);
int _glfwInitAndroid(void);
void _glfwTerminateAndroid(void);

void _glfwFreeMonitorAndroid(_GLFWmonitor* monitor);
void _glfwGetMonitorPosAndroid(_GLFWmonitor* monitor, int* xpos, int* ypos);
void _glfwGetMonitorContentScaleAndroid(_GLFWmonitor* monitor, float* xscale, float* yscale);
void _glfwGetMonitorWorkareaAndroid(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
GLFWvidmode* _glfwGetVideoModesAndroid(_GLFWmonitor* monitor, int* found);
GLFWbool _glfwGetVideoModeAndroid(_GLFWmonitor* monitor, GLFWvidmode* mode);
GLFWbool _glfwGetGammaRampAndroid(_GLFWmonitor* monitor, GLFWgammaramp* ramp);
void _glfwSetGammaRampAndroid(_GLFWmonitor* monitor, const GLFWgammaramp* ramp);
void _glfwPollMonitorsAndroid(void);

GLFWbool _glfwCreateWindowAndroid(_GLFWwindow* window, const _GLFWwndconfig* wndconfig, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig);
void _glfwDestroyWindowAndroid(_GLFWwindow* window);
void _glfwSetWindowTitleAndroid(_GLFWwindow* window, const char* title);
void _glfwSetWindowIconAndroid(_GLFWwindow* window, int count, const GLFWimage* images);
void _glfwSetWindowMonitorAndroid(_GLFWwindow* window, _GLFWmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
void _glfwGetWindowPosAndroid(_GLFWwindow* window, int* xpos, int* ypos);
void _glfwSetWindowPosAndroid(_GLFWwindow* window, int xpos, int ypos);
void _glfwGetWindowSizeAndroid(_GLFWwindow* window, int* width, int* height);
void _glfwSetWindowSizeAndroid(_GLFWwindow* window, int width, int height);
void _glfwSetWindowSizeLimitsAndroid(_GLFWwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _glfwSetWindowAspectRatioAndroid(_GLFWwindow* window, int n, int d);
void _glfwGetFramebufferSizeAndroid(_GLFWwindow* window, int* width, int* height);
void _glfwGetWindowFrameSizeAndroid(_GLFWwindow* window, int* left, int* top, int* right, int* bottom);
void _glfwGetWindowContentScaleAndroid(_GLFWwindow* window, float* xscale, float* yscale);
void _glfwIconifyWindowAndroid(_GLFWwindow* window);
void _glfwRestoreWindowAndroid(_GLFWwindow* window);
void _glfwMaximizeWindowAndroid(_GLFWwindow* window);
GLFWbool _glfwWindowMaximizedAndroid(_GLFWwindow* window);
GLFWbool _glfwWindowHoveredAndroid(_GLFWwindow* window);
GLFWbool _glfwFramebufferTransparentAndroid(_GLFWwindow* window);
void _glfwSetWindowResizableAndroid(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowDecoratedAndroid(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowFloatingAndroid(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowMousePassthroughAndroid(_GLFWwindow* window, GLFWbool enabled);
float _glfwGetWindowOpacityAndroid(_GLFWwindow* window);
void _glfwSetWindowOpacityAndroid(_GLFWwindow* window, float opacity);
void _glfwSetRawMouseMotionAndroid(_GLFWwindow *window, GLFWbool enabled);
GLFWbool _glfwRawMouseMotionSupportedAndroid(void);
void _glfwShowWindowAndroid(_GLFWwindow* window);
void _glfwRequestWindowAttentionAndroid(_GLFWwindow* window);
void _glfwHideWindowAndroid(_GLFWwindow* window);
void _glfwFocusWindowAndroid(_GLFWwindow* window);
GLFWbool _glfwWindowFocusedAndroid(_GLFWwindow* window);
GLFWbool _glfwWindowIconifiedAndroid(_GLFWwindow* window);
GLFWbool _glfwWindowVisibleAndroid(_GLFWwindow* window);
void _glfwPollEventsAndroid(void);
void _glfwWaitEventsAndroid(void);
void _glfwWaitEventsTimeoutAndroid(double timeout);
void _glfwPostEmptyEventAndroid(void);
void _glfwGetCursorPosAndroid(_GLFWwindow* window, double* xpos, double* ypos);
void _glfwSetCursorPosAndroid(_GLFWwindow* window, double x, double y);
void _glfwSetCursorModeAndroid(_GLFWwindow* window, int mode);
GLFWbool _glfwCreateCursorAndroid(_GLFWcursor* cursor, const GLFWimage* image, int xhot, int yhot);
GLFWbool _glfwCreateStandardCursorAndroid(_GLFWcursor* cursor, int shape);
void _glfwDestroyCursorAndroid(_GLFWcursor* cursor);
void _glfwSetCursorAndroid(_GLFWwindow* window, _GLFWcursor* cursor);
void _glfwSetClipboardStringAndroid(const char* string);
const char* _glfwGetClipboardStringAndroid(void);
const char* _glfwGetScancodeNameAndroid(int scancode);
int _glfwGetKeyScancodeAndroid(int key);

EGLenum _glfwGetEGLPlatformAndroid(EGLint** attribs);
EGLNativeDisplayType _glfwGetEGLNativeDisplayAndroid(void);
EGLNativeWindowType _glfwGetEGLNativeWindowAndroid(_GLFWwindow* window);

void _glfwGetRequiredInstanceExtensionsAndroid(char** extensions);
GLFWbool _glfwGetPhysicalDevicePresentationSupportAndroid(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
VkResult _glfwCreateWindowSurfaceAndroid(VkInstance instance, _GLFWwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

GLFWAPI struct android_app* glfwGetAndroidApp(void);
