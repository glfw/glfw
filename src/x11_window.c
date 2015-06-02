//========================================================================
// GLFW 3.1 X11 - www.glfw.org
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

#include "internal.h"

#include <X11/cursorfont.h>
#include <X11/Xmd.h>

#include <sys/select.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Action for EWMH client messages
#define _NET_WM_STATE_REMOVE        0
#define _NET_WM_STATE_ADD           1
#define _NET_WM_STATE_TOGGLE        2

// Additional mouse button names for XButtonEvent
#define Button6            6
#define Button7            7

typedef struct
{
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long input_mode;
    unsigned long status;
} MotifWmHints;

#define MWM_HINTS_DECORATIONS (1L << 1)


// Wait for data to arrive
//
void selectDisplayConnection(struct timeval* timeout)
{
    fd_set fds;
    const int fd = ConnectionNumber(_glfw.x11.display);

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    // select(1) is used instead of an X function like XNextEvent, as the
    // wait inside those are guarded by the mutex protecting the display
    // struct, locking out other threads from using X (including GLX)
    select(fd + 1, &fds, NULL, NULL, timeout);
}

// Returns whether the window is iconified
//
static int getWindowState(_GLFWwindow* window)
{
    int result = WithdrawnState;
    struct {
        CARD32 state;
        Window icon;
    } *state = NULL;

    if (_glfwGetWindowProperty(window->x11.handle,
                               _glfw.x11.WM_STATE,
                               _glfw.x11.WM_STATE,
                               (unsigned char**) &state) >= 2)
    {
        result = state->state;
    }

    XFree(state);
    return result;
}

// Returns whether the event is a selection event
//
static Bool isFrameExtentsEvent(Display* display, XEvent* event, XPointer pointer)
{
    _GLFWwindow* window = (_GLFWwindow*) pointer;
    return event->type == PropertyNotify &&
           event->xproperty.state == PropertyNewValue &&
           event->xproperty.window == window->x11.handle &&
           event->xproperty.atom == _glfw.x11.NET_FRAME_EXTENTS;
}

// Translates a GLFW standard cursor to a font cursor shape
//
static int translateCursorShape(int shape)
{
    switch (shape)
    {
        case GLFW_ARROW_CURSOR:
            return XC_arrow;
        case GLFW_IBEAM_CURSOR:
            return XC_xterm;
        case GLFW_CROSSHAIR_CURSOR:
            return XC_crosshair;
        case GLFW_HAND_CURSOR:
            return XC_hand1;
        case GLFW_HRESIZE_CURSOR:
            return XC_sb_h_double_arrow;
        case GLFW_VRESIZE_CURSOR:
            return XC_sb_v_double_arrow;
    }

    return 0;
}

// Translates an X event modifier state mask
//
static int translateState(int state)
{
    int mods = 0;

    if (state & ShiftMask)
        mods |= GLFW_MOD_SHIFT;
    if (state & ControlMask)
        mods |= GLFW_MOD_CONTROL;
    if (state & Mod1Mask)
        mods |= GLFW_MOD_ALT;
    if (state & Mod4Mask)
        mods |= GLFW_MOD_SUPER;

    return mods;
}

// Translates an X Window key to internal coding
//
static int translateKey(int scancode)
{
    // Use the pre-filled LUT (see createKeyTables() in x11_init.c)
    if (scancode < 0 || scancode > 255)
        return GLFW_KEY_UNKNOWN;

    return _glfw.x11.publicKeys[scancode];
}

// Return the GLFW window corresponding to the specified X11 window
//
static _GLFWwindow* findWindowByHandle(Window handle)
{
    _GLFWwindow* window;

    if (XFindContext(_glfw.x11.display,
                     handle,
                     _glfw.x11.context,
                     (XPointer*) &window) != 0)
    {
        return NULL;
    }

    return window;
}

// Sends an EWMH or ICCCM event to the window manager
//
static void sendEventToWM(_GLFWwindow* window, Atom type,
                          long a, long b, long c, long d, long e)
{
    XEvent event;
    memset(&event, 0, sizeof(event));

    event.type = ClientMessage;
    event.xclient.window = window->x11.handle;
    event.xclient.format = 32; // Data is 32-bit longs
    event.xclient.message_type = type;
    event.xclient.data.l[0] = a;
    event.xclient.data.l[1] = b;
    event.xclient.data.l[2] = c;
    event.xclient.data.l[3] = d;
    event.xclient.data.l[4] = e;

    XSendEvent(_glfw.x11.display, _glfw.x11.root,
               False,
               SubstructureNotifyMask | SubstructureRedirectMask,
               &event);
}

// Splits and translates a text/uri-list into separate file paths
// NOTE: This function destroys the provided string
//
static char** parseUriList(char* text, int* count)
{
    const char* prefix = "file://";
    char** paths = NULL;
    char* line;

    *count = 0;

    while ((line = strtok(text, "\r\n")))
    {
        text = NULL;

        if (line[0] == '#')
            continue;

        if (strncmp(line, prefix, strlen(prefix)) == 0)
            line += strlen(prefix);

        (*count)++;

        char* path = calloc(strlen(line) + 1, 1);
        paths = realloc(paths, *count * sizeof(char*));
        paths[*count - 1] = path;

        while (*line)
        {
            if (line[0] == '%' && line[1] && line[2])
            {
                const char digits[3] = { line[1], line[2], '\0' };
                *path = strtol(digits, NULL, 16);
                line += 2;
            }
            else
                *path = *line;

            path++;
            line++;
        }
    }

    return paths;
}

// Create the X11 window (and its colormap)
//
static GLboolean createWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig)
{
    unsigned long wamask;
    XSetWindowAttributes wa;
    XVisualInfo* visual = _GLFW_X11_CONTEXT_VISUAL;

    // Every window needs a colormap
    // Create one based on the visual used by the current context
    // TODO: Decouple this from context creation

    window->x11.colormap = XCreateColormap(_glfw.x11.display,
                                           _glfw.x11.root,
                                           visual->visual,
                                           AllocNone);

    // Create the actual window
    {
        wamask = CWBorderPixel | CWColormap | CWEventMask;

        wa.colormap = window->x11.colormap;
        wa.border_pixel = 0;
        wa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                        PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                        ExposureMask | FocusChangeMask | VisibilityChangeMask |
                        EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

        _glfwGrabXErrorHandler();

        window->x11.handle = XCreateWindow(_glfw.x11.display,
                                           _glfw.x11.root,
                                           0, 0,
                                           wndconfig->width, wndconfig->height,
                                           0,              // Border width
                                           visual->depth,  // Color depth
                                           InputOutput,
                                           visual->visual,
                                           wamask,
                                           &wa);

        _glfwReleaseXErrorHandler();

        if (!window->x11.handle)
        {
            _glfwInputXError(GLFW_PLATFORM_ERROR,
                             "X11: Failed to create window");
            return GL_FALSE;
        }

        if (!wndconfig->decorated)
        {
            MotifWmHints hints;
            hints.flags = MWM_HINTS_DECORATIONS;
            hints.decorations = 0;

            XChangeProperty(_glfw.x11.display, window->x11.handle,
                            _glfw.x11.MOTIF_WM_HINTS,
                            _glfw.x11.MOTIF_WM_HINTS, 32,
                            PropModeReplace,
                            (unsigned char*) &hints,
                            sizeof(MotifWmHints) / sizeof(long));
        }

        XSaveContext(_glfw.x11.display,
                     window->x11.handle,
                     _glfw.x11.context,
                     (XPointer) window);
    }

    if (window->monitor && !_glfw.x11.hasEWMH)
    {
        // This is the butcher's way of removing window decorations
        // Setting the override-redirect attribute on a window makes the window
        // manager ignore the window completely (ICCCM, section 4)
        // The good thing is that this makes undecorated full screen windows
        // easy to do; the bad thing is that we have to do everything manually
        // and some things (like iconify/restore) won't work at all, as those
        // are tasks usually performed by the window manager

        XSetWindowAttributes attributes;
        attributes.override_redirect = True;
        XChangeWindowAttributes(_glfw.x11.display,
                                window->x11.handle,
                                CWOverrideRedirect,
                                &attributes);

        window->x11.overrideRedirect = GL_TRUE;
    }

    // Declare the WM protocols supported by GLFW
    {
        int count = 0;
        Atom protocols[2];

        // The WM_DELETE_WINDOW ICCCM protocol
        // Basic window close notification protocol
        if (_glfw.x11.WM_DELETE_WINDOW)
            protocols[count++] = _glfw.x11.WM_DELETE_WINDOW;

        // The _NET_WM_PING EWMH protocol
        // Tells the WM to ping the GLFW window and flag the application as
        // unresponsive if the WM doesn't get a reply within a few seconds
        if (_glfw.x11.NET_WM_PING)
            protocols[count++] = _glfw.x11.NET_WM_PING;

        if (count > 0)
        {
            XSetWMProtocols(_glfw.x11.display, window->x11.handle,
                            protocols, count);
        }
    }

    if (_glfw.x11.NET_WM_PID)
    {
        const pid_t pid = getpid();

        XChangeProperty(_glfw.x11.display,  window->x11.handle,
                        _glfw.x11.NET_WM_PID, XA_CARDINAL, 32,
                        PropModeReplace,
                        (unsigned char*) &pid, 1);
    }

    // Set ICCCM WM_HINTS property
    {
        XWMHints* hints = XAllocWMHints();
        if (!hints)
        {
            _glfwInputError(GLFW_OUT_OF_MEMORY,
                            "X11: Failed to allocate WM hints");
            return GL_FALSE;
        }

        hints->flags = StateHint;
        hints->initial_state = NormalState;

        XSetWMHints(_glfw.x11.display, window->x11.handle, hints);
        XFree(hints);
    }

    // Set ICCCM WM_NORMAL_HINTS property (even if no parts are set)
    {
        XSizeHints* hints = XAllocSizeHints();
        hints->flags = 0;

        if (wndconfig->monitor)
        {
            hints->flags |= PPosition;
            _glfwPlatformGetMonitorPos(wndconfig->monitor, &hints->x, &hints->y);
        }
        else
        {
            // HACK: Explicitly setting PPosition to any value causes some WMs,
            //       notably Compiz and Metacity, to honor the position of
            //       unmapped windows set by XMoveWindow
            hints->flags |= PPosition;
            hints->x = hints->y = 0;
        }

        if (!wndconfig->resizable)
        {
            hints->flags |= (PMinSize | PMaxSize);
            hints->min_width  = hints->max_width  = wndconfig->width;
            hints->min_height = hints->max_height = wndconfig->height;
        }

        XSetWMNormalHints(_glfw.x11.display, window->x11.handle, hints);
        XFree(hints);
    }

    // Set ICCCM WM_CLASS property
    // HACK: Until a mechanism for specifying the application name is added, the
    //       initial window title is used as the window class name
    if (strlen(wndconfig->title))
    {
        XClassHint* hint = XAllocClassHint();
        hint->res_name = (char*) wndconfig->title;
        hint->res_class = (char*) wndconfig->title;

        XSetClassHint(_glfw.x11.display, window->x11.handle, hint);
        XFree(hint);
    }

#if defined(_GLFW_HAS_XINPUT)
    if (_glfw.x11.xi.available)
    {
        // Select for XInput2 events

        XIEventMask eventmask;
        unsigned char mask[] = { 0 };

        eventmask.deviceid = 2;
        eventmask.mask_len = sizeof(mask);
        eventmask.mask = mask;
        XISetMask(mask, XI_Motion);

        XISelectEvents(_glfw.x11.display, window->x11.handle, &eventmask, 1);
    }
#endif /*_GLFW_HAS_XINPUT*/

    if (_glfw.x11.XdndAware)
    {
        // Announce support for Xdnd (drag and drop)
        const Atom version = 5;
        XChangeProperty(_glfw.x11.display, window->x11.handle,
                        _glfw.x11.XdndAware, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*) &version, 1);
    }

    if (wndconfig->floating && !wndconfig->monitor)
    {
        if (_glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_ABOVE)
        {
            sendEventToWM(window,
                          _glfw.x11.NET_WM_STATE,
                          _NET_WM_STATE_ADD,
                          _glfw.x11.NET_WM_STATE_ABOVE,
                          0, 1, 0);
        }
    }

    _glfwPlatformSetWindowTitle(window, wndconfig->title);

    XRRSelectInput(_glfw.x11.display, window->x11.handle,
                   RRScreenChangeNotifyMask);

    if (_glfw.x11.im)
    {
        window->x11.ic = XCreateIC(_glfw.x11.im,
                                   XNInputStyle,
                                   XIMPreeditNothing | XIMStatusNothing,
                                   XNClientWindow,
                                   window->x11.handle,
                                   XNFocusWindow,
                                   window->x11.handle,
                                   NULL);
    }

    _glfwPlatformGetWindowPos(window, &window->x11.xpos, &window->x11.ypos);
    _glfwPlatformGetWindowSize(window, &window->x11.width, &window->x11.height);

    return GL_TRUE;
}

// Hide the mouse cursor
//
static void hideCursor(_GLFWwindow* window)
{
    XUngrabPointer(_glfw.x11.display, CurrentTime);
    XDefineCursor(_glfw.x11.display, window->x11.handle, _glfw.x11.cursor);
}

// Disable the mouse cursor
//
static void disableCursor(_GLFWwindow* window)
{
    XGrabPointer(_glfw.x11.display, window->x11.handle, True,
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync,
                 window->x11.handle, _glfw.x11.cursor, CurrentTime);
}

// Restores the mouse cursor
//
static void restoreCursor(_GLFWwindow* window)
{
    XUngrabPointer(_glfw.x11.display, CurrentTime);

    if (window->cursor)
    {
        XDefineCursor(_glfw.x11.display, window->x11.handle,
                      window->cursor->x11.handle);
    }
    else
        XUndefineCursor(_glfw.x11.display, window->x11.handle);
}

// Returns whether the event is a selection event
//
static Bool isSelectionEvent(Display* display, XEvent* event, XPointer pointer)
{
    return event->type == SelectionRequest ||
           event->type == SelectionNotify ||
           event->type == SelectionClear;
}

// Set the specified property to the selection converted to the requested target
//
static Atom writeTargetToProperty(const XSelectionRequestEvent* request)
{
    int i;
    const Atom formats[] = { _glfw.x11.UTF8_STRING,
                             _glfw.x11.COMPOUND_STRING,
                             XA_STRING };
    const int formatCount = sizeof(formats) / sizeof(formats[0]);

    if (request->property == None)
    {
        // The requester is a legacy client (ICCCM section 2.2)
        // We don't support legacy clients, so fail here
        return None;
    }

    if (request->target == _glfw.x11.TARGETS)
    {
        // The list of supported targets was requested

        const Atom targets[] = { _glfw.x11.TARGETS,
                                 _glfw.x11.MULTIPLE,
                                 _glfw.x11.UTF8_STRING,
                                 _glfw.x11.COMPOUND_STRING,
                                 XA_STRING };

        XChangeProperty(_glfw.x11.display,
                        request->requestor,
                        request->property,
                        XA_ATOM,
                        32,
                        PropModeReplace,
                        (unsigned char*) targets,
                        sizeof(targets) / sizeof(targets[0]));

        return request->property;
    }

    if (request->target == _glfw.x11.MULTIPLE)
    {
        // Multiple conversions were requested

        Atom* targets;
        unsigned long i, count;

        count = _glfwGetWindowProperty(request->requestor,
                                       request->property,
                                       _glfw.x11.ATOM_PAIR,
                                       (unsigned char**) &targets);

        for (i = 0;  i < count;  i += 2)
        {
            int j;

            for (j = 0;  j < formatCount;  j++)
            {
                if (targets[i] == formats[j])
                    break;
            }

            if (j < formatCount)
            {
                XChangeProperty(_glfw.x11.display,
                                request->requestor,
                                targets[i + 1],
                                targets[i],
                                8,
                                PropModeReplace,
                                (unsigned char*) _glfw.x11.clipboardString,
                                strlen(_glfw.x11.clipboardString));
            }
            else
                targets[i + 1] = None;
        }

        XChangeProperty(_glfw.x11.display,
                        request->requestor,
                        request->property,
                        _glfw.x11.ATOM_PAIR,
                        32,
                        PropModeReplace,
                        (unsigned char*) targets,
                        count);

        XFree(targets);

        return request->property;
    }

    if (request->target == _glfw.x11.SAVE_TARGETS)
    {
        // The request is a check whether we support SAVE_TARGETS
        // It should be handled as a no-op side effect target

        XChangeProperty(_glfw.x11.display,
                        request->requestor,
                        request->property,
                        _glfw.x11.NULL_,
                        32,
                        PropModeReplace,
                        NULL,
                        0);

        return request->property;
    }

    // Conversion to a data target was requested

    for (i = 0;  i < formatCount;  i++)
    {
        if (request->target == formats[i])
        {
            // The requested target is one we support

            XChangeProperty(_glfw.x11.display,
                            request->requestor,
                            request->property,
                            request->target,
                            8,
                            PropModeReplace,
                            (unsigned char*) _glfw.x11.clipboardString,
                            strlen(_glfw.x11.clipboardString));

            return request->property;
        }
    }

    // The requested target is not supported

    return None;
}

static void handleSelectionClear(XEvent* event)
{
    free(_glfw.x11.clipboardString);
    _glfw.x11.clipboardString = NULL;
}

static void handleSelectionRequest(XEvent* event)
{
    const XSelectionRequestEvent* request = &event->xselectionrequest;

    XEvent reply;
    memset(&reply, 0, sizeof(reply));

    reply.xselection.property = writeTargetToProperty(request);
    reply.xselection.type = SelectionNotify;
    reply.xselection.display = request->display;
    reply.xselection.requestor = request->requestor;
    reply.xselection.selection = request->selection;
    reply.xselection.target = request->target;
    reply.xselection.time = request->time;

    XSendEvent(_glfw.x11.display, request->requestor, False, 0, &reply);
}

static void pushSelectionToManager(_GLFWwindow* window)
{
    XConvertSelection(_glfw.x11.display,
                      _glfw.x11.CLIPBOARD_MANAGER,
                      _glfw.x11.SAVE_TARGETS,
                      None,
                      window->x11.handle,
                      CurrentTime);

    for (;;)
    {
        XEvent event;

        while (XCheckIfEvent(_glfw.x11.display, &event, isSelectionEvent, NULL))
        {
            switch (event.type)
            {
                case SelectionRequest:
                    handleSelectionRequest(&event);
                    break;

                case SelectionClear:
                    handleSelectionClear(&event);
                    break;

                case SelectionNotify:
                {
                    if (event.xselection.target == _glfw.x11.SAVE_TARGETS)
                    {
                        // This means one of two things; either the selection was
                        // not owned, which means there is no clipboard manager, or
                        // the transfer to the clipboard manager has completed
                        // In either case, it means we are done here
                        return;
                    }

                    break;
                }
            }
        }

        selectDisplayConnection(NULL);
    }
}

// Enter full screen mode
//
static void enterFullscreenMode(_GLFWwindow* window)
{
    if (_glfw.x11.saver.count == 0)
    {
        // Remember old screen saver settings
        XGetScreenSaver(_glfw.x11.display,
                        &_glfw.x11.saver.timeout,
                        &_glfw.x11.saver.interval,
                        &_glfw.x11.saver.blanking,
                        &_glfw.x11.saver.exposure);

        // Disable screen saver
        XSetScreenSaver(_glfw.x11.display, 0, 0, DontPreferBlanking,
                        DefaultExposures);
    }

    _glfw.x11.saver.count++;

    _glfwSetVideoMode(window->monitor, &window->videoMode);

    if (_glfw.x11.NET_WM_BYPASS_COMPOSITOR)
    {
        const unsigned long value = 1;

        XChangeProperty(_glfw.x11.display,  window->x11.handle,
                        _glfw.x11.NET_WM_BYPASS_COMPOSITOR, XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char*) &value, 1);
    }

    if (_glfw.x11.xinerama.available && _glfw.x11.NET_WM_FULLSCREEN_MONITORS)
    {
        sendEventToWM(window,
                      _glfw.x11.NET_WM_FULLSCREEN_MONITORS,
                      window->monitor->x11.index,
                      window->monitor->x11.index,
                      window->monitor->x11.index,
                      window->monitor->x11.index,
                      0);
    }

    if (_glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_FULLSCREEN)
    {
        int x, y;
        _glfwPlatformGetMonitorPos(window->monitor, &x, &y);
        _glfwPlatformSetWindowPos(window, x, y);

        if (_glfw.x11.NET_ACTIVE_WINDOW)
        {
            // Ask the window manager to raise and focus the GLFW window
            // Only focused windows with the _NET_WM_STATE_FULLSCREEN state end
            // up on top of all other windows ("Stacking order" in EWMH spec)
            sendEventToWM(window, _glfw.x11.NET_ACTIVE_WINDOW, 1, 0, 0, 0, 0);
        }

        // Ask the window manager to make the GLFW window a full screen window
        // Full screen windows are undecorated and, when focused, are kept
        // on top of all other windows
        sendEventToWM(window,
                      _glfw.x11.NET_WM_STATE,
                      _NET_WM_STATE_ADD,
                      _glfw.x11.NET_WM_STATE_FULLSCREEN,
                      0, 1, 0);
    }
    else if (window->x11.overrideRedirect)
    {
        // In override-redirect mode we have divorced ourselves from the
        // window manager, so we need to do everything manually
        int xpos, ypos;
        GLFWvidmode mode;

        _glfwPlatformGetMonitorPos(window->monitor, &xpos, &ypos);
        _glfwPlatformGetVideoMode(window->monitor, &mode);

        XRaiseWindow(_glfw.x11.display, window->x11.handle);
        XSetInputFocus(_glfw.x11.display, window->x11.handle,
                       RevertToParent, CurrentTime);
        XMoveWindow(_glfw.x11.display, window->x11.handle, xpos, ypos);
        XResizeWindow(_glfw.x11.display, window->x11.handle,
                      mode.width, mode.height);
    }
}

// Leave full screen mode
//
static void leaveFullscreenMode(_GLFWwindow* window)
{
    _glfwRestoreVideoMode(window->monitor);

    _glfw.x11.saver.count--;

    if (_glfw.x11.saver.count == 0)
    {
        // Restore old screen saver settings
        XSetScreenSaver(_glfw.x11.display,
                        _glfw.x11.saver.timeout,
                        _glfw.x11.saver.interval,
                        _glfw.x11.saver.blanking,
                        _glfw.x11.saver.exposure);
    }

    if (_glfw.x11.NET_WM_BYPASS_COMPOSITOR)
    {
        const unsigned long value = 0;

        XChangeProperty(_glfw.x11.display,  window->x11.handle,
                        _glfw.x11.NET_WM_BYPASS_COMPOSITOR, XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char*) &value, 1);
    }

    if (_glfw.x11.NET_WM_STATE && _glfw.x11.NET_WM_STATE_FULLSCREEN)
    {
        // Ask the window manager to make the GLFW window a normal window
        // Normal windows usually have frames and other decorations
        sendEventToWM(window,
                      _glfw.x11.NET_WM_STATE,
                      _NET_WM_STATE_REMOVE,
                      _glfw.x11.NET_WM_STATE_FULLSCREEN,
                      0, 1, 0);
    }
}

// Process the specified X event
//
static void processEvent(XEvent *event)
{
    _GLFWwindow* window = NULL;

    if (event->type != GenericEvent)
    {
        window = findWindowByHandle(event->xany.window);
        if (window == NULL)
        {
            // This is an event for a window that has already been destroyed
            return;
        }
    }

    switch (event->type)
    {
        case KeyPress:
        {
            const int key = translateKey(event->xkey.keycode);
            const int mods = translateState(event->xkey.state);
            const int plain = !(mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT));

            if (event->xkey.keycode)
                _glfwInputKey(window, key, event->xkey.keycode, GLFW_PRESS, mods);

            if (window->x11.ic)
            {
                // Translate keys to characters with XIM input context

                int i;
                Status status;
                wchar_t buffer[16];

                if (XFilterEvent(event, None))
                {
                    // Discard intermediary (dead key) events for character input
                    break;
                }

                const int count = XwcLookupString(window->x11.ic,
                                                  &event->xkey,
                                                  buffer, sizeof(buffer),
                                                  NULL, &status);

                for (i = 0;  i < count;  i++)
                    _glfwInputChar(window, buffer[i], mods, plain);
            }
            else
            {
                // Translate keys to characters with fallback lookup table

                KeySym keysym;
                XLookupString(&event->xkey, NULL, 0, &keysym, NULL);

                const long character = _glfwKeySym2Unicode(keysym);
                if (character != -1)
                    _glfwInputChar(window, character, mods, plain);
            }

            break;
        }

        case KeyRelease:
        {
            const int key = translateKey(event->xkey.keycode);
            const int mods = translateState(event->xkey.state);

            if (!_glfw.x11.xkb.detectable)
            {
                // HACK: Key repeat events will arrive as KeyRelease/KeyPress
                //       pairs with similar or identical time stamps
                //       The key repeat logic in _glfwInputKey expects only key
                //       presses to repeat, so detect and discard release events
                if (XEventsQueued(_glfw.x11.display, QueuedAfterReading))
                {
                    XEvent nextEvent;
                    XPeekEvent(_glfw.x11.display, &nextEvent);

                    if (nextEvent.type == KeyPress &&
                        nextEvent.xkey.window == event->xkey.window &&
                        nextEvent.xkey.keycode == event->xkey.keycode)
                    {
                        // HACK: Repeat events sometimes leak through due to
                        //       some sort of time drift, so add an epsilon
                        //       Toshiyuki Takahashi can press a button 16 times
                        //       per second so it's fairly safe to assume that
                        //       no human is pressing the key 50 times per
                        //       second (value is ms)
                        if ((nextEvent.xkey.time - event->xkey.time) < 20)
                        {
                            // This is very likely a server-generated key repeat
                            // event, so ignore it
                            break;
                        }
                    }
                }
            }

            _glfwInputKey(window, key, event->xkey.keycode, GLFW_RELEASE, mods);
            break;
        }

        case ButtonPress:
        {
            const int mods = translateState(event->xbutton.state);

            if (event->xbutton.button == Button1)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, mods);
            else if (event->xbutton.button == Button2)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS, mods);
            else if (event->xbutton.button == Button3)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, mods);

            // Modern X provides scroll events as mouse button presses
            else if (event->xbutton.button == Button4)
                _glfwInputScroll(window, 0.0, 1.0);
            else if (event->xbutton.button == Button5)
                _glfwInputScroll(window, 0.0, -1.0);
            else if (event->xbutton.button == Button6)
                _glfwInputScroll(window, 1.0, 0.0);
            else if (event->xbutton.button == Button7)
                _glfwInputScroll(window, -1.0, 0.0);

            else
            {
                // Additional buttons after 7 are treated as regular buttons
                // We subtract 4 to fill the gap left by scroll input above
                _glfwInputMouseClick(window,
                                     event->xbutton.button - Button1 - 4,
                                     GLFW_PRESS,
                                     mods);
            }

            break;
        }

        case ButtonRelease:
        {
            const int mods = translateState(event->xbutton.state);

            if (event->xbutton.button == Button1)
            {
                _glfwInputMouseClick(window,
                                     GLFW_MOUSE_BUTTON_LEFT,
                                     GLFW_RELEASE,
                                     mods);
            }
            else if (event->xbutton.button == Button2)
            {
                _glfwInputMouseClick(window,
                                     GLFW_MOUSE_BUTTON_MIDDLE,
                                     GLFW_RELEASE,
                                     mods);
            }
            else if (event->xbutton.button == Button3)
            {
                _glfwInputMouseClick(window,
                                     GLFW_MOUSE_BUTTON_RIGHT,
                                     GLFW_RELEASE,
                                     mods);
            }
            else if (event->xbutton.button > Button7)
            {
                // Additional buttons after 7 are treated as regular buttons
                // We subtract 4 to fill the gap left by scroll input above
                _glfwInputMouseClick(window,
                                     event->xbutton.button - Button1 - 4,
                                     GLFW_RELEASE,
                                     mods);
            }
            break;
        }

        case EnterNotify:
        {
            // HACK: This is a workaround for WMs (KWM, Fluxbox) that otherwise
            //       ignore the defined cursor for hidden cursor mode
            if (window->cursorMode == GLFW_CURSOR_HIDDEN)
                hideCursor(window);

            _glfwInputCursorEnter(window, GL_TRUE);
            break;
        }

        case LeaveNotify:
        {
            _glfwInputCursorEnter(window, GL_FALSE);
            break;
        }

        case MotionNotify:
        {
            const int x = event->xmotion.x;
            const int y = event->xmotion.y;

            if (x != window->x11.warpPosX || y != window->x11.warpPosY)
            {
                // The cursor was moved by something other than GLFW

                if (window->cursorMode == GLFW_CURSOR_DISABLED)
                {
                    if (_glfw.focusedWindow != window)
                        break;

                    _glfwInputCursorMotion(window,
                                           x - window->x11.cursorPosX,
                                           y - window->x11.cursorPosY);
                }
                else
                    _glfwInputCursorMotion(window, x, y);
            }

            window->x11.cursorPosX = x;
            window->x11.cursorPosY = y;
            break;
        }

        case ConfigureNotify:
        {
            if (event->xconfigure.width != window->x11.width ||
                event->xconfigure.height != window->x11.height)
            {
                _glfwInputFramebufferSize(window,
                                          event->xconfigure.width,
                                          event->xconfigure.height);

                _glfwInputWindowSize(window,
                                     event->xconfigure.width,
                                     event->xconfigure.height);

                window->x11.width = event->xconfigure.width;
                window->x11.height = event->xconfigure.height;
            }

            if (event->xconfigure.x != window->x11.xpos ||
                event->xconfigure.y != window->x11.ypos)
            {
                _glfwInputWindowPos(window,
                                    event->xconfigure.x,
                                    event->xconfigure.y);

                window->x11.xpos = event->xconfigure.x;
                window->x11.ypos = event->xconfigure.y;
            }

            break;
        }

        case ClientMessage:
        {
            // Custom client message, probably from the window manager

            if (event->xclient.message_type == None)
                break;

            if (event->xclient.message_type == _glfw.x11.WM_PROTOCOLS)
            {
                const Atom protocol = event->xclient.data.l[0];
                if (protocol == None)
                    break;

                if (protocol == _glfw.x11.WM_DELETE_WINDOW)
                {
                    // The window manager was asked to close the window, for example by
                    // the user pressing a 'close' window decoration button
                    _glfwInputWindowCloseRequest(window);
                }
                else if (protocol == _glfw.x11.NET_WM_PING)
                {
                    // The window manager is pinging the application to ensure it's
                    // still responding to events

                    XEvent reply = *event;
                    reply.xclient.window = _glfw.x11.root;

                    XSendEvent(_glfw.x11.display, _glfw.x11.root,
                               False,
                               SubstructureNotifyMask | SubstructureRedirectMask,
                               &reply);
                }
            }
            else if (event->xclient.message_type == _glfw.x11.XdndEnter)
            {
                // A drag operation has entered the window
                // TODO: Check if UTF-8 string is supported by the source
            }
            else if (event->xclient.message_type == _glfw.x11.XdndDrop)
            {
                // The drag operation has finished dropping on
                // the window, ask to convert it to a UTF-8 string
                _glfw.x11.xdnd.source = event->xclient.data.l[0];
                XConvertSelection(_glfw.x11.display,
                                  _glfw.x11.XdndSelection,
                                  _glfw.x11.UTF8_STRING,
                                  _glfw.x11.XdndSelection,
                                  window->x11.handle, CurrentTime);
            }
            else if (event->xclient.message_type == _glfw.x11.XdndPosition)
            {
                // The drag operation has moved over the window
                const int absX = (event->xclient.data.l[2] >> 16) & 0xFFFF;
                const int absY = (event->xclient.data.l[2]) & 0xFFFF;
                int x, y;

                _glfwPlatformGetWindowPos(window, &x, &y);
                _glfwInputCursorMotion(window, absX - x, absY - y);

                // Reply that we are ready to copy the dragged data
                XEvent reply;
                memset(&reply, 0, sizeof(reply));

                reply.type = ClientMessage;
                reply.xclient.window = event->xclient.data.l[0];
                reply.xclient.message_type = _glfw.x11.XdndStatus;
                reply.xclient.format = 32;
                reply.xclient.data.l[0] = window->x11.handle;
                reply.xclient.data.l[1] = 1; // Always accept the dnd with no rectangle
                reply.xclient.data.l[2] = 0; // Specify an empty rectangle
                reply.xclient.data.l[3] = 0;
                reply.xclient.data.l[4] = _glfw.x11.XdndActionCopy;

                XSendEvent(_glfw.x11.display, event->xclient.data.l[0],
                           False, NoEventMask, &reply);
                XFlush(_glfw.x11.display);
            }

            break;
        }

        case SelectionNotify:
        {
            if (event->xselection.property)
            {
                // The converted data from the drag operation has arrived
                char* data;
                const int result =
                    _glfwGetWindowProperty(event->xselection.requestor,
                                           event->xselection.property,
                                           event->xselection.target,
                                           (unsigned char**) &data);

                if (result)
                {
                    int i, count;
                    char** paths = parseUriList(data, &count);

                    _glfwInputDrop(window, count, (const char**) paths);

                    for (i = 0;  i < count;  i++)
                        free(paths[i]);
                    free(paths);
                }

                XFree(data);

                XEvent reply;
                memset(&reply, 0, sizeof(reply));

                reply.type = ClientMessage;
                reply.xclient.window = _glfw.x11.xdnd.source;
                reply.xclient.message_type = _glfw.x11.XdndFinished;
                reply.xclient.format = 32;
                reply.xclient.data.l[0] = window->x11.handle;
                reply.xclient.data.l[1] = result;
                reply.xclient.data.l[2] = _glfw.x11.XdndActionCopy;

                // Reply that all is well
                XSendEvent(_glfw.x11.display, _glfw.x11.xdnd.source,
                           False, NoEventMask, &reply);
                XFlush(_glfw.x11.display);
            }

            break;
        }

        case FocusIn:
        {
            if (event->xfocus.mode == NotifyNormal)
            {
                _glfwInputWindowFocus(window, GL_TRUE);

                if (window->cursorMode == GLFW_CURSOR_DISABLED)
                    disableCursor(window);
            }

            break;
        }

        case FocusOut:
        {
            if (event->xfocus.mode == NotifyNormal)
            {
                _glfwInputWindowFocus(window, GL_FALSE);

                if (window->cursorMode == GLFW_CURSOR_DISABLED)
                    restoreCursor(window);
            }

            break;
        }

        case Expose:
        {
            _glfwInputWindowDamage(window);
            break;
        }

        case PropertyNotify:
        {
            if (event->xproperty.atom == _glfw.x11.WM_STATE &&
                event->xproperty.state == PropertyNewValue)
            {
                const int state = getWindowState(window);
                if (state == IconicState)
                    _glfwInputWindowIconify(window, GL_TRUE);
                else if (state == NormalState)
                    _glfwInputWindowIconify(window, GL_FALSE);
            }

            break;
        }

        case SelectionClear:
        {
            handleSelectionClear(event);
            break;
        }

        case SelectionRequest:
        {
            handleSelectionRequest(event);
            break;
        }

        case DestroyNotify:
            return;

#if defined(_GLFW_HAS_XINPUT)
        case GenericEvent:
        {
            if (event->xcookie.extension == _glfw.x11.xi.majorOpcode &&
                XGetEventData(_glfw.x11.display, &event->xcookie))
            {
                if (event->xcookie.evtype == XI_Motion)
                {
                    XIDeviceEvent* data = (XIDeviceEvent*) event->xcookie.data;

                    window = findWindowByHandle(data->event);
                    if (window)
                    {
                        if (data->event_x != window->x11.warpPosX ||
                            data->event_y != window->x11.warpPosY)
                        {
                            // The cursor was moved by something other than GLFW

                            double x, y;

                            if (window->cursorMode == GLFW_CURSOR_DISABLED)
                            {
                                if (_glfw.focusedWindow != window)
                                    break;

                                x = data->event_x - window->x11.cursorPosX;
                                y = data->event_y - window->x11.cursorPosY;
                            }
                            else
                            {
                                x = data->event_x;
                                y = data->event_y;
                            }

                            _glfwInputCursorMotion(window, x, y);
                        }

                        window->x11.cursorPosX = data->event_x;
                        window->x11.cursorPosY = data->event_y;
                    }
                }
            }

            XFreeEventData(_glfw.x11.display, &event->xcookie);
            break;
        }
#endif /*_GLFW_HAS_XINPUT*/

        default:
        {
            switch (event->type - _glfw.x11.randr.eventBase)
            {
                case RRScreenChangeNotify:
                {
                    XRRUpdateConfiguration(event);
                    break;
                }
            }

            break;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Retrieve a single window property of the specified type
// Inspired by fghGetWindowProperty from freeglut
//
unsigned long _glfwGetWindowProperty(Window window,
                                     Atom property,
                                     Atom type,
                                     unsigned char** value)
{
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty(_glfw.x11.display,
                       window,
                       property,
                       0,
                       LONG_MAX,
                       False,
                       type,
                       &actualType,
                       &actualFormat,
                       &itemCount,
                       &bytesAfter,
                       value);

    if (type != AnyPropertyType && actualType != type)
        return 0;

    return itemCount;
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

    if (!createWindow(window, wndconfig))
        return GL_FALSE;

    if (wndconfig->monitor)
    {
        _glfwPlatformShowWindow(window);
        enterFullscreenMode(window);
    }

    return GL_TRUE;
}

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
    if (window->monitor)
        leaveFullscreenMode(window);

    if (window->x11.ic)
    {
        XDestroyIC(window->x11.ic);
        window->x11.ic = NULL;
    }

    _glfwDestroyContext(window);

    if (window->x11.handle)
    {
        if (XGetSelectionOwner(_glfw.x11.display, _glfw.x11.CLIPBOARD) ==
            window->x11.handle)
        {
            pushSelectionToManager(window);
        }

        XDeleteContext(_glfw.x11.display, window->x11.handle, _glfw.x11.context);
        XUnmapWindow(_glfw.x11.display, window->x11.handle);
        XDestroyWindow(_glfw.x11.display, window->x11.handle);
        window->x11.handle = (Window) 0;
    }

    if (window->x11.colormap)
    {
        XFreeColormap(_glfw.x11.display, window->x11.colormap);
        window->x11.colormap = (Colormap) 0;
    }

    XFlush(_glfw.x11.display);
}

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
#if defined(X_HAVE_UTF8_STRING)
    Xutf8SetWMProperties(_glfw.x11.display,
                         window->x11.handle,
                         title, title,
                         NULL, 0,
                         NULL, NULL, NULL);
#else
    // This may be a slightly better fallback than using XStoreName and
    // XSetIconName, which always store their arguments using STRING
    XmbSetWMProperties(_glfw.x11.display,
                       window->x11.handle,
                       title, title,
                       NULL, 0,
                       NULL, NULL, NULL);
#endif

    if (_glfw.x11.NET_WM_NAME)
    {
        XChangeProperty(_glfw.x11.display,  window->x11.handle,
                        _glfw.x11.NET_WM_NAME, _glfw.x11.UTF8_STRING, 8,
                        PropModeReplace,
                        (unsigned char*) title, strlen(title));
    }

    if (_glfw.x11.NET_WM_ICON_NAME)
    {
        XChangeProperty(_glfw.x11.display,  window->x11.handle,
                        _glfw.x11.NET_WM_ICON_NAME, _glfw.x11.UTF8_STRING, 8,
                        PropModeReplace,
                        (unsigned char*) title, strlen(title));
    }

    XFlush(_glfw.x11.display);
}

void _glfwPlatformGetWindowPos(_GLFWwindow* window, int* xpos, int* ypos)
{
    Window child;
    int x, y;

    XTranslateCoordinates(_glfw.x11.display, window->x11.handle, _glfw.x11.root,
                          0, 0, &x, &y, &child);

    if (child)
    {
        int left, top;
        XTranslateCoordinates(_glfw.x11.display, window->x11.handle, child,
                              0, 0, &left, &top, &child);

        x -= left;
        y -= top;
    }

    if (xpos)
        *xpos = x;
    if (ypos)
        *ypos = y;
}

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int xpos, int ypos)
{
    XMoveWindow(_glfw.x11.display, window->x11.handle, xpos, ypos);
    XFlush(_glfw.x11.display);
}

void _glfwPlatformGetWindowSize(_GLFWwindow* window, int* width, int* height)
{
    XWindowAttributes attribs;
    XGetWindowAttributes(_glfw.x11.display, window->x11.handle, &attribs);

    if (width)
        *width = attribs.width;
    if (height)
        *height = attribs.height;
}

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    if (window->monitor)
    {
        _glfwSetVideoMode(window->monitor, &window->videoMode);

        if (window->x11.overrideRedirect)
        {
            GLFWvidmode mode;
            _glfwPlatformGetVideoMode(window->monitor, &mode);
            XResizeWindow(_glfw.x11.display, window->x11.handle,
                          mode.width, mode.height);
        }
    }
    else
    {
        if (!window->resizable)
        {
            // Update window size restrictions to match new window size

            XSizeHints* hints = XAllocSizeHints();

            hints->flags |= (PMinSize | PMaxSize);
            hints->min_width  = hints->max_width  = width;
            hints->min_height = hints->max_height = height;

            XSetWMNormalHints(_glfw.x11.display, window->x11.handle, hints);
            XFree(hints);
        }

        XResizeWindow(_glfw.x11.display, window->x11.handle, width, height);
    }

    XFlush(_glfw.x11.display);
}

void _glfwPlatformGetFramebufferSize(_GLFWwindow* window, int* width, int* height)
{
    _glfwPlatformGetWindowSize(window, width, height);
}

void _glfwPlatformGetWindowFrameSize(_GLFWwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
    long* extents = NULL;

    if (_glfw.x11.NET_FRAME_EXTENTS == None)
        return;

    if (!_glfwPlatformWindowVisible(window) &&
        _glfw.x11.NET_REQUEST_FRAME_EXTENTS)
    {
        double base;
        XEvent event;

        // Ensure _NET_FRAME_EXTENTS is set, allowing glfwGetWindowFrameSize to
        // function before the window is mapped
        sendEventToWM(window, _glfw.x11.NET_REQUEST_FRAME_EXTENTS,
                      0, 0, 0, 0, 0);

        // HACK: Poll with timeout for the required reply instead of blocking
        //       This is done because some window managers (at least Unity,
        //       Fluxbox and Xfwm) failed to send the required reply
        //       They have been fixed but broken versions are still in the wild
        //       If you are affected by this and your window manager is NOT
        //       listed above, PLEASE report it to their and our issue trackers
        base = _glfwPlatformGetTime();
        while (!XCheckIfEvent(_glfw.x11.display,
                              &event,
                              isFrameExtentsEvent,
                              (XPointer) window))
        {
            double remaining;
            struct timeval timeout;

            remaining = 0.5 + base - _glfwPlatformGetTime();
            if (remaining <= 0.0)
            {
                _glfwInputError(GLFW_PLATFORM_ERROR,
                                "X11: The window manager has a broken _NET_REQUEST_FRAME_EXTENTS implementation; please report this issue");
                return;
            }

            timeout.tv_sec = 0;
            timeout.tv_usec = (long) (remaining * 1e6);
            selectDisplayConnection(&timeout);
        }
    }

    if (_glfwGetWindowProperty(window->x11.handle,
                               _glfw.x11.NET_FRAME_EXTENTS,
                               XA_CARDINAL,
                               (unsigned char**) &extents) == 4)
    {
        if (left)
            *left = extents[0];
        if (top)
            *top = extents[2];
        if (right)
            *right = extents[1];
        if (bottom)
            *bottom = extents[3];
    }

    if (extents)
        XFree(extents);
}

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
    if (window->x11.overrideRedirect)
    {
        // Override-redirect windows cannot be iconified or restored, as those
        // tasks are performed by the window manager
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "X11: Iconification of full screen windows requires a WM that supports EWMH");
        return;
    }

    XIconifyWindow(_glfw.x11.display, window->x11.handle, _glfw.x11.screen);
    XFlush(_glfw.x11.display);
}

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
    if (window->x11.overrideRedirect)
    {
        // Override-redirect windows cannot be iconified or restored, as those
        // tasks are performed by the window manager
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "X11: Iconification of full screen windows requires a WM that supports EWMH");
        return;
    }

    XMapWindow(_glfw.x11.display, window->x11.handle);
    XFlush(_glfw.x11.display);
}

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
    XMapRaised(_glfw.x11.display, window->x11.handle);
    XFlush(_glfw.x11.display);
}

void _glfwPlatformUnhideWindow(_GLFWwindow* window)
{
    XMapWindow(_glfw.x11.display, window->x11.handle);
    XFlush(_glfw.x11.display);
}

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
    XUnmapWindow(_glfw.x11.display, window->x11.handle);
    XFlush(_glfw.x11.display);
}

int _glfwPlatformWindowFocused(_GLFWwindow* window)
{
    Window focused;
    int state;

    XGetInputFocus(_glfw.x11.display, &focused, &state);
    return window->x11.handle == focused;
}

int _glfwPlatformWindowIconified(_GLFWwindow* window)
{
    return getWindowState(window) == IconicState;
}

int _glfwPlatformWindowVisible(_GLFWwindow* window)
{
    XWindowAttributes wa;
    XGetWindowAttributes(_glfw.x11.display, window->x11.handle, &wa);
    return wa.map_state == IsViewable;
}

void _glfwPlatformPollEvents(void)
{
    int count = XPending(_glfw.x11.display);
    while (count--)
    {
        XEvent event;
        XNextEvent(_glfw.x11.display, &event);
        processEvent(&event);
    }

    _GLFWwindow* window = _glfw.focusedWindow;
    if (window && window->cursorMode == GLFW_CURSOR_DISABLED)
    {
        int width, height;
        _glfwPlatformGetWindowSize(window, &width, &height);
        _glfwPlatformSetCursorPos(window, width / 2, height / 2);
    }
}

void _glfwPlatformWaitEvents(void)
{
    if (!XPending(_glfw.x11.display))
        selectDisplayConnection(NULL);

    _glfwPlatformPollEvents();
}

void _glfwPlatformPostEmptyEvent(void)
{
    XEvent event;
    _GLFWwindow* window = _glfw.windowListHead;

    memset(&event, 0, sizeof(event));
    event.type = ClientMessage;
    event.xclient.window = window->x11.handle;
    event.xclient.format = 32; // Data is 32-bit longs
    event.xclient.message_type = _glfw.x11.NULL_;

    XSendEvent(_glfw.x11.display, window->x11.handle, False, 0, &event);
    XFlush(_glfw.x11.display);
}

void _glfwPlatformGetCursorPos(_GLFWwindow* window, double* xpos, double* ypos)
{
    Window root, child;
    int rootX, rootY, childX, childY;
    unsigned int mask;

    XQueryPointer(_glfw.x11.display, window->x11.handle,
                  &root, &child,
                  &rootX, &rootY, &childX, &childY,
                  &mask);

    if (xpos)
        *xpos = childX;
    if (ypos)
        *ypos = childY;
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double x, double y)
{
    // Store the new position so it can be recognized later
    window->x11.warpPosX = (int) x;
    window->x11.warpPosY = (int) y;

    XWarpPointer(_glfw.x11.display, None, window->x11.handle,
                 0,0,0,0, (int) x, (int) y);
}

void _glfwPlatformApplyCursorMode(_GLFWwindow* window)
{
    switch (window->cursorMode)
    {
        case GLFW_CURSOR_NORMAL:
            restoreCursor(window);
            break;
        case GLFW_CURSOR_HIDDEN:
            hideCursor(window);
            break;
        case GLFW_CURSOR_DISABLED:
            disableCursor(window);
            break;
    }
}

int _glfwPlatformCreateCursor(_GLFWcursor* cursor,
                              const GLFWimage* image,
                              int xhot, int yhot)
{
    cursor->x11.handle = _glfwCreateCursor(image, xhot, yhot);
    if (!cursor->x11.handle)
        return GL_FALSE;

    return GL_TRUE;
}

int _glfwPlatformCreateStandardCursor(_GLFWcursor* cursor, int shape)
{
    cursor->x11.handle = XCreateFontCursor(_glfw.x11.display,
                                           translateCursorShape(shape));
    if (!cursor->x11.handle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "X11: Failed to create standard cursor");
        return GL_FALSE;
    }

    return GL_TRUE;
}

void _glfwPlatformDestroyCursor(_GLFWcursor* cursor)
{
    if (cursor->x11.handle)
        XFreeCursor(_glfw.x11.display, cursor->x11.handle);
}

void _glfwPlatformSetCursor(_GLFWwindow* window, _GLFWcursor* cursor)
{
    if (window->cursorMode == GLFW_CURSOR_NORMAL)
    {
        if (cursor)
            XDefineCursor(_glfw.x11.display, window->x11.handle, cursor->x11.handle);
        else
            XUndefineCursor(_glfw.x11.display, window->x11.handle);

        XFlush(_glfw.x11.display);
    }
}

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string)
{
    free(_glfw.x11.clipboardString);
    _glfw.x11.clipboardString = strdup(string);

    XSetSelectionOwner(_glfw.x11.display,
                       _glfw.x11.CLIPBOARD,
                       window->x11.handle, CurrentTime);

    if (XGetSelectionOwner(_glfw.x11.display, _glfw.x11.CLIPBOARD) !=
        window->x11.handle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "X11: Failed to become owner of clipboard selection");
    }
}

const char* _glfwPlatformGetClipboardString(_GLFWwindow* window)
{
    size_t i;
    const Atom formats[] = { _glfw.x11.UTF8_STRING,
                             _glfw.x11.COMPOUND_STRING,
                             XA_STRING };
    const size_t formatCount = sizeof(formats) / sizeof(formats[0]);

    if (findWindowByHandle(XGetSelectionOwner(_glfw.x11.display,
                                              _glfw.x11.CLIPBOARD)))
    {
        // Instead of doing a large number of X round-trips just to put this
        // string into a window property and then read it back, just return it
        return _glfw.x11.clipboardString;
    }

    free(_glfw.x11.clipboardString);
    _glfw.x11.clipboardString = NULL;

    for (i = 0;  i < formatCount;  i++)
    {
        char* data;
        XEvent event;

        XConvertSelection(_glfw.x11.display,
                          _glfw.x11.CLIPBOARD,
                          formats[i],
                          _glfw.x11.GLFW_SELECTION,
                          window->x11.handle, CurrentTime);

        // XCheckTypedEvent is used instead of XIfEvent in order not to lock
        // other threads out from the display during the entire wait period
        while (!XCheckTypedEvent(_glfw.x11.display, SelectionNotify, &event))
            selectDisplayConnection(NULL);

        if (event.xselection.property == None)
            continue;

        if (_glfwGetWindowProperty(event.xselection.requestor,
                                   event.xselection.property,
                                   event.xselection.target,
                                   (unsigned char**) &data))
        {
            _glfw.x11.clipboardString = strdup(data);
        }

        XFree(data);

        XDeleteProperty(_glfw.x11.display,
                        event.xselection.requestor,
                        event.xselection.property);

        if (_glfw.x11.clipboardString)
            break;
    }

    if (_glfw.x11.clipboardString == NULL)
    {
        _glfwInputError(GLFW_FORMAT_UNAVAILABLE,
                        "X11: Failed to convert clipboard to string");
    }

    return _glfw.x11.clipboardString;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI Display* glfwGetX11Display(void)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    return _glfw.x11.display;
}

GLFWAPI Window glfwGetX11Window(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(None);
    return window->x11.handle;
}

