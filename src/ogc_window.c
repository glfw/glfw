//========================================================================
// GLFW 3.5 OGC - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2024 Alberto Mardegan <info@mardy.it>
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

#define _GNU_SOURCE

#include "internal.h"

#if defined(_GLFW_OGC)

#include "ogc_cursor.h"

#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <opengx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiikeyboard/keyboard.h>
#ifdef __wii__
#include <wiiuse/wpad.h>
#endif

static void keysym_to_utf8(uint16_t symbol, char *utf8)
{
    /* ignore private symbols, used by wiikeyboard for special keys */
    if ((symbol >= 0xE000 && symbol <= 0xF8FF) || symbol == 0xFFFF)
        return;

    /* convert UCS-2 to UTF-8 */
    if (symbol < 0x80) {
        utf8[0] = symbol;
    } else if (symbol < 0x800) {
        utf8[0] = 0xC0 | (symbol >> 6);
        utf8[1] = 0x80 | (symbol & 0x3F);
    } else {
        utf8[0] = 0xE0 |  (symbol >> 12);
        utf8[1] = 0x80 | ((symbol >> 6) & 0x3F);
        utf8[2] = 0x80 |  (symbol & 0x3F);
    }
}

static void acquireMonitor(_GLFWwindow* window)
{
    fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__);
    if (!window->monitor) {
        window->monitor = _glfw.monitors[0];
    }
    _glfwInputMonitorWindow(window->monitor, window);
    _glfwSetVideoModeOgc(window->monitor, &window->videoMode);
}

static void releaseMonitor(_GLFWwindow* window)
{
    if (window->monitor->window != window)
        return;

    _glfwInputMonitorWindow(window->monitor, NULL);
}

/* Convert the RGBA pixels to the 4x4 tile ARGB texture used by GX */
static void setPixelToTextureARGB(char *texture, u32 offset, u32 color)
{
    *(u16*)(texture + offset) = color >> 16;
    *(u16*)(texture + offset + 32) = color;
}

static void setPixelToTextureRGBA(char *texture, u32 offset, u32 color)
{
    setPixelToTextureARGB(texture, offset, (color << 24) | (color >> 8));
}

static void pixelsToTextureARGB(const void *pixels, s16 width, s16 height,
                                s16 pitch, void *texture)
{
    s16 tex_pitch = (width + 3) / 4 * 4;
    int row, col;

    for (row = 0; row < height; row++) {
        int y = row;
        u32 *src = (u32 *)((u8 *)pixels + pitch * row);
        for (col = 0; col < width; col++) {
            int x = col;
            u32 offset = (((y >> 2) << 4) * tex_pitch) +
                ((x >> 2) << 6) + (((y % 4 << 2) + x % 4) << 1);
            setPixelToTextureRGBA(texture, offset, *src++);
        }
    }
}

static void* createRGBAtexture(const GLFWimage* image)
{
    u32 textureSize;
    void *texels;

    textureSize = GX_GetTexBufferSize(image->width, image->height, GX_TF_RGBA8,
                                      GX_FALSE, 0);
    texels = memalign(32, textureSize);
    if (!texels) {
        _glfwInputError(GLFW_OUT_OF_MEMORY,
                        "OGC: Failed to allocate cursor texture");
        return NULL;
    }

    pixelsToTextureARGB(image->pixels, image->width, image->height,
                        image->width * 4, texels);
    DCStoreRange(texels, textureSize);
    return texels;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwCreateWindowOgc(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWctxconfig* ctxconfig,
                              const _GLFWfbconfig* fbconfig)
{
    fprintf(stderr, "%s:%d %s %p\n", __FILE__, __LINE__, __func__, window);

    acquireMonitor(window);
    window->ogc.width = window->monitor->ogc.ogcMode->fbWidth;
    window->ogc.height = window->monitor->ogc.ogcMode->efbHeight;

    if (ctxconfig->client != GLFW_NO_API)
    {
        ogx_initialize();

        if (!_glfwCreateContextOgc(window, ctxconfig, fbconfig))
            return GLFW_FALSE;

        if (!_glfwRefreshContextAttribs(window, ctxconfig))
            return GLFW_FALSE;
    }

    /* Set the default Wii cursor */
    GLFWcursor *cursor = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);
    _glfwSetCursorOgc(window, (_GLFWcursor*) cursor);

    return GLFW_TRUE;
}

void _glfwDestroyWindowOgc(_GLFWwindow* window)
{
    if (window->monitor)
        releaseMonitor(window);
}

void _glfwSetWindowTitleOgc(_GLFWwindow* window, const char* title)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "setting the window title");
}

void _glfwSetWindowIconOgc(_GLFWwindow* window,
                               int count, const GLFWimage* images)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "setting the window icon");
}

void _glfwGetWindowPosOgc(_GLFWwindow* window, int* xpos, int* ypos)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "getting the window position");
}

void _glfwSetWindowPosOgc(_GLFWwindow* window, int xpos, int ypos)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "setting the window position");
}

void _glfwGetWindowSizeOgc(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->ogc.width;
    if (height)
        *height = window->ogc.height;
}

void _glfwSetWindowSizeOgc(_GLFWwindow* window, int width, int height)
{
    fprintf(stderr, "%s:%d %s %dx%d\n", __FILE__, __LINE__, __func__, width, height);
    if (window->monitor)
    {
    }
    else
    {
        _glfwInputWindowDamage(window);
    }
}

void _glfwSetWindowSizeLimitsOgc(_GLFWwindow* window,
                                 int minwidth, int minheight,
                                 int maxwidth, int maxheight)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "setting the size limits");
}

void _glfwSetWindowAspectRatioOgc(_GLFWwindow* window, int numer, int denom)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "setting the aspect ratio");
}

void _glfwGetFramebufferSizeOgc(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->monitor->ogc.ogcMode->fbWidth;
    if (height)
        *height = window->monitor->ogc.ogcMode->efbHeight;;
}

void _glfwGetWindowFrameSizeOgc(_GLFWwindow* window,
                                int* left, int* top,
                                int* right, int* bottom)
{
    *left = *top = *right = *bottom = 0;
}

void _glfwGetWindowContentScaleOgc(_GLFWwindow* window,
                                   float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.0f;
    if (yscale)
        *yscale = 1.0f;
}

void _glfwIconifyWindowOgc(_GLFWwindow* window)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "iconifying the window");
}

void _glfwRestoreWindowOgc(_GLFWwindow* window)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "restoring the window");
}

void _glfwMaximizeWindowOgc(_GLFWwindow* window)
{
}

void _glfwShowWindowOgc(_GLFWwindow* window)
{
}

void _glfwHideWindowOgc(_GLFWwindow* window)
{
}

void _glfwRequestWindowAttentionOgc(_GLFWwindow* window)
{
}

void _glfwFocusWindowOgc(_GLFWwindow* window)
{
}

void _glfwSetWindowMonitorOgc(_GLFWwindow* window,
                              _GLFWmonitor* monitor,
                              int xpos, int ypos,
                              int width, int height,
                              int refreshRate)
{
}

GLFWbool _glfwWindowFocusedOgc(_GLFWwindow* window)
{
    return GLFW_TRUE;
}

GLFWbool _glfwWindowIconifiedOgc(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

GLFWbool _glfwWindowVisibleOgc(_GLFWwindow* window)
{
    return GLFW_TRUE;
}

GLFWbool _glfwWindowMaximizedOgc(_GLFWwindow* window)
{
    return GLFW_TRUE;
}

GLFWbool _glfwWindowHoveredOgc(_GLFWwindow* window)
{
    return window->ogc.hovered;
}

GLFWbool _glfwFramebufferTransparentOgc(_GLFWwindow* window)
{
    return GLFW_FALSE;
}

void _glfwSetWindowResizableOgc(_GLFWwindow* window, GLFWbool enabled)
{
}

void _glfwSetWindowDecoratedOgc(_GLFWwindow* window, GLFWbool enabled)
{
}

void _glfwSetWindowFloatingOgc(_GLFWwindow* window, GLFWbool enabled)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "making a window floating");
}

void _glfwSetWindowMousePassthroughOgc(_GLFWwindow* window, GLFWbool enabled)
{
}

float _glfwGetWindowOpacityOgc(_GLFWwindow* window)
{
    return 1.f;
}

void _glfwSetWindowOpacityOgc(_GLFWwindow* window, float opacity)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "setting the window opacity");
}

void _glfwSetRawMouseMotionOgc(_GLFWwindow* window, GLFWbool enabled)
{
}

GLFWbool _glfwRawMouseMotionSupportedOgc(void)
{
    return GLFW_FALSE;
}

#ifdef __wii__
static void handleWiimoteCursorOgc(_GLFWwindow* window)
{
    WPADData *data = WPAD_Data(0);
    float x, y;

    GLFWbool mouseActive = !!data->ir.valid;
    if (mouseActive != window->ogc.hovered) {
        window->ogc.hovered = mouseActive;
        _glfwInputCursorEnter(window, mouseActive);
    }

    if (!mouseActive) return;

    x = data->ir.x * window->ogc.width / data->ir.vres[0];
    y = data->ir.y * window->ogc.height / data->ir.vres[1];
    _glfwInputCursorPos(window, x, y);

    if (data->btns_d & WPAD_BUTTON_A)
        _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_1, GLFW_PRESS, 0);
    if (data->btns_u & WPAD_BUTTON_A)
        _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_1, GLFW_RELEASE, 0);
    if (data->btns_d & WPAD_BUTTON_B)
        _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_2, GLFW_PRESS, 0);
    if (data->btns_u & WPAD_BUTTON_B)
        _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_2, GLFW_RELEASE, 0);
}
#endif

void _glfwPollEventsOgc(void)
{
    //fprintf(stderr, "Polling for events %d\n", 0);
    GLFWbool hasEvents = GLFW_FALSE;

    if (_glfwPollJoysticksOgc()) {
        hasEvents = GLFW_TRUE;
#ifdef __wii__
        /* Use the first wiimote as mouse */
        if (_glfw.windowListHead)
            handleWiimoteCursorOgc(_glfw.windowListHead);
#endif
    }

}

void _glfwWaitEventsOgc(void)
{
    // TODO: check if events were produced or sleep?
    _glfwPollEventsOgc();
}

void _glfwWaitEventsTimeoutOgc(double timeout)
{
    // TODO: check if events were produced or sleep?
    _glfwPollEventsOgc();
}

void _glfwPostEmptyEventOgc(void)
{
}

void _glfwGetCursorPosOgc(_GLFWwindow* window, double* xpos, double* ypos)
{
    if (xpos)
        *xpos = window->virtualCursorPosX;
    if (ypos)
        *ypos = window->virtualCursorPosY;
}

void _glfwSetCursorPosOgc(_GLFWwindow* window, double x, double y)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "setting the cursor position");
}

void _glfwSetCursorModeOgc(_GLFWwindow* window, int mode)
{
    // TODO
}

const char* _glfwGetScancodeNameOgc(int scancode)
{
    static char name[10];

    name[0] = name[1] = '\0';

    switch (scancode) {
    case KS_Return: name[0] = '\n'; break;
    case KS_Escape: return "ESC";
    default:
        keysym_to_utf8(scancode, name);
    }

    return name;
}

int _glfwGetKeyScancodeOgc(int key)
{
    return _glfw.ogc.scancodes[key];
}

GLFWbool _glfwCreateCursorOgc(_GLFWcursor* cursor,
                              const GLFWimage* image,
                              int xhot, int yhot)
{
    void* texels = createRGBAtexture(image);
    if (!texels) return GLFW_FALSE;

    cursor->ogc.xhot = xhot;
    cursor->ogc.yhot = yhot;

    GX_InvalidateTexAll();
    GX_InitTexObj(&cursor->ogc.texobj, texels,
                  image->width, image->height,
                  GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GX_InitTexObjLOD(&cursor->ogc.texobj, GX_LINEAR, GX_LINEAR,
                     0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);

    return GLFW_TRUE;
}

GLFWbool _glfwCreateStandardCursorOgc(_GLFWcursor* cursor, int shape)
{
    const _GLFWcursorDataOgc* cd = NULL;

    switch (shape)
    {
        case GLFW_ARROW_CURSOR:
            cd = &OGC_cursor_arrow;
            break;
        case GLFW_POINTING_HAND_CURSOR:
            cd = &OGC_cursor_hand;
            break;
        default:
            return GLFW_FALSE;
    }

    GLFWimage image = {
        cd->width, cd->height, cd->pixel_data
    };
    return _glfwCreateCursorOgc(cursor, &image,
                                cd->hot_x, cd->hot_y);
}

void _glfwDestroyCursorOgc(_GLFWcursor* cursor)
{
    void *data = GX_GetTexObjData(&cursor->ogc.texobj);
    if (data != 0)
        free(MEM_PHYSICAL_TO_K0(data));

}

void _glfwSetCursorOgc(_GLFWwindow* window, _GLFWcursor* cursor)
{
    window->ogc.currentCursor = cursor;
}

void _glfwSetClipboardStringOgc(const char* string)
{
}

const char* _glfwGetClipboardStringOgc(void)
{
    return NULL;
}

EGLenum _glfwGetEGLPlatformOgc(EGLint** attribs)
{
    return EGL_PLATFORM_ANGLE_ANGLE;
}

EGLNativeDisplayType _glfwGetEGLNativeDisplayOgc(void)
{
    return EGL_DEFAULT_DISPLAY;
}

EGLNativeWindowType _glfwGetEGLNativeWindowOgc(_GLFWwindow* window)
{
    return 0;
}

void _glfwGetRequiredInstanceExtensionsOgc(char** extensions)
{
}

GLFWbool _glfwGetPhysicalDevicePresentationSupportOgc(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    return GLFW_FALSE;
}

VkResult _glfwCreateWindowSurfaceOgc(VkInstance instance,
                                     _GLFWwindow* window,
                                     const VkAllocationCallbacks* allocator,
                                     VkSurfaceKHR* surface)
{
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

#endif // _GLFW_OGC
