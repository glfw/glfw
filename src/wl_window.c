//========================================================================
// GLFW 3.5 Wayland - www.glfw.org
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

#define _GNU_SOURCE

#include "internal.h"

#if defined(_GLFW_WAYLAND)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <linux/input-event-codes.h>

#include "wayland-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"
#include "viewporter-client-protocol.h"
#include "relative-pointer-unstable-v1-client-protocol.h"
#include "pointer-constraints-unstable-v1-client-protocol.h"
#include "xdg-activation-v1-client-protocol.h"
#include "idle-inhibit-unstable-v1-client-protocol.h"
#include "xdg-toplevel-drag-v1-client-protocol.h"
#include "fractional-scale-v1-client-protocol.h"

// Marker mime for self-initiated toplevel drags; no payload transferred.
#define GLFW_WAYLAND_WINDOW_DRAG_MIME "application/x.glfw-window-drag"
#include "cursor-shape-v1-client-protocol.h"

#define GLFW_BORDER_SIZE    4
#define GLFW_CAPTION_HEIGHT 24

#define GLFW_PENDING_SURFACE    1
#define GLFW_PENDING_BUTTON     2
#define GLFW_PENDING_MOTION     4
#define GLFW_PENDING_SCROLL     8
#define GLFW_PENDING_DISCRETE   16

typedef struct {
    int                 glfwShape;  // GLFW_*_CURSOR
    uint32_t            resizeEdge; // XDG_TOPLEVEL_RESIZE_EDGE_* (NONE for non-resizing-related shapes)
    uint32_t            shape;      // WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_*
    const char*         name;       // XDG cursor name
    const char*         fallback;   // X11 core cursor name
} _GLFWcursorDesc;

// This table relates various mouse-cursor-shape-related constants to each other so if you already
// have one you can find an equivalent value for a different library/context.
static const _GLFWcursorDesc cursorTable[] =
{
    { GLFW_ARROW_CURSOR,         XDG_TOPLEVEL_RESIZE_EDGE_NONE,        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT,     "default",       "left_ptr"           },
    { GLFW_IBEAM_CURSOR,         XDG_TOPLEVEL_RESIZE_EDGE_NONE,        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_TEXT,        "text",          "xterm"              },
    { GLFW_CROSSHAIR_CURSOR,     XDG_TOPLEVEL_RESIZE_EDGE_NONE,        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CROSSHAIR,   "crosshair",     "crosshair"          },
    { GLFW_POINTING_HAND_CURSOR, XDG_TOPLEVEL_RESIZE_EDGE_NONE,        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_POINTER,     "pointer",       "hand2"              },
    { GLFW_RESIZE_ALL_CURSOR,    XDG_TOPLEVEL_RESIZE_EDGE_NONE,        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALL_SCROLL,  "all-scroll",    "fleur"              },
    { GLFW_NOT_ALLOWED_CURSOR,   XDG_TOPLEVEL_RESIZE_EDGE_NONE,        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NOT_ALLOWED, "not-allowed",   "not-allowed"        },

    // Bidirectional resize cursors (canonical GLFW entries, no specific edge)
    { GLFW_RESIZE_EW_CURSOR,     XDG_TOPLEVEL_RESIZE_EDGE_NONE,        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_EW_RESIZE,   "ew-resize",     "sb_h_double_arrow"  },
    { GLFW_RESIZE_NS_CURSOR,     XDG_TOPLEVEL_RESIZE_EDGE_NONE,        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NS_RESIZE,   "ns-resize",     "sb_v_double_arrow"  },
    { GLFW_RESIZE_NWSE_CURSOR,   XDG_TOPLEVEL_RESIZE_EDGE_NONE,        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NWSE_RESIZE, "nwse-resize",   "nwse-resize"        },
    { GLFW_RESIZE_NESW_CURSOR,   XDG_TOPLEVEL_RESIZE_EDGE_NONE,        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NESW_RESIZE, "nesw-resize",   "nesw-resize"        },

    // Directional resize cursors (per-edge)
    { GLFW_RESIZE_NS_CURSOR,     XDG_TOPLEVEL_RESIZE_EDGE_TOP,         WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_N_RESIZE,    "n-resize",      "top_side"           },
    { GLFW_RESIZE_NS_CURSOR,     XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM,      WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_S_RESIZE,    "s-resize",      "bottom_side"        },
    { GLFW_RESIZE_EW_CURSOR,     XDG_TOPLEVEL_RESIZE_EDGE_LEFT,        WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_W_RESIZE,    "w-resize",      "left_side"          },
    { GLFW_RESIZE_EW_CURSOR,     XDG_TOPLEVEL_RESIZE_EDGE_RIGHT,       WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_E_RESIZE,    "e-resize",      "right_side"         },
    { GLFW_RESIZE_NWSE_CURSOR,   XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT,    WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NW_RESIZE,   "nw-resize",     "top_left_corner"    },
    { GLFW_RESIZE_NWSE_CURSOR,   XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT,WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SE_RESIZE,   "se-resize",     "bottom_right_corner"},
    { GLFW_RESIZE_NESW_CURSOR,   XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT,   WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NE_RESIZE,   "ne-resize",     "top_right_corner"   },
    { GLFW_RESIZE_NESW_CURSOR,   XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT, WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SW_RESIZE,   "sw-resize",     "bottom_left_corner" },
};

// Finds the canonical entry for a GLFW standard cursor shape (edge == NONE).
static const _GLFWcursorDesc* findCursorDescByShape(int glfwShape)
{
    for (size_t i = 0; i < sizeof(cursorTable) / sizeof(cursorTable[0]); i++)
    {
        if (cursorTable[i].glfwShape == glfwShape &&
            cursorTable[i].resizeEdge == XDG_TOPLEVEL_RESIZE_EDGE_NONE)
            return &cursorTable[i];
    }
    return NULL;
}

// Finds the directional entry for a specific resize edge.
static const _GLFWcursorDesc* findCursorDescByEdge(uint32_t edge)
{
    for (size_t i = 0; i < sizeof(cursorTable) / sizeof(cursorTable[0]); i++)
    {
        if (cursorTable[i].resizeEdge == edge)
            return &cursorTable[i];
    }
    return NULL;
}

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
    const int stride = image->width * 4;
    const int length = image->width * image->height * 4;

    const int fd = createAnonymousFile(length);
    if (fd < 0)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create buffer file of size %d: %s",
                        length, strerror(errno));
        return NULL;
    }

    void* data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to map file: %s", strerror(errno));
        close(fd);
        return NULL;
    }

    struct wl_shm_pool* pool = wl_shm_create_pool(_glfw.wl.shm, fd, length);

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

    struct wl_buffer* buffer =
        wl_shm_pool_create_buffer(pool, 0,
                                  image->width,
                                  image->height,
                                  stride, WL_SHM_FORMAT_ARGB8888);
    munmap(data, length);
    wl_shm_pool_destroy(pool);

    return buffer;
}

static void createFallbackEdge(_GLFWwindow* window,
                               _GLFWfallbackEdgeWayland* edge,
                               struct wl_surface* parent,
                               struct wl_buffer* buffer,
                               int x, int y,
                               int width, int height)
{
    edge->surface = wl_compositor_create_surface(_glfw.wl.compositor);
    wl_surface_set_user_data(edge->surface, window);
    wl_proxy_set_tag((struct wl_proxy*) edge->surface, &_glfw.wl.tag);
    edge->subsurface = wl_subcompositor_get_subsurface(_glfw.wl.subcompositor,
                                                       edge->surface, parent);
    wl_subsurface_set_position(edge->subsurface, x, y);
    edge->viewport = wp_viewporter_get_viewport(_glfw.wl.viewporter,
                                                edge->surface);
    wp_viewport_set_destination(edge->viewport, width, height);
    wl_surface_attach(edge->surface, buffer, 0, 0);

    struct wl_region* region = wl_compositor_create_region(_glfw.wl.compositor);
    wl_region_add(region, 0, 0, width, height);
    wl_surface_set_opaque_region(edge->surface, region);
    wl_surface_commit(edge->surface);
    wl_region_destroy(region);
}

static void createFallbackDecorations(_GLFWwindow* window)
{
    unsigned char data[] = { 224, 224, 224, 255 };
    const GLFWimage image = { 1, 1, data };

    if (!_glfw.wl.viewporter)
        return;

    if (!window->wl.fallback.buffer)
        window->wl.fallback.buffer = createShmBuffer(&image);
    if (!window->wl.fallback.buffer)
        return;

    createFallbackEdge(window, &window->wl.fallback.top, window->wl.surface,
                       window->wl.fallback.buffer,
                       0, -GLFW_CAPTION_HEIGHT,
                       window->wl.width, GLFW_CAPTION_HEIGHT);
    createFallbackEdge(window, &window->wl.fallback.left, window->wl.surface,
                       window->wl.fallback.buffer,
                       -GLFW_BORDER_SIZE, -GLFW_CAPTION_HEIGHT,
                       GLFW_BORDER_SIZE, window->wl.height + GLFW_CAPTION_HEIGHT);
    createFallbackEdge(window, &window->wl.fallback.right, window->wl.surface,
                       window->wl.fallback.buffer,
                       window->wl.width, -GLFW_CAPTION_HEIGHT,
                       GLFW_BORDER_SIZE, window->wl.height + GLFW_CAPTION_HEIGHT);
    createFallbackEdge(window, &window->wl.fallback.bottom, window->wl.surface,
                       window->wl.fallback.buffer,
                       -GLFW_BORDER_SIZE, window->wl.height,
                       window->wl.width + GLFW_BORDER_SIZE * 2, GLFW_BORDER_SIZE);

    window->wl.fallback.decorations = GLFW_TRUE;
}

static void destroyFallbackEdge(_GLFWfallbackEdgeWayland* edge)
{
    if (edge->surface == _glfw.wl.pointerSurface)
        _glfw.wl.pointerSurface = NULL;

    if (edge->subsurface)
        wl_subsurface_destroy(edge->subsurface);
    if (edge->surface)
        wl_surface_destroy(edge->surface);
    if (edge->viewport)
        wp_viewport_destroy(edge->viewport);

    edge->surface = NULL;
    edge->subsurface = NULL;
    edge->viewport = NULL;
}

static void destroyFallbackDecorations(_GLFWwindow* window)
{
    window->wl.fallback.decorations = GLFW_FALSE;

    destroyFallbackEdge(&window->wl.fallback.top);
    destroyFallbackEdge(&window->wl.fallback.left);
    destroyFallbackEdge(&window->wl.fallback.right);
    destroyFallbackEdge(&window->wl.fallback.bottom);
}

static void setCursorImage(_GLFWwindow* window, _GLFWcursorWayland* cursorWayland)
{
    struct itimerspec timer = {0};
    struct wl_cursor* wlCursor = cursorWayland->cursor;
    struct wl_cursor_image* image;
    struct wl_buffer* buffer;
    struct wl_surface* surface = _glfw.wl.cursorSurface;
    int scale = 1;

    if (!wlCursor)
        buffer = cursorWayland->buffer;
    else
    {
        if (window->wl.bufferScale > 1 && cursorWayland->cursorHiDPI)
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

static _GLFWcursorWayland lookupNamedCursor(const char* name, const char* fallback)
{
    _GLFWcursorWayland cw = { NULL, NULL, NULL, 0, 0, 0, 0, 0 };

    cw.cursor = wl_cursor_theme_get_cursor(_glfw.wl.cursorTheme, name);
    if (!cw.cursor)
        cw.cursor = wl_cursor_theme_get_cursor(_glfw.wl.cursorTheme, fallback);

    if (_glfw.wl.cursorThemeHiDPI)
    {
        cw.cursorHiDPI = wl_cursor_theme_get_cursor(_glfw.wl.cursorThemeHiDPI, name);
        if (!cw.cursorHiDPI)
            cw.cursorHiDPI = wl_cursor_theme_get_cursor(_glfw.wl.cursorThemeHiDPI, fallback);
    }

    return cw;
}

// Translates fallback decoration subsurface-local coordinates to
// main window coordinates so detectResizeEdge can handle both paths.
static GLFWbool fallbackToWindowCoords(_GLFWwindow* window,
                                       double sx, double sy,
                                       int* wx, int* wy)
{
    if (_glfw.wl.pointerSurface == window->wl.fallback.top.surface)
    {
        *wx = (int) sx;
        *wy = (int) sy - GLFW_CAPTION_HEIGHT;
    }
    else if (_glfw.wl.pointerSurface == window->wl.fallback.left.surface)
    {
        *wx = (int) sx - GLFW_BORDER_SIZE;
        *wy = (int) sy - GLFW_CAPTION_HEIGHT;
    }
    else if (_glfw.wl.pointerSurface == window->wl.fallback.right.surface)
    {
        *wx = (int) sx + window->wl.width;
        *wy = (int) sy - GLFW_CAPTION_HEIGHT;
    }
    else if (_glfw.wl.pointerSurface == window->wl.fallback.bottom.surface)
    {
        *wx = (int) sx - GLFW_BORDER_SIZE;
        *wy = (int) sy + window->wl.height;
    }
    else
        return GLFW_FALSE;

    return GLFW_TRUE;
}

static uint32_t detectResizeEdge(_GLFWwindow* window, int x, int y)
{
    const int w = window->wl.width;
    const int h = window->wl.height;
    // Note that this is not the same as GLFW_BORDER_SIZE, because undecorated windows need a bigger
    // hit zone than decorated ones, where a resize can be started from slightly outside the window.
    const int border = 8;

    const GLFWbool onLeft   = (x < border);
    const GLFWbool onRight  = (x >= w - border);
    const GLFWbool onTop    = (y < border);
    const GLFWbool onBottom = (y >= h - border);

    if      (onTop    && onLeft)  return XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
    else if (onTop    && onRight) return XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
    else if (onBottom && onLeft)  return XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
    else if (onBottom && onRight) return XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
    else if (onTop)               return XDG_TOPLEVEL_RESIZE_EDGE_TOP;
    else if (onBottom)            return XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
    else if (onLeft)              return XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
    else if (onRight)             return XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;

    return XDG_TOPLEVEL_RESIZE_EDGE_NONE;
}

// Sets the cursor from a descriptor, preferring wp_cursor_shape_v1 when
// available and falling back to wl_cursor_theme lookup.
static void setDescCursor(_GLFWwindow* window, const _GLFWcursorDesc* desc)
{
    if (_glfw.wl.cursorShapeDevice)
    {
        wp_cursor_shape_device_v1_set_shape(
            _glfw.wl.cursorShapeDevice,
            _glfw.wl.pointerEnterSerial,
            desc->shape);
    }
    else
    {
        _GLFWcursorWayland cw = lookupNamedCursor(desc->name, desc->fallback);
        if (cw.cursor)
            setCursorImage(window, &cw);
    }
}

/* Anything that wants to change the cursor shape should call this. */
static void applyCursor(_GLFWwindow* window)
{
    if (_glfw.wl.pointer && _glfw.wl.pointerSurface)
    {
        _GLFWwindow* pointerWindow =
            wl_surface_get_user_data(_glfw.wl.pointerSurface);

        if (pointerWindow == window)
        {
            // Determine the resize edge (if any) for both main and
            // fallback decoration surfaces
            uint32_t edge = XDG_TOPLEVEL_RESIZE_EDGE_NONE;

            if (window->wl.surface == _glfw.wl.pointerSurface)
            {
                edge = window->wl.resizeEdge;
            }
            else if (window->wl.fallback.decorations && window->resizable)
            {
                int wx, wy;
                if (fallbackToWindowCoords(window,
                                           window->wl.fallback.pointerX,
                                           window->wl.fallback.pointerY,
                                           &wx, &wy))
                {
                    edge = detectResizeEdge(window, wx, wy);
                }
            }

            // Hidden/disabled cursor
            if (window->wl.surface == _glfw.wl.pointerSurface &&
                (window->cursorMode == GLFW_CURSOR_HIDDEN ||
                 window->cursorMode == GLFW_CURSOR_DISABLED))
            {
                wl_pointer_set_cursor(_glfw.wl.pointer,
                                      _glfw.wl.pointerEnterSerial,
                                      NULL, 0, 0);
            }
            // Resize edge cursor
            else if (edge != XDG_TOPLEVEL_RESIZE_EDGE_NONE)
            {
                const _GLFWcursorDesc* desc = findCursorDescByEdge(edge);
                if (desc)
                    setDescCursor(window, desc);
            }
            // Application cursor (main surface only)
            else if (window->wl.surface == _glfw.wl.pointerSurface &&
                     window->cursor)
            {
                if (_glfw.wl.cursorShapeDevice &&
                    window->cursor->wl.cursorShape)
                {
                    wp_cursor_shape_device_v1_set_shape(
                        _glfw.wl.cursorShapeDevice,
                        _glfw.wl.pointerEnterSerial,
                        window->cursor->wl.cursorShape);
                }
                else
                    setCursorImage(window, &window->cursor->wl);
            }
            // Default cursor
            else
            {
                const _GLFWcursorDesc* desc = findCursorDescByShape(GLFW_ARROW_CURSOR);
                if (desc)
                    setDescCursor(window, desc);
                else
                    _glfwInputError(GLFW_PLATFORM_ERROR,
                                    "Wayland: Standard cursor not found");
            }
        }
    }
}


static void handleFallbackDecorationButton(_GLFWwindow* window, int button, int action)
{
    if (action != GLFW_PRESS)
        return;

    const double xpos = window->wl.fallback.pointerX;
    const double ypos = window->wl.fallback.pointerY;
    const uint32_t serial = window->wl.fallback.buttonPressSerial;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        int wx, wy;
        uint32_t edges = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
        if (window->resizable &&
            fallbackToWindowCoords(window, xpos, ypos, &wx, &wy))
        {
            edges = detectResizeEdge(window, wx, wy);
        }

        if (edges != XDG_TOPLEVEL_RESIZE_EDGE_NONE)
            xdg_toplevel_resize(window->wl.xdg.toplevel, _glfw.wl.seat, serial, edges);
        else if (_glfw.wl.pointerSurface == window->wl.fallback.top.surface)
            xdg_toplevel_move(window->wl.xdg.toplevel, _glfw.wl.seat, serial);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (!window->wl.xdg.toplevel)
            return;

        if (_glfw.wl.pointerSurface != window->wl.fallback.top.surface)
            return;

        if (ypos < GLFW_BORDER_SIZE)
            return;

        xdg_toplevel_show_window_menu(window->wl.xdg.toplevel, _glfw.wl.seat, serial,
                                      xpos, ypos - GLFW_CAPTION_HEIGHT - GLFW_BORDER_SIZE);
    }
}

static void xdgDecorationHandleConfigure(void* userData,
                                         struct zxdg_toplevel_decoration_v1* decoration,
                                         uint32_t mode)
{
    _GLFWwindow* window = userData;

    window->wl.xdg.decorationMode = mode;

    if (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE)
    {
        if (window->decorated && !window->monitor)
            createFallbackDecorations(window);
    }
    else
        destroyFallbackDecorations(window);
}

static const struct zxdg_toplevel_decoration_v1_listener xdgDecorationListener =
{
    xdgDecorationHandleConfigure,
};

// Makes the surface considered as XRGB instead of ARGB.
static void setContentAreaOpaque(_GLFWwindow* window)
{
    struct wl_region* region;

    region = wl_compositor_create_region(_glfw.wl.compositor);
    if (!region)
        return;

    wl_region_add(region, 0, 0, window->wl.width, window->wl.height);
    wl_surface_set_opaque_region(window->wl.surface, region);
    wl_region_destroy(region);
}

static void resizeFramebuffer(_GLFWwindow* window)
{
    if (window->wl.fractionalScale)
    {
        window->wl.fbWidth = (window->wl.width * window->wl.scalingNumerator) / 120;
        window->wl.fbHeight = (window->wl.height * window->wl.scalingNumerator) / 120;
    }
    else
    {
        window->wl.fbWidth = window->wl.width * window->wl.bufferScale;
        window->wl.fbHeight = window->wl.height * window->wl.bufferScale;
    }

    if (window->wl.egl.window)
    {
        wl_egl_window_resize(window->wl.egl.window,
                             window->wl.fbWidth,
                             window->wl.fbHeight,
                             0, 0);
    }

    if (!window->wl.transparent)
        setContentAreaOpaque(window);

    _glfwInputFramebufferSize(window, window->wl.fbWidth, window->wl.fbHeight);
}

static GLFWbool resizeWindow(_GLFWwindow* window, int width, int height)
{
    width = _glfw_max(width, 1);
    height = _glfw_max(height, 1);

    if (width == window->wl.width && height == window->wl.height)
        return GLFW_FALSE;

    window->wl.width = width;
    window->wl.height = height;

    resizeFramebuffer(window);

    if (window->wl.scalingViewport)
    {
        wp_viewport_set_destination(window->wl.scalingViewport,
                                    window->wl.width,
                                    window->wl.height);
    }

    if (window->wl.fallback.decorations)
    {
        wp_viewport_set_destination(window->wl.fallback.top.viewport,
                                    window->wl.width,
                                    GLFW_CAPTION_HEIGHT);
        wl_surface_commit(window->wl.fallback.top.surface);

        wp_viewport_set_destination(window->wl.fallback.left.viewport,
                                    GLFW_BORDER_SIZE,
                                    window->wl.height + GLFW_CAPTION_HEIGHT);
        wl_surface_commit(window->wl.fallback.left.surface);

        wl_subsurface_set_position(window->wl.fallback.right.subsurface,
                                window->wl.width, -GLFW_CAPTION_HEIGHT);
        wp_viewport_set_destination(window->wl.fallback.right.viewport,
                                    GLFW_BORDER_SIZE,
                                    window->wl.height + GLFW_CAPTION_HEIGHT);
        wl_surface_commit(window->wl.fallback.right.surface);

        wl_subsurface_set_position(window->wl.fallback.bottom.subsurface,
                                -GLFW_BORDER_SIZE, window->wl.height);
        wp_viewport_set_destination(window->wl.fallback.bottom.viewport,
                                    window->wl.width + GLFW_BORDER_SIZE * 2,
                                    GLFW_BORDER_SIZE);
        wl_surface_commit(window->wl.fallback.bottom.surface);
    }

    return GLFW_TRUE;
}

void _glfwUpdateBufferScaleFromOutputsWayland(_GLFWwindow* window)
{
    if (wl_compositor_get_version(_glfw.wl.compositor) <
        WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION)
    {
        return;
    }

    if (!window->wl.scaleFramebuffer)
        return;

    // When using fractional scaling, the buffer scale should remain at 1
    if (window->wl.fractionalScale)
        return;

    // Get the scale factor from the highest scale monitor.
    int32_t maxScale = 1;

    for (size_t i = 0; i < window->wl.outputScaleCount; i++)
        maxScale = _glfw_max(window->wl.outputScales[i].factor, maxScale);

    // Only change the framebuffer size if the scale changed.
    if (window->wl.bufferScale != maxScale)
    {
        window->wl.bufferScale = maxScale;
        wl_surface_set_buffer_scale(window->wl.surface, maxScale);
        resizeFramebuffer(window);

        float xscale, yscale;
        _glfwGetWindowContentScaleWayland(window, &xscale, &yscale);
        _glfwInputWindowContentScale(window, xscale, yscale);

        if (window->wl.visible)
            _glfwInputWindowDamage(window);
    }
}

static void surfaceHandleEnter(void* userData,
                               struct wl_surface* surface,
                               struct wl_output* output)
{
    if (wl_proxy_get_tag((struct wl_proxy*) output) != &_glfw.wl.tag)
        return;

    _GLFWwindow* window = userData;
    _GLFWmonitor* monitor = wl_output_get_user_data(output);
    if (!window || !monitor)
        return;

    if (window->wl.outputScaleCount + 1 > window->wl.outputScaleSize)
    {
        window->wl.outputScaleSize++;
        window->wl.outputScales =
            _glfw_realloc(window->wl.outputScales,
                          window->wl.outputScaleSize * sizeof(_GLFWscaleWayland));
    }

    window->wl.outputScaleCount++;
    window->wl.outputScales[window->wl.outputScaleCount - 1] =
        (_GLFWscaleWayland) { output, monitor->wl.scale };

    _glfwUpdateBufferScaleFromOutputsWayland(window);
}

static void surfaceHandleLeave(void* userData,
                               struct wl_surface* surface,
                               struct wl_output* output)
{
    if (wl_proxy_get_tag((struct wl_proxy*) output) != &_glfw.wl.tag)
        return;

    _GLFWwindow* window = userData;

    for (size_t i = 0; i < window->wl.outputScaleCount; i++)
    {
        if (window->wl.outputScales[i].output == output)
        {
            window->wl.outputScales[i] =
                window->wl.outputScales[window->wl.outputScaleCount - 1];
            window->wl.outputScaleCount--;
            break;
        }
    }

    _glfwUpdateBufferScaleFromOutputsWayland(window);
}

static const struct wl_surface_listener surfaceListener =
{
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

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_GLFWwindow* window)
{
    if (window->wl.libdecor.frame)
    {
        libdecor_frame_set_fullscreen(window->wl.libdecor.frame,
                                      window->monitor->wl.output);
    }
    else if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel,
                                    window->monitor->wl.output);
    }

    setIdleInhibitor(window, GLFW_TRUE);

    if (window->wl.fallback.decorations)
        destroyFallbackDecorations(window);
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_GLFWwindow* window)
{
    if (window->wl.libdecor.frame)
        libdecor_frame_unset_fullscreen(window->wl.libdecor.frame);
    else if (window->wl.xdg.toplevel)
        xdg_toplevel_unset_fullscreen(window->wl.xdg.toplevel);

    setIdleInhibitor(window, GLFW_FALSE);

    if (!window->wl.libdecor.frame &&
        window->wl.xdg.decorationMode != ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE)
    {
        if (window->decorated)
            createFallbackDecorations(window);
    }
}

void fractionalScaleHandlePreferredScale(void* userData,
                                         struct wp_fractional_scale_v1* fractionalScale,
                                         uint32_t numerator)
{
    _GLFWwindow* window = userData;

    window->wl.scalingNumerator = numerator;
    resizeFramebuffer(window);

    float xscale, yscale;
    _glfwGetWindowContentScaleWayland(window, &xscale, &yscale);
    _glfwInputWindowContentScale(window, xscale, yscale);

    // Update all monitors with the fractional scale so glfwGetMonitorContentScale
    // returns the accurate value instead of the integer wl_output scale.
    for (int i = 0; i < _glfw.monitorCount; i++)
        _glfw.monitors[i]->wl.fractionalScaleNumerator = numerator;

    if (window->wl.visible)
        _glfwInputWindowDamage(window);
}

const struct wp_fractional_scale_v1_listener fractionalScaleListener =
{
    fractionalScaleHandlePreferredScale,
};

static void xdgToplevelHandleConfigure(void* userData,
                                       struct xdg_toplevel* toplevel,
                                       int32_t width,
                                       int32_t height,
                                       struct wl_array* states)
{
    _GLFWwindow* window = userData;
    uint32_t* state;

    window->wl.pending.activated  = GLFW_FALSE;
    window->wl.pending.maximized  = GLFW_FALSE;
    window->wl.pending.fullscreen = GLFW_FALSE;

    wl_array_for_each(state, states)
    {
        switch (*state)
        {
            case XDG_TOPLEVEL_STATE_MAXIMIZED:
                window->wl.pending.maximized = GLFW_TRUE;
                break;
            case XDG_TOPLEVEL_STATE_FULLSCREEN:
                window->wl.pending.fullscreen = GLFW_TRUE;
                break;
            case XDG_TOPLEVEL_STATE_RESIZING:
                break;
            case XDG_TOPLEVEL_STATE_ACTIVATED:
                window->wl.pending.activated = GLFW_TRUE;
                break;
        }
    }

    if (width && height)
    {
        if (window->wl.fallback.decorations)
        {
            window->wl.pending.width  = _glfw_max(0, width - GLFW_BORDER_SIZE * 2);
            window->wl.pending.height =
                _glfw_max(0, height - GLFW_BORDER_SIZE - GLFW_CAPTION_HEIGHT);
        }
        else
        {
            window->wl.pending.width  = width;
            window->wl.pending.height = height;
        }
    }
    else
    {
        window->wl.pending.width  = window->wl.width;
        window->wl.pending.height = window->wl.height;
    }
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
    _GLFWwindow* window = userData;

    xdg_surface_ack_configure(surface, serial);

    if (window->wl.activated != window->wl.pending.activated)
    {
        window->wl.activated = window->wl.pending.activated;
        if (!window->wl.activated)
        {
            if (window->monitor && window->autoIconify)
                xdg_toplevel_set_minimized(window->wl.xdg.toplevel);
        }
    }

    if (window->wl.maximized != window->wl.pending.maximized)
    {
        window->wl.maximized = window->wl.pending.maximized;
        _glfwInputWindowMaximize(window, window->wl.maximized);
    }

    window->wl.fullscreen = window->wl.pending.fullscreen;

    int width  = window->wl.pending.width;
    int height = window->wl.pending.height;

    if (!window->wl.maximized && !window->wl.fullscreen)
    {
        if (window->numer != GLFW_DONT_CARE && window->denom != GLFW_DONT_CARE)
        {
            const float aspectRatio = (float) width / (float) height;
            const float targetRatio = (float) window->numer / (float) window->denom;
            if (aspectRatio < targetRatio)
                height = width / targetRatio;
            else if (aspectRatio > targetRatio)
                width = height * targetRatio;
        }
    }

    if (resizeWindow(window, width, height))
    {
        _glfwInputWindowSize(window, window->wl.width, window->wl.height);

        if (window->wl.visible)
            _glfwInputWindowDamage(window);
    }

    if (!window->wl.visible)
    {
        // Allow the window to be mapped only if it either has no XDG
        // decorations or they have already received a configure event
        if (!window->wl.xdg.decoration || window->wl.xdg.decorationMode)
        {
            window->wl.visible = GLFW_TRUE;
            _glfwInputWindowDamage(window);
        }
    }
}

static const struct xdg_surface_listener xdgSurfaceListener =
{
    xdgSurfaceHandleConfigure
};

void libdecorFrameHandleConfigure(struct libdecor_frame* frame,
                                  struct libdecor_configuration* config,
                                  void* userData)
{
    _GLFWwindow* window = userData;
    int width, height;

    enum libdecor_window_state windowState;
    GLFWbool fullscreen, activated, maximized;

    if (libdecor_configuration_get_window_state(config, &windowState))
    {
        fullscreen = (windowState & LIBDECOR_WINDOW_STATE_FULLSCREEN) != 0;
        activated = (windowState & LIBDECOR_WINDOW_STATE_ACTIVE) != 0;
        maximized = (windowState & LIBDECOR_WINDOW_STATE_MAXIMIZED) != 0;
    }
    else
    {
        fullscreen = window->wl.fullscreen;
        activated = window->wl.activated;
        maximized = window->wl.maximized;
    }

    if (!libdecor_configuration_get_content_size(config, frame, &width, &height))
    {
        width = window->wl.width;
        height = window->wl.height;
    }

    if (!maximized && !fullscreen)
    {
        if (window->numer != GLFW_DONT_CARE && window->denom != GLFW_DONT_CARE)
        {
            const float aspectRatio = (float) width / (float) height;
            const float targetRatio = (float) window->numer / (float) window->denom;
            if (aspectRatio < targetRatio)
                height = width / targetRatio;
            else if (aspectRatio > targetRatio)
                width = height * targetRatio;
        }
    }

    struct libdecor_state* frameState = libdecor_state_new(width, height);
    libdecor_frame_commit(frame, frameState, config);
    libdecor_state_free(frameState);

    if (window->wl.activated != activated)
    {
        window->wl.activated = activated;
        if (!window->wl.activated)
        {
            if (window->monitor && window->autoIconify)
                libdecor_frame_set_minimized(window->wl.libdecor.frame);
        }
    }

    if (window->wl.maximized != maximized)
    {
        window->wl.maximized = maximized;
        _glfwInputWindowMaximize(window, window->wl.maximized);
    }

    window->wl.fullscreen = fullscreen;

    GLFWbool damaged = GLFW_FALSE;

    if (!window->wl.visible)
    {
        window->wl.visible = GLFW_TRUE;
        damaged = GLFW_TRUE;
    }

    if (resizeWindow(window, width, height))
    {
        _glfwInputWindowSize(window, window->wl.width, window->wl.height);
        damaged = GLFW_TRUE;
    }

    if (damaged)
        _glfwInputWindowDamage(window);
    else
        wl_surface_commit(window->wl.surface);
}

void libdecorFrameHandleClose(struct libdecor_frame* frame, void* userData)
{
    _GLFWwindow* window = userData;
    _glfwInputWindowCloseRequest(window);
}

void libdecorFrameHandleCommit(struct libdecor_frame* frame, void* userData)
{
    _GLFWwindow* window = userData;
    wl_surface_commit(window->wl.surface);
}

void libdecorFrameHandleDismissPopup(struct libdecor_frame* frame,
                                     const char* seatName,
                                     void* userData)
{
}

static const struct libdecor_frame_interface libdecorFrameInterface =
{
    libdecorFrameHandleConfigure,
    libdecorFrameHandleClose,
    libdecorFrameHandleCommit,
    libdecorFrameHandleDismissPopup
};

static _GLFWwindow* findWaylandPopupParent(_GLFWwindow* self);

static GLFWbool createLibdecorFrame(_GLFWwindow* window)
{
    // Allow libdecor to finish initialization of itself and its plugin
    while (!_glfw.wl.libdecor.ready)
        _glfwWaitEventsWayland();

    window->wl.libdecor.frame = libdecor_decorate(_glfw.wl.libdecor.context,
                                                  window->wl.surface,
                                                  &libdecorFrameInterface,
                                                  window);
    if (!window->wl.libdecor.frame)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create libdecor frame");
        return GLFW_FALSE;
    }

    struct libdecor_state* frameState =
        libdecor_state_new(window->wl.width, window->wl.height);
    libdecor_frame_commit(window->wl.libdecor.frame, frameState, NULL);
    libdecor_state_free(frameState);

    if (strlen(window->wl.appId))
        libdecor_frame_set_app_id(window->wl.libdecor.frame, window->wl.appId);

    libdecor_frame_set_title(window->wl.libdecor.frame, window->title);

    if (window->minwidth != GLFW_DONT_CARE &&
        window->minheight != GLFW_DONT_CARE)
    {
        libdecor_frame_set_min_content_size(window->wl.libdecor.frame,
                                            window->minwidth,
                                            window->minheight);
    }

    if (window->maxwidth != GLFW_DONT_CARE &&
        window->maxheight != GLFW_DONT_CARE)
    {
        libdecor_frame_set_max_content_size(window->wl.libdecor.frame,
                                            window->maxwidth,
                                            window->maxheight);
    }

    if (!window->resizable)
    {
        libdecor_frame_unset_capabilities(window->wl.libdecor.frame,
                                          LIBDECOR_ACTION_RESIZE);
    }

    if (window->monitor)
    {
        libdecor_frame_set_fullscreen(window->wl.libdecor.frame,
                                      window->monitor->wl.output);
        setIdleInhibitor(window, GLFW_TRUE);
    }
    else
    {
        if (window->wl.maximized)
            libdecor_frame_set_maximized(window->wl.libdecor.frame);

        if (!window->decorated)
            libdecor_frame_set_visibility(window->wl.libdecor.frame, false);

        setIdleInhibitor(window, GLFW_FALSE);
    }

    // Same as createXdgShellObjects: parent secondary toplevels.
    if (window->wl.createdUnfocused && !window->monitor)
    {
        _GLFWwindow* parent = findWaylandPopupParent(window);
        if (parent)
        {
            struct xdg_toplevel* parentToplevel = parent->wl.xdg.toplevel;
            if (!parentToplevel && parent->wl.libdecor.frame)
                parentToplevel = libdecor_frame_get_xdg_toplevel(parent->wl.libdecor.frame);
            struct xdg_toplevel* ownToplevel =
                libdecor_frame_get_xdg_toplevel(window->wl.libdecor.frame);
            if (parentToplevel && ownToplevel)
                xdg_toplevel_set_parent(ownToplevel, parentToplevel);
        }
    }

    libdecor_frame_map(window->wl.libdecor.frame);
    wl_display_roundtrip(_glfw.wl.display);
    return GLFW_TRUE;
}

static void updateXdgSizeLimits(_GLFWwindow* window)
{
    int minwidth, minheight, maxwidth, maxheight;

    if (window->resizable)
    {
        if (window->minwidth == GLFW_DONT_CARE || window->minheight == GLFW_DONT_CARE)
            minwidth = minheight = 0;
        else
        {
            minwidth  = window->minwidth;
            minheight = window->minheight;

            if (window->wl.fallback.decorations)
            {
                minwidth  += GLFW_BORDER_SIZE * 2;
                minheight += GLFW_CAPTION_HEIGHT + GLFW_BORDER_SIZE;
            }
        }

        if (window->maxwidth == GLFW_DONT_CARE || window->maxheight == GLFW_DONT_CARE)
            maxwidth = maxheight = 0;
        else
        {
            maxwidth  = window->maxwidth;
            maxheight = window->maxheight;

            if (window->wl.fallback.decorations)
            {
                maxwidth  += GLFW_BORDER_SIZE * 2;
                maxheight += GLFW_CAPTION_HEIGHT + GLFW_BORDER_SIZE;
            }
        }
    }
    else
    {
        minwidth = maxwidth = window->wl.width;
        minheight = maxheight = window->wl.height;
    }

    xdg_toplevel_set_min_size(window->wl.xdg.toplevel, minwidth, minheight);
    xdg_toplevel_set_max_size(window->wl.xdg.toplevel, maxwidth, maxheight);
}

static void xdgPopupSurfaceHandleConfigure(void* userData,
                                           struct xdg_surface* surface,
                                           uint32_t serial)
{
    // Popups skip the toplevel state machine; just ACK and mark visible.
    _GLFWwindow* window = userData;
    xdg_surface_ack_configure(surface, serial);

    if (!window->wl.visible)
    {
        window->wl.visible = GLFW_TRUE;
        _glfwInputWindowDamage(window);
    }
}

static const struct xdg_surface_listener xdgPopupSurfaceListener =
{
    xdgPopupSurfaceHandleConfigure
};

static void xdgPopupHandleConfigure(void* userData,
                                    struct xdg_popup* popup,
                                    int32_t x, int32_t y,
                                    int32_t width, int32_t height)
{
    // Resize framebuffer to match compositor-confirmed popup geometry.
    _GLFWwindow* window = userData;
    if (width > 0 && height > 0 &&
        (width != window->wl.width || height != window->wl.height))
    {
        window->wl.width = width;
        window->wl.height = height;
        resizeFramebuffer(window);
        _glfwInputWindowSize(window, width, height);
    }
}

static void xdgPopupHandlePopupDone(void* userData, struct xdg_popup* popup)
{
    // Compositor dismissed the popup (e.g. outside click).
    _GLFWwindow* window = userData;
    _glfwInputWindowCloseRequest(window);
}

static void xdgPopupHandleRepositioned(void* userData,
                                       struct xdg_popup* popup,
                                       uint32_t token)
{
    // Unused: we don't currently issue xdg_popup.reposition requests.
}

static const struct xdg_popup_listener xdgPopupListener =
{
    xdgPopupHandleConfigure,
    xdgPopupHandlePopupDone,
    xdgPopupHandleRepositioned,
};

// Resolve the xdg_surface of a window that owns a mapped toplevel, whether
// created directly via xdg-shell or wrapped by libdecor. Returns NULL if the
// window has no queryable xdg_surface (libdecor < 0.2 lacks the accessor).
static struct xdg_surface* getWindowXdgSurface(_GLFWwindow* w)
{
    if (w->wl.xdg.surface)
        return w->wl.xdg.surface;
    if (w->wl.libdecor.frame && _glfw.wl.libdecor.libdecor_frame_get_xdg_surface_)
        return libdecor_frame_get_xdg_surface(w->wl.libdecor.frame);
    return NULL;
}

// Walk the GLFW window list and return the first window that owns a mapped
// xdg_toplevel (directly or through libdecor) and has a queryable xdg_surface.
// That window acts as the parent surface for any popup we spawn.
static _GLFWwindow* findWaylandPopupParent(_GLFWwindow* self)
{
    for (_GLFWwindow* w = _glfw.windowListHead; w; w = w->next)
    {
        if (w == self) continue;
        if (w->wl.xdg.popup) continue;
        if (!getWindowXdgSurface(w)) continue;
        if (w->wl.xdg.toplevel || w->wl.libdecor.frame)
            return w;
    }
    return NULL;
}

static GLFWbool createXdgPopupShellObjects(_GLFWwindow* window,
                                           _GLFWwindow* parent)
{
    window->wl.xdg.surface = xdg_wm_base_get_xdg_surface(_glfw.wl.wmBase,
                                                         window->wl.surface);
    if (!window->wl.xdg.surface)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create xdg-surface for popup");
        return GLFW_FALSE;
    }
    xdg_surface_add_listener(window->wl.xdg.surface, &xdgPopupSurfaceListener, window);

    struct xdg_positioner* positioner =
        xdg_wm_base_create_positioner(_glfw.wl.wmBase);
    if (!positioner)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create xdg-positioner for popup");
        return GLFW_FALSE;
    }

    const int w = window->wl.width  > 0 ? window->wl.width  : 1;
    const int h = window->wl.height > 0 ? window->wl.height : 1;
    xdg_positioner_set_size(positioner, w, h);

    // pendingPos is parent-relative (no global coords on Wayland).
    const int px = window->wl.pendingPosSet ? window->wl.pendingPosX : 0;
    const int py = window->wl.pendingPosSet ? window->wl.pendingPosY : 0;
    xdg_positioner_set_anchor_rect(positioner, px, py, 1, 1);
    xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
    xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
    xdg_positioner_set_constraint_adjustment(positioner,
        XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X |
        XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y |
        XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X |
        XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y);

    window->wl.xdg.popup = xdg_surface_get_popup(window->wl.xdg.surface,
                                                 getWindowXdgSurface(parent),
                                                 positioner);
    xdg_positioner_destroy(positioner);

    if (!window->wl.xdg.popup)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create xdg-popup");
        return GLFW_FALSE;
    }
    xdg_popup_add_listener(window->wl.xdg.popup, &xdgPopupListener, window);

    // Grab with a press serial so the compositor auto-dismisses on outside click.
    if (_glfw.wl.seat && _glfw.wl.pointerButtonSerial)
        xdg_popup_grab(window->wl.xdg.popup, _glfw.wl.seat, _glfw.wl.pointerButtonSerial);

    wl_surface_commit(window->wl.surface);
    wl_display_roundtrip(_glfw.wl.display);
    return GLFW_TRUE;
}

static GLFWbool createXdgShellObjects(_GLFWwindow* window)
{
    window->wl.xdg.surface = xdg_wm_base_get_xdg_surface(_glfw.wl.wmBase,
                                                         window->wl.surface);
    if (!window->wl.xdg.surface)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create xdg-surface for window");
        return GLFW_FALSE;
    }

    xdg_surface_add_listener(window->wl.xdg.surface, &xdgSurfaceListener, window);

    window->wl.xdg.toplevel = xdg_surface_get_toplevel(window->wl.xdg.surface);
    if (!window->wl.xdg.toplevel)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to create xdg-toplevel for window");
        return GLFW_FALSE;
    }

    xdg_toplevel_add_listener(window->wl.xdg.toplevel, &xdgToplevelListener, window);

    if (window->wl.appId)
        xdg_toplevel_set_app_id(window->wl.xdg.toplevel, window->wl.appId);

    xdg_toplevel_set_title(window->wl.xdg.toplevel, window->title);

    // Parent secondary toplevels so the compositor keeps them grouped
    // (prevents the main window from getting buried during drags).
    if (window->wl.createdUnfocused && !window->monitor)
    {
        _GLFWwindow* parent = findWaylandPopupParent(window);
        if (parent)
        {
            struct xdg_toplevel* parentToplevel = parent->wl.xdg.toplevel;
            if (!parentToplevel && parent->wl.libdecor.frame)
                parentToplevel = libdecor_frame_get_xdg_toplevel(parent->wl.libdecor.frame);
            if (parentToplevel)
                xdg_toplevel_set_parent(window->wl.xdg.toplevel, parentToplevel);
        }
    }

    if (window->monitor)
    {
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel, window->monitor->wl.output);
        setIdleInhibitor(window, GLFW_TRUE);
    }
    else
    {
        if (window->wl.maximized)
            xdg_toplevel_set_maximized(window->wl.xdg.toplevel);

        setIdleInhibitor(window, GLFW_FALSE);
    }

    if (_glfw.wl.decorationManager)
    {
        window->wl.xdg.decoration =
            zxdg_decoration_manager_v1_get_toplevel_decoration(
                _glfw.wl.decorationManager, window->wl.xdg.toplevel);
        zxdg_toplevel_decoration_v1_add_listener(window->wl.xdg.decoration,
                                                 &xdgDecorationListener,
                                                 window);

        uint32_t mode;

        if (window->decorated)
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
        else
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;

        zxdg_toplevel_decoration_v1_set_mode(window->wl.xdg.decoration, mode);
    }
    else
    {
        if (window->decorated && !window->monitor)
            createFallbackDecorations(window);
    }

    updateXdgSizeLimits(window);

    wl_surface_commit(window->wl.surface);
    wl_display_roundtrip(_glfw.wl.display);
    return GLFW_TRUE;
}

static GLFWbool startToplevelDragSession(_GLFWwindow* window, uint32_t serial);

static void issuePendingDragMove(_GLFWwindow* window)
{
    if (!window->wl.dragPending || !_glfw.wl.seat)
        return;
    window->wl.dragPending = GLFW_FALSE;

    // Button was released while waiting for map; serial is stale.
    if (_glfw.wl.pointerButtonsDown <= 0)
        return;

    if (startToplevelDragSession(window, window->wl.dragPendingSerial))
        return;

    struct xdg_toplevel* toplevel = window->wl.xdg.toplevel;
    if (!toplevel && window->wl.libdecor.frame)
        toplevel = libdecor_frame_get_xdg_toplevel(window->wl.libdecor.frame);
    if (!toplevel)
        return;

    xdg_toplevel_move(toplevel, _glfw.wl.seat, window->wl.dragPendingSerial);

    // Fallback path: compositor swallows release, synthesize one.
    if (_glfw.wl.pointerButtonsDown > 0)
        _glfw.wl.pointerButtonsDown--;
    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
}

static void mappedFrameHandleDone(void* userData,
                                  struct wl_callback* callback,
                                  uint32_t data)
{
    _GLFWwindow* window = userData;
    wl_callback_destroy(callback);
    window->wl.mappedCallback = NULL;
    window->wl.mapped = GLFW_TRUE;

    // Now that the surface is mapped, fire any deferred drag request.
    issuePendingDragMove(window);
}

static const struct wl_callback_listener mappedFrameListener =
{
    mappedFrameHandleDone
};

static void armMappedFrameCallback(_GLFWwindow* window)
{
    if (window->wl.mapped || window->wl.mappedCallback)
        return;
    window->wl.mappedCallback = wl_surface_frame(window->wl.surface);
    if (window->wl.mappedCallback)
    {
        wl_callback_add_listener(window->wl.mappedCallback,
                                 &mappedFrameListener, window);
    }
}

static GLFWbool createShellObjects(_GLFWwindow* window)
{
    // Menu-style windows (undecorated + !focusOnShow) become xdg_popups.
    if (!window->focusOnShow && !window->decorated && !window->monitor)
    {
        _GLFWwindow* parent = findWaylandPopupParent(window);
        if (parent)
            return createXdgPopupShellObjects(window, parent);
    }

    GLFWbool ok;
    if (_glfw.wl.libdecor.context && createLibdecorFrame(window))
        ok = GLFW_TRUE;
    else
        ok = createXdgShellObjects(window);

    // Deferred drag-moves wait for this "truly mapped" signal.
    if (ok)
        armMappedFrameCallback(window);

    return ok;
}

static void destroyShellObjects(_GLFWwindow* window)
{
    destroyFallbackDecorations(window);

    if (window->wl.libdecor.frame)
        libdecor_frame_unref(window->wl.libdecor.frame);

    if (window->wl.xdg.decoration)
        zxdg_toplevel_decoration_v1_destroy(window->wl.xdg.decoration);

    if (window->wl.xdg.popup)
        xdg_popup_destroy(window->wl.xdg.popup);

    if (window->wl.xdg.toplevel)
        xdg_toplevel_destroy(window->wl.xdg.toplevel);

    if (window->wl.xdg.surface)
        xdg_surface_destroy(window->wl.xdg.surface);

    if (window->wl.mappedCallback)
        wl_callback_destroy(window->wl.mappedCallback);

    window->wl.libdecor.frame = NULL;
    window->wl.xdg.decoration = NULL;
    window->wl.xdg.decorationMode = 0;
    window->wl.xdg.popup = NULL;
    window->wl.xdg.toplevel = NULL;
    window->wl.xdg.surface = NULL;
    window->wl.mappedCallback = NULL;
    window->wl.mapped = GLFW_FALSE;
    window->wl.dragPending = GLFW_FALSE;
}

static GLFWbool createNativeSurface(_GLFWwindow* window,
                                    const _GLFWwndconfig* wndconfig,
                                    const _GLFWfbconfig* fbconfig)
{
    window->wl.surface = wl_compositor_create_surface(_glfw.wl.compositor);
    if (!window->wl.surface)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Wayland: Failed to create window surface");
        return GLFW_FALSE;
    }

    wl_proxy_set_tag((struct wl_proxy*) window->wl.surface, &_glfw.wl.tag);
    wl_surface_add_listener(window->wl.surface,
                            &surfaceListener,
                            window);

    window->wl.width = wndconfig->width;
    window->wl.height = wndconfig->height;
    window->wl.fbWidth = wndconfig->width;
    window->wl.fbHeight = wndconfig->height;
    window->wl.appId = _glfw_strdup(wndconfig->wl.appId);

    window->wl.bufferScale = 1;
    window->wl.scalingNumerator = 120;
    window->wl.scaleFramebuffer = wndconfig->scaleFramebuffer;

    window->wl.maximized = wndconfig->maximized;

    window->wl.transparent = fbconfig->transparent;
    if (!window->wl.transparent)
        setContentAreaOpaque(window);

    if (_glfw.wl.fractionalScaleManager)
    {
        if (window->wl.scaleFramebuffer)
        {
            window->wl.scalingViewport =
                wp_viewporter_get_viewport(_glfw.wl.viewporter, window->wl.surface);

            wp_viewport_set_destination(window->wl.scalingViewport,
                                        window->wl.width,
                                        window->wl.height);

            window->wl.fractionalScale =
                wp_fractional_scale_manager_v1_get_fractional_scale(
                    _glfw.wl.fractionalScaleManager,
                    window->wl.surface);

            wp_fractional_scale_v1_add_listener(window->wl.fractionalScale,
                                                &fractionalScaleListener,
                                                window);
        }
    }

    return GLFW_TRUE;
}

static void incrementCursorImage(void)
{
    if (_glfw.wl.pointerSurface)
    {
        _GLFWwindow* window = wl_surface_get_user_data(_glfw.wl.pointerSurface);
        if (window->wl.surface == _glfw.wl.pointerSurface)
        {
            _GLFWcursor* cursor = window->cursor;
            if (cursor && cursor->wl.cursor)
            {
                cursor->wl.currentImage += 1;
                cursor->wl.currentImage %= cursor->wl.cursor->image_count;
                applyCursor(window);
            }
        }
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

static void inputText(_GLFWwindow* window, uint32_t scancode)
{
    const xkb_keysym_t* keysyms;
    const xkb_keycode_t keycode = scancode + 8;

    if (xkb_state_key_get_syms(_glfw.wl.xkb.state, keycode, &keysyms) == 1)
    {
        const xkb_keysym_t keysym = composeSymbol(keysyms[0]);
        const uint32_t codepoint = xkb_keysym_to_utf32(keysym);
        if (codepoint != 0)
        {
            const int mods = _glfw.wl.xkb.modifiers;
            const int plain = !(mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT));
            _glfwInputChar(window, codepoint, mods, plain);
        }
    }
}

static void handleEvents(double* timeout)
{
#if defined(GLFW_BUILD_LINUX_JOYSTICK)
    if (_glfw.joysticksInitialized)
        _glfwDetectJoystickConnectionLinux();
#endif

    GLFWbool event = GLFW_FALSE;
    enum { DISPLAY_FD, KEYREPEAT_FD, CURSOR_FD, LIBDECOR_FD };
    struct pollfd fds[] =
    {
        [DISPLAY_FD] = { wl_display_get_fd(_glfw.wl.display), POLLIN },
        [KEYREPEAT_FD] = { _glfw.wl.keyRepeatTimerfd, POLLIN },
        [CURSOR_FD] = { _glfw.wl.cursorTimerfd, POLLIN },
        [LIBDECOR_FD] = { -1, POLLIN }
    };

    if (_glfw.wl.libdecor.context)
        fds[LIBDECOR_FD].fd = libdecor_get_fd(_glfw.wl.libdecor.context);

    while (!event)
    {
        while (wl_display_prepare_read(_glfw.wl.display) != 0)
        {
            if (wl_display_dispatch_pending(_glfw.wl.display) > 0)
                return;
        }

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

        if (!_glfwPollPOSIX(fds, sizeof(fds) / sizeof(fds[0]), timeout))
        {
            wl_display_cancel_read(_glfw.wl.display);
            return;
        }

        if (fds[DISPLAY_FD].revents & POLLIN)
        {
            wl_display_read_events(_glfw.wl.display);
            if (wl_display_dispatch_pending(_glfw.wl.display) > 0)
                event = GLFW_TRUE;
        }
        else
            wl_display_cancel_read(_glfw.wl.display);

        if (fds[KEYREPEAT_FD].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_glfw.wl.keyRepeatTimerfd, &repeats, sizeof(repeats)) == 8)
            {
                if (_glfw.wl.keyboardFocus)
                {
                    for (uint64_t i = 0; i < repeats; i++)
                    {
                        _glfwInputKey(_glfw.wl.keyboardFocus,
                                      translateKey(_glfw.wl.keyRepeatScancode),
                                      _glfw.wl.keyRepeatScancode,
                                      GLFW_PRESS,
                                      _glfw.wl.xkb.modifiers);
                        inputText(_glfw.wl.keyboardFocus, _glfw.wl.keyRepeatScancode);
                    }

                    event = GLFW_TRUE;
                }

            }
        }

        if (fds[CURSOR_FD].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_glfw.wl.cursorTimerfd, &repeats, sizeof(repeats)) == 8)
                incrementCursorImage();
        }

        if (fds[LIBDECOR_FD].revents & POLLIN)
        {
            if (libdecor_dispatch(_glfw.wl.libdecor.context, 0) > 0)
                event = GLFW_TRUE;
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
                _glfw_free(string);
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
            _glfw_free(string);
            close(fds[0]);
            return NULL;
        }

        length += result;
    }

    close(fds[0]);

    string[length] = '\0';
    return string;
}

static void processPointerEnterSurface(struct wl_surface* surface)
{
    _glfw.wl.pointerSurface = surface;

    _GLFWwindow* window = wl_surface_get_user_data(_glfw.wl.pointerSurface);
    if (window->wl.surface == _glfw.wl.pointerSurface)
    {
        _glfwSetCursorWayland(window, window->cursor);
        _glfwInputCursorEnter(window, GLFW_TRUE);
    }
}

static void processPointerLeaveSurface(struct wl_surface* surface)
{
    _glfw.wl.pointerSurface = NULL;

    _GLFWwindow* window = wl_surface_get_user_data(surface);
    if (window->wl.surface == surface)
    {
        window->wl.resizeEdge = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
        // Suppress cursor-leave during toplevel drag so the app doesn't
        // park the cursor at FLT_MAX. Drag-and-drop events provide position instead.
        if (!_glfw.wl.toplevelDragSession.source)
            _glfwInputCursorEnter(window, GLFW_FALSE);
    }
}

static void updateResizeEdge(_GLFWwindow* window)
{
    uint32_t edge = XDG_TOPLEVEL_RESIZE_EDGE_NONE;

    if (window->resizable && !window->decorated && !window->monitor &&
        !window->wl.maximized && !window->wl.fullscreen)
    {
        edge = detectResizeEdge(window,
                                (int) window->wl.cursorPosX,
                                (int) window->wl.cursorPosY);
    }

    if (edge != window->wl.resizeEdge)
    {
        window->wl.resizeEdge = edge;
        applyCursor(window);
    }
}

static void processPointerMotion(double xpos, double ypos)
{
    _GLFWwindow* window = wl_surface_get_user_data(_glfw.wl.pointerSurface);
    if (window->wl.surface == _glfw.wl.pointerSurface)
    {
        if (window->cursorMode != GLFW_CURSOR_DISABLED)
        {
            window->wl.cursorPosX = xpos;
            window->wl.cursorPosY = ypos;
            _glfwInputCursorPos(window, window->wl.cursorPosX, window->wl.cursorPosY);
            updateResizeEdge(window);
        }
    }
    else
    {
        if (window->wl.fallback.decorations)
        {
            window->wl.fallback.pointerX = xpos;
            window->wl.fallback.pointerY = ypos;
            applyCursor(window);
        }
    }
}

static void processPointerButton(int button, int action)
{
    // Compositor sends a spurious release when transferring the grab to drag-and-drop.
    // Suppress it; endToplevelDragSession synthesizes the real release.
    if (_glfw.wl.toplevelDragSession.source &&
        button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        return;
    }

    _GLFWwindow* window = wl_surface_get_user_data(_glfw.wl.pointerSurface);
    if (window->wl.surface == _glfw.wl.pointerSurface)
    {
        // For undecorated windows, provide OS-level resize borders
        // and titlebar drag via xdg_toplevel — matching the X11
        // _NET_WM_MOVERESIZE behaviour
        GLFWbool handled = GLFW_FALSE;

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS &&
            !window->decorated && !window->monitor)
        {
            struct xdg_toplevel* toplevel = window->wl.xdg.toplevel;
            if (window->wl.libdecor.frame)
                toplevel = libdecor_frame_get_xdg_toplevel(window->wl.libdecor.frame);

            if (toplevel)
            {
                const int x = (int) window->wl.cursorPosX;
                const int y = (int) window->wl.cursorPosY;

                // Resize edges are only active when not maximized/fullscreen
                uint32_t edges = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
                if (window->resizable &&
                    !window->wl.maximized && !window->wl.fullscreen)
                {
                    edges = detectResizeEdge(window, x, y);
                }

                if (edges != XDG_TOPLEVEL_RESIZE_EDGE_NONE)
                {
                    xdg_toplevel_resize(toplevel, _glfw.wl.seat,
                                        _glfw.wl.serial, edges);
                    handled = GLFW_TRUE;
                }
                else
                {
                    int titlebarHit = 0;
                    _glfwInputTitleBarHitTest(window, x, y, &titlebarHit);
                    if (titlebarHit)
                    {
                        xdg_toplevel_move(toplevel, _glfw.wl.seat, _glfw.wl.serial);
                        handled = GLFW_TRUE;
                    }
                }
            }
        }

        if (!handled)
            _glfwInputMouseClick(window, button, action, _glfw.wl.xkb.modifiers);
    }
    else
    {
        if (window->wl.fallback.decorations)
            handleFallbackDecorationButton(window, button, action);
    }
}

static void processPointerScroll(double xoffset, double yoffset)
{
    _GLFWwindow* window = wl_surface_get_user_data(_glfw.wl.pointerSurface);
    if (window->wl.surface == _glfw.wl.pointerSurface)
        _glfwInputScroll(window, xoffset, yoffset);
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

    if (wl_proxy_get_tag((struct wl_proxy*) surface) != &_glfw.wl.tag)
        return;

    _glfw.wl.serial = serial;
    _glfw.wl.pointerEnterSerial = serial;

    const double xpos = wl_fixed_to_double(sx);
    const double ypos = wl_fixed_to_double(sy);

    if (wl_pointer_get_version(pointer) >= WL_POINTER_FRAME_SINCE_VERSION)
    {
        _glfw.wl.pending.events |= (GLFW_PENDING_SURFACE | GLFW_PENDING_MOTION);
        _glfw.wl.pending.pointerSurface = surface;
        _glfw.wl.pending.pointerX = xpos;
        _glfw.wl.pending.pointerY = ypos;
    }
    else
    {
        processPointerEnterSurface(surface);
        processPointerMotion(xpos, ypos);
    }
}

static void pointerHandleLeave(void* userData,
                               struct wl_pointer* pointer,
                               uint32_t serial,
                               struct wl_surface* surface)
{
    if (!surface)
        return;

    if (wl_proxy_get_tag((struct wl_proxy*) surface) != &_glfw.wl.tag)
        return;

    _glfw.wl.serial = serial;

    if (wl_pointer_get_version(pointer) >= WL_POINTER_FRAME_SINCE_VERSION)
    {
        _glfw.wl.pending.events |= GLFW_PENDING_SURFACE;
        _glfw.wl.pending.pointerSurface = NULL;
    }
    else
        processPointerLeaveSurface(surface);
}

static void pointerHandleMotion(void* userData,
                                struct wl_pointer* pointer,
                                uint32_t time,
                                wl_fixed_t sx,
                                wl_fixed_t sy)
{
    if (!_glfw.wl.pointerSurface)
        return;

    const double xpos = wl_fixed_to_double(sx);
    const double ypos = wl_fixed_to_double(sy);

    if (wl_pointer_get_version(pointer) >= WL_POINTER_FRAME_SINCE_VERSION)
    {
        _glfw.wl.pending.events |= GLFW_PENDING_MOTION;
        _glfw.wl.pending.pointerX = xpos;
        _glfw.wl.pending.pointerY = ypos;
    }
    else
        processPointerMotion(xpos, ypos);
}

static void pointerHandleButton(void* userData,
                                struct wl_pointer* pointer,
                                uint32_t serial,
                                uint32_t time,
                                uint32_t buttonID,
                                uint32_t state)
{
    if (!_glfw.wl.pointerSurface)
        return;

    _glfw.wl.serial = serial;

    // Suppress the spurious release the compositor sends when transferring
    // the implicit grab to the drag-and-drop session.
    const int btnIdx = buttonID - BTN_LEFT;
    if (_glfw.wl.toplevelDragSession.source &&
        btnIdx == GLFW_MOUSE_BUTTON_LEFT &&
        state != WL_POINTER_BUTTON_STATE_PRESSED)
    {
        return;
    }

    if (state == WL_POINTER_BUTTON_STATE_PRESSED)
    {
        _glfw.wl.pointerButtonSerial = serial;
        _glfw.wl.pointerButtonSurface = _glfw.wl.pointerSurface;
        if (_glfw.wl.pointerSurface &&
            wl_proxy_get_tag((struct wl_proxy*) _glfw.wl.pointerSurface)
                == &_glfw.wl.tag)
        {
            _GLFWwindow* w = wl_surface_get_user_data(_glfw.wl.pointerSurface);
            if (w)
            {
                _glfw.wl.pointerButtonX = w->wl.cursorPosX;
                _glfw.wl.pointerButtonY = w->wl.cursorPosY;
            }
        }
        _glfw.wl.pointerButtonsDown++;
    }
    else if (_glfw.wl.pointerButtonsDown > 0)
    {
        _glfw.wl.pointerButtonsDown--;
    }

    const int button = buttonID - BTN_LEFT;
    const int action = (state == WL_POINTER_BUTTON_STATE_PRESSED);

    _GLFWwindow* window = wl_surface_get_user_data(_glfw.wl.pointerSurface);
    if (window->wl.fallback.decorations)
    {
        if (action == GLFW_PRESS)
            window->wl.fallback.buttonPressSerial = serial;
    }

    if (wl_pointer_get_version(pointer) >= WL_POINTER_FRAME_SINCE_VERSION)
    {
        _glfw.wl.pending.events |= GLFW_PENDING_BUTTON;
        _glfw.wl.pending.button = button;
        _glfw.wl.pending.action = action;
    }
    else
        processPointerButton(button, action);
}

static void pointerHandleAxis(void* userData,
                              struct wl_pointer* pointer,
                              uint32_t time,
                              uint32_t axis,
                              wl_fixed_t value)
{
    if (!_glfw.wl.pointerSurface)
        return;

    if (wl_pointer_get_version(pointer) >= WL_POINTER_FRAME_SINCE_VERSION)
    {
        _glfw.wl.pending.events |= GLFW_PENDING_SCROLL;
        if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
            _glfw.wl.pending.scrollX = -wl_fixed_to_double(value) / 10.0;
        else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
            _glfw.wl.pending.scrollY = -wl_fixed_to_double(value) / 10.0;
    }
    else
    {
        // NOTE: 10 units of motion per mouse wheel step seems to be a common ratio
        if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
            processPointerScroll(-wl_fixed_to_double(value) / 10.0, 0.0);
        else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
            processPointerScroll(0.0, -wl_fixed_to_double(value) / 10.0);
    }
}

static void pointerHandleFrame(void* userData, struct wl_pointer* pointer)
{
    if (_glfw.wl.pending.events & GLFW_PENDING_SURFACE)
    {
        if (_glfw.wl.pointerSurface)
            processPointerLeaveSurface(_glfw.wl.pointerSurface);

        if (_glfw.wl.pending.pointerSurface)
            processPointerEnterSurface(_glfw.wl.pending.pointerSurface);
    }

    if (!_glfw.wl.pointerSurface)
        return;

    if (_glfw.wl.pending.events & GLFW_PENDING_MOTION)
        processPointerMotion(_glfw.wl.pending.pointerX, _glfw.wl.pending.pointerY);

    if (_glfw.wl.pending.events & GLFW_PENDING_BUTTON)
        processPointerButton(_glfw.wl.pending.button, _glfw.wl.pending.action);

    if (_glfw.wl.pending.events & GLFW_PENDING_DISCRETE)
        processPointerScroll(_glfw.wl.pending.discreteX, _glfw.wl.pending.discreteY);
    else if (_glfw.wl.pending.events & GLFW_PENDING_SCROLL)
        processPointerScroll(_glfw.wl.pending.scrollX, _glfw.wl.pending.scrollY);

    memset(&_glfw.wl.pending, 0, sizeof(_glfw.wl.pending));
}

static void pointerHandleAxisSource(void* userData,
                                    struct wl_pointer* pointer,
                                    uint32_t axisSource)
{
}

static void pointerHandleAxisStop(void* userData,
                                  struct wl_pointer* pointer,
                                  uint32_t time,
                                  uint32_t axis)
{
}

static void pointerHandleAxisDiscrete(void* userData,
                                      struct wl_pointer* pointer,
                                      uint32_t axis,
                                      int32_t discrete)
{
}

static void pointerHandleAxisValue120(void* data,
                                      struct wl_pointer* pointer,
                                      uint32_t axis,
                                      int32_t value120)
{
    if (!_glfw.wl.pointerSurface)
        return;

    _glfw.wl.pending.events |= GLFW_PENDING_DISCRETE;
    if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
        _glfw.wl.pending.discreteX = -(value120 / 120.0);
    else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
        _glfw.wl.pending.discreteY = -(value120 / 120.0);
}

static const struct wl_pointer_listener pointerListener =
{
    pointerHandleEnter,
    pointerHandleLeave,
    pointerHandleMotion,
    pointerHandleButton,
    pointerHandleAxis,
    pointerHandleFrame,
    pointerHandleAxisSource,
    pointerHandleAxisStop,
    pointerHandleAxisDiscrete,
    pointerHandleAxisValue120
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

    mapStr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapStr == MAP_FAILED)
    {
        close(fd);
        return;
    }

    keymap = xkb_keymap_new_from_string(_glfw.wl.xkb.context,
                                        mapStr,
                                        XKB_KEYMAP_FORMAT_TEXT_V1,
                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
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

    if (wl_proxy_get_tag((struct wl_proxy*) surface) != &_glfw.wl.tag)
        return;

    _GLFWwindow* window = wl_surface_get_user_data(surface);
    if (surface != window->wl.surface)
        return;

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

    struct itimerspec timer = {0};
    timerfd_settime(_glfw.wl.keyRepeatTimerfd, 0, &timer, NULL);

    _glfw.wl.serial = serial;
    _glfw.wl.keyboardFocus = NULL;

    // _glfwInputWindowFocus synthesizes button releases, but during a drag
    // the physical button is still held. Hide button state across the call.
    if (_glfw.wl.pointerButtonsDown > 0)
    {
        GLFWbool savedButtons[GLFW_MOUSE_BUTTON_LAST + 1];
        for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i++)
        {
            savedButtons[i] = window->mouseButtons[i];
            window->mouseButtons[i] = GLFW_RELEASE;
        }
        _glfwInputWindowFocus(window, GLFW_FALSE);
        for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i++)
            window->mouseButtons[i] = savedButtons[i];
    }
    else
    {
        _glfwInputWindowFocus(window, GLFW_FALSE);
    }
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

    struct itimerspec timer = {0};

    if (action == GLFW_PRESS)
    {
        const xkb_keycode_t keycode = scancode + 8;

        if (xkb_keymap_key_repeats(_glfw.wl.xkb.keymap, keycode) &&
            _glfw.wl.keyRepeatRate > 0)
        {
            _glfw.wl.keyRepeatScancode = scancode;
            if (_glfw.wl.keyRepeatRate > 1)
                timer.it_interval.tv_nsec = 1000000000 / _glfw.wl.keyRepeatRate;
            else
                timer.it_interval.tv_sec = 1;

            timer.it_value.tv_sec = _glfw.wl.keyRepeatDelay / 1000;
            timer.it_value.tv_nsec = (_glfw.wl.keyRepeatDelay % 1000) * 1000000;
            timerfd_settime(_glfw.wl.keyRepeatTimerfd, 0, &timer, NULL);
        }
    }
    else if (scancode == _glfw.wl.keyRepeatScancode)
    {
        timerfd_settime(_glfw.wl.keyRepeatTimerfd, 0, &timer, NULL);
    }

    _glfwInputKey(window, key, scancode, action, _glfw.wl.xkb.modifiers);

    if (action == GLFW_PRESS)
        inputText(window, scancode);
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

    _glfw.wl.xkb.modifiers = 0;

    struct
    {
        xkb_mod_index_t index;
        unsigned int bit;
    } modifiers[] =
    {
        { _glfw.wl.xkb.controlIndex,  GLFW_MOD_CONTROL },
        { _glfw.wl.xkb.altIndex,      GLFW_MOD_ALT },
        { _glfw.wl.xkb.shiftIndex,    GLFW_MOD_SHIFT },
        { _glfw.wl.xkb.superIndex,    GLFW_MOD_SUPER },
        { _glfw.wl.xkb.capsLockIndex, GLFW_MOD_CAPS_LOCK },
        { _glfw.wl.xkb.numLockIndex,  GLFW_MOD_NUM_LOCK }
    };

    for (size_t i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); i++)
    {
        if (xkb_state_mod_index_is_active(_glfw.wl.xkb.state,
                                          modifiers[i].index,
                                          XKB_STATE_MODS_EFFECTIVE) == 1)
        {
            _glfw.wl.xkb.modifiers |= modifiers[i].bit;
        }
    }
}

static void keyboardHandleRepeatInfo(void* userData,
                                     struct wl_keyboard* keyboard,
                                     int32_t rate,
                                     int32_t delay)
{
    if (keyboard != _glfw.wl.keyboard)
        return;

    _glfw.wl.keyRepeatRate = rate;
    _glfw.wl.keyRepeatDelay = delay;
}

static const struct wl_keyboard_listener keyboardListener =
{
    keyboardHandleKeymap,
    keyboardHandleEnter,
    keyboardHandleLeave,
    keyboardHandleKey,
    keyboardHandleModifiers,
    keyboardHandleRepeatInfo,
};

static void seatHandleCapabilities(void* userData,
                                   struct wl_seat* seat,
                                   enum wl_seat_capability caps)
{
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !_glfw.wl.pointer)
    {
        _glfw.wl.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(_glfw.wl.pointer, &pointerListener, NULL);

        if (_glfw.wl.cursorShapeManager)
        {
            _glfw.wl.cursorShapeDevice =
                wp_cursor_shape_manager_v1_get_pointer(
                    _glfw.wl.cursorShapeManager, _glfw.wl.pointer);
        }
    }
    else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && _glfw.wl.pointer)
    {
        if (_glfw.wl.cursorShapeDevice)
        {
            wp_cursor_shape_device_v1_destroy(_glfw.wl.cursorShapeDevice);
            _glfw.wl.cursorShapeDevice = NULL;
        }

        if (wl_pointer_get_version(_glfw.wl.pointer) >= WL_POINTER_RELEASE_SINCE_VERSION)
            wl_pointer_release(_glfw.wl.pointer);
        else
            wl_pointer_destroy(_glfw.wl.pointer);

        _glfw.wl.pointer = NULL;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !_glfw.wl.keyboard)
    {
        _glfw.wl.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(_glfw.wl.keyboard, &keyboardListener, NULL);

        if (wl_keyboard_get_version(_glfw.wl.keyboard) <
            WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
        {
            _glfw.wl.keyRepeatRate = 4;
            _glfw.wl.keyRepeatDelay = 500;
        }
    }
    else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && _glfw.wl.keyboard)
    {
        if (wl_keyboard_get_version(_glfw.wl.keyboard) >= WL_KEYBOARD_RELEASE_SINCE_VERSION)
            wl_keyboard_release(_glfw.wl.keyboard);
        else
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
            else if (strcmp(mimeType, GLFW_WAYLAND_WINDOW_DRAG_MIME) == 0)
                _glfw.wl.offers[i].glfw_window_drag = GLFW_TRUE;

            break;
        }
    }
}

static void dataOfferHandleSourceActions(void* userData,
                                         struct wl_data_offer* offer,
                                         uint32_t source_actions)
{
    // v3+ only. We don't act on it directly — the source's advertised action
    // set is negotiated by the compositor and reported back via .action.
    (void)userData; (void)offer; (void)source_actions;
}

static void dataOfferHandleAction(void* userData,
                                  struct wl_data_offer* offer,
                                  uint32_t dnd_action)
{
    // v3+ only. No-op: we don't branch behavior on the negotiated action.
    (void)userData; (void)offer; (void)dnd_action;
}

static const struct wl_data_offer_listener dataOfferListener =
{
    dataOfferHandleOffer,
    dataOfferHandleSourceActions,
    dataOfferHandleAction,
};

static void dataDeviceHandleDataOffer(void* userData,
                                      struct wl_data_device* device,
                                      struct wl_data_offer* offer)
{
    _GLFWofferWayland* offers =
        _glfw_realloc(_glfw.wl.offers,
                      sizeof(_GLFWofferWayland) * (_glfw.wl.offerCount + 1));
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

    unsigned int i;

    for (i = 0; i < _glfw.wl.offerCount; i++)
    {
        if (_glfw.wl.offers[i].offer == offer)
            break;
    }

    if (i == _glfw.wl.offerCount)
    {
        return;
    }

    if (surface && wl_proxy_get_tag((struct wl_proxy*) surface) == &_glfw.wl.tag)
    {
        _GLFWwindow* window = wl_surface_get_user_data(surface);
        if (window->wl.surface == surface)
        {
            if (_glfw.wl.offers[i].text_uri_list)
            {
                _glfw.wl.dragOffer = offer;
                _glfw.wl.dragFocus = window;
                _glfw.wl.dragSerial = serial;

                wl_data_offer_accept(offer, serial, "text/uri-list");
            }
            else if (_glfw.wl.offers[i].glfw_window_drag)
            {
                _glfw.wl.dragOffer = offer;
                _glfw.wl.dragFocus = window;
                _glfw.wl.dragSerial = serial;

                wl_data_offer_accept(offer, serial, GLFW_WAYLAND_WINDOW_DRAG_MIME);
                if (wl_data_offer_get_version(offer) >= 3)
                {
                    wl_data_offer_set_actions(offer,
                        WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
                        WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE);
                }

                const double xpos = wl_fixed_to_double(x);
                const double ypos = wl_fixed_to_double(y);
                _glfwInputCursorPos(window, xpos, ypos);
            }
        }
    }

    if (!_glfw.wl.dragOffer)
    {
        wl_data_offer_accept(offer, serial, NULL);
        wl_data_offer_destroy(offer);
    }

    _glfw.wl.offers[i] = _glfw.wl.offers[_glfw.wl.offerCount - 1];
    _glfw.wl.offerCount--;
}

static void dataDeviceHandleLeave(void* userData,
                                  struct wl_data_device* device)
{
    // Park cursor off-screen during drag leave so the app keeps the drag
    // alive. Invalidate virtualCursorPos so the next enter isn't deduped.
    if (_glfw.wl.toplevelDragSession.source && _glfw.wl.dragFocus)
    {
        _GLFWwindow* focus = _glfw.wl.dragFocus;
        focus->virtualCursorPosX = -DBL_MAX;
        focus->virtualCursorPosY = -DBL_MAX;
        _glfwInputCursorPos(focus, -100000.0, -100000.0);
    }

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
    // wl_pointer.motion is captured during drag-and-drop; forward as cursor events.
    if (_glfw.wl.dragFocus && _glfw.wl.toplevelDragSession.source)
    {
        const double xpos = wl_fixed_to_double(x);
        const double ypos = wl_fixed_to_double(y);
        _glfwInputCursorPos(_glfw.wl.dragFocus, xpos, ypos);

        _GLFWwindow* dragged = _glfw.wl.toplevelDragSession.window;
        if (dragged)
        {
            int newX = _glfw.wl.dragFocus->wl.pendingPosX
                     + (int) lround(xpos) - _glfw.wl.toplevelDragSession.offsetX;
            int newY = _glfw.wl.dragFocus->wl.pendingPosY
                     + (int) lround(ypos) - _glfw.wl.toplevelDragSession.offsetY;
            if (newX != dragged->wl.pendingPosX ||
                newY != dragged->wl.pendingPosY)
            {
                dragged->wl.pendingPosX = newX;
                dragged->wl.pendingPosY = newY;
                dragged->wl.pendingPosSet = GLFW_TRUE;
                _glfwInputWindowPos(dragged, newX, newY);
            }
        }
    }
}

static void dataDeviceHandleDrop(void* userData,
                                 struct wl_data_device* device)
{
    if (!_glfw.wl.dragOffer)
        return;

    // No payload for our own toplevel drag; just complete the protocol so
    // the compositor stops routing input to drag-and-drop handlers.
    if (_glfw.wl.toplevelDragSession.source)
    {
        // wl_data_offer.finish and set_actions are v3+. Calling either on
        // an older offer raises a protocol error (which aborts the client).
        if (wl_data_offer_get_version(_glfw.wl.dragOffer) >= 3)
            wl_data_offer_finish(_glfw.wl.dragOffer);
        wl_data_offer_destroy(_glfw.wl.dragOffer);
        _glfw.wl.dragOffer = NULL;
        _glfw.wl.dragFocus = NULL;
        return;
    }

    char* string = readDataOfferAsString(_glfw.wl.dragOffer, "text/uri-list");
    if (string)
    {
        int count;
        char** paths = _glfwParseUriList(string, &count);
        if (paths)
        {
            _glfwInputDrop(_glfw.wl.dragFocus, count, (const char**) paths);

            for (int i = 0; i < count; i++)
                _glfw_free(paths[i]);

            _glfw_free(paths);
        }

        _glfw_free(string);
    }
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

static void xdgActivationHandleDone(void* userData,
                                    struct xdg_activation_token_v1* activationToken,
                                    const char* token)
{
    _GLFWwindow* window = userData;

    if (activationToken != window->wl.activationToken)
        return;

    xdg_activation_v1_activate(_glfw.wl.activationManager, token, window->wl.surface);
    xdg_activation_token_v1_destroy(window->wl.activationToken);
    window->wl.activationToken = NULL;
}

static const struct xdg_activation_token_v1_listener xdgActivationListener =
{
    xdgActivationHandleDone
};

static void callbackHandleFrame(void* userData, struct wl_callback* callback, uint32_t data)
{
    _GLFWwindow* window = userData;
    wl_callback_destroy(callback);
    window->wl.egl.callback = NULL;
}

static const struct wl_callback_listener frameCallbackListener =
{
    callbackHandleFrame
};

void _glfwAddSeatListenerWayland(struct wl_seat* seat)
{
    wl_seat_add_listener(seat, &seatListener, NULL);
}

void _glfwAddDataDeviceListenerWayland(struct wl_data_device* device)
{
    wl_data_device_add_listener(device, &dataDeviceListener, NULL);
}

GLFWbool _glfwWaitForEGLFrameWayland(_GLFWwindow* window)
{
    double timeout = 0.02;

    while (window->wl.egl.callback)
    {
        while (wl_display_prepare_read_queue(_glfw.wl.display, window->wl.egl.queue) != 0)
            wl_display_dispatch_queue_pending(_glfw.wl.display, window->wl.egl.queue);

        if (!flushDisplay())
        {
            wl_display_cancel_read(_glfw.wl.display);
            return GLFW_FALSE;
        }

        struct pollfd fd = { wl_display_get_fd(_glfw.wl.display), POLLIN };

        if (!_glfwPollPOSIX(&fd, 1, &timeout))
        {
            wl_display_cancel_read(_glfw.wl.display);
            return GLFW_FALSE;
        }

        wl_display_read_events(_glfw.wl.display);
        wl_display_dispatch_queue_pending(_glfw.wl.display, window->wl.egl.queue);
    }

    window->wl.egl.callback = wl_surface_frame(window->wl.egl.wrapper);
    wl_callback_add_listener(window->wl.egl.callback, &frameCallbackListener, window);

    // If the window is hidden when the wait is over then don't swap
    return window->wl.visible;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwCreateWindowWayland(_GLFWwindow* window,
                                  const _GLFWwndconfig* wndconfig,
                                  const _GLFWctxconfig* ctxconfig,
                                  const _GLFWfbconfig* fbconfig)
{
    window->wl.createdUnfocused = !wndconfig->focused;

    if (!createNativeSurface(window, wndconfig, fbconfig))
        return GLFW_FALSE;

    if (ctxconfig->client != GLFW_NO_API)
    {
        if (ctxconfig->source == GLFW_EGL_CONTEXT_API ||
            ctxconfig->source == GLFW_NATIVE_CONTEXT_API)
        {
            window->wl.egl.window = wl_egl_window_create(window->wl.surface,
                                                         window->wl.fbWidth,
                                                         window->wl.fbHeight);
            if (!window->wl.egl.window)
            {
                _glfwInputError(GLFW_PLATFORM_ERROR,
                                "Wayland: Failed to create EGL window");
                return GLFW_FALSE;
            }

            window->wl.egl.queue = wl_display_create_queue(_glfw.wl.display);
            if (!window->wl.egl.queue)
            {
                _glfwInputError(GLFW_PLATFORM_ERROR,
                                "Wayland: Failed to create EGL frame queue");
                return GLFW_FALSE;
            }

            window->wl.egl.wrapper = wl_proxy_create_wrapper(window->wl.surface);
            if (!window->wl.egl.wrapper)
            {
                _glfwInputError(GLFW_PLATFORM_ERROR,
                                "Wayland: Failed to create surface wrapper");
                return GLFW_FALSE;
            }

            wl_proxy_set_queue((struct wl_proxy*) window->wl.egl.wrapper,
                               window->wl.egl.queue);

            window->wl.egl.interval = 1;

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

    if (window->monitor || wndconfig->visible)
    {
        if (!createShellObjects(window))
            return GLFW_FALSE;
    }

    return GLFW_TRUE;
}

void _glfwDestroyWindowWayland(_GLFWwindow* window)
{
    // Detach from any active drag session so end-of-drag callbacks don't
    // touch a freed window.
    if (_glfw.wl.toplevelDragSession.window == window)
        _glfw.wl.toplevelDragSession.window = NULL;
    if (_glfw.wl.dragFocus == window)
        _glfw.wl.dragFocus = NULL;

    if (window->wl.surface == _glfw.wl.pointerSurface)
        _glfw.wl.pointerSurface = NULL;

    if (window == _glfw.wl.keyboardFocus)
    {
        struct itimerspec timer = {0};
        timerfd_settime(_glfw.wl.keyRepeatTimerfd, 0, &timer, NULL);

        _glfw.wl.keyboardFocus = NULL;
    }

    if (window->wl.fractionalScale)
        wp_fractional_scale_v1_destroy(window->wl.fractionalScale);

    if (window->wl.scalingViewport)
        wp_viewport_destroy(window->wl.scalingViewport);

    if (window->wl.activationToken)
        xdg_activation_token_v1_destroy(window->wl.activationToken);

    if (window->wl.idleInhibitor)
        zwp_idle_inhibitor_v1_destroy(window->wl.idleInhibitor);

    if (window->wl.relativePointer)
        zwp_relative_pointer_v1_destroy(window->wl.relativePointer);

    if (window->wl.lockedPointer)
        zwp_locked_pointer_v1_destroy(window->wl.lockedPointer);

    if (window->wl.confinedPointer)
        zwp_confined_pointer_v1_destroy(window->wl.confinedPointer);

    if (window->context.destroy)
        window->context.destroy(window);

    destroyShellObjects(window);

    if (window->wl.fallback.buffer)
        wl_buffer_destroy(window->wl.fallback.buffer);

    if (window->wl.egl.callback)
        wl_callback_destroy(window->wl.egl.callback);

    if (window->wl.egl.wrapper)
        wl_proxy_wrapper_destroy(window->wl.egl.wrapper);

    if (window->wl.egl.queue)
        wl_event_queue_destroy(window->wl.egl.queue);

    if (window->wl.egl.window)
        wl_egl_window_destroy(window->wl.egl.window);

    if (window->wl.surface)
        wl_surface_destroy(window->wl.surface);

    _glfw_free(window->wl.appId);
    _glfw_free(window->wl.outputScales);
}

void _glfwSetWindowTitleWayland(_GLFWwindow* window, const char* title)
{
    if (window->wl.libdecor.frame)
        libdecor_frame_set_title(window->wl.libdecor.frame, title);
    else if (window->wl.xdg.toplevel)
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
    // Popups have a parent-relative position; toplevels don't.
    if (window->wl.pendingPosSet)
    {
        if (xpos) *xpos = window->wl.pendingPosX;
        if (ypos) *ypos = window->wl.pendingPosY;
    }
}

void _glfwSetWindowPosWayland(_GLFWwindow* window, int xpos, int ypos)
{
    // Can't move a mapped toplevel on Wayland, but callers rely on
    // write/read consistency. Also used as the xdg_popup anchor origin.
    window->wl.pendingPosX = xpos;
    window->wl.pendingPosY = ypos;
    window->wl.pendingPosSet = GLFW_TRUE;
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
    if (window->monitor)
    {
        // Video mode setting is not available on Wayland
    }
    else
    {
        if (!resizeWindow(window, width, height))
            return;

        if (window->wl.libdecor.frame)
        {
            struct libdecor_state* frameState =
                libdecor_state_new(window->wl.width, window->wl.height);
            libdecor_frame_commit(window->wl.libdecor.frame, frameState, NULL);
            libdecor_state_free(frameState);
        }

        if (window->wl.visible)
            _glfwInputWindowDamage(window);
    }
}

void _glfwSetWindowSizeLimitsWayland(_GLFWwindow* window,
                                     int minwidth, int minheight,
                                     int maxwidth, int maxheight)
{
    if (window->wl.libdecor.frame)
    {
        if (minwidth == GLFW_DONT_CARE || minheight == GLFW_DONT_CARE)
            minwidth = minheight = 0;

        if (maxwidth == GLFW_DONT_CARE || maxheight == GLFW_DONT_CARE)
            maxwidth = maxheight = 0;

        libdecor_frame_set_min_content_size(window->wl.libdecor.frame,
                                            minwidth, minheight);
        libdecor_frame_set_max_content_size(window->wl.libdecor.frame,
                                            maxwidth, maxheight);
    }
    else if (window->wl.xdg.toplevel)
        updateXdgSizeLimits(window);
}

void _glfwSetWindowAspectRatioWayland(_GLFWwindow* window, int numer, int denom)
{
    if (window->wl.maximized || window->wl.fullscreen)
        return;

    int width = window->wl.width, height = window->wl.height;

    if (numer != GLFW_DONT_CARE && denom != GLFW_DONT_CARE)
    {
        const float aspectRatio = (float) width / (float) height;
        const float targetRatio = (float) numer / (float) denom;
        if (aspectRatio < targetRatio)
            height /= targetRatio;
        else if (aspectRatio > targetRatio)
            width *= targetRatio;
    }

    if (resizeWindow(window, width, height))
    {
        if (window->wl.libdecor.frame)
        {
            struct libdecor_state* frameState =
                libdecor_state_new(window->wl.width, window->wl.height);
            libdecor_frame_commit(window->wl.libdecor.frame, frameState, NULL);
            libdecor_state_free(frameState);
        }

        _glfwInputWindowSize(window, window->wl.width, window->wl.height);

        if (window->wl.visible)
            _glfwInputWindowDamage(window);
    }
}

void _glfwGetFramebufferSizeWayland(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->wl.fbWidth;
    if (height)
        *height = window->wl.fbHeight;
}

void _glfwGetWindowFrameSizeWayland(_GLFWwindow* window,
                                    int* left, int* top,
                                    int* right, int* bottom)
{
    if (window->wl.fallback.decorations)
    {
        if (top)
            *top = GLFW_CAPTION_HEIGHT;
        if (left)
            *left = GLFW_BORDER_SIZE;
        if (right)
            *right = GLFW_BORDER_SIZE;
        if (bottom)
            *bottom = GLFW_BORDER_SIZE;
    }
}

void _glfwGetWindowContentScaleWayland(_GLFWwindow* window,
                                       float* xscale, float* yscale)
{
    if (window->wl.fractionalScale)
    {
        if (xscale)
            *xscale = (float) window->wl.scalingNumerator / 120.f;
        if (yscale)
            *yscale = (float) window->wl.scalingNumerator / 120.f;
    }
    else
    {
        if (xscale)
            *xscale = (float) window->wl.bufferScale;
        if (yscale)
            *yscale = (float) window->wl.bufferScale;
    }
}

void _glfwIconifyWindowWayland(_GLFWwindow* window)
{
    if (window->wl.libdecor.frame)
        libdecor_frame_set_minimized(window->wl.libdecor.frame);
    else if (window->wl.xdg.toplevel)
        xdg_toplevel_set_minimized(window->wl.xdg.toplevel);
}

void _glfwRestoreWindowWayland(_GLFWwindow* window)
{
    if (window->monitor)
    {
        // There is no way to unset minimized, or even to know if we are
        // minimized, so there is nothing to do in this case.
    }
    else
    {
        // We assume we are not minimized and act only on maximization

        if (window->wl.maximized)
        {
            if (window->wl.libdecor.frame)
                libdecor_frame_unset_maximized(window->wl.libdecor.frame);
            else if (window->wl.xdg.toplevel)
                xdg_toplevel_unset_maximized(window->wl.xdg.toplevel);
            else
                window->wl.maximized = GLFW_FALSE;
        }
    }
}

void _glfwMaximizeWindowWayland(_GLFWwindow* window)
{
    if (window->wl.libdecor.frame)
        libdecor_frame_set_maximized(window->wl.libdecor.frame);
    else if (window->wl.xdg.toplevel)
        xdg_toplevel_set_maximized(window->wl.xdg.toplevel);
    else
        window->wl.maximized = GLFW_TRUE;
}

void _glfwShowWindowWayland(_GLFWwindow* window)
{
    if (!window->wl.libdecor.frame && !window->wl.xdg.toplevel && !window->wl.xdg.popup)
    {
        // NOTE: The XDG surface and role are created here so command-line applications
        //       with off-screen windows do not appear in for example the Unity dock
        createShellObjects(window);
    }
}

void _glfwHideWindowWayland(_GLFWwindow* window)
{
    if (window->wl.visible)
    {
        window->wl.visible = GLFW_FALSE;
        destroyShellObjects(window);

        wl_surface_attach(window->wl.surface, NULL, 0, 0);
        wl_surface_commit(window->wl.surface);
    }
}

void _glfwRequestWindowAttentionWayland(_GLFWwindow* window)
{
    if (!_glfw.wl.activationManager)
        return;

    // We're about to overwrite this with a new request
    if (window->wl.activationToken)
        xdg_activation_token_v1_destroy(window->wl.activationToken);

    window->wl.activationToken =
        xdg_activation_v1_get_activation_token(_glfw.wl.activationManager);
    xdg_activation_token_v1_add_listener(window->wl.activationToken,
                                         &xdgActivationListener,
                                         window);

    xdg_activation_token_v1_commit(window->wl.activationToken);
}

void _glfwFocusWindowWayland(_GLFWwindow* window)
{
    if (!_glfw.wl.activationManager)
        return;

    if (window->wl.activationToken)
        xdg_activation_token_v1_destroy(window->wl.activationToken);

    window->wl.activationToken =
        xdg_activation_v1_get_activation_token(_glfw.wl.activationManager);
    xdg_activation_token_v1_add_listener(window->wl.activationToken,
                                         &xdgActivationListener,
                                         window);

    xdg_activation_token_v1_set_serial(window->wl.activationToken,
                                       _glfw.wl.serial,
                                       _glfw.wl.seat);

    _GLFWwindow* requester = _glfw.wl.keyboardFocus;
    if (requester)
    {
        xdg_activation_token_v1_set_surface(window->wl.activationToken,
                                            requester->wl.surface);

        if (requester->wl.appId)
        {
            xdg_activation_token_v1_set_app_id(window->wl.activationToken,
                                               requester->wl.appId);
        }
    }

    xdg_activation_token_v1_commit(window->wl.activationToken);
}

static void endToplevelDragSession(void)
{
    _GLFWwindow* window = _glfw.wl.toplevelDragSession.window;

    if (_glfw.wl.toplevelDragSession.drag)
    {
        xdg_toplevel_drag_v1_destroy(_glfw.wl.toplevelDragSession.drag);
        _glfw.wl.toplevelDragSession.drag = NULL;
    }
    if (_glfw.wl.toplevelDragSession.source)
    {
        wl_data_source_destroy(_glfw.wl.toplevelDragSession.source);
        _glfw.wl.toplevelDragSession.source = NULL;
    }
    _glfw.wl.toplevelDragSession.window = NULL;

    // The compositor consumed the real release; account for it and synthesize
    // one so the application sees "drag ended".
    if (_glfw.wl.pointerButtonsDown > 0)
        _glfw.wl.pointerButtonsDown--;
    if (window)
        _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
}

static void dragSourceHandleTarget(void* data,
                                   struct wl_data_source* source,
                                   const char* mime) { (void)data; (void)source; (void)mime; }

static void dragSourceHandleSend(void* data, struct wl_data_source* source,
                                 const char* mime, int32_t fd)
{
    // No payload is transferred — this data source exists only to carry the
    // xdg_toplevel_drag attachment. Close the fd so the reader doesn't hang.
    (void)data; (void)source; (void)mime;
    close(fd);
}

static void dragSourceHandleCancelled(void* data, struct wl_data_source* source)
{
    (void)data; (void)source;
    endToplevelDragSession();
}

static void dragSourceHandleDndDropPerformed(void* data, struct wl_data_source* source)
{
    (void)data; (void)source;
    // Clean up immediately; dnd_finished may not fire if the drop landed
    // on a foreign client. endToplevelDragSession is idempotent.
    endToplevelDragSession();
}

static void dragSourceHandleDndFinished(void* data, struct wl_data_source* source)
{
    (void)data; (void)source;
    endToplevelDragSession();
}

static void dragSourceHandleAction(void* data, struct wl_data_source* source,
                                   uint32_t action) { (void)data; (void)source; (void)action; }

static const struct wl_data_source_listener dragSourceListener =
{
    dragSourceHandleTarget,
    dragSourceHandleSend,
    dragSourceHandleCancelled,
    dragSourceHandleDndDropPerformed,
    dragSourceHandleDndFinished,
    dragSourceHandleAction,
};

// Start a toplevel drag via xdg_toplevel_drag_v1 so the compositor delivers
// drag-and-drop events for drop-target hit-testing. Falls back to xdg_toplevel.move.
static GLFWbool startToplevelDragSession(_GLFWwindow* window, uint32_t serial)
{
    if (!_glfw.wl.toplevelDragManager ||
        !_glfw.wl.dataDeviceManager ||
        !_glfw.wl.dataDevice)
    {
        return GLFW_FALSE;
    }

    struct xdg_toplevel* toplevel = window->wl.xdg.toplevel;
    if (!toplevel && window->wl.libdecor.frame)
        toplevel = libdecor_frame_get_xdg_toplevel(window->wl.libdecor.frame);
    if (!toplevel)
        return GLFW_FALSE;

    // One session at a time. If we're already in a drag, end it first.
    if (_glfw.wl.toplevelDragSession.source)
        endToplevelDragSession();

    struct wl_data_source* source =
        wl_data_device_manager_create_data_source(_glfw.wl.dataDeviceManager);
    if (!source)
        return GLFW_FALSE;
    wl_data_source_add_listener(source, &dragSourceListener, NULL);
    wl_data_source_offer(source, GLFW_WAYLAND_WINDOW_DRAG_MIME);

    // Without a negotiated action, v3+ compositors won't deliver
    // dnd_drop_performed / dnd_finished.
    if (wl_data_source_get_version(source) >= 3)
    {
        wl_data_source_set_actions(source,
            WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE);
    }

    struct xdg_toplevel_drag_v1* drag =
        xdg_toplevel_drag_manager_v1_get_xdg_toplevel_drag(
            _glfw.wl.toplevelDragManager, source);
    if (!drag)
    {
        wl_data_source_destroy(source);
        return GLFW_FALSE;
    }

    _glfw.wl.toplevelDragSession.source = source;
    _glfw.wl.toplevelDragSession.drag = drag;
    _glfw.wl.toplevelDragSession.window = window;

    // attach() must precede start_drag() for already-mapped toplevels.
    // The offset is where the cursor sits relative to the window's
    // top-left. For cross-window drags (tab tear-out), use the origin
    // window's live cursor so the offset matches ImGui's click offset.
    double rawOffsetX, rawOffsetY;
    if (_glfw.wl.pointerButtonSurface == window->wl.surface)
    {
        rawOffsetX = _glfw.wl.pointerButtonX;
        rawOffsetY = _glfw.wl.pointerButtonY;
    }
    else
    {
        _GLFWwindow* origin = wl_surface_get_user_data(
            _glfw.wl.pointerButtonSurface);
        rawOffsetX = origin->wl.cursorPosX - window->wl.pendingPosX;
        rawOffsetY = origin->wl.cursorPosY - window->wl.pendingPosY;
    }
    int offsetX = (int) lround(rawOffsetX);
    int offsetY = (int) lround(rawOffsetY);
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;
    _glfw.wl.toplevelDragSession.offsetX = offsetX;
    _glfw.wl.toplevelDragSession.offsetY = offsetY;
    xdg_toplevel_drag_v1_attach(drag, toplevel, offsetX, offsetY);

    struct wl_surface* origin = _glfw.wl.pointerButtonSurface;
    if (!origin)
        origin = window->wl.surface;
    wl_data_device_start_drag(_glfw.wl.dataDevice, source,
                              origin, NULL, serial);
    return GLFW_TRUE;
}

void _glfwDragWindowWayland(_GLFWwindow* window)
{
    if (!_glfw.wl.seat || !_glfw.wl.pointerButtonSerial)
        return;

    // Already dragging this window.
    if (_glfw.wl.toplevelDragSession.source &&
        _glfw.wl.toplevelDragSession.window == window)
    {
        return;
    }

    // Defer until mapped; dragging an unmapped surface crashes KWin.
    if (!window->wl.mapped)
    {
        window->wl.dragPendingSerial = _glfw.wl.pointerButtonSerial;
        window->wl.dragPending = GLFW_TRUE;
        return;
    }

    // Preferred: xdg_toplevel_drag_v1 delivers drag-and-drop events for hit-testing.
    if (startToplevelDragSession(window, _glfw.wl.pointerButtonSerial))
        return;

    // Fallback for compositors that don't advertise the manager global.
    struct xdg_toplevel* toplevel = window->wl.xdg.toplevel;
    if (!toplevel && window->wl.libdecor.frame)
        toplevel = libdecor_frame_get_xdg_toplevel(window->wl.libdecor.frame);
    if (!toplevel)
        return;

    xdg_toplevel_move(toplevel, _glfw.wl.seat, _glfw.wl.pointerButtonSerial);

    // xdg_toplevel.move swallows the release; synthesize one.
    if (_glfw.wl.pointerButtonsDown > 0)
        _glfw.wl.pointerButtonsDown--;
    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
}

void _glfwSetWindowMonitorWayland(_GLFWwindow* window,
                                  _GLFWmonitor* monitor,
                                  int xpos, int ypos,
                                  int width, int height,
                                  int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (!monitor)
            _glfwSetWindowSizeWayland(window, width, height);

        return;
    }

    if (window->monitor)
        releaseMonitor(window);

    _glfwInputWindowMonitor(window, monitor);

    if (window->monitor)
        acquireMonitor(window);
    else
        _glfwSetWindowSizeWayland(window, width, height);
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
    return window->wl.surface == _glfw.wl.pointerSurface;
}

GLFWbool _glfwFramebufferTransparentWayland(_GLFWwindow* window)
{
    return window->wl.transparent;
}

void _glfwSetWindowResizableWayland(_GLFWwindow* window, GLFWbool enabled)
{
    if (window->wl.libdecor.frame)
    {
        if (enabled)
        {
            libdecor_frame_set_capabilities(window->wl.libdecor.frame,
                                            LIBDECOR_ACTION_RESIZE);
        }
        else
        {
            libdecor_frame_unset_capabilities(window->wl.libdecor.frame,
                                              LIBDECOR_ACTION_RESIZE);
        }
    }
    else if (window->wl.xdg.toplevel)
        updateXdgSizeLimits(window);
}

void _glfwSetWindowDecoratedWayland(_GLFWwindow* window, GLFWbool enabled)
{
    if (window->wl.libdecor.frame)
    {
        libdecor_frame_set_visibility(window->wl.libdecor.frame, enabled);
    }
    else if (window->wl.xdg.decoration)
    {
        uint32_t mode;

        if (enabled)
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
        else
            mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;

        zxdg_toplevel_decoration_v1_set_mode(window->wl.xdg.decoration, mode);
    }
    else if (window->wl.xdg.toplevel)
    {
        if (enabled)
            createFallbackDecorations(window);
        else
            destroyFallbackDecorations(window);
    }
}

void _glfwSetWindowTitlebarWayland(_GLFWwindow* window, GLFWbool enabled)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Wayland: Window titlebar attribute setting not implemented yet");
}

void _glfwSetWindowFloatingWayland(_GLFWwindow* window, GLFWbool enabled)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE,
                    "Wayland: Platform does not support making a window floating");
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
        wl_surface_set_input_region(window->wl.surface, NULL);
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

void _glfwSetCursorPosWayland(_GLFWwindow* window, double x, double y)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE,
                    "Wayland: The platform does not support setting the cursor position");
}

void _glfwSetCursorModeWayland(_GLFWwindow* window, int mode)
{
    _glfwSetCursorWayland(window, window->cursor);
}

const char* _glfwGetScancodeNameWayland(int scancode)
{
    if (scancode < 0 || scancode > 255)
    {
        _glfwInputError(GLFW_INVALID_VALUE,
                        "Wayland: Invalid scancode %i",
                        scancode);
        return NULL;
    }

    const int key = _glfw.wl.keycodes[scancode];
    if (key == GLFW_KEY_UNKNOWN)
        return NULL;

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

    // WORKAROUND: xkb_keysym_to_utf8() requires the third parameter (size of the output buffer)
    // to be at least 7 (6 bytes + a null terminator), because it was written when UTF-8
    // sequences could be up to 6 bytes long. The _glfw.wl.keynames buffers are only 5 bytes
    // long, because UTF-8 sequences are now limited to 4 bytes and no codepoints were ever assigned
    // that needed more than that. To work around this, we first copy to a temporary buffer.
    //
    // See: https://github.com/xkbcommon/libxkbcommon/issues/418
    char temp_buffer[7];
    const int bytes_written = xkb_keysym_to_utf8(keysyms[0], temp_buffer, sizeof(temp_buffer));
    if (bytes_written <= 0 || bytes_written > 5)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to encode keysym as UTF-8");
        return NULL;
    }
    memcpy(_glfw.wl.keynames[key], temp_buffer, bytes_written);

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
    const _GLFWcursorDesc* desc = findCursorDescByShape(shape);
    if (!desc)
    {
        _glfwInputError(GLFW_CURSOR_UNAVAILABLE,
                        "Wayland: Standard cursor shape unavailable");
        return GLFW_FALSE;
    }

    _GLFWcursorWayland cw = lookupNamedCursor(desc->name, desc->fallback);
    if (!cw.cursor && !_glfw.wl.cursorShapeManager)
    {
        _glfwInputError(GLFW_CURSOR_UNAVAILABLE,
                        "Wayland: Failed to create standard cursor \"%s\"",
                        desc->name);
        return GLFW_FALSE;
    }

    cursor->wl.cursor = cw.cursor;
    cursor->wl.cursorHiDPI = cw.cursorHiDPI;
    cursor->wl.cursorShape = desc->shape;
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
    if (!_glfw.wl.relativePointerManager)
    {
        _glfwInputError(GLFW_FEATURE_UNAVAILABLE,
                        "Wayland: The compositor does not support relative pointer motion");
        return;
    }

    if (!_glfw.wl.pointerConstraints)
    {
        _glfwInputError(GLFW_FEATURE_UNAVAILABLE,
                        "Wayland: The compositor does not support locking the pointer");
    }

    window->wl.relativePointer =
        zwp_relative_pointer_manager_v1_get_relative_pointer(
            _glfw.wl.relativePointerManager,
            _glfw.wl.pointer);
    zwp_relative_pointer_v1_add_listener(window->wl.relativePointer,
                                         &relativePointerListener,
                                         window);

    window->wl.lockedPointer =
        zwp_pointer_constraints_v1_lock_pointer(
            _glfw.wl.pointerConstraints,
            window->wl.surface,
            _glfw.wl.pointer,
            NULL,
            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
    zwp_locked_pointer_v1_add_listener(window->wl.lockedPointer,
                                       &lockedPointerListener,
                                       window);
}

static void unlockPointer(_GLFWwindow* window)
{
    zwp_relative_pointer_v1_destroy(window->wl.relativePointer);
    window->wl.relativePointer = NULL;

    zwp_locked_pointer_v1_destroy(window->wl.lockedPointer);
    window->wl.lockedPointer = NULL;
}

static void confinedPointerHandleConfined(void* userData,
                                          struct zwp_confined_pointer_v1* confinedPointer)
{
}

static void confinedPointerHandleUnconfined(void* userData,
                                            struct zwp_confined_pointer_v1* confinedPointer)
{
}

static const struct zwp_confined_pointer_v1_listener confinedPointerListener =
{
    confinedPointerHandleConfined,
    confinedPointerHandleUnconfined
};

static void confinePointer(_GLFWwindow* window)
{
    if (!_glfw.wl.pointerConstraints)
    {
        _glfwInputError(GLFW_FEATURE_UNAVAILABLE,
                        "Wayland: The compositor does not support confining the pointer");
    }

    window->wl.confinedPointer =
        zwp_pointer_constraints_v1_confine_pointer(
            _glfw.wl.pointerConstraints,
            window->wl.surface,
            _glfw.wl.pointer,
            NULL,
            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);

    zwp_confined_pointer_v1_add_listener(window->wl.confinedPointer,
                                         &confinedPointerListener,
                                         window);
}

static void unconfinePointer(_GLFWwindow* window)
{
    zwp_confined_pointer_v1_destroy(window->wl.confinedPointer);
    window->wl.confinedPointer = NULL;
}

void _glfwSetCursorWayland(_GLFWwindow* window, _GLFWcursor* cursor)
{
    if (!_glfw.wl.pointer)
        return;

    if (window->wl.surface != _glfw.wl.pointerSurface)
        return;

    // Update pointer lock to match cursor mode
    if (window->cursorMode == GLFW_CURSOR_DISABLED)
    {
        if (window->wl.confinedPointer)
            unconfinePointer(window);
        if (!window->wl.lockedPointer)
            lockPointer(window);
    }
    else if (window->cursorMode == GLFW_CURSOR_CAPTURED)
    {
        if (window->wl.lockedPointer)
            unlockPointer(window);
        if (!window->wl.confinedPointer)
            confinePointer(window);
    }
    else if (window->cursorMode == GLFW_CURSOR_NORMAL ||
             window->cursorMode == GLFW_CURSOR_HIDDEN)
    {
        if (window->wl.lockedPointer)
            unlockPointer(window);
        else if (window->wl.confinedPointer)
            unconfinePointer(window);
    }

    applyCursor(window);
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
    return window->wl.egl.window;
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
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    if (_glfw.platform.platformID != GLFW_PLATFORM_WAYLAND)
    {
        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE,
                        "Wayland: Platform not initialized");
        return NULL;
    }

    _GLFWwindow* window = (_GLFWwindow*) handle;
    assert(window != NULL);

    return window->wl.surface;
}

#endif // _GLFW_WAYLAND

