//========================================================================
// GLFW 3.4 Wayland - www.glfw.org
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#define _GNU_SOURCE

#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <poll.h>

#include "wayland-client-protocol.h"
#include "wayland-xdg-shell-client-protocol.h"
#include "wayland-xdg-decoration-client-protocol.h"
#include "wayland-viewporter-client-protocol.h"
#include "wayland-relative-pointer-unstable-v1-client-protocol.h"
#include "wayland-pointer-constraints-unstable-v1-client-protocol.h"
#include "wayland-idle-inhibit-unstable-v1-client-protocol.h"


static int createTmpfileCloexec(char* tmpname)
{
    int fd;

    fd = mkostemp(tmpname, O_CLOEXEC);
    if (fd >= 0)
        unlink(tmpname);

    return fd;
}

/*
 * Create a new, unique, anonymous file of the given size, and
 * return the file descriptor for it. The file descriptor is set
 * CLOEXEC. The file is immediately suitable for mmap()'ing
 * the given size at offset zero.
 *
 * The file should not have a permanent backing store like a disk,
 * but may have if XDG_RUNTIME_DIR is not properly implemented in OS.
 *
 * The file name is deleted from the file system.
 *
 * The file is suitable for buffer sharing between processes by
 * transmitting the file descriptor over Unix sockets using the
 * SCM_RIGHTS methods.
 *
 * posix_fallocate() is used to guarantee that disk space is available
 * for the file at the given size. If disk space is insufficient, errno
 * is set to ENOSPC. If posix_fallocate() is not supported, program may
 * receive SIGBUS on accessing mmap()'ed file contents instead.
 */
static int createAnonymousFile(off_t size)
{
    static const char template[] = "/glfw-shared-XXXXXX";
    const char* path;
    char* name;
    int fd;
    int ret;

#ifdef HAVE_MEMFD_CREATE
    fd = memfd_create("glfw-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    if (fd >= 0)
    {
        // We can add this seal before calling posix_fallocate(), as the file
        // is currently zero-sized anyway.
        //
        // There is also no need to check for the return value, we couldn’t do
        // anything with it anyway.
        fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_SEAL);
    }
    else
#elif defined(SHM_ANON)
    fd = shm_open(SHM_ANON, O_RDWR | O_CLOEXEC, 0600);
    if (fd < 0)
#endif
    {
        path = getenv("XDG_RUNTIME_DIR");
        if (!path)
        {
            errno = ENOENT;
            return -1;
        }

        name = _glfw_calloc(strlen(path) + sizeof(template), 1);
        strcpy(name, path);
        strcat(name, template);

        fd = createTmpfileCloexec(name);
        _glfw_free(name);
        if (fd < 0)
            return -1;
    }

#if defined(SHM_ANON)
    // posix_fallocate does not work on SHM descriptors
    ret = ftruncate(fd, size);
#else
    ret = posix_fallocate(fd, 0, size);
#endif
    if (ret != 0)
    {
        close(fd);
        errno = ret;
        return -1;
    }
    return fd;
}

static struct wl_buffer* createShmBuffer(const GLFWimage* image)
{
    struct wl_shm_pool* pool;
    struct wl_buffer* buffer;
    int stride = image->width * 4;
    int length = image->width * image->height * 4;
    void* data;

    const int fd = createAnonymousFile(length);
    if (fd < 0)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create buffer file of size %d: %s",
                        length, strerror(errno));
        return NULL;
    }

    data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to map file: %s", strerror(errno));
        close(fd);
        return NULL;
    }

    pool = wl_shm_create_pool(_glfw.wl.shm, fd, length);

    close(fd);
    unsigned char* source = (unsigned char*) image->pixels;
    unsigned char* target = data;
    for (int i = 0;  i < image->width * image->height;  i++, source += 4)
    {
        unsigned int alpha = source[3];

        *target++ = (unsigned char) ((source[2] * alpha) / 255);
        *target++ = (unsigned char) ((source[1] * alpha) / 255);
        *target++ = (unsigned char) ((source[0] * alpha) / 255);
        *target++ = (unsigned char) alpha;
    }

    buffer =
        wl_shm_pool_create_buffer(pool, 0,
                                  image->width,
                                  image->height,
                                  stride, WL_SHM_FORMAT_ARGB8888);
    munmap(data, length);
    wl_shm_pool_destroy(pool);

    return buffer;
}

static void createDecoration(_GLFWdecorationWayland* decoration,
                             struct wl_surface* parent,
                             struct wl_buffer* buffer, GLFWbool opaque,
                             int x, int y,
                             int width, int height)
{
    struct wl_region* region;

    decoration->surface = wl_compositor_create_surface(_glfw.wl.compositor);
    decoration->subsurface =
        wl_subcompositor_get_subsurface(_glfw.wl.subcompositor,
                                        decoration->surface, parent);
    wl_subsurface_set_position(decoration->subsurface, x, y);
    decoration->viewport = wp_viewporter_get_viewport(_glfw.wl.viewporter,
                                                      decoration->surface);
    wp_viewport_set_destination(decoration->viewport, width, height);
    wl_surface_attach(decoration->surface, buffer, 0, 0);

    if (opaque)
    {
        region = wl_compositor_create_region(_glfw.wl.compositor);
        wl_region_add(region, 0, 0, width, height);
        wl_surface_set_opaque_region(decoration->surface, region);
        wl_surface_commit(decoration->surface);
        wl_region_destroy(region);
    }
    else
        wl_surface_commit(decoration->surface);
}

static void createDecorations(_GLFWwindow* window)
{
    unsigned char data[] = { 224, 224, 224, 255 };
    const GLFWimage image = { 1, 1, data };
    GLFWbool opaque = (data[3] == 255);

    if (!_glfw.wl.viewporter || !window->decorated || window->wl.decorations.serverSide)
        return;

    if (!window->wl.decorations.buffer)
        window->wl.decorations.buffer = createShmBuffer(&image);
    if (!window->wl.decorations.buffer)
        return;

    createDecoration(&window->wl.decorations.top, window->wl.surface,
                     window->wl.decorations.buffer, opaque,
                     0, -_GLFW_DECORATION_TOP,
                     window->wl.width, _GLFW_DECORATION_TOP);
    createDecoration(&window->wl.decorations.left, window->wl.surface,
                     window->wl.decorations.buffer, opaque,
                     -_GLFW_DECORATION_WIDTH, -_GLFW_DECORATION_TOP,
                     _GLFW_DECORATION_WIDTH, window->wl.height + _GLFW_DECORATION_TOP);
    createDecoration(&window->wl.decorations.right, window->wl.surface,
                     window->wl.decorations.buffer, opaque,
                     window->wl.width, -_GLFW_DECORATION_TOP,
                     _GLFW_DECORATION_WIDTH, window->wl.height + _GLFW_DECORATION_TOP);
    createDecoration(&window->wl.decorations.bottom, window->wl.surface,
                     window->wl.decorations.buffer, opaque,
                     -_GLFW_DECORATION_WIDTH, window->wl.height,
                     window->wl.width + _GLFW_DECORATION_HORIZONTAL, _GLFW_DECORATION_WIDTH);
}

static void destroyDecoration(_GLFWdecorationWayland* decoration)
{
    if (decoration->subsurface)
        wl_subsurface_destroy(decoration->subsurface);
    if (decoration->surface)
        wl_surface_destroy(decoration->surface);
    if (decoration->viewport)
        wp_viewport_destroy(decoration->viewport);
    decoration->surface = NULL;
    decoration->subsurface = NULL;
    decoration->viewport = NULL;
}

static void destroyDecorations(_GLFWwindow* window)
{
    destroyDecoration(&window->wl.decorations.top);
    destroyDecoration(&window->wl.decorations.left);
    destroyDecoration(&window->wl.decorations.right);
    destroyDecoration(&window->wl.decorations.bottom);
}

static void xdgDecorationHandleConfigure(void* userData,
                                         struct zxdg_toplevel_decoration_v1* decoration,
                                         uint32_t mode)
{
    _GLFWwindow* window = userData;

    window->wl.decorations.serverSide = (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

    if (!window->wl.decorations.serverSide)
        createDecorations(window);
}

static const struct zxdg_toplevel_decoration_v1_listener xdgDecorationListener =
{
    xdgDecorationHandleConfigure,
};

// Makes the surface considered as XRGB instead of ARGB.
static void setOpaqueRegion(_GLFWwindow* window)
{
    struct wl_region* region;

    region = wl_compositor_create_region(_glfw.wl.compositor);
    if (!region)
        return;

    wl_region_add(region, 0, 0, window->wl.width, window->wl.height);
    wl_surface_set_opaque_region(window->wl.surface, region);
    wl_surface_commit(window->wl.surface);
    wl_region_destroy(region);
}


static void resizeWindow(_GLFWwindow* window)
{
    int scale = window->wl.scale;
    int scaledWidth = window->wl.width * scale;
    int scaledHeight = window->wl.height * scale;
    wl_egl_window_resize(window->wl.native, scaledWidth, scaledHeight, 0, 0);
    if (!window->wl.transparent)
        setOpaqueRegion(window);
    _glfwInputFramebufferSize(window, scaledWidth, scaledHeight);
    _glfwInputWindowContentScale(window, scale, scale);

    if (!window->wl.decorations.top.surface)
        return;

    // Top decoration.
    wp_viewport_set_destination(window->wl.decorations.top.viewport,
                                window->wl.width, _GLFW_DECORATION_TOP);
    wl_surface_commit(window->wl.decorations.top.surface);

    // Left decoration.
    wp_viewport_set_destination(window->wl.decorations.left.viewport,
                                _GLFW_DECORATION_WIDTH, window->wl.height + _GLFW_DECORATION_TOP);
    wl_surface_commit(window->wl.decorations.left.surface);

    // Right decoration.
    wl_subsurface_set_position(window->wl.decorations.right.subsurface,
                               window->wl.width, -_GLFW_DECORATION_TOP);
    wp_viewport_set_destination(window->wl.decorations.right.viewport,
                                _GLFW_DECORATION_WIDTH, window->wl.height + _GLFW_DECORATION_TOP);
    wl_surface_commit(window->wl.decorations.right.surface);

    // Bottom decoration.
    wl_subsurface_set_position(window->wl.decorations.bottom.subsurface,
                               -_GLFW_DECORATION_WIDTH, window->wl.height);
    wp_viewport_set_destination(window->wl.decorations.bottom.viewport,
                                window->wl.width + _GLFW_DECORATION_HORIZONTAL, _GLFW_DECORATION_WIDTH);
    wl_surface_commit(window->wl.decorations.bottom.surface);
}

static void checkScaleChange(_GLFWwindow* window)
{
    // Check if we will be able to set the buffer scale or not.
    if (_glfw.wl.compositorVersion < 3)
        return;

    // Get the scale factor from the highest scale monitor.
    int maxScale = 1;

    for (int i = 0; i < window->wl.monitorsCount; i++)
        maxScale = _glfw_max(window->wl.monitors[i]->wl.scale, maxScale);

    // Only change the framebuffer size if the scale changed.
    if (window->wl.scale != maxScale)
    {
        window->wl.scale = maxScale;
        wl_surface_set_buffer_scale(window->wl.surface, maxScale);
        resizeWindow(window);
    }
}

static void surfaceHandleEnter(void* userData,
                               struct wl_surface* surface,
                               struct wl_output* output)
{
    _GLFWwindow* window = userData;
    _GLFWmonitor* monitor = wl_output_get_user_data(output);

    if (window->wl.monitorsCount + 1 > window->wl.monitorsSize)
    {
        ++window->wl.monitorsSize;
        window->wl.monitors =
            _glfw_realloc(window->wl.monitors,
                          window->wl.monitorsSize * sizeof(_GLFWmonitor*));
    }

    window->wl.monitors[window->wl.monitorsCount++] = monitor;

    checkScaleChange(window);
}

static void surfaceHandleLeave(void* userData,
                               struct wl_surface* surface,
                               struct wl_output* output)
{
    _GLFWwindow* window = userData;
    _GLFWmonitor* monitor = wl_output_get_user_data(output);
    GLFWbool found = GLFW_FALSE;

    for (int i = 0; i < window->wl.monitorsCount - 1; ++i)
    {
        if (monitor == window->wl.monitors[i])
            found = GLFW_TRUE;
        if (found)
            window->wl.monitors[i] = window->wl.monitors[i + 1];
    }
    window->wl.monitors[--window->wl.monitorsCount] = NULL;

    checkScaleChange(window);
}

static const struct wl_surface_listener surfaceListener = {
    surfaceHandleEnter,
    surfaceHandleLeave
};

static void setIdleInhibitor(_GLFWwindow* window, GLFWbool enable)
{
    if (enable && !window->wl.idleInhibitor && _glfw.wl.idleInhibitManager)
    {
        window->wl.idleInhibitor =
            zwp_idle_inhibit_manager_v1_create_inhibitor(
                _glfw.wl.idleInhibitManager, window->wl.surface);
        if (!window->wl.idleInhibitor)
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "Wayland: Failed to create idle inhibitor");
    }
    else if (!enable && window->wl.idleInhibitor)
    {
        zwp_idle_inhibitor_v1_destroy(window->wl.idleInhibitor);
        window->wl.idleInhibitor = NULL;
    }
}

static void setFullscreen(_GLFWwindow* window, _GLFWmonitor* monitor,
                          int refreshRate)
{
    if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_set_fullscreen(
            window->wl.xdg.toplevel,
            monitor->wl.output);
    }
    setIdleInhibitor(window, GLFW_TRUE);
    if (!window->wl.decorations.serverSide)
        destroyDecorations(window);
}

static void xdgToplevelHandleConfigure(void* userData,
                                       struct xdg_toplevel* toplevel,
                                       int32_t width,
                                       int32_t height,
                                       struct wl_array* states)
{
    _GLFWwindow* window = userData;
    float aspectRatio;
    float targetRatio;
    uint32_t* state;
    GLFWbool maximized = GLFW_FALSE;
    GLFWbool fullscreen = GLFW_FALSE;
    GLFWbool activated = GLFW_FALSE;

    wl_array_for_each(state, states)
    {
        switch (*state)
        {
            case XDG_TOPLEVEL_STATE_MAXIMIZED:
                maximized = GLFW_TRUE;
                break;
            case XDG_TOPLEVEL_STATE_FULLSCREEN:
                fullscreen = GLFW_TRUE;
                break;
            case XDG_TOPLEVEL_STATE_RESIZING:
                break;
            case XDG_TOPLEVEL_STATE_ACTIVATED:
                activated = GLFW_TRUE;
                break;
        }
    }

    if (width != 0 && height != 0)
    {
        if (!maximized && !fullscreen)
        {
            if (window->numer != GLFW_DONT_CARE && window->denom != GLFW_DONT_CARE)
            {
                aspectRatio = (float)width / (float)height;
                targetRatio = (float)window->numer / (float)window->denom;
                if (aspectRatio < targetRatio)
                    height = width / targetRatio;
                else if (aspectRatio > targetRatio)
                    width = height * targetRatio;
            }
        }

        _glfwInputWindowSize(window, width, height);
        _glfwSetWindowSizeWayland(window, width, height);
        _glfwInputWindowDamage(window);
    }

    if (window->wl.wasFullscreen && window->autoIconify)
    {
        if (!activated || !fullscreen)
        {
            _glfwIconifyWindowWayland(window);
            window->wl.wasFullscreen = GLFW_FALSE;
        }
    }
    if (fullscreen && activated)
        window->wl.wasFullscreen = GLFW_TRUE;
}

static void xdgToplevelHandleClose(void* userData,
                                   struct xdg_toplevel* toplevel)
{
    _GLFWwindow* window = userData;
    _glfwInputWindowCloseRequest(window);
}

static const struct xdg_toplevel_listener xdgToplevelListener =
{
    xdgToplevelHandleConfigure,
    xdgToplevelHandleClose
};

static void xdgSurfaceHandleConfigure(void* userData,
                                      struct xdg_surface* surface,
                                      uint32_t serial)
{
    xdg_surface_ack_configure(surface, serial);
}

static const struct xdg_surface_listener xdgSurfaceListener = {
    xdgSurfaceHandleConfigure
};

static void setXdgDecorations(_GLFWwindow* window)
{
    if (_glfw.wl.decorationManager)
    {
        window->wl.xdg.decoration =
            zxdg_decoration_manager_v1_get_toplevel_decoration(
                _glfw.wl.decorationManager, window->wl.xdg.toplevel);
        zxdg_toplevel_decoration_v1_add_listener(window->wl.xdg.decoration,
                                                 &xdgDecorationListener,
                                                 window);
        zxdg_toplevel_decoration_v1_set_mode(
            window->wl.xdg.decoration,
            ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }
    else
    {
        window->wl.decorations.serverSide = GLFW_FALSE;
        createDecorations(window);
    }
}

static GLFWbool createXdgSurface(_GLFWwindow* window)
{
    window->wl.xdg.surface = xdg_wm_base_get_xdg_surface(_glfw.wl.wmBase,
                                                         window->wl.surface);
    if (!window->wl.xdg.surface)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create xdg-surface for window");
        return GLFW_FALSE;
    }

    xdg_surface_add_listener(window->wl.xdg.surface,
                             &xdgSurfaceListener,
                             window);

    window->wl.xdg.toplevel = xdg_surface_get_toplevel(window->wl.xdg.surface);
    if (!window->wl.xdg.toplevel)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create xdg-toplevel for window");
        return GLFW_FALSE;
    }

    xdg_toplevel_add_listener(window->wl.xdg.toplevel,
                              &xdgToplevelListener,
                              window);

    if (window->wl.title)
        xdg_toplevel_set_title(window->wl.xdg.toplevel, window->wl.title);

    if (window->minwidth != GLFW_DONT_CARE && window->minheight != GLFW_DONT_CARE)
        xdg_toplevel_set_min_size(window->wl.xdg.toplevel,
                                  window->minwidth, window->minheight);
    if (window->maxwidth != GLFW_DONT_CARE && window->maxheight != GLFW_DONT_CARE)
        xdg_toplevel_set_max_size(window->wl.xdg.toplevel,
                                  window->maxwidth, window->maxheight);

    if (window->monitor)
    {
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel,
                                    window->monitor->wl.output);
        setIdleInhibitor(window, GLFW_TRUE);
    }
    else if (window->wl.maximized)
    {
        xdg_toplevel_set_maximized(window->wl.xdg.toplevel);
        setIdleInhibitor(window, GLFW_FALSE);
        setXdgDecorations(window);
    }
    else
    {
        setIdleInhibitor(window, GLFW_FALSE);
        setXdgDecorations(window);
    }

    wl_surface_commit(window->wl.surface);
    wl_display_roundtrip(_glfw.wl.display);

    return GLFW_TRUE;
}

static GLFWbool createSurface(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWfbconfig* fbconfig)
{
    window->wl.surface = wl_compositor_create_surface(_glfw.wl.compositor);
    if (!window->wl.surface)
        return GLFW_FALSE;

    wl_surface_add_listener(window->wl.surface,
                            &surfaceListener,
                            window);

    wl_surface_set_user_data(window->wl.surface, window);

    window->wl.native = wl_egl_window_create(window->wl.surface,
                                             wndconfig->width,
                                             wndconfig->height);
    if (!window->wl.native)
        return GLFW_FALSE;

    window->wl.width = wndconfig->width;
    window->wl.height = wndconfig->height;
    window->wl.scale = 1;
    window->wl.title = _glfw_strdup(wndconfig->title);

    window->wl.transparent = fbconfig->transparent;
    if (!window->wl.transparent)
        setOpaqueRegion(window);

    if (window->monitor || wndconfig->visible)
    {
        if (!createXdgSurface(window))
            return GLFW_FALSE;

        window->wl.visible = GLFW_TRUE;
    }

    return GLFW_TRUE;
}

static void setCursorImage(_GLFWwindow* window,
                           _GLFWcursorWayland* cursorWayland)
{
    struct itimerspec timer = {};
    struct wl_cursor* wlCursor = cursorWayland->cursor;
    struct wl_cursor_image* image;
    struct wl_buffer* buffer;
    struct wl_surface* surface = _glfw.wl.cursorSurface;
    int scale = 1;

    if (!wlCursor)
        buffer = cursorWayland->buffer;
    else
    {
        if (window->wl.scale > 1 && cursorWayland->cursorHiDPI)
        {
            wlCursor = cursorWayland->cursorHiDPI;
            scale = 2;
        }

        image = wlCursor->images[cursorWayland->currentImage];
        buffer = wl_cursor_image_get_buffer(image);
        if (!buffer)
            return;

        timer.it_value.tv_sec = image->delay / 1000;
        timer.it_value.tv_nsec = (image->delay % 1000) * 1000000;
        timerfd_settime(_glfw.wl.cursorTimerfd, 0, &timer, NULL);

        cursorWayland->width = image->width;
        cursorWayland->height = image->height;
        cursorWayland->xhot = image->hotspot_x;
        cursorWayland->yhot = image->hotspot_y;
    }

    wl_pointer_set_cursor(_glfw.wl.pointer, _glfw.wl.pointerEnterSerial,
                          surface,
                          cursorWayland->xhot / scale,
                          cursorWayland->yhot / scale);
    wl_surface_set_buffer_scale(surface, scale);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0,
                      cursorWayland->width, cursorWayland->height);
    wl_surface_commit(surface);
}

static void incrementCursorImage(_GLFWwindow* window)
{
    _GLFWcursor* cursor;

    if (!window || window->wl.decorations.focus != mainWindow)
        return;

    cursor = window->wl.currentCursor;
    if (cursor && cursor->wl.cursor)
    {
        cursor->wl.currentImage += 1;
        cursor->wl.currentImage %= cursor->wl.cursor->image_count;
        setCursorImage(window, &cursor->wl);
    }
}

static GLFWbool flushDisplay(void)
{
    while (wl_display_flush(_glfw.wl.display) == -1)
    {
        if (errno != EAGAIN)
            return GLFW_FALSE;

        struct pollfd fd = { wl_display_get_fd(_glfw.wl.display), POLLOUT };

        while (poll(&fd, 1, -1) == -1)
        {
            if (errno != EINTR && errno != EAGAIN)
                return GLFW_FALSE;
        }
    }

    return GLFW_TRUE;
}

static void handleEvents(double* timeout)
{
    GLFWbool event = GLFW_FALSE;
    struct pollfd fds[] =
    {
        { wl_display_get_fd(_glfw.wl.display), POLLIN },
        { _glfw.wl.timerfd, POLLIN },
        { _glfw.wl.cursorTimerfd, POLLIN },
    };

    while (!event)
    {
        while (wl_display_prepare_read(_glfw.wl.display) != 0)
            wl_display_dispatch_pending(_glfw.wl.display);

        // If an error other than EAGAIN happens, we have likely been disconnected
        // from the Wayland session; try to handle that the best we can.
        if (!flushDisplay())
        {
            wl_display_cancel_read(_glfw.wl.display);

            _GLFWwindow* window = _glfw.windowListHead;
            while (window)
            {
                _glfwInputWindowCloseRequest(window);
                window = window->next;
            }

            return;
        }

        if (!_glfwPollPOSIX(fds, 3, timeout))
        {
            wl_display_cancel_read(_glfw.wl.display);
            return;
        }

        if (fds[0].revents & POLLIN)
        {
            wl_display_read_events(_glfw.wl.display);
            if (wl_display_dispatch_pending(_glfw.wl.display) > 0)
                event = GLFW_TRUE;
        }
        else
            wl_display_cancel_read(_glfw.wl.display);

        if (fds[1].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_glfw.wl.timerfd, &repeats, sizeof(repeats)) == 8)
            {
                for (uint64_t i = 0; i < repeats; i++)
                {
                    _glfwInputKey(_glfw.wl.keyboardFocus,
                                  _glfw.wl.keyboardLastKey,
                                  _glfw.wl.keyboardLastScancode,
                                  GLFW_PRESS,
                                  _glfw.wl.xkb.modifiers);
                    _glfwInputTextWayland(_glfw.wl.keyboardFocus,
                                          _glfw.wl.keyboardLastScancode);
                }

                event = GLFW_TRUE;
            }
        }

        if (fds[2].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_glfw.wl.cursorTimerfd, &repeats, sizeof(repeats)) == 8)
            {
                incrementCursorImage(_glfw.wl.pointerFocus);
                event = GLFW_TRUE;
            }
        }
    }
}

// Reads the specified data offer as the specified MIME type
//
static char* readDataOfferAsString(struct wl_data_offer* offer, const char* mimeType)
{
    int fds[2];

    if (pipe2(fds, O_CLOEXEC) == -1)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create pipe for data offer: %s",
                        strerror(errno));
        return NULL;
    }

    wl_data_offer_receive(offer, mimeType, fds[1]);
    flushDisplay();
    close(fds[1]);

    char* string = NULL;
    size_t size = 0;
    size_t length = 0;

    for (;;)
    {
        const size_t readSize = 4096;
        const size_t requiredSize = length + readSize + 1;
        if (requiredSize > size)
        {
            char* longer = _glfw_realloc(string, requiredSize);
            if (!longer)
            {
                _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
                close(fds[0]);
                return NULL;
            }

            string = longer;
            size = requiredSize;
        }

        const ssize_t result = read(fds[0], string + length, readSize);
        if (result == 0)
            break;
        else if (result == -1)
        {
            if (errno == EINTR)
                continue;

            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "Wayland: Failed to read from data offer pipe: %s",
                            strerror(errno));
            close(fds[0]);
            return NULL;
        }

        length += result;
    }

    close(fds[0]);

    string[length] = '\0';
    return string;
}

static _GLFWwindow* findWindowFromDecorationSurface(struct wl_surface* surface,
                                                    int* which)
{
    int focus;
    _GLFWwindow* window = _glfw.windowListHead;
    if (!which)
        which = &focus;
    while (window)
    {
        if (surface == window->wl.decorations.top.surface)
        {
            *which = topDecoration;
            break;
        }
        if (surface == window->wl.decorations.left.surface)
        {
            *which = leftDecoration;
            break;
        }
        if (surface == window->wl.decorations.right.surface)
        {
            *which = rightDecoration;
            break;
        }
        if (surface == window->wl.decorations.bottom.surface)
        {
            *which = bottomDecoration;
            break;
        }
        window = window->next;
    }
    return window;
}

static void pointerHandleEnter(void* userData,
                               struct wl_pointer* pointer,
                               uint32_t serial,
                               struct wl_surface* surface,
                               wl_fixed_t sx,
                               wl_fixed_t sy)
{
    // Happens in the case we just destroyed the surface.
    if (!surface)
        return;

    int focus = 0;
    _GLFWwindow* window = wl_surface_get_user_data(surface);
    if (!window)
    {
        window = findWindowFromDecorationSurface(surface, &focus);
        if (!window)
            return;
    }

    window->wl.decorations.focus = focus;
    _glfw.wl.serial = serial;
    _glfw.wl.pointerEnterSerial = serial;
    _glfw.wl.pointerFocus = window;

    window->wl.hovered = GLFW_TRUE;

    _glfwSetCursorWayland(window, window->wl.currentCursor);
    _glfwInputCursorEnter(window, GLFW_TRUE);
}

static void pointerHandleLeave(void* userData,
                               struct wl_pointer* pointer,
                               uint32_t serial,
                               struct wl_surface* surface)
{
    _GLFWwindow* window = _glfw.wl.pointerFocus;

    if (!window)
        return;

    window->wl.hovered = GLFW_FALSE;

    _glfw.wl.serial = serial;
    _glfw.wl.pointerFocus = NULL;
    _glfwInputCursorEnter(window, GLFW_FALSE);
    _glfw.wl.cursorPreviousName = NULL;
}

static void setCursor(_GLFWwindow* window, const char* name)
{
    struct wl_buffer* buffer;
    struct wl_cursor* cursor;
    struct wl_cursor_image* image;
    struct wl_surface* surface = _glfw.wl.cursorSurface;
    struct wl_cursor_theme* theme = _glfw.wl.cursorTheme;
    int scale = 1;

    if (window->wl.scale > 1 && _glfw.wl.cursorThemeHiDPI)
    {
        // We only support up to scale=2 for now, since libwayland-cursor
        // requires us to load a different theme for each size.
        scale = 2;
        theme = _glfw.wl.cursorThemeHiDPI;
    }

    cursor = wl_cursor_theme_get_cursor(theme, name);
    if (!cursor)
    {
        _glfwInputError(GLFW_CURSOR_UNAVAILABLE,
                        "Wayland: Standard cursor shape unavailable");
        return;
    }
    // TODO: handle animated cursors too.
    image = cursor->images[0];

    if (!image)
        return;

    buffer = wl_cursor_image_get_buffer(image);
    if (!buffer)
        return;
    wl_pointer_set_cursor(_glfw.wl.pointer, _glfw.wl.pointerEnterSerial,
                          surface,
                          image->hotspot_x / scale,
                          image->hotspot_y / scale);
    wl_surface_set_buffer_scale(surface, scale);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0,
                      image->width, image->height);
    wl_surface_commit(surface);
    _glfw.wl.cursorPreviousName = name;
}

static void pointerHandleMotion(void* userData,
                                struct wl_pointer* pointer,
                                uint32_t time,
                                wl_fixed_t sx,
                                wl_fixed_t sy)
{
    _GLFWwindow* window = _glfw.wl.pointerFocus;
    const char* cursorName = NULL;
    double x, y;

    if (!window)
        return;

    if (window->cursorMode == GLFW_CURSOR_DISABLED)
        return;
    x = wl_fixed_to_double(sx);
    y = wl_fixed_to_double(sy);
    window->wl.cursorPosX = x;
    window->wl.cursorPosY = y;

    switch (window->wl.decorations.focus)
    {
        case mainWindow:
            _glfwInputCursorPos(window, x, y);
            _glfw.wl.cursorPreviousName = NULL;
            return;
        case topDecoration:
            if (y < _GLFW_DECORATION_WIDTH)
                cursorName = "n-resize";
            else
                cursorName = "left_ptr";
            break;
        case leftDecoration:
            if (y < _GLFW_DECORATION_WIDTH)
                cursorName = "nw-resize";
            else
                cursorName = "w-resize";
            break;
        case rightDecoration:
            if (y < _GLFW_DECORATION_WIDTH)
                cursorName = "ne-resize";
            else
                cursorName = "e-resize";
            break;
        case bottomDecoration:
            if (x < _GLFW_DECORATION_WIDTH)
                cursorName = "sw-resize";
            else if (x > window->wl.width + _GLFW_DECORATION_WIDTH)
                cursorName = "se-resize";
            else
                cursorName = "s-resize";
            break;
        default:
            assert(0);
    }
    if (_glfw.wl.cursorPreviousName != cursorName)
        setCursor(window, cursorName);
}

static void pointerHandleButton(void* userData,
                                struct wl_pointer* pointer,
                                uint32_t serial,
                                uint32_t time,
                                uint32_t button,
                                uint32_t state)
{
    _GLFWwindow* window = _glfw.wl.pointerFocus;
    int glfwButton;
    uint32_t edges = XDG_TOPLEVEL_RESIZE_EDGE_NONE;

    if (!window)
        return;
    if (button == BTN_LEFT)
    {
        switch (window->wl.decorations.focus)
        {
            case mainWindow:
                break;
            case topDecoration:
                if (window->wl.cursorPosY < _GLFW_DECORATION_WIDTH)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
                else
                    xdg_toplevel_move(window->wl.xdg.toplevel, _glfw.wl.seat, serial);
                break;
            case leftDecoration:
                if (window->wl.cursorPosY < _GLFW_DECORATION_WIDTH)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
                break;
            case rightDecoration:
                if (window->wl.cursorPosY < _GLFW_DECORATION_WIDTH)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
                break;
            case bottomDecoration:
                if (window->wl.cursorPosX < _GLFW_DECORATION_WIDTH)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
                else if (window->wl.cursorPosX > window->wl.width + _GLFW_DECORATION_WIDTH)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
                break;
            default:
                assert(0);
        }
        if (edges != XDG_TOPLEVEL_RESIZE_EDGE_NONE)
        {
            xdg_toplevel_resize(window->wl.xdg.toplevel, _glfw.wl.seat,
                                serial, edges);
            return;
        }
    }
    else if (button == BTN_RIGHT)
    {
        if (window->wl.decorations.focus != mainWindow && window->wl.xdg.toplevel)
        {
            xdg_toplevel_show_window_menu(window->wl.xdg.toplevel,
                                          _glfw.wl.seat, serial,
                                          window->wl.cursorPosX,
                                          window->wl.cursorPosY);
            return;
        }
    }

    // Don’t pass the button to the user if it was related to a decoration.
    if (window->wl.decorations.focus != mainWindow)
        return;

    _glfw.wl.serial = serial;

    /* Makes left, right and middle 0, 1 and 2. Overall order follows evdev
     * codes. */
    glfwButton = button - BTN_LEFT;

    _glfwInputMouseClick(window,
                         glfwButton,
                         state == WL_POINTER_BUTTON_STATE_PRESSED
                                ? GLFW_PRESS
                                : GLFW_RELEASE,
                         _glfw.wl.xkb.modifiers);
}

static void pointerHandleAxis(void* userData,
                              struct wl_pointer* pointer,
                              uint32_t time,
                              uint32_t axis,
                              wl_fixed_t value)
{
    _GLFWwindow* window = _glfw.wl.pointerFocus;
    double x = 0.0, y = 0.0;
    // Wayland scroll events are in pointer motion coordinate space (think two
    // finger scroll).  The factor 10 is commonly used to convert to "scroll
    // step means 1.0.
    const double scrollFactor = 1.0 / 10.0;

    if (!window)
        return;

    assert(axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ||
           axis == WL_POINTER_AXIS_VERTICAL_SCROLL);

    if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
        x = -wl_fixed_to_double(value) * scrollFactor;
    else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
        y = -wl_fixed_to_double(value) * scrollFactor;

    _glfwInputScroll(window, x, y);
}

static const struct wl_pointer_listener pointerListener =
{
    pointerHandleEnter,
    pointerHandleLeave,
    pointerHandleMotion,
    pointerHandleButton,
    pointerHandleAxis,
};

static void keyboardHandleKeymap(void* userData,
                                 struct wl_keyboard* keyboard,
                                 uint32_t format,
                                 int fd,
                                 uint32_t size)
{
    struct xkb_keymap* keymap;
    struct xkb_state* state;
    struct xkb_compose_table* composeTable;
    struct xkb_compose_state* composeState;

    char* mapStr;
    const char* locale;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    {
        close(fd);
        return;
    }

    mapStr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapStr == MAP_FAILED) {
        close(fd);
        return;
    }

    keymap = xkb_keymap_new_from_string(_glfw.wl.xkb.context,
                                        mapStr,
                                        XKB_KEYMAP_FORMAT_TEXT_V1,
                                        0);
    munmap(mapStr, size);
    close(fd);

    if (!keymap)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to compile keymap");
        return;
    }

    state = xkb_state_new(keymap);
    if (!state)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create XKB state");
        xkb_keymap_unref(keymap);
        return;
    }

    // Look up the preferred locale, falling back to "C" as default.
    locale = getenv("LC_ALL");
    if (!locale)
        locale = getenv("LC_CTYPE");
    if (!locale)
        locale = getenv("LANG");
    if (!locale)
        locale = "C";

    composeTable =
        xkb_compose_table_new_from_locale(_glfw.wl.xkb.context, locale,
                                          XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (composeTable)
    {
        composeState =
            xkb_compose_state_new(composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
        xkb_compose_table_unref(composeTable);
        if (composeState)
            _glfw.wl.xkb.composeState = composeState;
        else
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "Wayland: Failed to create XKB compose state");
    }
    else
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create XKB compose table");
    }

    xkb_keymap_unref(_glfw.wl.xkb.keymap);
    xkb_state_unref(_glfw.wl.xkb.state);
    _glfw.wl.xkb.keymap = keymap;
    _glfw.wl.xkb.state = state;

    _glfw.wl.xkb.controlIndex  = xkb_keymap_mod_get_index(_glfw.wl.xkb.keymap, "Control");
    _glfw.wl.xkb.altIndex      = xkb_keymap_mod_get_index(_glfw.wl.xkb.keymap, "Mod1");
    _glfw.wl.xkb.shiftIndex    = xkb_keymap_mod_get_index(_glfw.wl.xkb.keymap, "Shift");
    _glfw.wl.xkb.superIndex    = xkb_keymap_mod_get_index(_glfw.wl.xkb.keymap, "Mod4");
    _glfw.wl.xkb.capsLockIndex = xkb_keymap_mod_get_index(_glfw.wl.xkb.keymap, "Lock");
    _glfw.wl.xkb.numLockIndex  = xkb_keymap_mod_get_index(_glfw.wl.xkb.keymap, "Mod2");
}

static void keyboardHandleEnter(void* userData,
                                struct wl_keyboard* keyboard,
                                uint32_t serial,
                                struct wl_surface* surface,
                                struct wl_array* keys)
{
    // Happens in the case we just destroyed the surface.
    if (!surface)
        return;

    _GLFWwindow* window = wl_surface_get_user_data(surface);
    if (!window)
    {
        window = findWindowFromDecorationSurface(surface, NULL);
        if (!window)
            return;
    }

    _glfw.wl.serial = serial;
    _glfw.wl.keyboardFocus = window;
    _glfwInputWindowFocus(window, GLFW_TRUE);
}

static void keyboardHandleLeave(void* userData,
                                struct wl_keyboard* keyboard,
                                uint32_t serial,
                                struct wl_surface* surface)
{
    _GLFWwindow* window = _glfw.wl.keyboardFocus;

    if (!window)
        return;

    struct itimerspec timer = {};
    timerfd_settime(_glfw.wl.timerfd, 0, &timer, NULL);

    _glfw.wl.serial = serial;
    _glfw.wl.keyboardFocus = NULL;
    _glfwInputWindowFocus(window, GLFW_FALSE);
}

static int translateKey(uint32_t scancode)
{
    if (scancode < sizeof(_glfw.wl.keycodes) / sizeof(_glfw.wl.keycodes[0]))
        return _glfw.wl.keycodes[scancode];

    return GLFW_KEY_UNKNOWN;
}

static xkb_keysym_t composeSymbol(xkb_keysym_t sym)
{
    if (sym == XKB_KEY_NoSymbol || !_glfw.wl.xkb.composeState)
        return sym;
    if (xkb_compose_state_feed(_glfw.wl.xkb.composeState, sym)
            != XKB_COMPOSE_FEED_ACCEPTED)
        return sym;
    switch (xkb_compose_state_get_status(_glfw.wl.xkb.composeState))
    {
        case XKB_COMPOSE_COMPOSED:
            return xkb_compose_state_get_one_sym(_glfw.wl.xkb.composeState);
        case XKB_COMPOSE_COMPOSING:
        case XKB_COMPOSE_CANCELLED:
            return XKB_KEY_NoSymbol;
        case XKB_COMPOSE_NOTHING:
        default:
            return sym;
    }
}

GLFWbool _glfwInputTextWayland(_GLFWwindow* window, uint32_t scancode)
{
    const xkb_keysym_t* keysyms;
    const xkb_keycode_t keycode = scancode + 8;

    if (xkb_state_key_get_syms(_glfw.wl.xkb.state, keycode, &keysyms) == 1)
    {
        const xkb_keysym_t keysym = composeSymbol(keysyms[0]);
        const uint32_t codepoint = _glfwKeySym2Unicode(keysym);
        if (codepoint != GLFW_INVALID_CODEPOINT)
        {
            const int mods = _glfw.wl.xkb.modifiers;
            const int plain = !(mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT));
            _glfwInputChar(window, codepoint, mods, plain);
        }
    }

    return xkb_keymap_key_repeats(_glfw.wl.xkb.keymap, keycode);
}

static void keyboardHandleKey(void* userData,
                              struct wl_keyboard* keyboard,
                              uint32_t serial,
                              uint32_t time,
                              uint32_t scancode,
                              uint32_t state)
{
    _GLFWwindow* window = _glfw.wl.keyboardFocus;
    if (!window)
        return;

    const int key = translateKey(scancode);
    const int action =
        state == WL_KEYBOARD_KEY_STATE_PRESSED ? GLFW_PRESS : GLFW_RELEASE;

    _glfw.wl.serial = serial;
    _glfwInputKey(window, key, scancode, action, _glfw.wl.xkb.modifiers);

    struct itimerspec timer = {};

    if (action == GLFW_PRESS)
    {
        const GLFWbool shouldRepeat = _glfwInputTextWayland(window, scancode);

        if (shouldRepeat && _glfw.wl.keyboardRepeatRate > 0)
        {
            _glfw.wl.keyboardLastKey = key;
            _glfw.wl.keyboardLastScancode = scancode;
            if (_glfw.wl.keyboardRepeatRate > 1)
                timer.it_interval.tv_nsec = 1000000000 / _glfw.wl.keyboardRepeatRate;
            else
                timer.it_interval.tv_sec = 1;

            timer.it_value.tv_sec = _glfw.wl.keyboardRepeatDelay / 1000;
            timer.it_value.tv_nsec = (_glfw.wl.keyboardRepeatDelay % 1000) * 1000000;
        }
    }

    timerfd_settime(_glfw.wl.timerfd, 0, &timer, NULL);
}

static void keyboardHandleModifiers(void* userData,
                                    struct wl_keyboard* keyboard,
                                    uint32_t serial,
                                    uint32_t modsDepressed,
                                    uint32_t modsLatched,
                                    uint32_t modsLocked,
                                    uint32_t group)
{
    _glfw.wl.serial = serial;

    if (!_glfw.wl.xkb.keymap)
        return;

    xkb_state_update_mask(_glfw.wl.xkb.state,
                          modsDepressed,
                          modsLatched,
                          modsLocked,
                          0,
                          0,
                          group);

    unsigned int mods = 0;

    if (xkb_state_mod_index_is_active(_glfw.wl.xkb.state,
                                      _glfw.wl.xkb.controlIndex,
                                      XKB_STATE_MODS_EFFECTIVE) == 1)
    {
        mods |= GLFW_MOD_CONTROL;
    }

    if (xkb_state_mod_index_is_active(_glfw.wl.xkb.state,
                                      _glfw.wl.xkb.altIndex,
                                      XKB_STATE_MODS_EFFECTIVE) == 1)
    {
        mods |= GLFW_MOD_ALT;
    }

    if (xkb_state_mod_index_is_active(_glfw.wl.xkb.state,
                                      _glfw.wl.xkb.shiftIndex,
                                      XKB_STATE_MODS_EFFECTIVE) == 1)
    {
        mods |= GLFW_MOD_SHIFT;
    }

    if (xkb_state_mod_index_is_active(_glfw.wl.xkb.state,
                                      _glfw.wl.xkb.superIndex,
                                      XKB_STATE_MODS_EFFECTIVE) == 1)
    {
        mods |= GLFW_MOD_SUPER;
    }

    if (xkb_state_mod_index_is_active(_glfw.wl.xkb.state,
                                      _glfw.wl.xkb.capsLockIndex,
                                      XKB_STATE_MODS_EFFECTIVE) == 1)
    {
        mods |= GLFW_MOD_CAPS_LOCK;
    }

    if (xkb_state_mod_index_is_active(_glfw.wl.xkb.state,
                                      _glfw.wl.xkb.numLockIndex,
                                      XKB_STATE_MODS_EFFECTIVE) == 1)
    {
        mods |= GLFW_MOD_NUM_LOCK;
    }

    _glfw.wl.xkb.modifiers = mods;
}

#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
static void keyboardHandleRepeatInfo(void* userData,
                                     struct wl_keyboard* keyboard,
                                     int32_t rate,
                                     int32_t delay)
{
    if (keyboard != _glfw.wl.keyboard)
        return;

    _glfw.wl.keyboardRepeatRate = rate;
    _glfw.wl.keyboardRepeatDelay = delay;
}
#endif

static const struct wl_keyboard_listener keyboardListener =
{
    keyboardHandleKeymap,
    keyboardHandleEnter,
    keyboardHandleLeave,
    keyboardHandleKey,
    keyboardHandleModifiers,
#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    keyboardHandleRepeatInfo,
#endif
};

static void seatHandleCapabilities(void* userData,
                                   struct wl_seat* seat,
                                   enum wl_seat_capability caps)
{
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !_glfw.wl.pointer)
    {
        _glfw.wl.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(_glfw.wl.pointer, &pointerListener, NULL);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && _glfw.wl.pointer)
    {
        wl_pointer_destroy(_glfw.wl.pointer);
        _glfw.wl.pointer = NULL;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !_glfw.wl.keyboard)
    {
        _glfw.wl.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(_glfw.wl.keyboard, &keyboardListener, NULL);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && _glfw.wl.keyboard)
    {
        wl_keyboard_destroy(_glfw.wl.keyboard);
        _glfw.wl.keyboard = NULL;
    }
}

static void seatHandleName(void* userData,
                           struct wl_seat* seat,
                           const char* name)
{
}

static const struct wl_seat_listener seatListener =
{
    seatHandleCapabilities,
    seatHandleName,
};

static void dataOfferHandleOffer(void* userData,
                                 struct wl_data_offer* offer,
                                 const char* mimeType)
{
    for (unsigned int i = 0; i < _glfw.wl.offerCount; i++)
    {
        if (_glfw.wl.offers[i].offer == offer)
        {
            if (strcmp(mimeType, "text/plain;charset=utf-8") == 0)
                _glfw.wl.offers[i].text_plain_utf8 = GLFW_TRUE;
            else if (strcmp(mimeType, "text/uri-list") == 0)
                _glfw.wl.offers[i].text_uri_list = GLFW_TRUE;

            break;
        }
    }
}

static const struct wl_data_offer_listener dataOfferListener =
{
    dataOfferHandleOffer
};

static void dataDeviceHandleDataOffer(void* userData,
                                      struct wl_data_device* device,
                                      struct wl_data_offer* offer)
{
    _GLFWofferWayland* offers =
        _glfw_realloc(_glfw.wl.offers, _glfw.wl.offerCount + 1);
    if (!offers)
    {
        _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
        return;
    }

    _glfw.wl.offers = offers;
    _glfw.wl.offerCount++;

    _glfw.wl.offers[_glfw.wl.offerCount - 1] = (_GLFWofferWayland) { offer };
    wl_data_offer_add_listener(offer, &dataOfferListener, NULL);
}

static void dataDeviceHandleEnter(void* userData,
                                  struct wl_data_device* device,
                                  uint32_t serial,
                                  struct wl_surface* surface,
                                  wl_fixed_t x,
                                  wl_fixed_t y,
                                  struct wl_data_offer* offer)
{
    if (_glfw.wl.dragOffer)
    {
        wl_data_offer_destroy(_glfw.wl.dragOffer);
        _glfw.wl.dragOffer = NULL;
        _glfw.wl.dragFocus = NULL;
    }

    for (unsigned int i = 0; i < _glfw.wl.offerCount; i++)
    {
        if (_glfw.wl.offers[i].offer == offer)
        {
            _GLFWwindow* window = NULL;

            if (surface)
                window = wl_surface_get_user_data(surface);

            if (window && _glfw.wl.offers[i].text_uri_list)
            {
                _glfw.wl.dragOffer = offer;
                _glfw.wl.dragFocus = window;
                _glfw.wl.dragSerial = serial;
            }

            _glfw.wl.offers[i] = _glfw.wl.offers[_glfw.wl.offerCount - 1];
            _glfw.wl.offerCount--;
            break;
        }
    }

    if (_glfw.wl.dragOffer)
        wl_data_offer_accept(offer, serial, "text/uri-list");
    else
    {
        wl_data_offer_accept(offer, serial, NULL);
        wl_data_offer_destroy(offer);
    }
}

static void dataDeviceHandleLeave(void* userData,
                                  struct wl_data_device* device)
{
    if (_glfw.wl.dragOffer)
    {
        wl_data_offer_destroy(_glfw.wl.dragOffer);
        _glfw.wl.dragOffer = NULL;
        _glfw.wl.dragFocus = NULL;
    }
}

static void dataDeviceHandleMotion(void* userData,
                                   struct wl_data_device* device,
                                   uint32_t time,
                                   wl_fixed_t x,
                                   wl_fixed_t y)
{
}

static void dataDeviceHandleDrop(void* userData,
                                 struct wl_data_device* device)
{
    if (!_glfw.wl.dragOffer)
        return;

    char* string = readDataOfferAsString(_glfw.wl.dragOffer, "text/uri-list");
    if (string)
    {
        int count;
        char** paths = _glfwParseUriList(string, &count);
        if (paths)
            _glfwInputDrop(_glfw.wl.dragFocus, count, (const char**) paths);

        for (int i = 0; i < count; i++)
            _glfw_free(paths[i]);

        _glfw_free(paths);
    }

    _glfw_free(string);
}

static void dataDeviceHandleSelection(void* userData,
                                      struct wl_data_device* device,
                                      struct wl_data_offer* offer)
{
    if (_glfw.wl.selectionOffer)
    {
        wl_data_offer_destroy(_glfw.wl.selectionOffer);
        _glfw.wl.selectionOffer = NULL;
    }

    for (unsigned int i = 0; i < _glfw.wl.offerCount; i++)
    {
        if (_glfw.wl.offers[i].offer == offer)
        {
            if (_glfw.wl.offers[i].text_plain_utf8)
                _glfw.wl.selectionOffer = offer;
            else
                wl_data_offer_destroy(offer);

            _glfw.wl.offers[i] = _glfw.wl.offers[_glfw.wl.offerCount - 1];
            _glfw.wl.offerCount--;
            break;
        }
    }
}

const struct wl_data_device_listener dataDeviceListener =
{
    dataDeviceHandleDataOffer,
    dataDeviceHandleEnter,
    dataDeviceHandleLeave,
    dataDeviceHandleMotion,
    dataDeviceHandleDrop,
    dataDeviceHandleSelection,
};

void _glfwAddSeatListenerWayland(struct wl_seat* seat)
{
    wl_seat_add_listener(seat, &seatListener, NULL);
}

void _glfwAddDataDeviceListenerWayland(struct wl_data_device* device)
{
    wl_data_device_add_listener(device, &dataDeviceListener, NULL);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwCreateWindowWayland(_GLFWwindow* window,
                                  const _GLFWwndconfig* wndconfig,
                                  const _GLFWctxconfig* ctxconfig,
                                  const _GLFWfbconfig* fbconfig)
{
    if (!createSurface(window, wndconfig, fbconfig))
        return GLFW_FALSE;

    if (ctxconfig->client != GLFW_NO_API)
    {
        if (ctxconfig->source == GLFW_EGL_CONTEXT_API ||
            ctxconfig->source == GLFW_NATIVE_CONTEXT_API)
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

    if (wndconfig->mousePassthrough)
        _glfwSetWindowMousePassthroughWayland(window, GLFW_TRUE);

    return GLFW_TRUE;
}

void _glfwDestroyWindowWayland(_GLFWwindow* window)
{
    if (window == _glfw.wl.pointerFocus)
    {
        _glfw.wl.pointerFocus = NULL;
        _glfwInputCursorEnter(window, GLFW_FALSE);
    }
    if (window == _glfw.wl.keyboardFocus)
    {
        _glfw.wl.keyboardFocus = NULL;
        _glfwInputWindowFocus(window, GLFW_FALSE);
    }

    if (window->wl.idleInhibitor)
        zwp_idle_inhibitor_v1_destroy(window->wl.idleInhibitor);

    if (window->context.destroy)
        window->context.destroy(window);

    destroyDecorations(window);
    if (window->wl.xdg.decoration)
        zxdg_toplevel_decoration_v1_destroy(window->wl.xdg.decoration);

    if (window->wl.decorations.buffer)
        wl_buffer_destroy(window->wl.decorations.buffer);

    if (window->wl.native)
        wl_egl_window_destroy(window->wl.native);

    if (window->wl.xdg.toplevel)
        xdg_toplevel_destroy(window->wl.xdg.toplevel);

    if (window->wl.xdg.surface)
        xdg_surface_destroy(window->wl.xdg.surface);

    if (window->wl.surface)
        wl_surface_destroy(window->wl.surface);

    _glfw_free(window->wl.title);
    _glfw_free(window->wl.monitors);
}

void _glfwSetWindowTitleWayland(_GLFWwindow* window, const char* title)
{
    if (window->wl.title)
        _glfw_free(window->wl.title);
    window->wl.title = _glfw_strdup(title);
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_title(window->wl.xdg.toplevel, title);
}

void _glfwSetWindowIconWayland(_GLFWwindow* window,
                               int count, const GLFWimage* images)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE,
                    "Wayland: The platform does not support setting the window icon");
}

void _glfwGetWindowPosWayland(_GLFWwindow* window, int* xpos, int* ypos)
{
    // A Wayland client is not aware of its position, so just warn and leave it
    // as (0, 0)

    _glfwInputError(GLFW_FEATURE_UNAVAILABLE,
                    "Wayland: The platform does not provide the window position");
}

void _glfwSetWindowPosWayland(_GLFWwindow* window, int xpos, int ypos)
{
    // A Wayland client can not set its position, so just warn

    _glfwInputError(GLFW_FEATURE_UNAVAILABLE,
                    "Wayland: The platform does not support setting the window position");
}

void _glfwGetWindowSizeWayland(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->wl.width;
    if (height)
        *height = window->wl.height;
}

void _glfwSetWindowSizeWayland(_GLFWwindow* window, int width, int height)
{
    window->wl.width = width;
    window->wl.height = height;
    resizeWindow(window);
}

void _glfwSetWindowSizeLimitsWayland(_GLFWwindow* window,
                                     int minwidth, int minheight,
                                     int maxwidth, int maxheight)
{
    if (window->wl.xdg.toplevel)
    {
        if (minwidth == GLFW_DONT_CARE || minheight == GLFW_DONT_CARE)
            minwidth = minheight = 0;
        if (maxwidth == GLFW_DONT_CARE || maxheight == GLFW_DONT_CARE)
            maxwidth = maxheight = 0;
        xdg_toplevel_set_min_size(window->wl.xdg.toplevel, minwidth, minheight);
        xdg_toplevel_set_max_size(window->wl.xdg.toplevel, maxwidth, maxheight);
        wl_surface_commit(window->wl.surface);
    }
}

void _glfwSetWindowAspectRatioWayland(_GLFWwindow* window, int numer, int denom)
{
    // TODO: find out how to trigger a resize.
    // The actual limits are checked in the xdg_toplevel::configure handler.
    _glfwInputError(GLFW_FEATURE_UNIMPLEMENTED,
                    "Wayland: Window aspect ratio not yet implemented");
}

void _glfwGetFramebufferSizeWayland(_GLFWwindow* window, int* width, int* height)
{
    _glfwGetWindowSizeWayland(window, width, height);
    if (width)
        *width *= window->wl.scale;
    if (height)
        *height *= window->wl.scale;
}

void _glfwGetWindowFrameSizeWayland(_GLFWwindow* window,
                                    int* left, int* top,
                                    int* right, int* bottom)
{
    if (window->decorated && !window->monitor && !window->wl.decorations.serverSide)
    {
        if (top)
            *top = _GLFW_DECORATION_TOP;
        if (left)
            *left = _GLFW_DECORATION_WIDTH;
        if (right)
            *right = _GLFW_DECORATION_WIDTH;
        if (bottom)
            *bottom = _GLFW_DECORATION_WIDTH;
    }
}

void _glfwGetWindowContentScaleWayland(_GLFWwindow* window,
                                       float* xscale, float* yscale)
{
    if (xscale)
        *xscale = (float) window->wl.scale;
    if (yscale)
        *yscale = (float) window->wl.scale;
}

void _glfwIconifyWindowWayland(_GLFWwindow* window)
{
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_minimized(window->wl.xdg.toplevel);
}

void _glfwRestoreWindowWayland(_GLFWwindow* window)
{
    if (window->wl.xdg.toplevel)
    {
        if (window->monitor)
            xdg_toplevel_unset_fullscreen(window->wl.xdg.toplevel);
        if (window->wl.maximized)
            xdg_toplevel_unset_maximized(window->wl.xdg.toplevel);
        // There is no way to unset minimized, or even to know if we are
        // minimized, so there is nothing to do in this case.
    }
    _glfwInputWindowMonitor(window, NULL);
    window->wl.maximized = GLFW_FALSE;
}

void _glfwMaximizeWindowWayland(_GLFWwindow* window)
{
    if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_set_maximized(window->wl.xdg.toplevel);
    }
    window->wl.maximized = GLFW_TRUE;
}

void _glfwShowWindowWayland(_GLFWwindow* window)
{
    if (!window->wl.visible)
    {
        // NOTE: The XDG surface and role are created here so command-line applications
        //       with off-screen windows do not appear in for example the Unity dock
        if (!window->wl.xdg.toplevel)
            createXdgSurface(window);

        window->wl.visible = GLFW_TRUE;
        _glfwInputWindowDamage(window);
    }
}

void _glfwHideWindowWayland(_GLFWwindow* window)
{
    if (window->wl.visible)
    {
        window->wl.visible = GLFW_FALSE;
        wl_surface_attach(window->wl.surface, NULL, 0, 0);
        wl_surface_commit(window->wl.surface);
    }
}

void _glfwRequestWindowAttentionWayland(_GLFWwindow* window)
{
    // TODO
    _glfwInputError(GLFW_FEATURE_UNIMPLEMENTED,
                    "Wayland: Window attention request not implemented yet");
}

void _glfwFocusWindowWayland(_GLFWwindow* window)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE,
                    "Wayland: The platform does not support setting the input focus");
}

void _glfwSetWindowMonitorWayland(_GLFWwindow* window,
                                  _GLFWmonitor* monitor,
                                  int xpos, int ypos,
                                  int width, int height,
                                  int refreshRate)
{
    if (monitor)
    {
        setFullscreen(window, monitor, refreshRate);
    }
    else
    {
        if (window->wl.xdg.toplevel)
            xdg_toplevel_unset_fullscreen(window->wl.xdg.toplevel);
        setIdleInhibitor(window, GLFW_FALSE);
        if (!_glfw.wl.decorationManager)
            createDecorations(window);
    }
    _glfwInputWindowMonitor(window, monitor);
}

GLFWbool _glfwWindowFocusedWayland(_GLFWwindow* window)
{
    return _glfw.wl.keyboardFocus == window;
}

GLFWbool _glfwWindowIconifiedWayland(_GLFWwindow* window)
{
    // xdg-shell doesn’t give any way to request whether a surface is
    // iconified.
    return GLFW_FALSE;
}

GLFWbool _glfwWindowVisibleWayland(_GLFWwindow* window)
{
    return window->wl.visible;
}

GLFWbool _glfwWindowMaximizedWayland(_GLFWwindow* window)
{
    return window->wl.maximized;
}

GLFWbool _glfwWindowHoveredWayland(_GLFWwindow* window)
{
    return window->wl.hovered;
}

GLFWbool _glfwFramebufferTransparentWayland(_GLFWwindow* window)
{
    return window->wl.transparent;
}

void _glfwSetWindowResizableWayland(_GLFWwindow* window, GLFWbool enabled)
{
    // TODO
    _glfwInputError(GLFW_FEATURE_UNIMPLEMENTED,
                    "Wayland: Window attribute setting not implemented yet");
}

void _glfwSetWindowDecoratedWayland(_GLFWwindow* window, GLFWbool enabled)
{
    if (!window->monitor)
    {
        if (enabled)
            createDecorations(window);
        else
            destroyDecorations(window);
    }
}

void _glfwSetWindowFloatingWayland(_GLFWwindow* window, GLFWbool enabled)
{
    // TODO
    _glfwInputError(GLFW_FEATURE_UNIMPLEMENTED,
                    "Wayland: Window attribute setting not implemented yet");
}

void _glfwSetWindowMousePassthroughWayland(_GLFWwindow* window, GLFWbool enabled)
{
    if (enabled)
    {
        struct wl_region* region = wl_compositor_create_region(_glfw.wl.compositor);
        wl_surface_set_input_region(window->wl.surface, region);
        wl_region_destroy(region);
    }
    else
        wl_surface_set_input_region(window->wl.surface, 0);
    wl_surface_commit(window->wl.surface);
}

float _glfwGetWindowOpacityWayland(_GLFWwindow* window)
{
    return 1.f;
}

void _glfwSetWindowOpacityWayland(_GLFWwindow* window, float opacity)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE,
                    "Wayland: The platform does not support setting the window opacity");
}

void _glfwSetRawMouseMotionWayland(_GLFWwindow* window, GLFWbool enabled)
{
    // This is handled in relativePointerHandleRelativeMotion
}

GLFWbool _glfwRawMouseMotionSupportedWayland(void)
{
    return GLFW_TRUE;
}

void _glfwPollEventsWayland(void)
{
    double timeout = 0.0;
    handleEvents(&timeout);
}

void _glfwWaitEventsWayland(void)
{
    handleEvents(NULL);
}

void _glfwWaitEventsTimeoutWayland(double timeout)
{
    handleEvents(&timeout);
}

void _glfwPostEmptyEventWayland(void)
{
    wl_display_sync(_glfw.wl.display);
    flushDisplay();
}

void _glfwGetCursorPosWayland(_GLFWwindow* window, double* xpos, double* ypos)
{
    if (xpos)
        *xpos = window->wl.cursorPosX;
    if (ypos)
        *ypos = window->wl.cursorPosY;
}

static GLFWbool isPointerLocked(_GLFWwindow* window);

void _glfwSetCursorPosWayland(_GLFWwindow* window, double x, double y)
{
    if (isPointerLocked(window))
    {
        zwp_locked_pointer_v1_set_cursor_position_hint(
            window->wl.pointerLock.lockedPointer,
            wl_fixed_from_double(x), wl_fixed_from_double(y));
        wl_surface_commit(window->wl.surface);
    }
}

void _glfwSetCursorModeWayland(_GLFWwindow* window, int mode)
{
    _glfwSetCursorWayland(window, window->wl.currentCursor);
}

const char* _glfwGetScancodeNameWayland(int scancode)
{
    if (scancode < 0 || scancode > 255 ||
        _glfw.wl.keycodes[scancode] == GLFW_KEY_UNKNOWN)
    {
        _glfwInputError(GLFW_INVALID_VALUE,
                        "Wayland: Invalid scancode %i",
                        scancode);
        return NULL;
    }

    const int key = _glfw.wl.keycodes[scancode];
    const xkb_keycode_t keycode = scancode + 8;
    const xkb_layout_index_t layout =
        xkb_state_key_get_layout(_glfw.wl.xkb.state, keycode);
    if (layout == XKB_LAYOUT_INVALID)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve layout for key name");
        return NULL;
    }

    const xkb_keysym_t* keysyms = NULL;
    xkb_keymap_key_get_syms_by_level(_glfw.wl.xkb.keymap,
                                     keycode,
                                     layout,
                                     0,
                                     &keysyms);
    if (keysyms == NULL)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve keysym for key name");
        return NULL;
    }

    const uint32_t codepoint = _glfwKeySym2Unicode(keysyms[0]);
    if (codepoint == GLFW_INVALID_CODEPOINT)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve codepoint for key name");
        return NULL;
    }

    const size_t count = _glfwEncodeUTF8(_glfw.wl.keynames[key],  codepoint);
    if (count == 0)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to encode codepoint for key name");
        return NULL;
    }

    _glfw.wl.keynames[key][count] = '\0';
    return _glfw.wl.keynames[key];
}

int _glfwGetKeyScancodeWayland(int key)
{
    return _glfw.wl.scancodes[key];
}

GLFWbool _glfwCreateCursorWayland(_GLFWcursor* cursor,
                                  const GLFWimage* image,
                                  int xhot, int yhot)
{
    cursor->wl.buffer = createShmBuffer(image);
    if (!cursor->wl.buffer)
        return GLFW_FALSE;

    cursor->wl.width = image->width;
    cursor->wl.height = image->height;
    cursor->wl.xhot = xhot;
    cursor->wl.yhot = yhot;
    return GLFW_TRUE;
}

GLFWbool _glfwCreateStandardCursorWayland(_GLFWcursor* cursor, int shape)
{
    const char* name = NULL;

    // Try the XDG names first
    switch (shape)
    {
        case GLFW_ARROW_CURSOR:
            name = "default";
            break;
        case GLFW_IBEAM_CURSOR:
            name = "text";
            break;
        case GLFW_CROSSHAIR_CURSOR:
            name = "crosshair";
            break;
        case GLFW_POINTING_HAND_CURSOR:
            name = "pointer";
            break;
        case GLFW_RESIZE_EW_CURSOR:
            name = "ew-resize";
            break;
        case GLFW_RESIZE_NS_CURSOR:
            name = "ns-resize";
            break;
        case GLFW_RESIZE_NWSE_CURSOR:
            name = "nwse-resize";
            break;
        case GLFW_RESIZE_NESW_CURSOR:
            name = "nesw-resize";
            break;
        case GLFW_RESIZE_ALL_CURSOR:
            name = "all-scroll";
            break;
        case GLFW_NOT_ALLOWED_CURSOR:
            name = "not-allowed";
            break;
    }

    cursor->wl.cursor = wl_cursor_theme_get_cursor(_glfw.wl.cursorTheme, name);

    if (_glfw.wl.cursorThemeHiDPI)
    {
        cursor->wl.cursorHiDPI =
            wl_cursor_theme_get_cursor(_glfw.wl.cursorThemeHiDPI, name);
    }

    if (!cursor->wl.cursor)
    {
        // Fall back to the core X11 names
        switch (shape)
        {
            case GLFW_ARROW_CURSOR:
                name = "left_ptr";
                break;
            case GLFW_IBEAM_CURSOR:
                name = "xterm";
                break;
            case GLFW_CROSSHAIR_CURSOR:
                name = "crosshair";
                break;
            case GLFW_POINTING_HAND_CURSOR:
                name = "hand2";
                break;
            case GLFW_RESIZE_EW_CURSOR:
                name = "sb_h_double_arrow";
                break;
            case GLFW_RESIZE_NS_CURSOR:
                name = "sb_v_double_arrow";
                break;
            case GLFW_RESIZE_ALL_CURSOR:
                name = "fleur";
                break;
            default:
                _glfwInputError(GLFW_CURSOR_UNAVAILABLE,
                                "Wayland: Standard cursor shape unavailable");
                return GLFW_FALSE;
        }

        cursor->wl.cursor = wl_cursor_theme_get_cursor(_glfw.wl.cursorTheme, name);
        if (!cursor->wl.cursor)
        {
            _glfwInputError(GLFW_CURSOR_UNAVAILABLE,
                            "Wayland: Failed to create standard cursor \"%s\"",
                            name);
            return GLFW_FALSE;
        }

        if (_glfw.wl.cursorThemeHiDPI)
        {
            if (!cursor->wl.cursorHiDPI)
            {
                cursor->wl.cursorHiDPI =
                    wl_cursor_theme_get_cursor(_glfw.wl.cursorThemeHiDPI, name);
            }
        }
    }

    return GLFW_TRUE;
}

void _glfwDestroyCursorWayland(_GLFWcursor* cursor)
{
    // If it's a standard cursor we don't need to do anything here
    if (cursor->wl.cursor)
        return;

    if (cursor->wl.buffer)
        wl_buffer_destroy(cursor->wl.buffer);
}

static void relativePointerHandleRelativeMotion(void* userData,
                                                struct zwp_relative_pointer_v1* pointer,
                                                uint32_t timeHi,
                                                uint32_t timeLo,
                                                wl_fixed_t dx,
                                                wl_fixed_t dy,
                                                wl_fixed_t dxUnaccel,
                                                wl_fixed_t dyUnaccel)
{
    _GLFWwindow* window = userData;
    double xpos = window->virtualCursorPosX;
    double ypos = window->virtualCursorPosY;

    if (window->cursorMode != GLFW_CURSOR_DISABLED)
        return;

    if (window->rawMouseMotion)
    {
        xpos += wl_fixed_to_double(dxUnaccel);
        ypos += wl_fixed_to_double(dyUnaccel);
    }
    else
    {
        xpos += wl_fixed_to_double(dx);
        ypos += wl_fixed_to_double(dy);
    }

    _glfwInputCursorPos(window, xpos, ypos);
}

static const struct zwp_relative_pointer_v1_listener relativePointerListener =
{
    relativePointerHandleRelativeMotion
};

static void lockedPointerHandleLocked(void* userData,
                                      struct zwp_locked_pointer_v1* lockedPointer)
{
}

static void unlockPointer(_GLFWwindow* window)
{
    struct zwp_relative_pointer_v1* relativePointer =
        window->wl.pointerLock.relativePointer;
    struct zwp_locked_pointer_v1* lockedPointer =
        window->wl.pointerLock.lockedPointer;

    zwp_relative_pointer_v1_destroy(relativePointer);
    zwp_locked_pointer_v1_destroy(lockedPointer);

    window->wl.pointerLock.relativePointer = NULL;
    window->wl.pointerLock.lockedPointer = NULL;
}

static void lockPointer(_GLFWwindow* window);

static void lockedPointerHandleUnlocked(void* userData,
                                        struct zwp_locked_pointer_v1* lockedPointer)
{
}

static const struct zwp_locked_pointer_v1_listener lockedPointerListener =
{
    lockedPointerHandleLocked,
    lockedPointerHandleUnlocked
};

static void lockPointer(_GLFWwindow* window)
{
    struct zwp_relative_pointer_v1* relativePointer;
    struct zwp_locked_pointer_v1* lockedPointer;

    if (!_glfw.wl.relativePointerManager)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: no relative pointer manager");
        return;
    }

    relativePointer =
        zwp_relative_pointer_manager_v1_get_relative_pointer(
            _glfw.wl.relativePointerManager,
            _glfw.wl.pointer);
    zwp_relative_pointer_v1_add_listener(relativePointer,
                                         &relativePointerListener,
                                         window);

    lockedPointer =
        zwp_pointer_constraints_v1_lock_pointer(
            _glfw.wl.pointerConstraints,
            window->wl.surface,
            _glfw.wl.pointer,
            NULL,
            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
    zwp_locked_pointer_v1_add_listener(lockedPointer,
                                       &lockedPointerListener,
                                       window);

    window->wl.pointerLock.relativePointer = relativePointer;
    window->wl.pointerLock.lockedPointer = lockedPointer;

    wl_pointer_set_cursor(_glfw.wl.pointer, _glfw.wl.pointerEnterSerial,
                          NULL, 0, 0);
}

static GLFWbool isPointerLocked(_GLFWwindow* window)
{
    return window->wl.pointerLock.lockedPointer != NULL;
}

void _glfwSetCursorWayland(_GLFWwindow* window, _GLFWcursor* cursor)
{
    struct wl_cursor* defaultCursor;
    struct wl_cursor* defaultCursorHiDPI = NULL;

    if (!_glfw.wl.pointer)
        return;

    window->wl.currentCursor = cursor;

    // If we're not in the correct window just save the cursor
    // the next time the pointer enters the window the cursor will change
    if (window != _glfw.wl.pointerFocus || window->wl.decorations.focus != mainWindow)
        return;

    // Unlock possible pointer lock if no longer disabled.
    if (window->cursorMode != GLFW_CURSOR_DISABLED && isPointerLocked(window))
        unlockPointer(window);

    if (window->cursorMode == GLFW_CURSOR_NORMAL)
    {
        if (cursor)
            setCursorImage(window, &cursor->wl);
        else
        {
            defaultCursor = wl_cursor_theme_get_cursor(_glfw.wl.cursorTheme,
                                                       "left_ptr");
            if (!defaultCursor)
            {
                _glfwInputError(GLFW_PLATFORM_ERROR,
                                "Wayland: Standard cursor not found");
                return;
            }
            if (_glfw.wl.cursorThemeHiDPI)
                defaultCursorHiDPI =
                    wl_cursor_theme_get_cursor(_glfw.wl.cursorThemeHiDPI,
                                               "left_ptr");
            _GLFWcursorWayland cursorWayland = {
                defaultCursor,
                defaultCursorHiDPI,
                NULL,
                0, 0,
                0, 0,
                0
            };
            setCursorImage(window, &cursorWayland);
        }
    }
    else if (window->cursorMode == GLFW_CURSOR_DISABLED)
    {
        if (!isPointerLocked(window))
            lockPointer(window);
    }
    else if (window->cursorMode == GLFW_CURSOR_HIDDEN)
    {
        wl_pointer_set_cursor(_glfw.wl.pointer, _glfw.wl.pointerEnterSerial, NULL, 0, 0);
    }
}

static void dataSourceHandleTarget(void* userData,
                                   struct wl_data_source* source,
                                   const char* mimeType)
{
    if (_glfw.wl.selectionSource != source)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Unknown clipboard data source");
        return;
    }
}

static void dataSourceHandleSend(void* userData,
                                 struct wl_data_source* source,
                                 const char* mimeType,
                                 int fd)
{
    // Ignore it if this is an outdated or invalid request
    if (_glfw.wl.selectionSource != source ||
        strcmp(mimeType, "text/plain;charset=utf-8") != 0)
    {
        close(fd);
        return;
    }

    char* string = _glfw.wl.clipboardString;
    size_t length = strlen(string);

    while (length > 0)
    {
        const ssize_t result = write(fd, string, length);
        if (result == -1)
        {
            if (errno == EINTR)
                continue;

            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "Wayland: Error while writing the clipboard: %s",
                            strerror(errno));
            break;
        }

        length -= result;
        string += result;
    }

    close(fd);
}

static void dataSourceHandleCancelled(void* userData,
                                      struct wl_data_source* source)
{
    wl_data_source_destroy(source);

    if (_glfw.wl.selectionSource != source)
        return;

    _glfw.wl.selectionSource = NULL;
}

static const struct wl_data_source_listener dataSourceListener =
{
    dataSourceHandleTarget,
    dataSourceHandleSend,
    dataSourceHandleCancelled,
};

void _glfwSetClipboardStringWayland(const char* string)
{
    if (_glfw.wl.selectionSource)
    {
        wl_data_source_destroy(_glfw.wl.selectionSource);
        _glfw.wl.selectionSource = NULL;
    }

    char* copy = _glfw_strdup(string);
    if (!copy)
    {
        _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
        return;
    }

    _glfw_free(_glfw.wl.clipboardString);
    _glfw.wl.clipboardString = copy;

    _glfw.wl.selectionSource =
        wl_data_device_manager_create_data_source(_glfw.wl.dataDeviceManager);
    if (!_glfw.wl.selectionSource)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create clipboard data source");
        return;
    }
    wl_data_source_add_listener(_glfw.wl.selectionSource,
                                &dataSourceListener,
                                NULL);
    wl_data_source_offer(_glfw.wl.selectionSource, "text/plain;charset=utf-8");
    wl_data_device_set_selection(_glfw.wl.dataDevice,
                                 _glfw.wl.selectionSource,
                                 _glfw.wl.serial);
}

const char* _glfwGetClipboardStringWayland(void)
{
    if (!_glfw.wl.selectionOffer)
    {
        _glfwInputError(GLFW_FORMAT_UNAVAILABLE,
                        "Wayland: No clipboard data available");
        return NULL;
    }

    if (_glfw.wl.selectionSource)
        return _glfw.wl.clipboardString;

    _glfw_free(_glfw.wl.clipboardString);
    _glfw.wl.clipboardString =
        readDataOfferAsString(_glfw.wl.selectionOffer, "text/plain;charset=utf-8");
    return _glfw.wl.clipboardString;
}

EGLenum _glfwGetEGLPlatformWayland(EGLint** attribs)
{
    if (_glfw.egl.EXT_platform_base && _glfw.egl.EXT_platform_wayland)
        return EGL_PLATFORM_WAYLAND_EXT;
    else
        return 0;
}

EGLNativeDisplayType _glfwGetEGLNativeDisplayWayland(void)
{
    return _glfw.wl.display;
}

EGLNativeWindowType _glfwGetEGLNativeWindowWayland(_GLFWwindow* window)
{
    return window->wl.native;
}

void _glfwGetRequiredInstanceExtensionsWayland(char** extensions)
{
    if (!_glfw.vk.KHR_surface || !_glfw.vk.KHR_wayland_surface)
        return;

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_KHR_wayland_surface";
}

GLFWbool _glfwGetPhysicalDevicePresentationSupportWayland(VkInstance instance,
                                                          VkPhysicalDevice device,
                                                          uint32_t queuefamily)
{
    PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR
        vkGetPhysicalDeviceWaylandPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
    if (!vkGetPhysicalDeviceWaylandPresentationSupportKHR)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "Wayland: Vulkan instance missing VK_KHR_wayland_surface extension");
        return VK_NULL_HANDLE;
    }

    return vkGetPhysicalDeviceWaylandPresentationSupportKHR(device,
                                                            queuefamily,
                                                            _glfw.wl.display);
}

VkResult _glfwCreateWindowSurfaceWayland(VkInstance instance,
                                         _GLFWwindow* window,
                                         const VkAllocationCallbacks* allocator,
                                         VkSurfaceKHR* surface)
{
    VkResult err;
    VkWaylandSurfaceCreateInfoKHR sci;
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;

    vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)
        vkGetInstanceProcAddr(instance, "vkCreateWaylandSurfaceKHR");
    if (!vkCreateWaylandSurfaceKHR)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "Wayland: Vulkan instance missing VK_KHR_wayland_surface extension");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    sci.display = _glfw.wl.display;
    sci.surface = window->wl.surface;

    err = vkCreateWaylandSurfaceKHR(instance, &sci, allocator, surface);
    if (err)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create Vulkan surface: %s",
                        _glfwGetVulkanResultString(err));
    }

    return err;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI struct wl_display* glfwGetWaylandDisplay(void)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    if (_glfw.platform.platformID != GLFW_PLATFORM_WAYLAND)
    {
        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE,
                        "Wayland: Platform not initialized");
        return NULL;
    }

    return _glfw.wl.display;
}

GLFWAPI struct wl_surface* glfwGetWaylandWindow(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    if (_glfw.platform.platformID != GLFW_PLATFORM_WAYLAND)
    {
        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE,
                        "Wayland: Platform not initialized");
        return NULL;
    }

    return window->wl.surface;
}

