//========================================================================
// GLFW - An OpenGL library
// Platform:    X11/GLX
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
// Checks whether the event is a MapNotify for the specified window
//========================================================================

static Bool isMapNotify(Display* d, XEvent* e, char* arg)
{
    return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}


//========================================================================
// Translates an X Window key to internal coding
//========================================================================

static int translateKey(int keycode)
{
    // Use the pre-filled LUT (see updateKeyCodeLUT() ).
    if ((keycode >= 0) && (keycode < 256))
    {
        return _glfwLibrary.X11.keyCodeLUT[keycode];
    }
    else
    {
        return -1;
    }
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
    XEvent event;
    unsigned long wamask;
    XSetWindowAttributes wa;
    XVisualInfo* visual = _glfwGetContextVisual(window);

    // Every window needs a colormap
    // Create one based on the visual used by the current context

    window->X11.colormap = XCreateColormap(_glfwLibrary.X11.display,
                                           _glfwLibrary.X11.root,
                                           visual->visual,
                                           AllocNone);

    // Create the actual window
    {
        wamask = CWBorderPixel | CWColormap | CWEventMask;

        wa.colormap = window->X11.colormap;
        wa.border_pixel = 0;
        wa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
            PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
            ExposureMask | FocusChangeMask | VisibilityChangeMask |
            EnterWindowMask | LeaveWindowMask;

        if (wndconfig->mode == GLFW_WINDOWED)
        {
            // The /only/ reason we are setting the background pixel here is
            // that otherwise our window wont get any decorations on systems
            // using Compiz on Intel hardware
            wa.background_pixel = BlackPixel(_glfwLibrary.X11.display,
                                             _glfwLibrary.X11.screen);
            wamask |= CWBackPixel;
        }

        window->X11.handle = XCreateWindow(
            _glfwLibrary.X11.display,
            _glfwLibrary.X11.root,
            0, 0,                            // Upper left corner of this window on root
            window->width, window->height,
            0,                               // Border width
            visual->depth,                   // Color depth
            InputOutput,
            visual->visual,
            wamask,
            &wa
        );

        if (!window->X11.handle)
        {
            // TODO: Handle all the various error codes here

            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "X11/GLX: Failed to create window");
            return GL_FALSE;
        }
    }

    if (window->mode == GLFW_FULLSCREEN && !_glfwLibrary.X11.hasEWMH)
    {
        // This is the butcher's way of removing window decorations
        // Setting the override-redirect attribute on a window makes the window
        // manager ignore the window completely (ICCCM, section 4)
        // The good thing is that this makes undecorated fullscreen windows
        // easy to do; the bad thing is that we have to do everything manually
        // and some things (like iconify/restore) won't work at all, as they're
        // usually performed by the window manager

        XSetWindowAttributes attributes;
        attributes.override_redirect = True;
        XChangeWindowAttributes(_glfwLibrary.X11.display,
                                window->X11.handle,
                                CWOverrideRedirect,
                                &attributes);

        window->X11.overrideRedirect = GL_TRUE;
    }

    // Find or create the protocol atom for window close notifications
    _glfwLibrary.X11.wmDeleteWindow = XInternAtom(_glfwLibrary.X11.display,
                                                  "WM_DELETE_WINDOW",
                                                  False);

    // Declare the WM protocols we support
    {
        int count = 0;
        Atom protocols[2];

        // The WM_DELETE_WINDOW ICCCM protocol
        // Basic window close notification protocol
        if (_glfwLibrary.X11.wmDeleteWindow != None)
            protocols[count++] = _glfwLibrary.X11.wmDeleteWindow;

        // The _NET_WM_PING EWMH protocol
        // Tells the WM to ping our window and flag us as unresponsive if we
        // don't reply within a few seconds
        if (_glfwLibrary.X11.wmPing != None)
            protocols[count++] = _glfwLibrary.X11.wmPing;

        if (count > 0)
        {
            XSetWMProtocols(_glfwLibrary.X11.display, window->X11.handle,
                            protocols, count);
        }
    }

    // Set ICCCM WM_HINTS property
    {
        XWMHints* hints = XAllocWMHints();
        if (!hints)
        {
            _glfwSetError(GLFW_OUT_OF_MEMORY,
                          "X11/GLX: Failed to allocate WM hints");
            return GL_FALSE;
        }

        hints->flags = StateHint;
        hints->initial_state = NormalState;

        XSetWMHints(_glfwLibrary.X11.display, window->X11.handle, hints);
        XFree(hints);
    }

    // Set ICCCM WM_NORMAL_HINTS property (even if no parts are set)
    {
        XSizeHints* hints = XAllocSizeHints();
        if (!hints)
        {
            _glfwSetError(GLFW_OUT_OF_MEMORY,
                          "X11/GLX: Failed to allocate size hints");
            return GL_FALSE;
        }

        hints->flags = 0;

        if (!wndconfig->resizable)
        {
            hints->flags |= (PMinSize | PMaxSize);
            hints->min_width  = hints->max_width  = window->width;
            hints->min_height = hints->max_height = window->height;
        }

        XSetWMNormalHints(_glfwLibrary.X11.display, window->X11.handle, hints);
        XFree(hints);
    }

    _glfwPlatformSetWindowTitle(window, wndconfig->title);

    // Make sure the window is mapped before proceeding
    XMapWindow(_glfwLibrary.X11.display, window->X11.handle);
    XPeekIfEvent(_glfwLibrary.X11.display, &event, isMapNotify,
                 (char*) window->X11.handle);

    return GL_TRUE;
}


//========================================================================
// Hide mouse cursor
//========================================================================

static void hideMouseCursor(_GLFWwindow* window)
{
    if (!window->X11.cursorHidden)
    {
        XDefineCursor(_glfwLibrary.X11.display,
                      window->X11.handle,
                      _glfwLibrary.X11.cursor);
        window->X11.cursorHidden = GL_TRUE;
    }
}


//========================================================================
// Capture mouse cursor
//========================================================================

static void captureMouseCursor(_GLFWwindow* window)
{
    hideMouseCursor(window);

    if (!window->X11.cursorGrabbed)
    {
        if (XGrabPointer(_glfwLibrary.X11.display, window->X11.handle, True,
                         ButtonPressMask | ButtonReleaseMask |
                         PointerMotionMask, GrabModeAsync, GrabModeAsync,
                         window->X11.handle, None, CurrentTime) ==
            GrabSuccess)
        {
            window->X11.cursorGrabbed = GL_TRUE;
            window->X11.cursorCentered = GL_FALSE;
        }
    }
}


//========================================================================
// Show mouse cursor
//========================================================================

static void showMouseCursor(_GLFWwindow* window)
{
    // Un-grab cursor (only in windowed mode: in fullscreen mode we still
    // want the mouse grabbed in order to confine the cursor to the window
    // area)
    if (window->X11.cursorGrabbed)
    {
        XUngrabPointer(_glfwLibrary.X11.display, CurrentTime);
        window->X11.cursorGrabbed = GL_FALSE;
    }

    // Show cursor
    if (window->X11.cursorHidden)
    {
        XUndefineCursor(_glfwLibrary.X11.display, window->X11.handle);
        window->X11.cursorHidden = GL_FALSE;
    }
}


//========================================================================
// Enter fullscreen mode
//========================================================================

static void enterFullscreenMode(_GLFWwindow* window)
{
    if (!_glfwLibrary.X11.saver.changed)
    {
        // Remember old screen saver settings
        XGetScreenSaver(_glfwLibrary.X11.display,
                        &_glfwLibrary.X11.saver.timeout,
                        &_glfwLibrary.X11.saver.interval,
                        &_glfwLibrary.X11.saver.blanking,
                        &_glfwLibrary.X11.saver.exposure);

        // Disable screen saver
        XSetScreenSaver(_glfwLibrary.X11.display, 0, 0, DontPreferBlanking,
                        DefaultExposures);

        _glfwLibrary.X11.saver.changed = GL_TRUE;
    }

    _glfwSetVideoMode(&window->width, &window->height,
                      &window->refreshRate);

    if (_glfwLibrary.X11.hasEWMH &&
        _glfwLibrary.X11.wmState != None &&
        _glfwLibrary.X11.wmStateFullscreen != None)
    {
        if (_glfwLibrary.X11.wmActiveWindow != None)
        {
            // Ask the window manager to raise and focus the GLFW window
            // Only focused windows with the _NET_WM_STATE_FULLSCREEN state end
            // up on top of all other windows ("Stacking order" in EWMH spec)

            XEvent event;
            memset(&event, 0, sizeof(event));

            event.type = ClientMessage;
            event.xclient.window = window->X11.handle;
            event.xclient.format = 32; // Data is 32-bit longs
            event.xclient.message_type = _glfwLibrary.X11.wmActiveWindow;
            event.xclient.data.l[0] = 1; // Sender is a normal application
            event.xclient.data.l[1] = 0; // We don't really know the timestamp

            XSendEvent(_glfwLibrary.X11.display,
                       _glfwLibrary.X11.root,
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
        event.xclient.window = window->X11.handle;
        event.xclient.format = 32; // Data is 32-bit longs
        event.xclient.message_type = _glfwLibrary.X11.wmState;
        event.xclient.data.l[0] = _NET_WM_STATE_ADD;
        event.xclient.data.l[1] = _glfwLibrary.X11.wmStateFullscreen;
        event.xclient.data.l[2] = 0; // No secondary property
        event.xclient.data.l[3] = 1; // Sender is a normal application

        XSendEvent(_glfwLibrary.X11.display,
                   _glfwLibrary.X11.root,
                   False,
                   SubstructureNotifyMask | SubstructureRedirectMask,
                   &event);
    }
    else if (window->X11.overrideRedirect)
    {
        // In override-redirect mode, we have divorced ourselves from the
        // window manager, so we need to do everything manually

        XRaiseWindow(_glfwLibrary.X11.display, window->X11.handle);
        XSetInputFocus(_glfwLibrary.X11.display, window->X11.handle,
                        RevertToParent, CurrentTime);
        XMoveWindow(_glfwLibrary.X11.display, window->X11.handle, 0, 0);
        XResizeWindow(_glfwLibrary.X11.display, window->X11.handle,
                      window->width, window->height);
    }

    // HACK: Try to get window inside viewport (for virtual displays) by moving
    // the mouse cursor to the upper left corner (and then to the center)
    // This hack should be harmless on saner systems as well
    XWarpPointer(_glfwLibrary.X11.display, None, window->X11.handle, 0,0,0,0, 0,0);
    XWarpPointer(_glfwLibrary.X11.display, None, window->X11.handle, 0,0,0,0,
                 window->width / 2, window->height / 2);
}


//========================================================================
// Leave fullscreen mode
//========================================================================

static void leaveFullscreenMode(_GLFWwindow* window)
{
    _glfwRestoreVideoMode();

    // Did we change the screen saver setting?
    if (_glfwLibrary.X11.saver.changed)
    {
        // Restore old screen saver settings
        XSetScreenSaver(_glfwLibrary.X11.display,
                        _glfwLibrary.X11.saver.timeout,
                        _glfwLibrary.X11.saver.interval,
                        _glfwLibrary.X11.saver.blanking,
                        _glfwLibrary.X11.saver.exposure);

        _glfwLibrary.X11.saver.changed = GL_FALSE;
    }

    if (_glfwLibrary.X11.hasEWMH &&
        _glfwLibrary.X11.wmState != None &&
        _glfwLibrary.X11.wmStateFullscreen != None)
    {
        // Ask the window manager to make the GLFW window a normal window
        // Normal windows usually have frames and other decorations

        XEvent event;
        memset(&event, 0, sizeof(event));

        event.type = ClientMessage;
        event.xclient.window = window->X11.handle;
        event.xclient.format = 32; // Data is 32-bit longs
        event.xclient.message_type = _glfwLibrary.X11.wmState;
        event.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
        event.xclient.data.l[1] = _glfwLibrary.X11.wmStateFullscreen;
        event.xclient.data.l[2] = 0; // No secondary property
        event.xclient.data.l[3] = 1; // Sender is a normal application

        XSendEvent(_glfwLibrary.X11.display,
                   _glfwLibrary.X11.root,
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

    for (window = _glfwLibrary.windowListHead;  window;  window = window->next)
    {
        if (window->X11.handle == handle)
            return window;
    }

    return NULL;
}


//========================================================================
// Get and process next X event (called by _glfwPlatformPollEvents)
//========================================================================

static void processSingleEvent(void)
{
    _GLFWwindow* window;

    XEvent event;
    XNextEvent(_glfwLibrary.X11.display, &event);

    switch (event.type)
    {
        case KeyPress:
        {
            // A keyboard key was pressed
            window = findWindow(event.xkey.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for KeyPress event\n");
                return;
            }

            // Translate and report key press
            _glfwInputKey(window, translateKey(event.xkey.keycode), GLFW_PRESS);

            // Translate and report character input
            _glfwInputChar(window, translateChar(&event.xkey));

            break;
        }

        case KeyRelease:
        {
            // A keyboard key was released
            window = findWindow(event.xkey.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for KeyRelease event\n");
                return;
            }

            // Do not report key releases for key repeats. For key repeats we
            // will get KeyRelease/KeyPress pairs with similar or identical
            // time stamps. User selected key repeat filtering is handled in
            // _glfwInputKey/_glfwInputChar.
            if (XEventsQueued(_glfwLibrary.X11.display, QueuedAfterReading))
            {
                XEvent nextEvent;
                XPeekEvent(_glfwLibrary.X11.display, &nextEvent);

                if (nextEvent.type == KeyPress &&
                    nextEvent.xkey.window == event.xkey.window &&
                    nextEvent.xkey.keycode == event.xkey.keycode)
                {
                    // This last check is a hack to work around key repeats
                    // leaking through due to some sort of time drift
                    // Toshiyuki Takahashi can press a button 16 times per
                    // second so it's fairly safe to assume that no human is
                    // pressing the key 50 times per second (value is ms)
                    if ((nextEvent.xkey.time - event.xkey.time) < 20)
                    {
                        // Do not report anything for this event
                        break;
                    }
                }
            }

            // Translate and report key release
            _glfwInputKey(window, translateKey(event.xkey.keycode), GLFW_RELEASE);

            break;
        }

        case ButtonPress:
        {
            // A mouse button was pressed or a scrolling event occurred
            window = findWindow(event.xbutton.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for ButtonPress event\n");
                return;
            }

            if (event.xbutton.button == Button1)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
            else if (event.xbutton.button == Button2)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS);
            else if (event.xbutton.button == Button3)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);

            // XFree86 3.3.2 and later translates mouse wheel up/down into
            // mouse button 4 & 5 presses
            else if (event.xbutton.button == Button4)
                _glfwInputScroll(window, 0.0, 1.0);
            else if (event.xbutton.button == Button5)
                _glfwInputScroll(window, 0.0, -1.0);

            else if (event.xbutton.button == Button6)
                _glfwInputScroll(window, -1.0, 0.0);
            else if (event.xbutton.button == Button7)
                _glfwInputScroll(window, 1.0, 0.0);

            break;
        }

        case ButtonRelease:
        {
            // A mouse button was released
            window = findWindow(event.xbutton.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for ButtonRelease event\n");
                return;
            }

            if (event.xbutton.button == Button1)
            {
                _glfwInputMouseClick(window,
                                     GLFW_MOUSE_BUTTON_LEFT,
                                     GLFW_RELEASE);
            }
            else if (event.xbutton.button == Button2)
            {
                _glfwInputMouseClick(window,
                                     GLFW_MOUSE_BUTTON_MIDDLE,
                                     GLFW_RELEASE);
            }
            else if (event.xbutton.button == Button3)
            {
                _glfwInputMouseClick(window,
                                     GLFW_MOUSE_BUTTON_RIGHT,
                                     GLFW_RELEASE);
            }
            break;
        }

        case EnterNotify:
        {
            // The mouse cursor enters the Window
            window = findWindow(event.xcrossing.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for EnterNotify event\n");
                return;
            }

            if (window->cursorMode == GLFW_CURSOR_HIDDEN)
                hideMouseCursor(window);

            _glfwInputCursorEnter(window, GL_TRUE);
            break;
        }

        case LeaveNotify:
        {
            // The mouse cursor leave the Window
            window = findWindow(event.xcrossing.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for LeaveNotify event\n");
                return;
            }

            if (window->cursorMode == GLFW_CURSOR_HIDDEN)
                showMouseCursor(window);

            _glfwInputCursorEnter(window, GL_FALSE);
            break;
        }

        case MotionNotify:
        {
            // The mouse cursor was moved
            window = findWindow(event.xmotion.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for MotionNotify event\n");
                return;
            }

            if (event.xmotion.x != window->X11.cursorPosX ||
                event.xmotion.y != window->X11.cursorPosY)
            {
                // The mouse cursor was moved and we didn't do it
                int x, y;

                if (window->cursorMode == GLFW_CURSOR_CAPTURED)
                {
                    if (_glfwLibrary.activeWindow != window)
                        break;

                    x = event.xmotion.x - window->X11.cursorPosX;
                    y = event.xmotion.y - window->X11.cursorPosY;
                }
                else
                {
                    x = event.xmotion.x;
                    y = event.xmotion.y;
                }

                window->X11.cursorPosX = event.xmotion.x;
                window->X11.cursorPosY = event.xmotion.y;
                window->X11.cursorCentered = GL_FALSE;

                _glfwInputCursorMotion(window, x, y);
            }

            break;
        }

        case ConfigureNotify:
        {
            // The window configuration changed somehow
            window = findWindow(event.xconfigure.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for ConfigureNotify event\n");
                return;
            }

            _glfwInputWindowSize(window,
                                 event.xconfigure.width,
                                 event.xconfigure.height);

            _glfwInputWindowPos(window,
                                event.xconfigure.x,
                                event.xconfigure.y);

            break;
        }

        case ClientMessage:
        {
            // Custom client message, probably from the window manager
            window = findWindow(event.xclient.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for ClientMessage event\n");
                return;
            }

            if ((Atom) event.xclient.data.l[0] == _glfwLibrary.X11.wmDeleteWindow)
            {
                // The window manager was asked to close the window, for example by
                // the user pressing a 'close' window decoration button

                window->closeRequested = GL_TRUE;
            }
            else if (_glfwLibrary.X11.wmPing != None &&
                     (Atom) event.xclient.data.l[0] == _glfwLibrary.X11.wmPing)
            {
                // The window manager is pinging us to make sure we are still
                // responding to events

                event.xclient.window = _glfwLibrary.X11.root;
                XSendEvent(_glfwLibrary.X11.display,
                           event.xclient.window,
                           False,
                           SubstructureNotifyMask | SubstructureRedirectMask,
                           &event);
            }

            break;
        }

        case MapNotify:
        {
            // The window was mapped
            window = findWindow(event.xmap.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for MapNotify event\n");
                return;
            }

            _glfwInputWindowIconify(window, GL_FALSE);
            break;
        }

        case UnmapNotify:
        {
            // The window was unmapped
            window = findWindow(event.xmap.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for UnmapNotify event\n");
                return;
            }

            _glfwInputWindowIconify(window, GL_TRUE);
            break;
        }

        case FocusIn:
        {
            // The window gained focus
            window = findWindow(event.xfocus.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for FocusIn event\n");
                return;
            }

            _glfwInputWindowFocus(window, GL_TRUE);

            if (window->cursorMode == GLFW_CURSOR_CAPTURED)
                captureMouseCursor(window);

            break;
        }

        case FocusOut:
        {
            // The window lost focus
            window = findWindow(event.xfocus.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for FocusOut event\n");
                return;
            }

            _glfwInputWindowFocus(window, GL_FALSE);

            if (window->cursorMode == GLFW_CURSOR_CAPTURED)
                showMouseCursor(window);

            break;
        }

        case Expose:
        {
            // The window's contents was damaged
            window = findWindow(event.xexpose.window);
            if (window == NULL)
            {
                fprintf(stderr, "Cannot find GLFW window structure for Expose event\n");
                return;
            }

            _glfwInputWindowDamage(window);
            break;
        }

        case SelectionClear:
        {
            // The ownership of the selection was lost

            free(_glfwLibrary.X11.selection.string);
            _glfwLibrary.X11.selection.string = NULL;
            break;
        }

        case SelectionNotify:
        {
            // The selection conversion status is available

            XSelectionEvent* request = &event.xselection;

            if (_glfwReadSelection(request))
                _glfwLibrary.X11.selection.status = _GLFW_CONVERSION_SUCCEEDED;
            else
                _glfwLibrary.X11.selection.status = _GLFW_CONVERSION_FAILED;

            break;
        }

        case SelectionRequest:
        {
            // The contents of the selection was requested

            XSelectionRequestEvent* request = &event.xselectionrequest;

            XEvent response;
            memset(&response, 0, sizeof(response));

            response.xselection.property = _glfwWriteSelection(request);
            response.xselection.type = SelectionNotify;
            response.xselection.display = request->display;
            response.xselection.requestor = request->requestor;
            response.xselection.selection = request->selection;
            response.xselection.target = request->target;
            response.xselection.time = request->time;

            XSendEvent(_glfwLibrary.X11.display,
                       request->requestor,
                       False, 0, &response);
            break;
        }

        case DestroyNotify:
            return;

        default:
        {
#if defined(_GLFW_HAS_XRANDR)
            switch (event.type - _glfwLibrary.X11.RandR.eventBase)
            {
                case RRScreenChangeNotify:
                {
                    // Show XRandR that we really care
                    XRRUpdateConfiguration(&event);
                    break;
                }
            }
#endif /*_GLFW_HAS_XRANDR*/
            break;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Processes all pending events
//========================================================================

void _glfwProcessPendingEvents(void)
{
    int i, count = XPending(_glfwLibrary.X11.display);

    for (i = 0;  i < count;  i++)
        processSingleEvent();
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Here is where the window is created, and
// the OpenGL rendering context is created
//========================================================================

int _glfwPlatformOpenWindow(_GLFWwindow* window,
                            const _GLFWwndconfig* wndconfig,
                            const _GLFWfbconfig* fbconfig)
{
    window->refreshRate = wndconfig->refreshRate;
    window->resizable   = wndconfig->resizable;

    if (!_glfwCreateContext(window, wndconfig, fbconfig))
        return GL_FALSE;

    if (!createWindow(window, wndconfig))
        return GL_FALSE;

    if (wndconfig->mode == GLFW_FULLSCREEN)
    {
#if defined(_GLFW_HAS_XRANDR)
        // Request screen change notifications
        if (_glfwLibrary.X11.RandR.available)
        {
            XRRSelectInput(_glfwLibrary.X11.display,
                           window->X11.handle,
                           RRScreenChangeNotifyMask);
        }
#endif /*_GLFW_HAS_XRANDR*/

        enterFullscreenMode(window);
    }

    // Process the window map event and any other that may have arrived
    _glfwProcessPendingEvents();

    // Retrieve and set initial cursor position
    {
        Window cursorWindow, cursorRoot;
        int windowX, windowY, rootX, rootY;
        unsigned int mask;

        XQueryPointer(_glfwLibrary.X11.display,
                      window->X11.handle,
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

void _glfwPlatformCloseWindow(_GLFWwindow* window)
{
    if (window->mode == GLFW_FULLSCREEN)
        leaveFullscreenMode(window);

    _glfwDestroyContext(window);

    if (window->X11.handle)
    {
        XUnmapWindow(_glfwLibrary.X11.display, window->X11.handle);
        XDestroyWindow(_glfwLibrary.X11.display, window->X11.handle);
        window->X11.handle = (Window) 0;
    }

    if (window->X11.colormap)
    {
        XFreeColormap(_glfwLibrary.X11.display, window->X11.colormap);
        window->X11.colormap = (Colormap) 0;
    }
}


//========================================================================
// Set the window title
//========================================================================

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
    Atom type = XInternAtom(_glfwLibrary.X11.display, "UTF8_STRING", False);

#if defined(X_HAVE_UTF8_STRING)
    Xutf8SetWMProperties(_glfwLibrary.X11.display,
                         window->X11.handle,
                         title, title,
                         NULL, 0,
                         NULL, NULL, NULL);
#else
    // This may be a slightly better fallback than using XStoreName and
    // XSetIconName, which always store their arguments using STRING
    XmbSetWMProperties(_glfwLibrary.X11.display,
                       window->X11.handle,
                       title, title,
                       NULL, 0,
                       NULL, NULL, NULL);
#endif

    if (_glfwLibrary.X11.wmName != None)
    {
        XChangeProperty(_glfwLibrary.X11.display,  window->X11.handle,
                        _glfwLibrary.X11.wmName, type, 8,
                        PropModeReplace,
                        (unsigned char*) title, strlen(title));
    }

    if (_glfwLibrary.X11.wmIconName != None)
    {
        XChangeProperty(_glfwLibrary.X11.display,  window->X11.handle,
                        _glfwLibrary.X11.wmIconName, type, 8,
                        PropModeReplace,
                        (unsigned char*) title, strlen(title));
    }
}


//========================================================================
// Set the window size
//========================================================================

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    int mode = 0, rate, sizeChanged = GL_FALSE;
    XSizeHints* sizehints;

    rate = window->refreshRate;

    if (window->mode == GLFW_FULLSCREEN)
    {
        // Get the closest matching video mode for the specified window size
        mode = _glfwGetClosestVideoMode(&width, &height, &rate);
    }

    if (!window->resizable)
    {
        // Update window size restrictions to match new window size

        sizehints = XAllocSizeHints();
        sizehints->flags = 0;

        sizehints->min_width  = sizehints->max_width  = width;
        sizehints->min_height = sizehints->max_height = height;

        XSetWMNormalHints(_glfwLibrary.X11.display, window->X11.handle, sizehints);
        XFree(sizehints);
    }

    // Change window size before changing fullscreen mode?
    if (window->mode == GLFW_FULLSCREEN && (width > window->width))
    {
        XResizeWindow(_glfwLibrary.X11.display, window->X11.handle, width, height);
        sizeChanged = GL_TRUE;
    }

    if (window->mode == GLFW_FULLSCREEN)
    {
        // Change video mode, keeping current refresh rate
        _glfwSetVideoModeMODE(mode, window->refreshRate);
    }

    // Set window size (if not already changed)
    if (!sizeChanged)
        XResizeWindow(_glfwLibrary.X11.display, window->X11.handle, width, height);
}


//========================================================================
// Set the window position.
//========================================================================

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int x, int y)
{
    XMoveWindow(_glfwLibrary.X11.display, window->X11.handle, x, y);
}


//========================================================================
// Window iconification
//========================================================================

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
    if (window->X11.overrideRedirect)
    {
        // We can't iconify/restore override-redirect windows, as that's
        // performed by the window manager
        return;
    }

    XIconifyWindow(_glfwLibrary.X11.display,
                   window->X11.handle,
                   _glfwLibrary.X11.screen);
}


//========================================================================
// Window un-iconification
//========================================================================

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
    if (window->X11.overrideRedirect)
    {
        // We can't iconify/restore override-redirect windows, as that's
        // performed by the window manager
        return;
    }

    XMapWindow(_glfwLibrary.X11.display, window->X11.handle);
}


//========================================================================
// Read back framebuffer parameters from the context
//========================================================================

void _glfwPlatformRefreshWindowParams(void)
{
#if defined(_GLFW_HAS_XRANDR)
    XRRScreenConfiguration* sc;
#endif /*_GLFW_HAS_XRANDR*/
#if defined(_GLFW_HAS_XF86VIDMODE)
    XF86VidModeModeLine modeline;
    int dotclock;
    float pixels_per_second, pixels_per_frame;
#endif /*_GLFW_HAS_XF86VIDMODE*/
    _GLFWwindow* window = _glfwLibrary.currentWindow;

    // Retrieve refresh rate if possible
    if (_glfwLibrary.X11.RandR.available)
    {
#if defined(_GLFW_HAS_XRANDR)
        sc = XRRGetScreenInfo(_glfwLibrary.X11.display, _glfwLibrary.X11.root);
        window->refreshRate = XRRConfigCurrentRate(sc);
        XRRFreeScreenConfigInfo(sc);
#endif /*_GLFW_HAS_XRANDR*/
    }
    else if (_glfwLibrary.X11.VidMode.available)
    {
#if defined(_GLFW_HAS_XF86VIDMODE)
        // Use the XF86VidMode extension to get current video mode
        XF86VidModeGetModeLine(_glfwLibrary.X11.display,
                               _glfwLibrary.X11.screen,
                               &dotclock, &modeline);
        pixels_per_second = 1000.0f * (float) dotclock;
        pixels_per_frame  = (float) modeline.htotal * modeline.vtotal;
        window->refreshRate = (int) (pixels_per_second / pixels_per_frame + 0.5);
#endif /*_GLFW_HAS_XF86VIDMODE*/
    }
    else
    {
        // Zero means unknown according to the GLFW spec
        window->refreshRate = 0;
    }
}


//========================================================================
// Poll for new window and input events
//========================================================================

void _glfwPlatformPollEvents(void)
{
    _GLFWwindow* window;

    // Process all pending events
    while (XPending(_glfwLibrary.X11.display))
        processSingleEvent();

    // Did the cursor move in an active window that has captured the cursor
    window = _glfwLibrary.activeWindow;
    if (window)
    {
        if (window->cursorMode == GLFW_CURSOR_CAPTURED &&
            !window->X11.cursorCentered)
        {
            _glfwPlatformSetMouseCursorPos(window,
                                           window->width / 2,
                                           window->height / 2);
            window->X11.cursorCentered = GL_TRUE;

            // NOTE: This is a temporary fix.  It works as long as you use
            //       offsets accumulated over the course of a frame, instead of
            //       performing the necessary actions per callback call.
            XFlush( _glfwLibrary.X11.display );
        }
    }
}


//========================================================================
// Wait for new window and input events
//========================================================================

void _glfwPlatformWaitEvents(void)
{
    XEvent event;

    // Block waiting for an event to arrive
    XNextEvent(_glfwLibrary.X11.display, &event);
    XPutBackEvent(_glfwLibrary.X11.display, &event);

    _glfwPlatformPollEvents();
}


//========================================================================
// Set physical mouse cursor position
//========================================================================

void _glfwPlatformSetMouseCursorPos(_GLFWwindow* window, int x, int y)
{
    // Store the new position so we can recognise it later
    window->X11.cursorPosX = x;
    window->X11.cursorPosY = y;

    XWarpPointer(_glfwLibrary.X11.display, None, window->X11.handle, 0,0,0,0, x, y);
}


//========================================================================
// Set physical mouse cursor mode
//========================================================================

void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode)
{
    switch (mode)
    {
        case GLFW_CURSOR_NORMAL:
            showMouseCursor(window);
            break;
        case GLFW_CURSOR_HIDDEN:
            hideMouseCursor(window);
            break;
        case GLFW_CURSOR_CAPTURED:
            captureMouseCursor(window);
            break;
    }
}

