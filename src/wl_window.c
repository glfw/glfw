//========================================================================
// GLFW 3.1 Wayland - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2014 Jonas Ådahl <jadahl@gmail.com>
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

#include <stdio.h>
#include <poll.h>
#include <GL/gl.h>
#include <wayland-egl.h>


static void handlePing(void* data,
                       struct wl_shell_surface* shellSurface,
                       uint32_t serial)
{
    wl_shell_surface_pong(shellSurface, serial);
}

static void handleConfigure(void* data,
                            struct wl_shell_surface* shellSurface,
                            uint32_t edges,
                            int32_t width,
                            int32_t height)
{
}

static void handlePopupDone(void* data,
                            struct wl_shell_surface* shellSurface)
{
}

static const struct wl_shell_surface_listener shellSurfaceListener = {
    handlePing,
    handleConfigure,
    handlePopupDone
};

static GLboolean createSurface(_GLFWwindow* window,
                               const _GLFWwndconfig* wndconfig)
{
    window->wl.surface = wl_compositor_create_surface(_glfw.wl.compositor);
    if (!window->wl.surface)
        return GL_FALSE;

    window->wl.native = wl_egl_window_create(window->wl.surface,
                                             wndconfig->width,
                                             wndconfig->height);
    if (!window->wl.native)
        return GL_FALSE;

    window->wl.shell_surface = wl_shell_get_shell_surface(_glfw.wl.shell,
                                                          window->wl.surface);
    if (!window->wl.shell_surface)
        return GL_FALSE;

    wl_shell_surface_add_listener(window->wl.shell_surface,
                                  &shellSurfaceListener,
                                  window);

    window->wl.width = wndconfig->width;
    window->wl.height = wndconfig->height;

    return GL_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWctxconfig* ctxconfig,
                              const _GLFWfbconfig* fbconfig)
{
    if (!_glfwCreateContext(window, ctxconfig, fbconfig))
        return GL_FALSE;

    if (!createSurface(window, wndconfig))
        return GL_FALSE;

    if (wndconfig->monitor)
    {
        wl_shell_surface_set_fullscreen(
            window->wl.shell_surface,
            WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
            0,
            wndconfig->monitor->wl.output);
    }
    else
    {
        wl_shell_surface_set_toplevel(window->wl.shell_surface);
    }

    return GL_TRUE;
}

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
    if (window->wl.native)
        wl_egl_window_destroy(window->wl.native);

    _glfwDestroyContext(window);

    if (window->wl.shell_surface)
        wl_shell_surface_destroy(window->wl.shell_surface);

    if (window->wl.surface)
        wl_surface_destroy(window->wl.surface);
}

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
    wl_shell_surface_set_title(window->wl.shell_surface, title);
}

void _glfwPlatformGetWindowPos(_GLFWwindow* window, int* xpos, int* ypos)
{
    // A Wayland client is not aware of its position, so just warn and set it
    // to (0, 0)

    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Wayland: Window position retreival not supported");

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;
}

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int xpos, int ypos)
{
    // A Wayland client can not set its position, so just warn

    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Wayland: Window position setting not supported");
}

void _glfwPlatformGetWindowSize(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->wl.width;
    if (height)
        *height = window->wl.height;
}

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    wl_egl_window_resize(window->wl.native, width, height, 0, 0);
    window->wl.width = width;
    window->wl.height = height;
}

void _glfwPlatformGetFramebufferSize(_GLFWwindow* window, int* width, int* height)
{
    _glfwPlatformGetWindowSize(window, width, height);
}

void _glfwPlatformGetWindowFrameSize(_GLFWwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
    // TODO
    fprintf(stderr, "_glfwPlatformGetWindowFrameSize not implemented yet\n");
}

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
    // TODO
    fprintf(stderr, "_glfwPlatformIconifyWindow not implemented yet\n");
}

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
    // TODO
    fprintf(stderr, "_glfwPlatformRestoreWindow not implemented yet\n");
}

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
    wl_shell_surface_set_toplevel(window->wl.shell_surface);
}

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
    wl_surface_attach(window->wl.surface, NULL, 0, 0);
    wl_surface_commit(window->wl.surface);
}

void _glfwPlatformPollEvents(void)
{
    struct wl_display* display = _glfw.wl.display;
    struct pollfd fds[] = {
        { wl_display_get_fd(display), POLLIN },
    };

    while (wl_display_prepare_read(display) != 0)
        wl_display_dispatch_pending(display);
    wl_display_flush(display);
    if (poll(fds, 1, 0) > 0)
    {
        wl_display_read_events(display);
        wl_display_dispatch_pending(display);
    }
    else
    {
        wl_display_cancel_read(display);
    }
}

void _glfwPlatformWaitEvents(void)
{
    struct wl_display* display = _glfw.wl.display;
    struct pollfd fds[] = {
        { wl_display_get_fd(display), POLLIN },
    };

    while (wl_display_prepare_read(display) != 0)
        wl_display_dispatch_pending(display);
    wl_display_flush(display);
    if (poll(fds, 1, -1) > 0)
    {
        wl_display_read_events(display);
        wl_display_dispatch_pending(display);
    }
    else
    {
        wl_display_cancel_read(display);
    }
}

void _glfwPlatformPostEmptyEvent(void)
{
    wl_display_sync(_glfw.wl.display);
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double x, double y)
{
    // A Wayland client can not set the cursor position
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Wayland: Cursor position setting not supported");
}

void _glfwPlatformApplyCursorMode(_GLFWwindow* window)
{
    fprintf(stderr, "_glfwPlatformApplyCursorMode not implemented yet\n");
    switch (window->cursorMode)
    {
        case GLFW_CURSOR_NORMAL:
            // TODO: enable showing cursor
            break;
        case GLFW_CURSOR_HIDDEN:
            // TODO: enable not showing cursor
            break;
        case GLFW_CURSOR_DISABLED:
            // TODO: enable pointer lock and hide cursor
            break;
    }
}

int _glfwPlatformCreateCursor(_GLFWcursor* cursor,
                              const GLFWimage* image,
                              int xhot, int yhot)
{
    fprintf(stderr, "_glfwPlatformCreateCursor not implemented yet\n");
    return GL_FALSE;
}

void _glfwPlatformDestroyCursor(_GLFWcursor* cursor)
{
    fprintf(stderr, "_glfwPlatformDestroyCursor not implemented yet\n");
}

void _glfwPlatformSetCursor(_GLFWwindow* window, _GLFWcursor* cursor)
{
    fprintf(stderr, "_glfwPlatformSetCursor not implemented yet\n");
}

