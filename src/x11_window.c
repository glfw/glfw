//========================================================================
// GLFW 3.0 X11 - www.glfw.org
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

#include <sys/select.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <wchar.h>
#include <locale.h>

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
static int translateKey(int keycode)
{
    // Use the pre-filled LUT (see updateKeyCodeLUT() in x11_init.c)
    if ((keycode >= 0) && (keycode < 256))
        return _glfw.x11.keyCodeLUT[keycode];

    return GLFW_KEY_UNKNOWN;
}

// Translates an X Window event to Unicode
//
static int translateChar(XKeyEvent* event)
{
    KeySym keysym;

    // Get X11 keysym
    XLookupString(event, NULL, 0, &keysym, NULL);

    // Convert to Unicode (see x11_unicode.c)
    return (int) _glfwKeySym2Unicode(keysym);
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

        if (wndconfig->monitor == NULL)
        {
            // HACK: This is a workaround for windows without a background pixel
            // not getting any decorations on certain older versions of Compiz
            // running on Intel hardware
            wa.background_pixel = BlackPixel(_glfw.x11.display,
                                             _glfw.x11.screen);
            wamask |= CWBackPixel;
        }

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

        if (!window->x11.handle)
        {
            // TODO: Handle all the various error codes here and translate them
            // to GLFW errors

            _glfwInputError(GLFW_PLATFORM_ERROR, "X11: Failed to create window");
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
        // The good thing is that this makes undecorated fullscreen windows
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
        if (_glfw.x11.WM_DELETE_WINDOW != None)
            protocols[count++] = _glfw.x11.WM_DELETE_WINDOW;

        // The _NET_WM_PING EWMH protocol
        // Tells the WM to ping the GLFW window and flag the application as
        // unresponsive if the WM doesn't get a reply within a few seconds
        if (_glfw.x11.NET_WM_PING != None)
            protocols[count++] = _glfw.x11.NET_WM_PING;

        if (count > 0)
        {
            XSetWMProtocols(_glfw.x11.display, window->x11.handle,
                            protocols, count);
        }
    }

    if (_glfw.x11.NET_WM_PID != None)
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

        if (!wndconfig->resizable)
        {
            hints->flags |= (PMinSize | PMaxSize);
            hints->min_width  = hints->max_width  = wndconfig->width;
            hints->min_height = hints->max_height = wndconfig->height;
        }

        XSetWMNormalHints(_glfw.x11.display, window->x11.handle, hints);
        XFree(hints);
    }

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

    _glfwPlatformSetWindowTitle(window, wndconfig->title);

    XRRSelectInput(_glfw.x11.display, window->x11.handle,
                   RRScreenChangeNotifyMask);

    _glfwPlatformGetWindowPos(window, &window->x11.xpos, &window->x11.ypos);
    _glfwPlatformGetWindowSize(window, &window->x11.width, &window->x11.height);

    return GL_TRUE;
}

// Hide cursor
//
static void hideCursor(_GLFWwindow* window)
{
    // Un-grab cursor (in windowed mode only; in fullscreen mode we still
    // want the cursor grabbed in order to confine the cursor to the window
    // area)
    if (window->x11.cursorGrabbed && window->monitor == NULL)
    {
        XUngrabPointer(_glfw.x11.display, CurrentTime);
        window->x11.cursorGrabbed = GL_FALSE;
    }

    if (!window->x11.cursorHidden)
    {
        XDefineCursor(_glfw.x11.display, window->x11.handle, _glfw.x11.cursor);
        window->x11.cursorHidden = GL_TRUE;
    }
}

// Capture cursor
//
static void captureCursor(_GLFWwindow* window)
{
    hideCursor(window);

    if (!window->x11.cursorGrabbed)
    {
        if (XGrabPointer(_glfw.x11.display, window->x11.handle, True,
                         ButtonPressMask | ButtonReleaseMask |
                         PointerMotionMask, GrabModeAsync, GrabModeAsync,
                         window->x11.handle, None, CurrentTime) ==
            GrabSuccess)
        {
            window->x11.cursorGrabbed = GL_TRUE;
        }
    }
}

// Show cursor
//
static void showCursor(_GLFWwindow* window)
{
    // Un-grab cursor (in windowed mode only; in fullscreen mode we still
    // want the cursor grabbed in order to confine the cursor to the window
    // area)
    if (window->x11.cursorGrabbed && window->monitor == NULL)
    {
        XUngrabPointer(_glfw.x11.display, CurrentTime);
        window->x11.cursorGrabbed = GL_FALSE;
    }

    // Show cursor
    if (window->x11.cursorHidden)
    {
        XUndefineCursor(_glfw.x11.display, window->x11.handle);
        window->x11.cursorHidden = GL_FALSE;
    }
}

// Enter fullscreen mode
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

    if (_glfw.x11.hasEWMH &&
        _glfw.x11.NET_WM_STATE != None &&
        _glfw.x11.NET_WM_STATE_FULLSCREEN != None)
    {
        int x, y;
        _glfwPlatformGetMonitorPos(window->monitor, &x, &y);
        _glfwPlatformSetWindowPos(window, x, y);

        if (_glfw.x11.NET_ACTIVE_WINDOW != None)
        {
            // Ask the window manager to raise and focus the GLFW window
            // Only focused windows with the _NET_WM_STATE_FULLSCREEN state end
            // up on top of all other windows ("Stacking order" in EWMH spec)

            XEvent event;
            memset(&event, 0, sizeof(event));

            event.type = ClientMessage;
            event.xclient.window = window->x11.handle;
            event.xclient.format = 32; // Data is 32-bit longs
            event.xclient.message_type = _glfw.x11.NET_ACTIVE_WINDOW;
            event.xclient.data.l[0] = 1; // Sender is a normal application
            event.xclient.data.l[1] = 0; // We don't really know the timestamp

            XSendEvent(_glfw.x11.display,
                       _glfw.x11.root,
                       False,
                       SubstructureNotifyMask | SubstructureRedirectMask,
                       &event);
        }

        // Ask the window manager to make the GLFW window a fullscreen window
        // Fullscreen windows are undecorated and, when focused, are kept
        // on top of all other windows

        XEvent event;
        memset(&event, 0, sizeof(event));

        event.type = ClientMessage;
        event.xclient.window = window->x11.handle;
        event.xclient.format = 32; // Data is 32-bit longs
        event.xclient.message_type = _glfw.x11.NET_WM_STATE;
        event.xclient.data.l[0] = _NET_WM_STATE_ADD;
        event.xclient.data.l[1] = _glfw.x11.NET_WM_STATE_FULLSCREEN;
        event.xclient.data.l[2] = 0; // No secondary property
        event.xclient.data.l[3] = 1; // Sender is a normal application

        XSendEvent(_glfw.x11.display,
                   _glfw.x11.root,
                   False,
                   SubstructureNotifyMask | SubstructureRedirectMask,
                   &event);
    }
    else if (window->x11.overrideRedirect)
    {
        // In override-redirect mode we have divorced ourselves from the
        // window manager, so we need to do everything manually

        GLFWvidmode mode;
        _glfwPlatformGetVideoMode(window->monitor, &mode);

        XRaiseWindow(_glfw.x11.display, window->x11.handle);
        XSetInputFocus(_glfw.x11.display, window->x11.handle,
                       RevertToParent, CurrentTime);
        XMoveWindow(_glfw.x11.display, window->x11.handle, 0, 0);
        XResizeWindow(_glfw.x11.display, window->x11.handle,
                      mode.width, mode.height);
    }
}

// Leave fullscreen mode
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

    if (_glfw.x11.hasEWMH &&
        _glfw.x11.NET_WM_STATE != None &&
        _glfw.x11.NET_WM_STATE_FULLSCREEN != None)
    {
        // Ask the window manager to make the GLFW window a normal window
        // Normal windows usually have frames and other decorations

        XEvent event;
        memset(&event, 0, sizeof(event));

        event.type = ClientMessage;
        event.xclient.window = window->x11.handle;
        event.xclient.format = 32; // Data is 32-bit longs
        event.xclient.message_type = _glfw.x11.NET_WM_STATE;
        event.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
        event.xclient.data.l[1] = _glfw.x11.NET_WM_STATE_FULLSCREEN;
        event.xclient.data.l[2] = 0; // No secondary property
        event.xclient.data.l[3] = 1; // Sender is a normal application

        XSendEvent(_glfw.x11.display,
                   _glfw.x11.root,
                   False,
                   SubstructureNotifyMask | SubstructureRedirectMask,
                   &event);
    }
}

// Process the specified X event
//
static void processEvent(XEvent *event)
{
    _GLFWwindow* window = NULL;

    if (event->type != GenericEvent)
    {
        window = _glfwFindWindowByHandle(event->xany.window);
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
            const int character = translateChar(&event->xkey);

            _glfwInputKey(window, key, event->xkey.keycode, GLFW_PRESS, mods);

            if (character != -1)
                _glfwInputChar(window, character);

            break;
        }

        case KeyRelease:
        {
            const int key = translateKey(event->xkey.keycode);
            const int mods = translateState(event->xkey.state);

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
                _glfwInputScroll(window, -1.0, 0.0);
            else if (event->xbutton.button == Button7)
                _glfwInputScroll(window, 1.0, 0.0);

            else
            {
                // Additional buttons after 7 are treated as regular buttons
                // We subtract 4 to fill the gap left by scroll input above
                _glfwInputMouseClick(window,
                                     event->xbutton.button - 4,
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
                                     event->xbutton.button - 4,
                                     GLFW_RELEASE,
                                     mods);
            }
            break;
        }

        case EnterNotify:
        {
            if (window->cursorMode == GLFW_CURSOR_HIDDEN)
                hideCursor(window);

            _glfwInputCursorEnter(window, GL_TRUE);
            break;
        }

        case LeaveNotify:
        {
            if (window->cursorMode == GLFW_CURSOR_HIDDEN)
                showCursor(window);

            _glfwInputCursorEnter(window, GL_FALSE);
            break;
        }

        case MotionNotify:
        {
            if (event->xmotion.x != window->x11.warpPosX ||
                event->xmotion.y != window->x11.warpPosY)
            {
                // The cursor was moved by something other than GLFW

                int x, y;

                if (window->cursorMode == GLFW_CURSOR_DISABLED)
                {
                    if (_glfw.focusedWindow != window)
                        break;

                    x = event->xmotion.x - window->x11.cursorPosX;
                    y = event->xmotion.y - window->x11.cursorPosY;
                }
                else
                {
                    x = event->xmotion.x;
                    y = event->xmotion.y;
                }

                _glfwInputCursorMotion(window, x, y);
            }

            window->x11.cursorPosX = event->xmotion.x;
            window->x11.cursorPosY = event->xmotion.y;
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

            if ((Atom) event->xclient.data.l[0] == _glfw.x11.WM_DELETE_WINDOW)
            {
                // The window manager was asked to close the window, for example by
                // the user pressing a 'close' window decoration button

                _glfwInputWindowCloseRequest(window);
            }
            else if (_glfw.x11.NET_WM_PING != None &&
                     (Atom) event->xclient.data.l[0] == _glfw.x11.NET_WM_PING)
            {
                // The window manager is pinging the application to ensure it's
                // still responding to events

                event->xclient.window = _glfw.x11.root;
                XSendEvent(_glfw.x11.display,
                           event->xclient.window,
                           False,
                           SubstructureNotifyMask | SubstructureRedirectMask,
                           event);
            }

            break;
        }

        case MapNotify:
        {
            _glfwInputWindowVisibility(window, GL_TRUE);
            break;
        }

        case UnmapNotify:
        {
            _glfwInputWindowVisibility(window, GL_FALSE);
            break;
        }

        case FocusIn:
        {
            _glfwInputWindowFocus(window, GL_TRUE);

            if (window->cursorMode == GLFW_CURSOR_DISABLED)
                captureCursor(window);

            break;
        }

        case FocusOut:
        {
            _glfwInputWindowFocus(window, GL_FALSE);

            if (window->cursorMode == GLFW_CURSOR_DISABLED)
                showCursor(window);

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
                struct {
                    CARD32 state;
                    Window icon;
                } *state = NULL;

                if (_glfwGetWindowProperty(window->x11.handle,
                                           _glfw.x11.WM_STATE,
                                           _glfw.x11.WM_STATE,
                                           (unsigned char**) &state) >= 2)
                {
                    if (state->state == IconicState)
                        _glfwInputWindowIconify(window, GL_TRUE);
                    else if (state->state == NormalState)
                        _glfwInputWindowIconify(window, GL_FALSE);
                }

                XFree(state);
            }

            break;
        }

        case SelectionClear:
        {
            _glfwHandleSelectionClear(event);
            break;
        }

        case SelectionRequest:
        {
            _glfwHandleSelectionRequest(event);
            break;
        }

        case DestroyNotify:
            return;

        case GenericEvent:
        {
            if (event->xcookie.extension == _glfw.x11.xi.majorOpcode &&
                XGetEventData(_glfw.x11.display, &event->xcookie))
            {
                if (event->xcookie.evtype == XI_Motion)
                {
                    XIDeviceEvent* data = (XIDeviceEvent*) event->xcookie.data;

                    window = _glfwFindWindowByHandle(data->event);
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

// Return the GLFW window corresponding to the specified X11 window
//
_GLFWwindow* _glfwFindWindowByHandle(Window handle)
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

    if (actualType != type)
        return 0;

    return itemCount;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWfbconfig* fbconfig)
{
    if (!_glfwCreateContext(window, wndconfig, fbconfig))
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

    _glfwDestroyContext(window);

    if (window->x11.handle)
    {
        if (window->x11.handle ==
            XGetSelectionOwner(_glfw.x11.display, _glfw.x11.CLIPBOARD))
        {
            _glfwPushSelectionToManager(window);
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

    if (_glfw.x11.NET_WM_NAME != None)
    {
        XChangeProperty(_glfw.x11.display,  window->x11.handle,
                        _glfw.x11.NET_WM_NAME, _glfw.x11.UTF8_STRING, 8,
                        PropModeReplace,
                        (unsigned char*) title, strlen(title));
    }

    if (_glfw.x11.NET_WM_ICON_NAME != None)
    {
        XChangeProperty(_glfw.x11.display,  window->x11.handle,
                        _glfw.x11.NET_WM_ICON_NAME, _glfw.x11.UTF8_STRING, 8,
                        PropModeReplace,
                        (unsigned char*) title, strlen(title));
    }
}

void _glfwPlatformGetWindowPos(_GLFWwindow* window, int* xpos, int* ypos)
{
    Window child;
    int x, y;

    XTranslateCoordinates(_glfw.x11.display, window->x11.handle, _glfw.x11.root,
                          0, 0, &x, &y, &child);

    if (child != None)
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

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
    if (window->x11.overrideRedirect)
    {
        // Override-redirect windows cannot be iconified or restored, as those
        // tasks are performed by the window manager
        return;
    }

    XIconifyWindow(_glfw.x11.display, window->x11.handle, _glfw.x11.screen);
}

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
    if (window->x11.overrideRedirect)
    {
        // Override-redirect windows cannot be iconified or restored, as those
        // tasks are performed by the window manager
        return;
    }

    XMapWindow(_glfw.x11.display, window->x11.handle);
}

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
    XMapRaised(_glfw.x11.display, window->x11.handle);
    XFlush(_glfw.x11.display);
}

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
    XUnmapWindow(_glfw.x11.display, window->x11.handle);
    XFlush(_glfw.x11.display);
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
    {
        int fd;
        fd_set fds;

        fd = ConnectionNumber(_glfw.x11.display);

        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        // select(1) is used instead of an X function like XNextEvent, as the
        // wait inside those are guarded by the mutex protecting the display
        // struct, locking out other threads from using X (including GLX)
        if (select(fd + 1, &fds, NULL, NULL, NULL) < 0)
            return;
    }

    _glfwPlatformPollEvents();
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double x, double y)
{
    // Store the new position so it can be recognized later
    window->x11.warpPosX = (int) x;
    window->x11.warpPosY = (int) y;

    XWarpPointer(_glfw.x11.display, None, window->x11.handle,
                 0,0,0,0, (int) x, (int) y);
}

void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode)
{
    switch (mode)
    {
        case GLFW_CURSOR_NORMAL:
            showCursor(window);
            break;
        case GLFW_CURSOR_HIDDEN:
            hideCursor(window);
            break;
        case GLFW_CURSOR_DISABLED:
            captureCursor(window);
            break;
    }
}

const char*_glfwPlatformGetKeyName(int key)
{
    int KeyCodeFound = -1;
    int printable = 0;
    int keySym, ucsChar, keyCode, length;
    char* locale;

    switch(key)
    {
		case GLFW_KEY_A:					printable=1;	break;
		case GLFW_KEY_B:                printable=1;	break;
		case GLFW_KEY_C:                printable=1;	break;
		case GLFW_KEY_D:                printable=1;	break;
		case GLFW_KEY_E:                printable=1;	break;
		case GLFW_KEY_F:                printable=1;	break;
		case GLFW_KEY_G:                printable=1;	break;
		case GLFW_KEY_H:                printable=1;	break;
		case GLFW_KEY_I:                printable=1;	break;
		case GLFW_KEY_J:                printable=1;	break;
		case GLFW_KEY_K:                printable=1;	break;
		case GLFW_KEY_L:                printable=1;	break;
		case GLFW_KEY_M:                printable=1;	break;
		case GLFW_KEY_N:                printable=1;	break;
		case GLFW_KEY_O:                printable=1;	break;
		case GLFW_KEY_P:                printable=1;	break;
		case GLFW_KEY_Q:                printable=1;	break;
		case GLFW_KEY_R:                printable=1;	break;
		case GLFW_KEY_S:                printable=1;	break;
		case GLFW_KEY_T:                printable=1;	break;
		case GLFW_KEY_U:                printable=1;	break;
		case GLFW_KEY_V:                printable=1;	break;
		case GLFW_KEY_W:                printable=1;	break;
		case GLFW_KEY_X:                printable=1;	break;
		case GLFW_KEY_Y:                printable=1;	break;
		case GLFW_KEY_Z:                printable=1;	break;
		case GLFW_KEY_1:                printable=1;	break;
		case GLFW_KEY_2:                printable=1;	break;
		case GLFW_KEY_3:                printable=1;	break;
		case GLFW_KEY_4:                printable=1;	break;
		case GLFW_KEY_5:                printable=1;	break;
		case GLFW_KEY_6:                printable=1;	break;
		case GLFW_KEY_7:                printable=1;	break;
		case GLFW_KEY_8:                printable=1;	break;
		case GLFW_KEY_9:                printable=1;	break;
		case GLFW_KEY_0:                printable=1;	break;
		case GLFW_KEY_SPACE:            printable=1;	break;
		case GLFW_KEY_MINUS:            printable=1;	break;
		case GLFW_KEY_EQUAL:            printable=1;	break;
		case GLFW_KEY_LEFT_BRACKET:     printable=1;	break;
		case GLFW_KEY_RIGHT_BRACKET:    printable=1;	break;
		case GLFW_KEY_BACKSLASH:        printable=1;	break;
		case GLFW_KEY_SEMICOLON:        printable=1;	break;
		case GLFW_KEY_APOSTROPHE:       printable=1;	break;
		case GLFW_KEY_GRAVE_ACCENT:     printable=1;	break;
		case GLFW_KEY_COMMA:            printable=1;	break;
		case GLFW_KEY_PERIOD:           printable=1;	break;
		case GLFW_KEY_SLASH:            printable=1;	break;
		case GLFW_KEY_WORLD_1: 			printable=1;	break;
		default:                		break;
    }

    if (printable)
    {
    	// We now need to search for the keyCode.
    	// This could be sped up through a table, but this function should not be called that frequently
        // Valid key code range is  [8,255], according to the XLib manual
        for(keyCode = 8; keyCode<=255; ++keyCode)
        {
        	if( translateKey(keyCode) == key )
        	{
        		KeyCodeFound = keyCode;
        		break;
        	}
        }

        if(KeyCodeFound>=0)
        {
        	// get keyboard group in use via state
            XkbStateRec state;
            XkbGetState(_glfw.x11.display, XkbUseCoreKbd, &state);

            keySym = XkbKeycodeToKeysym(_glfw.x11.display, KeyCodeFound, state.group, 0);
            ucsChar = _glfwKeySym2Unicode( keySym );
            if( ucsChar > 0 )
            {
            	// get current locale and set to for wctomb
            	locale = setlocale(LC_CTYPE,NULL);
            	setlocale(LC_CTYPE,"");
            	if(!_glfw.x11.keyName)
            	{
            		// only need to allocate once
            		_glfw.x11.keyName = calloc(MB_CUR_MAX+1,sizeof(char));
            	}
				length = wctomb(_glfw.x11.keyName, ucsChar);

				// reset locale
				setlocale(LC_CTYPE,locale);

				_glfw.x11.keyName[length]='\0';
				if(length>0)
				{
					// need to ensure common chars are interpreted similarily:
					if(length=1)
					{
						if(_glfw.x11.keyName[0]>='a' && _glfw.x11.keyName[0]<='z')
						{
							// Capitalize
							_glfw.x11.keyName[0] += 'A'-'a';
						}

						switch(_glfw.x11.keyName[0])
						{
							case ' ': 	return "SPACE";
							case '-': 	return "MINUS";
							case '=': 	return "EQUAL";
							case '[': 	return "LEFT BRACKET";
							case ']':	return "RIGHT BRACKET";
							case '\\':	return "BACKSLASH";
							case ';':   return "SEMICOLON";
							case '\'':  return "APOSTROPHE";
							case '`': 	return "GRAVE ACCENT";
							case ',':   return "COMMA";
							case '.':   return "PERIOD";
							case '/':   return "SLASH";
							default: 	break;
						}
					}
					return _glfw.x11.keyName;
				}
            }

        }
     }


    // Fallback to hard coded
    switch (key)
    {
        // Printable keys
        case GLFW_KEY_A:            return "A";
        case GLFW_KEY_B:            return "B";
        case GLFW_KEY_C:            return "C";
        case GLFW_KEY_D:            return "D";
        case GLFW_KEY_E:            return "E";
        case GLFW_KEY_F:            return "F";
        case GLFW_KEY_G:            return "G";
        case GLFW_KEY_H:            return "H";
        case GLFW_KEY_I:            return "I";
        case GLFW_KEY_J:            return "J";
        case GLFW_KEY_K:            return "K";
        case GLFW_KEY_L:            return "L";
        case GLFW_KEY_M:            return "M";
        case GLFW_KEY_N:            return "N";
        case GLFW_KEY_O:            return "O";
        case GLFW_KEY_P:            return "P";
        case GLFW_KEY_Q:            return "Q";
        case GLFW_KEY_R:            return "R";
        case GLFW_KEY_S:            return "S";
        case GLFW_KEY_T:            return "T";
        case GLFW_KEY_U:            return "U";
        case GLFW_KEY_V:            return "V";
        case GLFW_KEY_W:            return "W";
        case GLFW_KEY_X:            return "X";
        case GLFW_KEY_Y:            return "Y";
        case GLFW_KEY_Z:            return "Z";
        case GLFW_KEY_1:            return "1";
        case GLFW_KEY_2:            return "2";
        case GLFW_KEY_3:            return "3";
        case GLFW_KEY_4:            return "4";
        case GLFW_KEY_5:            return "5";
        case GLFW_KEY_6:            return "6";
        case GLFW_KEY_7:            return "7";
        case GLFW_KEY_8:            return "8";
        case GLFW_KEY_9:            return "9";
        case GLFW_KEY_0:            return "0";
        case GLFW_KEY_SPACE:        return "SPACE";
        case GLFW_KEY_MINUS:        return "MINUS";
        case GLFW_KEY_EQUAL:        return "EQUAL";
        case GLFW_KEY_LEFT_BRACKET: return "LEFT BRACKET";
        case GLFW_KEY_RIGHT_BRACKET: return "RIGHT BRACKET";
        case GLFW_KEY_BACKSLASH:    return "BACKSLASH";
        case GLFW_KEY_SEMICOLON:    return "SEMICOLON";
        case GLFW_KEY_APOSTROPHE:   return "APOSTROPHE";
        case GLFW_KEY_GRAVE_ACCENT: return "GRAVE ACCENT";
        case GLFW_KEY_COMMA:        return "COMMA";
        case GLFW_KEY_PERIOD:       return "PERIOD";
        case GLFW_KEY_SLASH:        return "SLASH";
        case GLFW_KEY_WORLD_1:      return "WORLD 1";
        case GLFW_KEY_WORLD_2:      return "WORLD 2";

        // Function keys
        case GLFW_KEY_ESCAPE:       return "ESCAPE";
        case GLFW_KEY_F1:           return "F1";
        case GLFW_KEY_F2:           return "F2";
        case GLFW_KEY_F3:           return "F3";
        case GLFW_KEY_F4:           return "F4";
        case GLFW_KEY_F5:           return "F5";
        case GLFW_KEY_F6:           return "F6";
        case GLFW_KEY_F7:           return "F7";
        case GLFW_KEY_F8:           return "F8";
        case GLFW_KEY_F9:           return "F9";
        case GLFW_KEY_F10:          return "F10";
        case GLFW_KEY_F11:          return "F11";
        case GLFW_KEY_F12:          return "F12";
        case GLFW_KEY_F13:          return "F13";
        case GLFW_KEY_F14:          return "F14";
        case GLFW_KEY_F15:          return "F15";
        case GLFW_KEY_F16:          return "F16";
        case GLFW_KEY_F17:          return "F17";
        case GLFW_KEY_F18:          return "F18";
        case GLFW_KEY_F19:          return "F19";
        case GLFW_KEY_F20:          return "F20";
        case GLFW_KEY_F21:          return "F21";
        case GLFW_KEY_F22:          return "F22";
        case GLFW_KEY_F23:          return "F23";
        case GLFW_KEY_F24:          return "F24";
        case GLFW_KEY_F25:          return "F25";
        case GLFW_KEY_UP:           return "UP";
        case GLFW_KEY_DOWN:         return "DOWN";
        case GLFW_KEY_LEFT:         return "LEFT";
        case GLFW_KEY_RIGHT:        return "RIGHT";
        case GLFW_KEY_LEFT_SHIFT:   return "LEFT SHIFT";
        case GLFW_KEY_RIGHT_SHIFT:  return "RIGHT SHIFT";
        case GLFW_KEY_LEFT_CONTROL: return "LEFT CONTROL";
        case GLFW_KEY_RIGHT_CONTROL: return "RIGHT CONTROL";
        case GLFW_KEY_LEFT_ALT:     return "LEFT ALT";
        case GLFW_KEY_RIGHT_ALT:    return "RIGHT ALT";
        case GLFW_KEY_TAB:          return "TAB";
        case GLFW_KEY_ENTER:        return "ENTER";
        case GLFW_KEY_BACKSPACE:    return "BACKSPACE";
        case GLFW_KEY_INSERT:       return "INSERT";
        case GLFW_KEY_DELETE:       return "DELETE";
        case GLFW_KEY_PAGE_UP:      return "PAGE UP";
        case GLFW_KEY_PAGE_DOWN:    return "PAGE DOWN";
        case GLFW_KEY_HOME:         return "HOME";
        case GLFW_KEY_END:          return "END";
        case GLFW_KEY_KP_0:         return "KEYPAD 0";
        case GLFW_KEY_KP_1:         return "KEYPAD 1";
        case GLFW_KEY_KP_2:         return "KEYPAD 2";
        case GLFW_KEY_KP_3:         return "KEYPAD 3";
        case GLFW_KEY_KP_4:         return "KEYPAD 4";
        case GLFW_KEY_KP_5:         return "KEYPAD 5";
        case GLFW_KEY_KP_6:         return "KEYPAD 6";
        case GLFW_KEY_KP_7:         return "KEYPAD 7";
        case GLFW_KEY_KP_8:         return "KEYPAD 8";
        case GLFW_KEY_KP_9:         return "KEYPAD 9";
        case GLFW_KEY_KP_DIVIDE:    return "KEYPAD DIVIDE";
        case GLFW_KEY_KP_MULTIPLY:  return "KEYPAD MULTPLY";
        case GLFW_KEY_KP_SUBTRACT:  return "KEYPAD SUBTRACT";
        case GLFW_KEY_KP_ADD:       return "KEYPAD ADD";
        case GLFW_KEY_KP_DECIMAL:   return "KEYPAD DECIMAL";
        case GLFW_KEY_KP_EQUAL:     return "KEYPAD EQUAL";
        case GLFW_KEY_KP_ENTER:     return "KEYPAD ENTER";
        case GLFW_KEY_PRINT_SCREEN: return "PRINT SCREEN";
        case GLFW_KEY_NUM_LOCK:     return "NUM LOCK";
        case GLFW_KEY_CAPS_LOCK:    return "CAPS LOCK";
        case GLFW_KEY_SCROLL_LOCK:  return "SCROLL LOCK";
        case GLFW_KEY_PAUSE:        return "PAUSE";
        case GLFW_KEY_LEFT_SUPER:   return "LEFT SUPER";
        case GLFW_KEY_RIGHT_SUPER:  return "RIGHT SUPER";
        case GLFW_KEY_MENU:         return "MENU";
        case GLFW_KEY_UNKNOWN:      return "UNKNOWN";

        default: break;
    }

    return NULL;
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

