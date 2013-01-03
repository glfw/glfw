//========================================================================
// GLFW - An OpenGL library
// Platform:    X11
// API version: 3.0
// WWW:         http://www.glfw.org/
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

// Action for EWMH client messages
#define _NET_WM_STATE_REMOVE        0
#define _NET_WM_STATE_ADD           1
#define _NET_WM_STATE_TOGGLE        2

// Additional mouse button names for XButtonEvent
#define Button6            6
#define Button7            7


//========================================================================
// Translates an X Window key to internal coding
//========================================================================

static int translateKey(int keycode)
{
    // Use the pre-filled LUT (see updateKeyCodeLUT() in x11_init.c)
    if ((keycode >= 0) && (keycode < 256))
        return _glfw.x11.keyCodeLUT[keycode];
    else
        return -1;
}


//========================================================================
// Translates an X Window event to Unicode
//========================================================================

static int translateChar(XKeyEvent* event)
{
    KeySym keysym;

    // Get X11 keysym
    XLookupString(event, NULL, 0, &keysym, NULL);

    // Convert to Unicode (see x11_keysym2unicode.c)
    return (int) _glfwKeySym2Unicode(keysym);
}


//========================================================================
// Create the X11 window (and its colormap)
//========================================================================

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
                        EnterWindowMask | LeaveWindowMask;

        if (wndconfig->monitor == NULL)
        {
            // The /only/ reason for setting the background pixel here is that
            // otherwise our window won't get any decorations on systems using
            // certain versions of Compiz on Intel hardware
            wa.background_pixel = BlackPixel(_glfw.x11.display,
                                             _glfw.x11.screen);
            wamask |= CWBackPixel;
        }

        window->x11.handle = XCreateWindow(_glfw.x11.display,
                                           _glfw.x11.root,
                                           wndconfig->positionX, wndconfig->positionY,
                                           window->width, window->height,
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

        // Request a window position to be set once the window is shown
        // (see _glfwPlatformShowWindow)
        window->x11.windowPosSet = GL_FALSE;
        window->x11.positionX = wndconfig->positionX;
        window->x11.positionY = wndconfig->positionY;
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

    // Find or create the protocol atom for window close notifications
    _glfw.x11.wmDeleteWindow = XInternAtom(_glfw.x11.display,
                                           "WM_DELETE_WINDOW",
                                           False);

    // Declare the WM protocols supported by GLFW
    {
        int count = 0;
        Atom protocols[2];

        // The WM_DELETE_WINDOW ICCCM protocol
        // Basic window close notification protocol
        if (_glfw.x11.wmDeleteWindow != None)
            protocols[count++] = _glfw.x11.wmDeleteWindow;

        // The _NET_WM_PING EWMH protocol
        // Tells the WM to ping the GLFW window and flag the application as
        // unresponsive if the WM doesn't get a reply within a few seconds
        if (_glfw.x11.wmPing != None)
            protocols[count++] = _glfw.x11.wmPing;

        if (count > 0)
        {
            XSetWMProtocols(_glfw.x11.display, window->x11.handle,
                            protocols, count);
        }
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
        if (!hints)
        {
            _glfwInputError(GLFW_OUT_OF_MEMORY,
                            "X11: Failed to allocate size hints");
            return GL_FALSE;
        }

        hints->flags = 0;

        if (wndconfig->monitor)
        {
            hints->flags |= PPosition;
            hints->x = wndconfig->monitor->positionX;
            hints->y = wndconfig->monitor->positionY;
        }

        if (!wndconfig->resizable)
        {
            hints->flags |= (PMinSize | PMaxSize);
            hints->min_width  = hints->max_width  = window->width;
            hints->min_height = hints->max_height = window->height;
        }

        XSetWMNormalHints(_glfw.x11.display, window->x11.handle, hints);
        XFree(hints);
    }

    _glfwPlatformSetWindowTitle(window, wndconfig->title);

    return GL_TRUE;
}


//========================================================================
// Hide cursor
//========================================================================

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


//========================================================================
// Capture cursor
//========================================================================

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
            window->x11.cursorCentered = GL_FALSE;
        }
    }
}


//========================================================================
// Show cursor
//========================================================================

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


//========================================================================
// Enter fullscreen mode
//========================================================================

static void enterFullscreenMode(_GLFWwindow* window)
{
    if (!_glfw.x11.saver.changed)
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

        _glfw.x11.saver.changed = GL_TRUE;
    }

    _glfwSetVideoMode(window->monitor, &window->width, &window->height);

    if (_glfw.x11.hasEWMH &&
        _glfw.x11.wmState != None &&
        _glfw.x11.wmStateFullscreen != None)
    {
        if (_glfw.x11.wmActiveWindow != None)
        {
            // Ask the window manager to raise and focus the GLFW window
            // Only focused windows with the _NET_WM_STATE_FULLSCREEN state end
            // up on top of all other windows ("Stacking order" in EWMH spec)

            XEvent event;
            memset(&event, 0, sizeof(event));

            event.type = ClientMessage;
            event.xclient.window = window->x11.handle;
            event.xclient.format = 32; // Data is 32-bit longs
            event.xclient.message_type = _glfw.x11.wmActiveWindow;
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
        event.xclient.message_type = _glfw.x11.wmState;
        event.xclient.data.l[0] = _NET_WM_STATE_ADD;
        event.xclient.data.l[1] = _glfw.x11.wmStateFullscreen;
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

        XRaiseWindow(_glfw.x11.display, window->x11.handle);
        XSetInputFocus(_glfw.x11.display, window->x11.handle,
                       RevertToParent, CurrentTime);
        XMoveWindow(_glfw.x11.display, window->x11.handle, 0, 0);
        XResizeWindow(_glfw.x11.display, window->x11.handle,
                      window->width, window->height);
    }

    // HACK: Try to get window inside viewport (for virtual displays) by moving
    // the cursor to the upper left corner (and then to the center)
    // This hack should be harmless on saner systems as well
    XWarpPointer(_glfw.x11.display, None, window->x11.handle, 0,0,0,0, 0,0);
    XWarpPointer(_glfw.x11.display, None, window->x11.handle, 0,0,0,0,
                 window->width / 2, window->height / 2);
}


//========================================================================
// Leave fullscreen mode
//========================================================================

static void leaveFullscreenMode(_GLFWwindow* window)
{
    _glfwRestoreVideoMode(window->monitor);

    if (_glfw.x11.saver.changed)
    {
        // Restore old screen saver settings
        XSetScreenSaver(_glfw.x11.display,
                        _glfw.x11.saver.timeout,
                        _glfw.x11.saver.interval,
                        _glfw.x11.saver.blanking,
                        _glfw.x11.saver.exposure);

        _glfw.x11.saver.changed = GL_FALSE;
    }

    if (_glfw.x11.hasEWMH &&
        _glfw.x11.wmState != None &&
        _glfw.x11.wmStateFullscreen != None)
    {
        // Ask the window manager to make the GLFW window a normal window
        // Normal windows usually have frames and other decorations

        XEvent event;
        memset(&event, 0, sizeof(event));

        event.type = ClientMessage;
        event.xclient.window = window->x11.handle;
        event.xclient.format = 32; // Data is 32-bit longs
        event.xclient.message_type = _glfw.x11.wmState;
        event.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
        event.xclient.data.l[1] = _glfw.x11.wmStateFullscreen;
        event.xclient.data.l[2] = 0; // No secondary property
        event.xclient.data.l[3] = 1; // Sender is a normal application

        XSendEvent(_glfw.x11.display,
                   _glfw.x11.root,
                   False,
                   SubstructureNotifyMask | SubstructureRedirectMask,
                   &event);
    }
}


//========================================================================
// Return the GLFW window corresponding to the specified X11 window
//========================================================================

static _GLFWwindow* findWindow(Window handle)
{
    _GLFWwindow* window;

    for (window = _glfw.windowListHead;  window;  window = window->next)
    {
        if (window->x11.handle == handle)
            return window;
    }

    return NULL;
}


//========================================================================
// Process the specified X event
//========================================================================

static void processEvent(XEvent *event)
{
    _GLFWwindow* window;

    switch (event->type)
    {
        case KeyPress:
        {
            // A keyboard key was pressed
            window = findWindow(event->xkey.window);
            if (window == NULL)
                return;

            _glfwInputKey(window, translateKey(event->xkey.keycode), GLFW_PRESS);
            _glfwInputChar(window, translateChar(&event->xkey));
            break;
        }

        case KeyRelease:
        {
            // A keyboard key was released
            window = findWindow(event->xkey.window);
            if (window == NULL)
                return;

            // Do not report key releases for key repeats. For key repeats we
            // will get KeyRelease/KeyPress pairs with similar or identical
            // time stamps. User selected key repeat filtering is handled in
            // _glfwInputKey/_glfwInputChar.
            if (XEventsQueued(_glfw.x11.display, QueuedAfterReading))
            {
                XEvent nextEvent;
                XPeekEvent(_glfw.x11.display, &nextEvent);

                if (nextEvent.type == KeyPress &&
                    nextEvent.xkey.window == event->xkey.window &&
                    nextEvent.xkey.keycode == event->xkey.keycode)
                {
                    // This last check is a hack to work around key repeats
                    // leaking through due to some sort of time drift
                    // Toshiyuki Takahashi can press a button 16 times per
                    // second so it's fairly safe to assume that no human is
                    // pressing the key 50 times per second (value is ms)
                    if ((nextEvent.xkey.time - event->xkey.time) < 20)
                    {
                        // Do not report anything for this event
                        break;
                    }
                }
            }

            _glfwInputKey(window, translateKey(event->xkey.keycode), GLFW_RELEASE);
            break;
        }

        case ButtonPress:
        {
            // A mouse button was pressed or a scrolling event occurred
            window = findWindow(event->xbutton.window);
            if (window == NULL)
                return;

            if (event->xbutton.button == Button1)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
            else if (event->xbutton.button == Button2)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS);
            else if (event->xbutton.button == Button3)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);

            // XFree86 3.3.2 and later translates mouse wheel up/down into
            // mouse button 4 & 5 presses
            else if (event->xbutton.button == Button4)
                _glfwInputScroll(window, 0.0, 1.0);
            else if (event->xbutton.button == Button5)
                _glfwInputScroll(window, 0.0, -1.0);

            else if (event->xbutton.button == Button6)
                _glfwInputScroll(window, -1.0, 0.0);
            else if (event->xbutton.button == Button7)
                _glfwInputScroll(window, 1.0, 0.0);

            break;
        }

        case ButtonRelease:
        {
            // A mouse button was released
            window = findWindow(event->xbutton.window);
            if (window == NULL)
                return;

            if (event->xbutton.button == Button1)
            {
                _glfwInputMouseClick(window,
                                     GLFW_MOUSE_BUTTON_LEFT,
                                     GLFW_RELEASE);
            }
            else if (event->xbutton.button == Button2)
            {
                _glfwInputMouseClick(window,
                                     GLFW_MOUSE_BUTTON_MIDDLE,
                                     GLFW_RELEASE);
            }
            else if (event->xbutton.button == Button3)
            {
                _glfwInputMouseClick(window,
                                     GLFW_MOUSE_BUTTON_RIGHT,
                                     GLFW_RELEASE);
            }
            break;
        }

        case EnterNotify:
        {
            // The cursor entered the window
            window = findWindow(event->xcrossing.window);
            if (window == NULL)
                return;

            if (window->cursorMode == GLFW_CURSOR_HIDDEN)
                hideCursor(window);

            _glfwInputCursorEnter(window, GL_TRUE);
            break;
        }

        case LeaveNotify:
        {
            // The cursor left the window
            window = findWindow(event->xcrossing.window);
            if (window == NULL)
                return;

            if (window->cursorMode == GLFW_CURSOR_HIDDEN)
                showCursor(window);

            _glfwInputCursorEnter(window, GL_FALSE);
            break;
        }

        case MotionNotify:
        {
            // The cursor was moved
            window = findWindow(event->xmotion.window);
            if (window == NULL)
                return;

            if (event->xmotion.x != window->x11.cursorPosX ||
                event->xmotion.y != window->x11.cursorPosY)
            {
                // The cursor was moved by something other than GLFW

                int x, y;

                if (window->cursorMode == GLFW_CURSOR_CAPTURED)
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

                window->x11.cursorPosX = event->xmotion.x;
                window->x11.cursorPosY = event->xmotion.y;
                window->x11.cursorCentered = GL_FALSE;

                _glfwInputCursorMotion(window, x, y);
            }

            break;
        }

        case ConfigureNotify:
        {
            // The window configuration changed somehow
            window = findWindow(event->xconfigure.window);
            if (window == NULL)
                return;

            _glfwInputWindowSize(window,
                                 event->xconfigure.width,
                                 event->xconfigure.height);

            _glfwInputWindowPos(window,
                                event->xconfigure.x,
                                event->xconfigure.y);

            break;
        }

        case ClientMessage:
        {
            // Custom client message, probably from the window manager
            window = findWindow(event->xclient.window);
            if (window == NULL)
                return;

            if ((Atom) event->xclient.data.l[0] == _glfw.x11.wmDeleteWindow)
            {
                // The window manager was asked to close the window, for example by
                // the user pressing a 'close' window decoration button

                _glfwInputWindowCloseRequest(window);
            }
            else if (_glfw.x11.wmPing != None &&
                     (Atom) event->xclient.data.l[0] == _glfw.x11.wmPing)
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
            // The window was mapped
            window = findWindow(event->xmap.window);
            if (window == NULL)
                return;

            _glfwInputWindowVisibility(window, GL_TRUE);
            _glfwInputWindowIconify(window, GL_FALSE);
            break;
        }

        case UnmapNotify:
        {
            // The window was unmapped
            window = findWindow(event->xmap.window);
            if (window == NULL)
                return;

            _glfwInputWindowVisibility(window, GL_FALSE);
            _glfwInputWindowIconify(window, GL_TRUE);
            break;
        }

        case FocusIn:
        {
            // The window gained focus
            window = findWindow(event->xfocus.window);
            if (window == NULL)
                return;

            _glfwInputWindowFocus(window, GL_TRUE);

            if (window->cursorMode == GLFW_CURSOR_CAPTURED)
                captureCursor(window);

            break;
        }

        case FocusOut:
        {
            // The window lost focus
            window = findWindow(event->xfocus.window);
            if (window == NULL)
                return;

            _glfwInputWindowFocus(window, GL_FALSE);

            if (window->cursorMode == GLFW_CURSOR_CAPTURED)
                showCursor(window);

            break;
        }

        case Expose:
        {
            // The window's contents was damaged
            window = findWindow(event->xexpose.window);
            if (window == NULL)
                return;

            _glfwInputWindowDamage(window);
            break;
        }

        case SelectionClear:
        {
            // The ownership of the selection was lost

            free(_glfw.x11.selection.string);
            _glfw.x11.selection.string = NULL;
            break;
        }

        case SelectionNotify:
        {
            // The selection conversion status is available

            XSelectionEvent* request = &event->xselection;

            if (_glfwReadSelection(request))
                _glfw.x11.selection.status = _GLFW_CONVERSION_SUCCEEDED;
            else
                _glfw.x11.selection.status = _GLFW_CONVERSION_FAILED;

            break;
        }

        case SelectionRequest:
        {
            // The contents of the selection was requested

            XSelectionRequestEvent* request = &event->xselectionrequest;

            XEvent response;
            memset(&response, 0, sizeof(response));

            response.xselection.property = _glfwWriteSelection(request);
            response.xselection.type = SelectionNotify;
            response.xselection.display = request->display;
            response.xselection.requestor = request->requestor;
            response.xselection.selection = request->selection;
            response.xselection.target = request->target;
            response.xselection.time = request->time;

            XSendEvent(_glfw.x11.display,
                       request->requestor,
                       False, 0, &response);
            break;
        }

        case DestroyNotify:
            return;

        default:
        {
#if defined(_GLFW_HAS_XRANDR)
            switch (event->type - _glfw.x11.randr.eventBase)
            {
                case RRScreenChangeNotify:
                {
                    XRRUpdateConfiguration(event);
                    _glfwInputMonitorChange();
                    break;
                }
            }
#endif /*_GLFW_HAS_XRANDR*/
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Here is where the window is created, and
// the OpenGL rendering context is created
//========================================================================

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

    // Retrieve and set initial cursor position
    {
        Window cursorWindow, cursorRoot;
        int windowX, windowY, rootX, rootY;
        unsigned int mask;

        XQueryPointer(_glfw.x11.display,
                      window->x11.handle,
                      &cursorRoot,
                      &cursorWindow,
                      &rootX, &rootY,
                      &windowX, &windowY,
                      &mask);

        // TODO: Probably check for some corner cases here.

        window->cursorPosX = windowX;
        window->cursorPosY = windowY;
    }

    return GL_TRUE;
}


//========================================================================
// Properly kill the window/video display
//========================================================================

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
    if (window->monitor)
        leaveFullscreenMode(window);

    _glfwDestroyContext(window);

    if (window->x11.handle)
    {
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


//========================================================================
// Set the window title
//========================================================================

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
    Atom type = XInternAtom(_glfw.x11.display, "UTF8_STRING", False);

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

    if (_glfw.x11.wmName != None)
    {
        XChangeProperty(_glfw.x11.display,  window->x11.handle,
                        _glfw.x11.wmName, type, 8,
                        PropModeReplace,
                        (unsigned char*) title, strlen(title));
    }

    if (_glfw.x11.wmIconName != None)
    {
        XChangeProperty(_glfw.x11.display,  window->x11.handle,
                        _glfw.x11.wmIconName, type, 8,
                        PropModeReplace,
                        (unsigned char*) title, strlen(title));
    }
}


//========================================================================
// Set the window size
//========================================================================

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    int mode = 0, sizeChanged = GL_FALSE;

    if (window->monitor)
    {
        // Get the closest matching video mode for the specified window size
        mode = _glfwGetClosestVideoMode(window->monitor, &width, &height);
    }

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

    if (window->monitor)
    {
        // Change window size before changing fullscreen mode?
        if (width > window->width)
        {
            XResizeWindow(_glfw.x11.display, window->x11.handle, width, height);
            sizeChanged = GL_TRUE;
        }

        _glfwSetVideoModeMODE(window->monitor, mode);
    }

    // Set window size (if not already changed)
    if (!sizeChanged)
        XResizeWindow(_glfw.x11.display, window->x11.handle, width, height);
}


//========================================================================
// Window iconification
//========================================================================

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


//========================================================================
// Window un-iconification
//========================================================================

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


//========================================================================
// Show window
//========================================================================

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
    XMapRaised(_glfw.x11.display, window->x11.handle);
    XFlush(_glfw.x11.display);

    // Set the window position the first time the window is shown
    // Note: XMoveWindow has no effect before the window has been mapped.
    if (!window->x11.windowPosSet)
    {
        XMoveWindow(_glfw.x11.display, window->x11.handle,
                    window->x11.positionX, window->x11.positionY);
        window->x11.windowPosSet = GL_TRUE;
    }
}


//========================================================================
// Hide window
//========================================================================

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
    XUnmapWindow(_glfw.x11.display, window->x11.handle);
    XFlush(_glfw.x11.display);
}


//========================================================================
// Poll for new window and input events
//========================================================================

void _glfwPlatformPollEvents(void)
{
    XEvent event;

    while (XCheckMaskEvent(_glfw.x11.display, ~0, &event) ||
           XCheckTypedEvent(_glfw.x11.display, ClientMessage, &event))
    {
        processEvent(&event);
    }

    // Check whether the cursor has moved inside an focused window that has
    // captured the cursor (because then it needs to be re-centered)

    _GLFWwindow* window;
    window = _glfw.focusedWindow;
    if (window)
    {
        if (window->cursorMode == GLFW_CURSOR_CAPTURED &&
            !window->x11.cursorCentered)
        {
            _glfwPlatformSetCursorPos(window,
                                      window->width / 2,
                                      window->height / 2);
            window->x11.cursorCentered = GL_TRUE;

            // NOTE: This is a temporary fix.  It works as long as you use
            //       offsets accumulated over the course of a frame, instead of
            //       performing the necessary actions per callback call.
            XFlush(_glfw.x11.display);
        }
    }
}


//========================================================================
// Wait for new window and input events
//========================================================================

void _glfwPlatformWaitEvents(void)
{
    int fd;
    fd_set fds;

    fd = ConnectionNumber(_glfw.x11.display);

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    XFlush(_glfw.x11.display);

    if (select(fd + 1, &fds, NULL, NULL, NULL) > 0)
        _glfwPlatformPollEvents();
}


//========================================================================
// Set physical cursor position
//========================================================================

void _glfwPlatformSetCursorPos(_GLFWwindow* window, int x, int y)
{
    // Store the new position so it can be recognized later
    window->x11.cursorPosX = x;
    window->x11.cursorPosY = y;

    XWarpPointer(_glfw.x11.display, None, window->x11.handle, 0,0,0,0, x, y);
}


//========================================================================
// Set physical cursor mode
//========================================================================

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
        case GLFW_CURSOR_CAPTURED:
            captureCursor(window);
            break;
    }
}

