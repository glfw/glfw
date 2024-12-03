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

#include "internal.h"

#include <jni.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <android/configuration.h>
#include <android/window.h>
#include <android/input.h>

#include <string.h>

static float lastCursorPosX = 0.0f;
static float lastCursorPosY = 0.0f;

static void moveNativeWindowToBackground(ANativeActivity* nativeActivity)
{
    JNIEnv* env = NULL;
    (*nativeActivity->vm)->AttachCurrentThread(nativeActivity->vm, &env, NULL);

    jmethodID moveTaskToBackMethod = (*env)->GetMethodID(env, nativeActivity->clazz, "moveTaskToBack", "(Z)Z");
    if (moveTaskToBackMethod == NULL)
        return;

    (*env)->CallBooleanMethod(env, nativeActivity->clazz, moveTaskToBackMethod, JNI_TRUE);
}

static int32_t handleInput(struct android_app* app, AInputEvent* event)
{
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
        size_t pointerCount = AMotionEvent_getPointerCount(event);

        for (size_t i = 0; i < pointerCount; ++i)
        {
            lastCursorPosX = AMotionEvent_getX(event, i);
            lastCursorPosY = AMotionEvent_getY(event, i);
            int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

            // Map Android touch events to GLFW touch events
            switch (action)
            {
                case AMOTION_EVENT_ACTION_DOWN:
                case AMOTION_EVENT_ACTION_POINTER_DOWN:
                    _glfwInputMouseClick(_glfw.windowListHead, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
                    break;

                case AMOTION_EVENT_ACTION_UP:
                case AMOTION_EVENT_ACTION_POINTER_UP:
                    _glfwInputMouseClick(_glfw.windowListHead, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
                    break;

                case AMOTION_EVENT_ACTION_MOVE:
                    _glfwInputCursorPos(_glfw.windowListHead, lastCursorPosX, lastCursorPosY);
                    break;

                case AMOTION_EVENT_ACTION_CANCEL:
                    // Handle cancel if necessary
                    break;
            }
        }

        return 1;
    }
    else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
    {
        _glfwInputKey(_glfw.windowListHead, 0 , AKeyEvent_getKeyCode(event), GLFW_PRESS, 0);

        return 1;
    }

    return 0;
}

static void handleEvents(int timeout)
{
    ALooper_pollOnce(timeout, NULL, NULL, (void**)&_glfw.gstate.source);

    if (_glfw.gstate.source != NULL)
        _glfw.gstate.source->process(_glfw.gstate.app, _glfw.gstate.source);
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwCreateWindowAndroid(_GLFWwindow* window,
                                  const _GLFWwndconfig* wndconfig,
                                  const _GLFWctxconfig* ctxconfig,
                                  const _GLFWfbconfig* fbconfig)
{
    // wait for window to become ready
    while (_glfw.gstate.app->window == NULL)
        handleEvents(-1);

    // hmmm maybe should be ANative_Window only?
    window->android = _glfw.gstate.app;
    window->android->onInputEvent = handleInput;

    if (ctxconfig->client != GLFW_NO_API)
    {
        if ((ctxconfig->source == GLFW_NATIVE_CONTEXT_API) |
            (ctxconfig->source == GLFW_EGL_CONTEXT_API))
        {
            if (!_glfwInitEGL())
                return GLFW_FALSE;

            if (!_glfwCreateContextEGL(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }
        else if (ctxconfig->source == GLFW_OSMESA_CONTEXT_API)
        {
            if (!_glfwInitOSMesa())
                return GLFW_FALSE;

            if (!_glfwCreateContextOSMesa(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }

        if (!_glfwRefreshContextAttribs(window, ctxconfig))
            return GLFW_FALSE;
    }

    return GLFW_TRUE;
}

void _glfwDestroyWindowAndroid(_GLFWwindow* window)
{
    if (window->context.destroy)
        window->context.destroy(window);

    ANativeActivity_finish(window->android->activity);
}

void _glfwSetWindowTitleAndroid(_GLFWwindow* window, const char* title)
{
}

void _glfwSetWindowIconAndroid(_GLFWwindow* window, int count,
                               const GLFWimage* images)
{
}

void _glfwSetWindowMonitorAndroid(_GLFWwindow* window,
                                  _GLFWmonitor* monitor,
                                  int xpos, int ypos,
                                  int width, int height,
                                  int refreshRate)
{
}

void _glfwGetWindowPosAndroid(_GLFWwindow* window, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = 0;

    if (ypos)
        *ypos = 0;
}

void _glfwSetWindowPosAndroid(_GLFWwindow* window, int xpos, int ypos)
{
}

void _glfwGetWindowSizeAndroid(_GLFWwindow* window, int* width, int* height)
{
    if (width)
    {
        *width = (window->android->window != NULL)
                 ? ANativeWindow_getWidth(window->android->window)
                 : 0;
    }

    if (height)
    {
        *height = (window->android->window != NULL)
                  ? ANativeWindow_getHeight(window->android->window)
                  : 0;
    }
}

void _glfwSetWindowSizeAndroid(_GLFWwindow* window, int width, int height)
{
}

void _glfwSetWindowSizeLimitsAndroid(_GLFWwindow* window,
                                     int minwidth, int minheight,
                                     int maxwidth, int maxheight)
{
}

void _glfwSetWindowAspectRatioAndroid(_GLFWwindow* window, int n, int d)
{
}

void _glfwGetFramebufferSizeAndroid(_GLFWwindow* window, int* width, int* height)
{
    // the underlying buffer geometry is currently being initialized from the window width and height...
    // so high resolution displays are currently not supported...so it is safe to just call _glfwGetWindowSizeAndroid() for now
    _glfwGetWindowSizeAndroid(window, width, height);
}

void _glfwGetWindowFrameSizeAndroid(_GLFWwindow* window,
                                    int* left, int* top,
                                    int* right, int* bottom)
{
    if (left)
        *left = window->android->contentRect.left;

    if (top)
        *top = window->android->contentRect.top;

    if (right)
    {
        int windowWidth = (window->android->window != NULL)
            ? ANativeWindow_getWidth(window->android->window)
            : 0;

        int rightFrame = windowWidth - window->android->contentRect.right;
        if (rightFrame < 0) rightFrame = 0;

        *right = rightFrame;
    }

    if (bottom)
    {
        int windowHeight = (window->android->window != NULL)
            ? ANativeWindow_getHeight(window->android->window)
            : 0;

        int bottomFrame = windowHeight - window->android->contentRect.bottom;
        if (bottomFrame < 0) bottomFrame = 0;

        *bottom = bottomFrame;
    }
}

void _glfwGetWindowContentScaleAndroid(_GLFWwindow* window, float* xscale, float* yscale)
{
    if (xscale)
    {
        int32_t widthDensity = AConfiguration_getScreenWidthDp(window->android->config);
        if (widthDensity == ACONFIGURATION_SCREEN_WIDTH_DP_ANY)
        {
            *xscale = 1.0f;
        }
        else
        {
            int32_t widthPixels = ANativeWindow_getWidth(window->android->window);
            *xscale = (float)widthPixels / (float)widthDensity;
        }
    }

    if (yscale)
    {
        int32_t heightDensity = AConfiguration_getScreenHeightDp(window->android->config);
        if (heightDensity == ACONFIGURATION_SCREEN_HEIGHT_DP_ANY)
        {
            *yscale = 1.0f;
        }
        else
        {
            int32_t heightPixels = ANativeWindow_getHeight(window->android->window);
            *yscale = (float)heightPixels / (float)heightDensity;
        }
    }
}

void _glfwIconifyWindowAndroid(_GLFWwindow* window)
{
    moveNativeWindowToBackground(window->android->activity);
}

void _glfwRestoreWindowAndroid(_GLFWwindow* window)
{
}

void _glfwMaximizeWindowAndroid(_GLFWwindow* window)
{
}

GLFWbool _glfwWindowMaximizedAndroid(_GLFWwindow* window)
{
    return GLFW_TRUE;
}

GLFWbool _glfwWindowHoveredAndroid(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

GLFWbool _glfwFramebufferTransparentAndroid(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

void _glfwSetWindowResizableAndroid(_GLFWwindow* window, GLFWbool enabled)
{
}

void _glfwSetWindowDecoratedAndroid(_GLFWwindow* window, GLFWbool enabled)
{
}

void _glfwSetWindowFloatingAndroid(_GLFWwindow* window, GLFWbool enabled)
{
}

void _glfwSetWindowMousePassthroughAndroid(_GLFWwindow* window, GLFWbool enabled)
{
}

float _glfwGetWindowOpacityAndroid(_GLFWwindow* window)
{
    return 1.0f;
}

void _glfwSetWindowOpacityAndroid(_GLFWwindow* window, float opacity)
{
}

void _glfwSetRawMouseMotionAndroid(_GLFWwindow *window, GLFWbool enabled)
{
}

GLFWbool _glfwRawMouseMotionSupportedAndroid(void)
{
    return GLFW_FALSE;
}

void _glfwShowWindowAndroid(_GLFWwindow* window)
{
}

void _glfwRequestWindowAttentionAndroid(_GLFWwindow* window)
{
}

void _glfwHideWindowAndroid(_GLFWwindow* window)
{
}

void _glfwFocusWindowAndroid(_GLFWwindow* window)
{
}

GLFWbool _glfwWindowFocusedAndroid(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

GLFWbool _glfwWindowIconifiedAndroid(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

GLFWbool _glfwWindowVisibleAndroid(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

void _glfwPollEventsAndroid(void)
{
    handleEvents(0);
}

void _glfwWaitEventsAndroid(void)
{
    handleEvents(-1);
}

void _glfwWaitEventsTimeoutAndroid(double timeout)
{
    handleEvents(timeout * 1e3);
}

void _glfwPostEmptyEventAndroid(void)
{
}

void _glfwGetCursorPosAndroid(_GLFWwindow* window, double* xpos, double* ypos)
{
    if (xpos)
        *xpos = (double)lastCursorPosX;

    if (ypos)
        *ypos = (double)lastCursorPosY;
}

void _glfwSetCursorPosAndroid(_GLFWwindow* window, double x, double y)
{
}

void _glfwSetCursorModeAndroid(_GLFWwindow* window, int mode)
{
}

GLFWbool _glfwCreateCursorAndroid(_GLFWcursor* cursor,
                                  const GLFWimage* image,
                                  int xhot, int yhot)
{
    return GLFW_TRUE;
}

GLFWbool _glfwCreateStandardCursorAndroid(_GLFWcursor* cursor, int shape)
{
    return GLFW_TRUE;
}

void _glfwDestroyCursorAndroid(_GLFWcursor* cursor)
{
}

void _glfwSetCursorAndroid(_GLFWwindow* window, _GLFWcursor* cursor)
{
}

void _glfwSetClipboardStringAndroid(const char* string)
{
}

const char* _glfwGetClipboardStringAndroid(void)
{
    return NULL;
}

const char* _glfwGetScancodeNameAndroid(int scancode)
{
    return "";
}

int _glfwGetKeyScancodeAndroid(int key)
{
    return -1;
}

EGLenum _glfwGetEGLPlatformAndroid(EGLint** attribs)
{
    if (_glfw.egl.ANGLE_platform_angle)
    {
        int type = 0;

        if (_glfw.egl.ANGLE_platform_angle_opengl)
        {
            if (_glfw.hints.init.angleType == GLFW_ANGLE_PLATFORM_TYPE_OPENGL)
                type = EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE;
            else if (_glfw.hints.init.angleType == GLFW_ANGLE_PLATFORM_TYPE_OPENGLES)
                type = EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE;
        }

        if (_glfw.egl.ANGLE_platform_angle_vulkan)
        {
            if (_glfw.hints.init.angleType == GLFW_ANGLE_PLATFORM_TYPE_VULKAN)
                type = EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE;
        }

        if (type)
        {
            *attribs = _glfw_calloc(3, sizeof(EGLint));
            (*attribs)[0] = EGL_PLATFORM_ANGLE_TYPE_ANGLE;
            (*attribs)[1] = type;
            (*attribs)[2] = EGL_NONE;
            return EGL_PLATFORM_ANGLE_ANGLE;
        }
    }

    return 0;
}

EGLNativeDisplayType _glfwGetEGLNativeDisplayAndroid(void)
{
    return EGL_DEFAULT_DISPLAY;
}

EGLNativeWindowType _glfwGetEGLNativeWindowAndroid(_GLFWwindow* window)
{
    return ((EGLNativeWindowType) window->android->window);
}

void _glfwGetRequiredInstanceExtensionsAndroid(char** extensions)
{
    if (!_glfw.vk.KHR_surface || !_glfw.vk.KHR_android_surface)
        return;

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_KHR_android_surface";
}

GLFWbool _glfwGetPhysicalDevicePresentationSupportAndroid(VkInstance instance,
                                                          VkPhysicalDevice device,
                                                          uint32_t queuefamily)
{
    return GLFW_TRUE;
}

VkResult _glfwCreateWindowSurfaceAndroid(VkInstance instance,
                                         _GLFWwindow* window,
                                         const VkAllocationCallbacks* allocator,
                                         VkSurfaceKHR* surface)
{
    VkResult err;
    VkAndroidSurfaceCreateInfoKHR sci;
    PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;

    vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR");
    if (!vkCreateAndroidSurfaceKHR)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "Android: Vulkan instance missing VK_KHR_android_surface extension");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    sci.window = window->android->window;

    err = vkCreateAndroidSurfaceKHR(instance, &sci, allocator, surface);
    if (err)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Android: Failed to create Vulkan surface: %s",
                        _glfwGetVulkanResultString(err));
    }

    return err;
}

//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI struct android_app* glfwGetAndroidApp(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*)handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    return window->android;
}
