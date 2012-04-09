//========================================================================
// GLFW - An OpenGL library
// Platform:    X11/GLX
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

// TODO: Incremental support? Overkill perhaps.

#include "internal.h"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// X11 selection request event
//========================================================================

Atom _glfwSelectionRequest(XSelectionRequestEvent* request)
{
    Atom* formats = _glfwLibrary.X11.selection.formats;
    char* target = _glfwLibrary.X11.selection.string;

    if (request->target == XA_STRING)
    {
        // TODO: ISO Latin-1 specific characters don't get converted
        // (yet). For cleanliness, would we need something like iconv?
        XChangeProperty(_glfwLibrary.X11.display,
                        request->requestor,
                        request->target,
                        request->target,
                        8,
                        PropModeReplace,
                        (unsigned char*) target,
                        8);
    }
    else if (request->target == formats[_GLFW_CLIPBOARD_FORMAT_COMPOUND] ||
             request->target == formats[_GLFW_CLIPBOARD_FORMAT_UTF8])
    {
        XChangeProperty(_glfwLibrary.X11.display,
                        request->requestor,
                        request->target,
                        request->target,
                        8,
                        PropModeReplace,
                        (unsigned char*) target,
                        _glfwLibrary.X11.selection.stringLength);
    }
    else
    {
        // TODO: Should we set an error? Probably not.
        return None;
    }

    return request->target;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Set the clipboard contents
//========================================================================

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string)
{
    size_t size = strlen(string) + 1;

    // Store the new string in preparation for a request event
    free(_glfwLibrary.X11.selection.string);
    _glfwLibrary.X11.selection.string = malloc(size);
    _glfwLibrary.X11.selection.stringLength = size;
    memcpy(_glfwLibrary.X11.selection.string, string, size);

    // Set the selection owner to our active window
    XSetSelectionOwner(_glfwLibrary.X11.display, XA_PRIMARY,
                       window->X11.handle, CurrentTime);
    XSetSelectionOwner(_glfwLibrary.X11.display,
                       _glfwLibrary.X11.selection.atom,
                       window->X11.handle, CurrentTime);
    XFlush(_glfwLibrary.X11.display);
}


//========================================================================
// Return the current clipboard contents
//========================================================================

size_t _glfwPlatformGetClipboardString(_GLFWwindow* window, char* data, size_t size)
{
    size_t len, rembytes, dummy;
    unsigned char* d;
    int i, fmt;
    Atom type;

    for (i = 0;  i < _GLFW_CLIPBOARD_FORMAT_COUNT;  i++)
    {
        // Specify the format we would like.
        _glfwLibrary.X11.selection.request =
            _glfwLibrary.X11.selection.formats[i];

        // Convert the selection into a format we would like.
        XConvertSelection(_glfwLibrary.X11.display,
                          _glfwLibrary.X11.selection.atom,
                          _glfwLibrary.X11.selection.request,
                          None, window->X11.handle, CurrentTime);
        XFlush(_glfwLibrary.X11.display);

        // Process pending events until we get a SelectionNotify.
        while (!_glfwLibrary.X11.selection.converted)
            _glfwPlatformWaitEvents();

        // Successful?
        if (_glfwLibrary.X11.selection.converted == 1)
            break;
    }

    // Successful?
    if (_glfwLibrary.X11.selection.converted == 1)
        _glfwLibrary.X11.selection.converted = 0;

    // Unsuccessful conversion, bail with no clipboard data
    if (_glfwLibrary.X11.selection.converted)
    {
        _glfwSetError(GLFW_FORMAT_UNAVAILABLE,
                      "X11/GLX: Failed to convert selection to string");
        return 0;
    }

    // Reset for the next selection
    _glfwLibrary.X11.selection.converted = 0;

    // Check the length of data to receive (rembytes)
    XGetWindowProperty(_glfwLibrary.X11.display,
                       window->X11.handle,
                       _glfwLibrary.X11.selection.request,
                       0, 0,
                       0,
                       AnyPropertyType,
                       &type,
                       &fmt,
                       &len, &rembytes,
                       &d);

    // The number of bytes remaining (which is all of them)
    if (rembytes > 0)
    {
        int result = XGetWindowProperty(_glfwLibrary.X11.display,
                                        window->X11.handle,
                                        _glfwLibrary.X11.selection.request,
                                        0, rembytes,
                                        0,
                                        AnyPropertyType,
                                        &type,
                                        &fmt,
                                        &len, &dummy,
                                        &d);
        if (result == Success)
        {
            size_t s;

            if (rembytes < size - 1)
                s = rembytes;
            else
                s = size - 1;

            // Copy the data out.
            memcpy(data, d, s);

            // Null-terminate strings.
            ((char*) data)[s] = '\0';

            // Free the data allocated using X11.
            XFree(d);

            // Return the actual number of bytes.
            return rembytes;
        }
        else
        {
            // Free the data allocated using X11.
            XFree(d);
            return 0;
        }
    }

    return 0;
}

