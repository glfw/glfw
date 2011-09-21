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

#include "internal.h"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Get the corresponding X11 format for a given GLFW format.
//========================================================================

static Atom *getInternalFormat(int fmt)
{
    // Get the necessary atoms
    
    switch (fmt)
    {
        case GLFW_CLIPBOARD_FORMAT_STRING:
            return _glfwLibrary.X11.selection.stringatoms;
        default:
            return 0;
    }
}

//========================================================================
// Set the clipboard contents
//========================================================================

void _glfwPlatformSetClipboardData(void *data, size_t size, int format)
{

}

//========================================================================
// Return the current clipboard contents
// TODO: Incremental support? Overkill perhaps.
//========================================================================

size_t _glfwPlatformGetClipboardData(void *data, size_t size, int format)
{
    size_t len, rembytes, dummy;
    unsigned char *d;
    int fmt;
    Window window;
    Atom *xfmt, type;

    // Try different formats that relate to the GLFW format with preference
    // for better formats first
    for (xfmt = getInternalFormat(format); *xfmt; xfmt++)
    {
        // Specify the format we would like.
        _glfwLibrary.X11.selection.request = *xfmt;

        // Convert the selection into a format we would like.
        window = _glfwLibrary.activeWindow->X11.handle;
        XConvertSelection(_glfwLibrary.X11.display, XA_PRIMARY,
            *xfmt, None, window,
            CurrentTime);
        XFlush(_glfwLibrary.X11.display);

        // Process pending events until we get a SelectionNotify.
        while (!_glfwLibrary.X11.selection.converted)
            _glfwPlatformWaitEvents();

        // If there is no owner to the selection/wrong request, bail out.
        if (_glfwLibrary.X11.selection.converted == 2)
        {
            _glfwLibrary.X11.selection.converted = 0;
            _glfwSetError(GLFW_CLIPBOARD_FORMAT_UNAVAILABLE,
                          "X11/GLX: Unavailable clipboard format");
            return 0;
        }
        else // Right format, stop checking
        {
            _glfwLibrary.X11.selection.converted = 0;
            break;
        }
    }

    // Reset for the next selection
    _glfwLibrary.X11.selection.converted = 0;

    // Check the length of data to receive
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
            {
                ((char *)data)[s] = '\0';
            }
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

