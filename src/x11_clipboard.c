//========================================================================
// GLFW - An OpenGL library
// Platform:    X11
// API version: 3.0
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2010 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <limits.h>
#include <string.h>
#include <stdlib.h>


// Returns whether the event is a selection event
//
static Bool isSelectionMessage(Display* display, XEvent* event, XPointer pointer)
{
    if (event->type == SelectionRequest ||
        event->type == SelectionNotify ||
        event->type == SelectionClear)
    {
        return True;
    }

    return False;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Set the specified property to the contents of the requested selection
//
Atom _glfwWriteSelection(XSelectionRequestEvent* request)
{
    int i;
    const Atom formats[] = { _glfw.x11.UTF8_STRING,
                             _glfw.x11.COMPOUND_STRING,
                             XA_STRING };
    const int formatCount = sizeof(formats) / sizeof(formats[0]);

    if (request->property == None)
    {
        // The requestor is a legacy client (ICCCM section 2.2)
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
                                (unsigned char*) _glfw.x11.selection.string,
                                strlen(_glfw.x11.selection.string));
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
        // Conversion by clients to SAVE_TARGETS should be treated like
        // a side-effect target without side effects

        XChangeProperty(_glfw.x11.display,
                        request->requestor,
                        request->property,
                        XInternAtom(_glfw.x11.display, "NULL", False),
                        32,
                        PropModeReplace,
                        NULL,
                        0);

        return request->property;
    }

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
                            (unsigned char*) _glfw.x11.selection.string,
                            strlen(_glfw.x11.selection.string));

            return request->property;
        }
    }

    return None;
}

// Save clipboard data to clipboard manager
//
void _glfwPushSelectionToManager(_GLFWwindow* window)
{
    XEvent request;

    if (XGetSelectionOwner(_glfw.x11.display, _glfw.x11.CLIPBOARD) !=
        window->x11.handle)
    {
        // This window does not own the clipboard selection
        return;
    }

    if (XGetSelectionOwner(_glfw.x11.display, _glfw.x11.CLIPBOARD_MANAGER) ==
        None)
    {
        // There is no running clipboard manager
        return;
    }

    XConvertSelection(_glfw.x11.display,
                      _glfw.x11.CLIPBOARD_MANAGER,
                      _glfw.x11.SAVE_TARGETS,
                      None,
                      window->x11.handle,
                      CurrentTime);

    for (;;)
    {
        if (!XCheckIfEvent(_glfw.x11.display, &request, isSelectionMessage, NULL))
            continue;

        switch (request.type)
        {
            case SelectionRequest:
            {
                XEvent response;
                memset(&response, 0, sizeof(response));

                response.xselection.property = _glfwWriteSelection(&request.xselectionrequest);
                response.xselection.type = SelectionNotify;
                response.xselection.display = request.xselectionrequest.display;
                response.xselection.requestor = request.xselectionrequest.requestor;
                response.xselection.selection = request.xselectionrequest.selection;
                response.xselection.target = request.xselectionrequest.target;
                response.xselection.time = request.xselectionrequest.time;

                XSendEvent(_glfw.x11.display,
                           request.xselectionrequest.requestor,
                           False, 0, &response);

                break;
            }

            case SelectionClear:
            {
                free(_glfw.x11.selection.string);
                _glfw.x11.selection.string = NULL;
                break;
            }

            case SelectionNotify:
            {
                if (request.xselection.target == _glfw.x11.SAVE_TARGETS)
                    return;

                break;
            }
        }
    }
}



//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string)
{
    free(_glfw.x11.selection.string);
    _glfw.x11.selection.string = strdup(string);

    XSetSelectionOwner(_glfw.x11.display,
                       _glfw.x11.CLIPBOARD,
                       window->x11.handle, CurrentTime);

    if (XGetSelectionOwner(_glfw.x11.display, _glfw.x11.CLIPBOARD) !=
        window->x11.handle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "X11: Failed to become owner of the clipboard selection");
    }
}

const char* _glfwPlatformGetClipboardString(_GLFWwindow* window)
{
    int i;
    const Atom formats[] = { _glfw.x11.UTF8_STRING,
                             _glfw.x11.COMPOUND_STRING,
                             XA_STRING };

    if (_glfwFindWindowByHandle(XGetSelectionOwner(_glfw.x11.display,
                                                   _glfw.x11.CLIPBOARD)))
    {
        // Instead of doing a large number of X round-trips just to put this
        // string into a window property and then read it back, just return it
        return _glfw.x11.selection.string;
    }

    free(_glfw.x11.selection.string);
    _glfw.x11.selection.string = NULL;

    for (i = 0;  i < sizeof(formats) / sizeof(formats[0]);  i++)
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
            ;

        if (event.xselection.property == None)
            continue;

        if (_glfwGetWindowProperty(event.xselection.requestor,
                                   event.xselection.property,
                                   event.xselection.target,
                                   (unsigned char**) &data))
        {
            _glfw.x11.selection.string = strdup(data);
        }

        XFree(data);

        XDeleteProperty(_glfw.x11.display,
                        event.xselection.requestor,
                        event.xselection.property);

        if (_glfw.x11.selection.string)
            break;
    }

    if (_glfw.x11.selection.string == NULL)
    {
        _glfwInputError(GLFW_FORMAT_UNAVAILABLE,
                        "X11: Failed to convert selection to string");
    }

    return _glfw.x11.selection.string;
}

