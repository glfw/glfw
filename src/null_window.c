//========================================================================
// GLFW 3.5 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2019 Camilla Löwy <elmindreda@glfw.org>
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

#include <stdlib.h>
#include <string.h>

static void applySizeLimits(_GLFWwindow* window, int* width, int* height)
{
    if (window->numer != GLFW_DONT_CARE && window->denom != GLFW_DONT_CARE)
    {
        const float ratio = (float) window->numer / (float) window->denom;
        *height = (int) (*width / ratio);
    }

    if (window->minwidth != GLFW_DONT_CARE)
        *width = _glfw_max(*width, window->minwidth);
    else if (window->maxwidth != GLFW_DONT_CARE)
        *width = _glfw_min(*width, window->maxwidth);

    if (window->minheight != GLFW_DONT_CARE)
        *height = _glfw_min(*height, window->minheight);
    else if (window->maxheight != GLFW_DONT_CARE)
        *height = _glfw_max(*height, window->maxheight);
}

static void fitToMonitor(_GLFWwindow* window)
{
    GLFWvidmode mode;
    _glfwGetVideoModeNull(window->monitor, &mode);
    _glfwGetMonitorPosNull(window->monitor,
                           &window->null.xpos,
                           &window->null.ypos);
    window->null.width = mode.width;
    window->null.height = mode.height;
}

static void acquireMonitor(_GLFWwindow* window)
{
    _glfwInputMonitorWindow(window->monitor, window);
}

static void releaseMonitor(_GLFWwindow* window)
{
    if (window->monitor->window != window)
        return;

    _glfwInputMonitorWindow(window->monitor, NULL);
}

static int createNativeWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWfbconfig* fbconfig)
{
    if (window->monitor)
        fitToMonitor(window);
    else
    {
        if (wndconfig->xpos == GLFW_ANY_POSITION && wndconfig->ypos == GLFW_ANY_POSITION)
        {
            window->null.xpos = 17;
            window->null.ypos = 17;
        }
        else
        {
            window->null.xpos = wndconfig->xpos;
            window->null.ypos = wndconfig->ypos;
        }

        window->null.width = wndconfig->width;
        window->null.height = wndconfig->height;
    }

    window->null.visible = wndconfig->visible;
    window->null.decorated = wndconfig->decorated;
    window->null.maximized = wndconfig->maximized;
    window->null.floating = wndconfig->floating;
    window->null.transparent = fbconfig->transparent;
    window->null.opacity = 1.f;

    return GLFW_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwCreateWindowNull(_GLFWwindow* window,
                               const _GLFWwndconfig* wndconfig,
                               const _GLFWctxconfig* ctxconfig,
                               const _GLFWfbconfig* fbconfig)
{
    if (!createNativeWindow(window, wndconfig, fbconfig))
        return GLFW_FALSE;

    if (ctxconfig->client != GLFW_NO_API)
    {
        if (ctxconfig->source == GLFW_NATIVE_CONTEXT_API ||
            ctxconfig->source == GLFW_OSMESA_CONTEXT_API)
        {
            if (!_glfwInitOSMesa())
                return GLFW_FALSE;
            if (!_glfwCreateContextOSMesa(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }
        else if (ctxconfig->source == GLFW_EGL_CONTEXT_API)
        {
            if (!_glfwInitEGL())
                return GLFW_FALSE;
            if (!_glfwCreateContextEGL(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }

        if (!_glfwRefreshContextAttribs(window, ctxconfig))
            return GLFW_FALSE;
    }

    if (wndconfig->mousePassthrough)
        _glfwSetWindowMousePassthroughNull(window, GLFW_TRUE);

    if (window->monitor)
    {
        _glfwShowWindowNull(window);
        _glfwFocusWindowNull(window);
        acquireMonitor(window);

        if (wndconfig->centerCursor)
            _glfwCenterCursorInContentArea(window);
    }
    else
    {
        if (wndconfig->visible)
        {
            _glfwShowWindowNull(window);
            if (wndconfig->focused)
                _glfwFocusWindowNull(window);
        }
    }

    return GLFW_TRUE;
}

void _glfwDestroyWindowNull(_GLFWwindow* window)
{
    if (window->monitor)
        releaseMonitor(window);

    if (_glfw.null.focusedWindow == window)
        _glfw.null.focusedWindow = NULL;

    if (window->context.destroy)
        window->context.destroy(window);
}

void _glfwSetWindowTitleNull(_GLFWwindow* window, const char* title)
{
}

void _glfwSetWindowIconNull(_GLFWwindow* window, int count, const GLFWimage* images)
{
}

void _glfwSetWindowMonitorNull(_GLFWwindow* window,
                               _GLFWmonitor* monitor,
                               int xpos, int ypos,
                               int width, int height,
                               int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (!monitor)
        {
            _glfwSetWindowPosNull(window, xpos, ypos);
            _glfwSetWindowSizeNull(window, width, height);
        }

        return;
    }

    if (window->monitor)
        releaseMonitor(window);

    _glfwInputWindowMonitor(window, monitor);

    if (window->monitor)
    {
        window->null.visible = GLFW_TRUE;
        acquireMonitor(window);
        fitToMonitor(window);
    }
    else
    {
        _glfwSetWindowPosNull(window, xpos, ypos);
        _glfwSetWindowSizeNull(window, width, height);
    }
}

void _glfwGetWindowPosNull(_GLFWwindow* window, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = window->null.xpos;
    if (ypos)
        *ypos = window->null.ypos;
}

void _glfwSetWindowPosNull(_GLFWwindow* window, int xpos, int ypos)
{
    if (window->monitor)
        return;

    if (window->null.xpos != xpos || window->null.ypos != ypos)
    {
        window->null.xpos = xpos;
        window->null.ypos = ypos;
        _glfwInputWindowPos(window, xpos, ypos);
    }
}

void _glfwGetWindowSizeNull(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->null.width;
    if (height)
        *height = window->null.height;
}

void _glfwSetWindowSizeNull(_GLFWwindow* window, int width, int height)
{
    if (window->monitor)
        return;

    if (window->null.width != width || window->null.height != height)
    {
        window->null.width = width;
        window->null.height = height;
        _glfwInputFramebufferSize(window, width, height);
        _glfwInputWindowDamage(window);
        _glfwInputWindowSize(window, width, height);
    }
}

void _glfwSetWindowSizeLimitsNull(_GLFWwindow* window,
                                  int minwidth, int minheight,
                                  int maxwidth, int maxheight)
{
    int width = window->null.width;
    int height = window->null.height;
    applySizeLimits(window, &width, &height);
    _glfwSetWindowSizeNull(window, width, height);
}

void _glfwSetWindowAspectRatioNull(_GLFWwindow* window, int n, int d)
{
    int width = window->null.width;
    int height = window->null.height;
    applySizeLimits(window, &width, &height);
    _glfwSetWindowSizeNull(window, width, height);
}

void _glfwGetFramebufferSizeNull(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->null.width;
    if (height)
        *height = window->null.height;
}

void _glfwGetWindowFrameSizeNull(_GLFWwindow* window,
                                 int* left, int* top,
                                 int* right, int* bottom)
{
    if (window->null.decorated && !window->monitor)
    {
        if (left)
            *left = 1;
        if (top)
            *top = 10;
        if (right)
            *right = 1;
        if (bottom)
            *bottom = 1;
    }
    else
    {
        if (left)
            *left = 0;
        if (top)
            *top = 0;
        if (right)
            *right = 0;
        if (bottom)
            *bottom = 0;
    }
}

void _glfwGetWindowContentScaleNull(_GLFWwindow* window, float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _glfwIconifyWindowNull(_GLFWwindow* window)
{
    if (_glfw.null.focusedWindow == window)
    {
        _glfw.null.focusedWindow = NULL;
        _glfwInputWindowFocus(window, GLFW_FALSE);
    }

    if (!window->null.iconified)
    {
        window->null.iconified = GLFW_TRUE;
        _glfwInputWindowIconify(window, GLFW_TRUE);

        if (window->monitor)
            releaseMonitor(window);
    }
}

void _glfwRestoreWindowNull(_GLFWwindow* window)
{
    if (window->null.iconified)
    {
        window->null.iconified = GLFW_FALSE;
        _glfwInputWindowIconify(window, GLFW_FALSE);

        if (window->monitor)
            acquireMonitor(window);
    }
    else if (window->null.maximized)
    {
        window->null.maximized = GLFW_FALSE;
        _glfwInputWindowMaximize(window, GLFW_FALSE);
    }
}

void _glfwMaximizeWindowNull(_GLFWwindow* window)
{
    if (!window->null.maximized)
    {
        window->null.maximized = GLFW_TRUE;
        _glfwInputWindowMaximize(window, GLFW_TRUE);
    }
}

GLFWbool _glfwWindowMaximizedNull(_GLFWwindow* window)
{
    return window->null.maximized;
}

GLFWbool _glfwWindowHoveredNull(_GLFWwindow* window)
{
    return _glfw.null.xcursor >= window->null.xpos &&
           _glfw.null.ycursor >= window->null.ypos &&
           _glfw.null.xcursor <= window->null.xpos + window->null.width - 1 &&
           _glfw.null.ycursor <= window->null.ypos + window->null.height - 1;
}

GLFWbool _glfwFramebufferTransparentNull(_GLFWwindow* window)
{
    return window->null.transparent;
}

void _glfwSetWindowResizableNull(_GLFWwindow* window, GLFWbool enabled)
{
    window->null.resizable = enabled;
}

void _glfwSetWindowDecoratedNull(_GLFWwindow* window, GLFWbool enabled)
{
    window->null.decorated = enabled;
}

void _glfwSetWindowFloatingNull(_GLFWwindow* window, GLFWbool enabled)
{
    window->null.floating = enabled;
}

void _glfwSetWindowMousePassthroughNull(_GLFWwindow* window, GLFWbool enabled)
{
}

float _glfwGetWindowOpacityNull(_GLFWwindow* window)
{
    return window->null.opacity;
}

void _glfwSetWindowOpacityNull(_GLFWwindow* window, float opacity)
{
    window->null.opacity = opacity;
}

void _glfwSetRawMouseMotionNull(_GLFWwindow *window, GLFWbool enabled)
{
}

GLFWbool _glfwRawMouseMotionSupportedNull(void)
{
    return GLFW_TRUE;
}

void _glfwShowWindowNull(_GLFWwindow* window)
{
    window->null.visible = GLFW_TRUE;
}

void _glfwRequestWindowAttentionNull(_GLFWwindow* window)
{
}

void _glfwHideWindowNull(_GLFWwindow* window)
{
    if (_glfw.null.focusedWindow == window)
    {
        _glfw.null.focusedWindow = NULL;
        _glfwInputWindowFocus(window, GLFW_FALSE);
    }

    window->null.visible = GLFW_FALSE;
}

void _glfwFocusWindowNull(_GLFWwindow* window)
{
    _GLFWwindow* previous;

    if (_glfw.null.focusedWindow == window)
        return;

    if (!window->null.visible)
        return;

    previous = _glfw.null.focusedWindow;
    _glfw.null.focusedWindow = window;

    if (previous)
    {
        _glfwInputWindowFocus(previous, GLFW_FALSE);
        if (previous->monitor && previous->autoIconify)
            _glfwIconifyWindowNull(previous);
    }

    _glfwInputWindowFocus(window, GLFW_TRUE);
}

GLFWbool _glfwWindowFocusedNull(_GLFWwindow* window)
{
    return _glfw.null.focusedWindow == window;
}

GLFWbool _glfwWindowIconifiedNull(_GLFWwindow* window)
{
    return window->null.iconified;
}

GLFWbool _glfwWindowVisibleNull(_GLFWwindow* window)
{
    return window->null.visible;
}

void _glfwPollEventsNull(void)
{
}

void _glfwWaitEventsNull(void)
{
}

void _glfwWaitEventsTimeoutNull(double timeout)
{
}

void _glfwPostEmptyEventNull(void)
{
}

void _glfwGetCursorPosNull(_GLFWwindow* window, double* xpos, double* ypos)
{
    if (xpos)
        *xpos = _glfw.null.xcursor - window->null.xpos;
    if (ypos)
        *ypos = _glfw.null.ycursor - window->null.ypos;
}

void _glfwSetCursorPosNull(_GLFWwindow* window, double x, double y)
{
    _glfw.null.xcursor = window->null.xpos + (int) x;
    _glfw.null.ycursor = window->null.ypos + (int) y;
}

void _glfwSetCursorModeNull(_GLFWwindow* window, int mode)
{
}

GLFWbool _glfwCreateCursorNull(_GLFWcursor* cursor,
                               const GLFWimage* image,
                               int xhot, int yhot)
{
    return GLFW_TRUE;
}

GLFWbool _glfwCreateStandardCursorNull(_GLFWcursor* cursor, int shape)
{
    return GLFW_TRUE;
}

void _glfwDestroyCursorNull(_GLFWcursor* cursor)
{
}

void _glfwSetCursorNull(_GLFWwindow* window, _GLFWcursor* cursor)
{
}

void _glfwSetClipboardStringNull(const char* string)
{
    char* copy = _glfw_strdup(string);
    _glfw_free(_glfw.null.clipboardString);
    _glfw.null.clipboardString = copy;
}

const char* _glfwGetClipboardStringNull(void)
{
    return _glfw.null.clipboardString;
}

EGLenum _glfwGetEGLPlatformNull(EGLint** attribs)
{
    if (_glfw.egl.EXT_platform_base && _glfw.egl.MESA_platform_surfaceless)
        return EGL_PLATFORM_SURFACELESS_MESA;
    else
        return 0;
}

EGLNativeDisplayType _glfwGetEGLNativeDisplayNull(void)
{
    return EGL_DEFAULT_DISPLAY;
}

EGLNativeWindowType _glfwGetEGLNativeWindowNull(_GLFWwindow* window)
{
    return 0;
}

// Store the string representations of the scan codes.
// This relies on the index in the array corresponding to the
// GLFW_NULL_SC_* constant. For each element, it defines the different
// names depending on that key's shift level, which can be either 0
// (no modifiers), 1 (shift held), 2 (ctrl+alt/altgr held), or 3 (both
// shift and altgr held).
//
// The used key names are from the en-US ANSI keyboard layout.
const char* keyNames[GLFW_NULL_SC_LAST+1][4] = {
    // 0 = neutral, 1 = shift, 2 = altgr, 3 = shift+altgr
    {/**/               NULL, NULL, NULL, NULL},
    {/*SPACE*/          " ", " ", " ", " "},
    {/*APOSTROPHE*/     "'", "\"", NULL, NULL},
    {/*COMMA*/          ",", "<", NULL, NULL},
    {/*MINUS*/          "-", "_", NULL, NULL},
    {/*PERIOD*/         ".", ">", NULL, NULL},
    {/*SLASH*/          "/", "?", NULL, NULL},
    {/*0*/              "0", ")", NULL, NULL},
    {/*1*/              "1", "!", NULL, NULL},
    {/*2*/              "2", "@", NULL, NULL},
    {/*3*/              "3", "#", NULL, NULL},
    {/*4*/              "4", "$", NULL, NULL},
    {/*5*/              "5", "%", NULL, NULL},
    {/*6*/              "6", "^", NULL, NULL},
    {/*7*/              "7", "&", NULL, NULL},
    {/*8*/              "8", "*", NULL, NULL},
    {/*9*/              "9", "(", NULL, NULL},
    {/*SEMICOLON*/      ";", ":", NULL, NULL},
    {/*EQUAL*/          "=", "+", NULL, NULL},
    {/*LEFT_BRACKET*/   "[", "{", NULL, NULL},
    {/*BACKSLASH*/      "\\", "|", NULL, NULL},
    {/*RIGHT_BRACKET*/  "]", "}", NULL, NULL},
    {/*GRAVE_ACCENT*/   NULL, NULL, NULL, NULL},
    {/*WORLD_1*/        "\\", NULL, NULL, NULL},
    {/*WORLD_2*/        "\\", NULL, NULL, NULL},
    {/*ESCAPE*/         NULL, NULL, NULL, NULL},
    {/*ENTER*/          NULL, NULL, NULL, NULL},
    {/*TAB*/            NULL, NULL, NULL, NULL},
    {/*BACKSPACE*/      NULL, NULL, NULL, NULL},
    {/*INSERT*/         NULL, NULL, NULL, NULL},
    {/*DELETE*/         NULL, NULL, NULL, NULL},
    {/*RIGHT*/          NULL, NULL, NULL, NULL},
    {/*LEFT*/           NULL, NULL, NULL, NULL},
    {/*DOWN*/           NULL, NULL, NULL, NULL},
    {/*UP*/             NULL, NULL, NULL, NULL},
    {/*PAGE_UP*/        NULL, NULL, NULL, NULL},
    {/*PAGE_DOWN*/      NULL, NULL, NULL, NULL},
    {/*HOME*/           NULL, NULL, NULL, NULL},
    {/*END*/            NULL, NULL, NULL, NULL},
    {/*CAPS_LOCK*/      NULL, NULL, NULL, NULL},
    {/*SCROLL_LOCK*/    NULL, NULL, NULL, NULL},
    {/*NUM_LOCK*/       NULL, NULL, NULL, NULL},
    {/*PRINT_SCREEN*/   NULL, NULL, NULL, NULL},
    {/*PAUSE*/          NULL, NULL, NULL, NULL},
    {/*A*/              "a", "A", NULL, NULL},
    {/*B*/              "b", "B", NULL, NULL},
    {/*C*/              "c", "C", NULL, NULL},
    {/*D*/              "d", "D", NULL, NULL},
    {/*E*/              "e", "E", NULL, NULL},
    {/*F*/              "f", "F", NULL, NULL},
    {/*G*/              "g", "G", NULL, NULL},
    {/*H*/              "h", "H", NULL, NULL},
    {/*I*/              "i", "I", NULL, NULL},
    {/*J*/              "j", "J", NULL, NULL},
    {/*K*/              "k", "K", NULL, NULL},
    {/*L*/              "l", "L", NULL, NULL},
    {/*M*/              "m", "M", NULL, NULL},
    {/*N*/              "n", "N", NULL, NULL},
    {/*O*/              "o", "O", NULL, NULL},
    {/*P*/              "p", "P", NULL, NULL},
    {/*Q*/              "q", "Q", NULL, NULL},
    {/*R*/              "r", "R", NULL, NULL},
    {/*S*/              "s", "S", NULL, NULL},
    {/*T*/              "t", "T", NULL, NULL},
    {/*U*/              "u", "U", NULL, NULL},
    {/*V*/              "v", "V", NULL, NULL},
    {/*W*/              "w", "W", NULL, NULL},
    {/*X*/              "x", "X", NULL, NULL},
    {/*Y*/              "y", "Y", NULL, NULL},
    {/*Z*/              "z", "Z", NULL, NULL},
    {/*F1*/             NULL, NULL, NULL, NULL},
    {/*F2*/             NULL, NULL, NULL, NULL},
    {/*F3*/             NULL, NULL, NULL, NULL},
    {/*F4*/             NULL, NULL, NULL, NULL},
    {/*F5*/             NULL, NULL, NULL, NULL},
    {/*F6*/             NULL, NULL, NULL, NULL},
    {/*F7*/             NULL, NULL, NULL, NULL},
    {/*F8*/             NULL, NULL, NULL, NULL},
    {/*F9*/             NULL, NULL, NULL, NULL},
    {/*F10*/            NULL, NULL, NULL, NULL},
    {/*F11*/            NULL, NULL, NULL, NULL},
    {/*F12*/            NULL, NULL, NULL, NULL},
    {/*F13*/            NULL, NULL, NULL, NULL},
    {/*F14*/            NULL, NULL, NULL, NULL},
    {/*F15*/            NULL, NULL, NULL, NULL},
    {/*F16*/            NULL, NULL, NULL, NULL},
    {/*F17*/            NULL, NULL, NULL, NULL},
    {/*F18*/            NULL, NULL, NULL, NULL},
    {/*F19*/            NULL, NULL, NULL, NULL},
    {/*F20*/            NULL, NULL, NULL, NULL},
    {/*F21*/            NULL, NULL, NULL, NULL},
    {/*F22*/            NULL, NULL, NULL, NULL},
    {/*F23*/            NULL, NULL, NULL, NULL},
    {/*F24*/            NULL, NULL, NULL, NULL},
    {/*F25*/            NULL, NULL, NULL, NULL},
    {/*KP_0*/           "0", NULL, NULL, NULL},
    {/*KP_1*/           "1", NULL, NULL, NULL},
    {/*KP_2*/           "2", NULL, NULL, NULL},
    {/*KP_3*/           "3", NULL, NULL, NULL},
    {/*KP_4*/           "4", NULL, NULL, NULL},
    {/*KP_5*/           "5", NULL, NULL, NULL},
    {/*KP_6*/           "6", NULL, NULL, NULL},
    {/*KP_7*/           "7", NULL, NULL, NULL},
    {/*KP_8*/           "8", NULL, NULL, NULL},
    {/*KP_9*/           "9", NULL, NULL, NULL},
    {/*KP_DECIMAL*/     ".", NULL, NULL, NULL},
    {/*KP_DIVIDE*/      "/", NULL, NULL, NULL},
    {/*KP_MULTIPLY*/    "*", NULL, NULL, NULL},
    {/*KP_SUBTRACT*/    "-", NULL, NULL, NULL},
    {/*KP_ADD*/         "+", NULL, NULL, NULL},
    {/*KP_ENTER*/       NULL, NULL, NULL, NULL},
    {/*KP_EQUAL*/       "=", NULL, NULL, NULL},
    {/*LEFT_SHIFT*/     NULL, NULL, NULL, NULL},
    {/*LEFT_CONTROL*/   NULL, NULL, NULL, NULL},
    {/*LEFT_ALT*/       NULL, NULL, NULL, NULL},
    {/*LEFT_SUPER*/     NULL, NULL, NULL, NULL},
    {/*RIGHT_SHIFT*/    NULL, NULL, NULL, NULL},
    {/*RIGHT_CONTROL*/  NULL, NULL, NULL, NULL},
    {/*RIGHT_ALT*/      NULL, NULL, NULL, NULL},
    {/*RIGHT_SUPER*/    NULL, NULL, NULL, NULL},
    {/*MENUP*/          NULL, NULL, NULL, NULL},
};

const char* _glfwGetScancodeNameNull(int scancode, int modifiers)
{
    if (scancode < GLFW_NULL_SC_FIRST || scancode > GLFW_NULL_SC_LAST)
    {
        _glfwInputError(GLFW_INVALID_VALUE, "Invalid scancode %i", scancode);
        return NULL;
    }

    int level = 0;
    if (modifiers & GLFW_MOD_SHIFT)
        level |= 1;
    if ((modifiers & GLFW_MOD_CONTROL) && (modifiers & GLFW_MOD_ALT))
        level |= 2;
    return keyNames[scancode][level];
}

int _glfwGetKeyScancodeNull(int key)
{
    return _glfw.null.scancodes[key];
}

void _glfwGetRequiredInstanceExtensionsNull(char** extensions)
{
    if (!_glfw.vk.KHR_surface || !_glfw.vk.EXT_headless_surface)
        return;

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_EXT_headless_surface";
}

GLFWbool _glfwGetPhysicalDevicePresentationSupportNull(VkInstance instance,
                                                       VkPhysicalDevice device,
                                                       uint32_t queuefamily)
{
    return GLFW_TRUE;
}

VkResult _glfwCreateWindowSurfaceNull(VkInstance instance,
                                      _GLFWwindow* window,
                                      const VkAllocationCallbacks* allocator,
                                      VkSurfaceKHR* surface)
{
    PFN_vkCreateHeadlessSurfaceEXT vkCreateHeadlessSurfaceEXT =
        (PFN_vkCreateHeadlessSurfaceEXT)
        vkGetInstanceProcAddr(instance, "vkCreateHeadlessSurfaceEXT");
    if (!vkCreateHeadlessSurfaceEXT)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "Null: Vulkan instance missing VK_EXT_headless_surface extension");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    VkHeadlessSurfaceCreateInfoEXT sci;
    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;

    const VkResult err = vkCreateHeadlessSurfaceEXT(instance, &sci, allocator, surface);
    if (err)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Null: Failed to create Vulkan surface: %s",
                        _glfwGetVulkanResultString(err));
    }

    return err;
}

