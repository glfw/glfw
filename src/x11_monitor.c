//========================================================================
// GLFW - An OpenGL library
// Platform:    X11 (Unix)
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

#include <stdlib.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////


//========================================================================
// Return a list of available monitors
//========================================================================

_GLFWmonitor** _glfwPlatformGetMonitors(int* count)
{
    int found = 0;
    _GLFWmonitor** monitors = NULL;

    if (_glfwLibrary.X11.RandR.available)
    {
#if defined (_GLFW_HAS_XRANDR)
        int i;
        XRRScreenResources* sr;

        sr = XRRGetScreenResources(_glfwLibrary.X11.display,
                                   _glfwLibrary.X11.root);

        monitors = (_GLFWmonitor**) calloc(sr->noutput, sizeof(_GLFWmonitor*));
        if (!monitors)
        {
            _glfwSetError(GLFW_OUT_OF_MEMORY, NULL);
            return NULL;
        }

        for (i = 0;  i < sr->noutput;  i++)
        {
            XRROutputInfo* oi;
            XRRCrtcInfo* ci;

            oi = XRRGetOutputInfo(_glfwLibrary.X11.display, sr, sr->outputs[i]);
            if (oi->connection != RR_Connected)
            {
                XRRFreeOutputInfo(oi);
                continue;
            }

            ci = XRRGetCrtcInfo(_glfwLibrary.X11.display, sr, oi->crtc);

            monitors[found] = _glfwCreateMonitor(oi->name,
                                                 oi->mm_width, oi->mm_height,
                                                 ci->x, ci->y);

            XRRFreeCrtcInfo(ci);

            if (!monitors[found])
            {
                // TODO: wat
                return NULL;
            }

            // This is retained until the monitor object is destroyed
            monitors[found]->X11.output = oi;

            found++;
        }
#endif /*_GLFW_HAS_XRANDR*/
    }

    *count = found;
    return monitors;
}


//========================================================================
// Destroy a monitor struct
//========================================================================

void _glfwPlatformDestroyMonitor(_GLFWmonitor* monitor)
{
#if defined (_GLFW_HAS_XRANDR)
    XRRFreeOutputInfo(monitor->X11.output);
#endif /*_GLFW_HAS_XRANDR*/
}

