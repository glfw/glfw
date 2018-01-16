//========================================================================
// GLFW 3.3 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2017 Curi0 <curi0minecraft@gmail.com>
// Copyright (c) 2006-2016 Camilla Löwy <elmindreda@glfw.org>
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

static int32_t handle_input(struct android_app* app, AInputEvent* event)
{
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
        for (size_t i = 0; i < AMotionEvent_getPointerCount(event); ++i)
        {
            x = AMotionEvent_getX(event, i);
            y = AMotionEvent_getY(event, i);
        }
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
        _glfwInputKey(_glfw.windowListHead, 0 , AKeyEvent_getKeyCode(event), GLFW_PRESS,0);

    return 0;
}

static void handleEvents(int timeout) {
    ALooper_pollOnce(0, NULL, NULL,(void**)&_glfw.gstate.source);

    if (_glfw.gstate.source != NULL) {
        _glfw.gstate.source->process(_glfw.gstate.app, _glfw.gstate.source);
    }
    _glfwInputCursorPos(_glfw.windowListHead, x, y);
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWctxconfig* ctxconfig,
                              const _GLFWfbconfig* fbconfig)
{
    // wait for window to become ready
    while (_glfw.gstate.app->window == NULL) {
        handleEvents(-1);
    }
    // hmmm maybe should be ANative_Window only?
    window->android = _glfw.gstate.app;
    window->android->onInputEvent = handle_input;

    //ANativeWindow_setBuffersGeometry(window->android->window, wndconfig->width, wndconfig->height, 0);

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

    }
    return GLFW_TRUE;
}


void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
    if (window->context.destroy)
        window->context.destroy(window);
    ANativeActivity_finish(window->android->activity);
}

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
}

void _glfwPlatformSetWindowIcon(_GLFWwindow* window, int count,
                                const GLFWimage* images)
{
}

void _glfwPlatformSetWindowMonitor(_GLFWwindow* window,
                                   _GLFWmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate)
{
}

void _glfwPlatformGetWindowPos(_GLFWwindow* window, int* xpos, int* ypos)
{

}

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int xpos, int ypos)
{
}

void _glfwPlatformGetWindowSize(_GLFWwindow* window, int* width, int* height)
{
    if (height)
        *height = ANativeWindow_getHeight(window->android->window);
    if (width)
        *width = ANativeWindow_getWidth(window->android->window);
}

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
}

void _glfwPlatformSetWindowSizeLimits(_GLFWwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{

}

void _glfwPlatformSetWindowAspectRatio(_GLFWwindow* window, int n, int d)
{
}

void _glfwPlatformGetFramebufferSize(_GLFWwindow* window, int* width, int* height)
{
    // the underlying buffergeometry is currently being initialized from the
    // window width and height...so high resolution displays are currently
    // not supported...so it is safe to just call GetWindowSize() for now
    _glfwPlatformGetWindowSize(window, width, height);
}

void _glfwPlatformGetWindowFrameSize(_GLFWwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
}

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
}

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
}

void _glfwPlatformMaximizeWindow(_GLFWwindow* window)
{
}

int _glfwPlatformWindowMaximized(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

void _glfwPlatformSetWindowResizable(_GLFWwindow* window, GLFWbool enabled)
{
}

void _glfwPlatformSetWindowDecorated(_GLFWwindow* window, GLFWbool enabled)
{
}

void _glfwPlatformSetWindowFloating(_GLFWwindow* window, GLFWbool enabled)
{
}

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
}


void _glfwPlatformRequestWindowAttention(_GLFWwindow* window)
{
}

void _glfwPlatformUnhideWindow(_GLFWwindow* window)
{
}

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
}

void _glfwPlatformFocusWindow(_GLFWwindow* window)
{
}

int _glfwPlatformWindowFocused(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

int _glfwPlatformWindowIconified(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

int _glfwPlatformWindowVisible(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

void _glfwPlatformPollEvents(void)
{
    handleEvents(0);
}

void _glfwPlatformWaitEvents(void)
{
    handleEvents(-1);
}

void _glfwPlatformWaitEventsTimeout(double timeout)
{
    handleEvents(timeout * 1e3);
}

void _glfwPlatformPostEmptyEvent(void)
{
}

void _glfwPlatformGetCursorPos(_GLFWwindow* window, double* xpos, double* ypos)
{
    if (xpos)
        *xpos = x;
    if (ypos)
        *ypos = y;
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double x, double y)
{
}

void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode)
{
}

int _glfwPlatformCreateCursor(_GLFWcursor* cursor,
                              const GLFWimage* image,
                              int xhot, int yhot)
{
    return GLFW_TRUE;
}

int _glfwPlatformCreateStandardCursor(_GLFWcursor* cursor, int shape)
{
    return GLFW_TRUE;
}

void _glfwPlatformDestroyCursor(_GLFWcursor* cursor)
{
}

void _glfwPlatformSetCursor(_GLFWwindow* window, _GLFWcursor* cursor)
{
}

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string)
{
}

const char* _glfwPlatformGetClipboardString(_GLFWwindow* window)
{
    return NULL;
}

const char* _glfwPlatformGetScancodeName(int scancode)
{
    return "";
}

int _glfwPlatformGetKeyScancode(int key)
{
    return -1;
}

void _glfwPlatformGetRequiredInstanceExtensions(char** extensions)
{
    if (!_glfw.vk.KHR_surface || !_glfw.vk.KHR_android_surface)
        return;

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_KHR_android_surface";
}

int _glfwPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    return GLFW_TRUE;
}

int _glfwPlatformFramebufferTransparent(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

VkResult _glfwPlatformCreateWindowSurface(VkInstance instance,
                                          _GLFWwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface)
{
    VkResult err;
    VkAndroidSurfaceCreateInfoKHR sci;
    PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;

    vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)
            vkGetInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR");
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

GLFWAPI struct android_app * glfwGetAndroidApp(GLFWwindow* handle)
{
    _GLFWwindow *window = (_GLFWwindow*)handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    return window->android;
}
