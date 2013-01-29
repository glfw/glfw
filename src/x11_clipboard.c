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

//========================================================================
// Save the contents of the specified property
//========================================================================

GLboolean _glfwReadSelection(XSelectionEvent* request)
{
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;
    char* data;

    if (request->property == None)
        return GL_FALSE;

    XGetWindowProperty(_glfw.x11.display,
                       request->requestor,
                       request->property,
                       0, LONG_MAX,
                       False,
                       request->target,
                       &actualType,
                       &actualFormat,
                       &itemCount,
                       &bytesAfter,
                       (unsigned char**) &data);

    if (actualType == None)
        return GL_FALSE;

    free(_glfw.x11.selection.string);
    _glfw.x11.selection.string = strdup(data);

    XFree(data);
    return GL_TRUE;
}


//========================================================================
// Set the specified property to the contents of the requested selection
//========================================================================

Atom _glfwWriteSelection(XSelectionRequestEvent* request)
{
    int i;
    Atom property = request->property;

    if (property == None)
        property = _glfw.x11.selection.property;

    if (request->target == _glfw.x11.TARGETS)
    {
        // The list of supported targets was requested

        XChangeProperty(_glfw.x11.display,
                        request->requestor,
                        property,
                        XA_ATOM,
                        32,
                        PropModeReplace,
                        (unsigned char*) _glfw.x11.selection.formats,
                        _GLFW_CLIPBOARD_FORMAT_COUNT);

        return property;
    }

    for (i = 0;  i < _GLFW_CLIPBOARD_FORMAT_COUNT;  i++)
    {
        if (request->target == _glfw.x11.selection.formats[i])
        {
            // The requested target is one we support

            XChangeProperty(_glfw.x11.display,
                            request->requestor,
                            property,
                            request->target,
                            8,
                            PropModeReplace,
                            (unsigned char*) _glfw.x11.selection.string,
                            strlen(_glfw.x11.selection.string));

            return property;
        }
    }

    return None;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string)
{
    // Store the new string in preparation for a selection request event
    free(_glfw.x11.selection.string);
    _glfw.x11.selection.string = strdup(string);

    // Set the specified window as owner of the selection
    XSetSelectionOwner(_glfw.x11.display,
                       _glfw.x11.CLIPBOARD,
                       window->x11.handle, CurrentTime);
}

const char* _glfwPlatformGetClipboardString(_GLFWwindow* window)
{
    int i;

    _glfw.x11.selection.status = _GLFW_CONVERSION_INACTIVE;

    for (i = 0;  i < _GLFW_CLIPBOARD_FORMAT_COUNT;  i++)
    {
        // Request conversion to the selected format
        _glfw.x11.selection.target = _glfw.x11.selection.formats[i];

        XConvertSelection(_glfw.x11.display,
                          _glfw.x11.CLIPBOARD,
                          _glfw.x11.selection.target,
                          _glfw.x11.selection.property,
                          window->x11.handle, CurrentTime);

        // Process the resulting SelectionNotify event
        XSync(_glfw.x11.display, False);
        while (_glfw.x11.selection.status == _GLFW_CONVERSION_INACTIVE)
            _glfwPlatformWaitEvents();

        if (_glfw.x11.selection.status == _GLFW_CONVERSION_SUCCEEDED)
            break;
    }

    if (_glfw.x11.selection.status == _GLFW_CONVERSION_FAILED)
    {
        _glfwInputError(GLFW_FORMAT_UNAVAILABLE,
                        "X11: Failed to convert selection to string");
        return NULL;
    }

    return _glfw.x11.selection.string;
}

