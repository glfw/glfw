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


//========================================================================
// Get the corresponding X11 format for a given GLFW format.
//========================================================================

static Atom* getInternalFormat(int format)
{
    // Get the necessary atoms
    switch (format)
    {
        case GLFW_CLIPBOARD_FORMAT_STRING:
            return _glfwLibrary.X11.selection.atoms.string;
        default:
            return 0;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// X11 selection request event
//========================================================================

Atom _glfwSelectionRequest(XSelectionRequestEvent* request)
{
    Atom* atoms = _glfwLibrary.X11.selection.atoms.string;
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
            (unsigned char*) _glfwLibrary.X11.selection.clipboard.string,
            8);
    }
    else if (request->target == atoms[_GLFW_STRING_ATOM_COMPOUND] ||
             request->target == atoms[_GLFW_STRING_ATOM_UTF8])
    {
        XChangeProperty(_glfwLibrary.X11.display,
            request->requestor,
            request->target,
            request->target,
            8,
            PropModeReplace,
            (unsigned char*) _glfwLibrary.X11.selection.clipboard.string,
            _glfwLibrary.X11.selection.clipboard.stringlen);
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

void _glfwPlatformSetClipboardData(void *data, size_t size, int format)
{
    switch (format)
    {
        case GLFW_CLIPBOARD_FORMAT_STRING:
        {
            // Allocate memory to keep track of the clipboard
            char *cb = malloc(size+1);

            // Copy the clipboard data
            memcpy(cb, data, size);

            // Set the string length
            _glfwLibrary.X11.selection.clipboard.stringlen = size;

            // Check if existing clipboard memory needs to be freed
            if (_glfwLibrary.X11.selection.clipboard.string)
                free(_glfwLibrary.X11.selection.clipboard.string);

            // Now set the clipboard (awaiting the event SelectionRequest)
            _glfwLibrary.X11.selection.clipboard.string = cb;
            break;
        }

        default:
            _glfwSetError(GLFW_CLIPBOARD_FORMAT_UNAVAILABLE,
                          "X11/GLX: Unavailable clipboard format");
            return;
    }

    // Set the selection owner to our active window
    XSetSelectionOwner(_glfwLibrary.X11.display, XA_PRIMARY,
                       _glfwLibrary.activeWindow->X11.handle, CurrentTime);
    XSetSelectionOwner(_glfwLibrary.X11.display,
                       _glfwLibrary.X11.selection.atoms.clipboard
                       [_GLFW_CLIPBOARD_ATOM_CLIPBOARD],
                       _glfwLibrary.activeWindow->X11.handle, CurrentTime);
    XFlush(_glfwLibrary.X11.display);
}


//========================================================================
// Return the current clipboard contents
//========================================================================

size_t _glfwPlatformGetClipboardData(void *data, size_t size, int format)
{
    size_t len, rembytes, dummy;
    unsigned char *d;
    int fmt;
    Atom type;

    // Try different clipboards and formats that relate to the GLFW
    // format with preference for more appropriate formats first
    Atom *xcbrd = _glfwLibrary.X11.selection.atoms.clipboard;
    Atom *xcbrdend = _glfwLibrary.X11.selection.atoms.clipboard +
                     _GLFW_CLIPBOARD_ATOM_COUNT;
    Atom *xfmt = getInternalFormat(format);
    Atom *xfmtend = xfmt + _GLFW_STRING_ATOM_COUNT;

    // Get the currently active window
    Window window = _glfwLibrary.activeWindow->X11.handle;

    for ( ;  xcbrd != xcbrdend;  xcbrd++)
    {
        for ( ;  xfmt != xfmtend;  xfmt++)
        {
            // Specify the format we would like.
            _glfwLibrary.X11.selection.request = *xfmt;

            // Convert the selection into a format we would like.
            XConvertSelection(_glfwLibrary.X11.display, *xcbrd,
                *xfmt, None, window, CurrentTime);
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
        {
            _glfwLibrary.X11.selection.converted = 0;
            break;
        }
    }

    // Unsuccessful conversion, bail with no clipboard data
    if (_glfwLibrary.X11.selection.converted)
    {
        _glfwSetError(GLFW_CLIPBOARD_FORMAT_UNAVAILABLE,
                      "X11/GLX: Unavailable clipboard format");
        return 0;
    }

    // Reset for the next selection
    _glfwLibrary.X11.selection.converted = 0;

    // Check the length of data to receive (rembytes)
    XGetWindowProperty(_glfwLibrary.X11.display,
                       window,
                       *xfmt,
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
        int result = XGetWindowProperty(_glfwLibrary.X11.display, window,
                                        *xfmt, 0, rembytes, 0,
                                        AnyPropertyType, &type, &fmt,
                                        &len, &dummy, &d);
        if (result == Success)
        {
            size_t s = size - 1 > rembytes ? rembytes : size - 1;

            // Copy the data out.
            memcpy(data, d, s);

            // Null-terminate strings.
            if (format == GLFW_CLIPBOARD_FORMAT_STRING)
                ((char *)data)[s] = '\0';

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

