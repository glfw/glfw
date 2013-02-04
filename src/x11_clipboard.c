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


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Set the specified property to the contents of the requested selection
//
Atom _glfwWriteSelection(XSelectionRequestEvent* request)
{
    int i;

    if (request->property == None)
    {
        // The requestor is a legacy client (ICCCM section 2.2)
        return None;
    }

    if (request->target == _glfw.x11.TARGETS)
    {
        // The list of supported targets was requested

        XChangeProperty(_glfw.x11.display,
                        request->requestor,
                        request->property,
                        XA_ATOM,
                        32,
                        PropModeReplace,
                        (unsigned char*) _glfw.x11.selection.formats,
                        _GLFW_CLIPBOARD_FORMAT_COUNT);

        return request->property;
    }

    for (i = 0;  i < _GLFW_CLIPBOARD_FORMAT_COUNT;  i++)
    {
        if (request->target == _glfw.x11.selection.formats[i])
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

    if (_glfwFindWindowByHandle(XGetSelectionOwner(_glfw.x11.display,
                                                   _glfw.x11.CLIPBOARD)))
    {
        // Instead of doing a large number of X round-trips just to put this
        // string into a window property and then read it back, just return it
        return _glfw.x11.selection.string;
    }

    free(_glfw.x11.selection.string);
    _glfw.x11.selection.string = NULL;

    for (i = 0;  i < _GLFW_CLIPBOARD_FORMAT_COUNT;  i++)
    {
        Atom actualType;
        int actualFormat;
        unsigned long itemCount, bytesAfter;
        char* data;
        XEvent event;

        XConvertSelection(_glfw.x11.display,
                          _glfw.x11.CLIPBOARD,
                          _glfw.x11.selection.formats[i],
                          _glfw.x11.selection.property,
                          window->x11.handle, CurrentTime);

        // XCheckTypedEvent is used instead of XIfEvent in order not to lock
        // other threads out from the display during the entire wait period
        while (!XCheckTypedEvent(_glfw.x11.display, SelectionNotify, &event))
            ;

        if (event.xselection.property == None)
            continue;

        XGetWindowProperty(_glfw.x11.display,
                           event.xselection.requestor,
                           event.xselection.property,
                           0, LONG_MAX,
                           False,
                           event.xselection.target,
                           &actualType,
                           &actualFormat,
                           &itemCount,
                           &bytesAfter,
                           (unsigned char**) &data);

        XDeleteProperty(_glfw.x11.display,
                        event.xselection.requestor,
                        event.xselection.property);

        if (actualType == event.xselection.target)
            _glfw.x11.selection.string = strdup(data);

        XFree(data);

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

