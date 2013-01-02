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

#include <limits.h>
#include <stdlib.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Finds the video mode closest in size to the specified desired size
//========================================================================

int _glfwGetClosestVideoMode(_GLFWmonitor* monitor, int* width, int* height)
{
    int i, match, bestmatch;

    if (_glfw.x11.randr.available)
    {
#if defined(_GLFW_HAS_XRANDR)
        int sizecount, bestsize;
        XRRScreenConfiguration* sc;
        XRRScreenSize* sizelist;

        sc = XRRGetScreenInfo(_glfw.x11.display, _glfw.x11.root);

        sizelist = XRRConfigSizes(sc, &sizecount);

        // Find the best matching mode
        bestsize  = -1;
        bestmatch = INT_MAX;
        for (i = 0;  i < sizecount;  i++)
        {
            match = (*width - sizelist[i].width) *
                    (*width - sizelist[i].width) +
                    (*height - sizelist[i].height) *
                    (*height - sizelist[i].height);
            if (match < bestmatch)
            {
                bestmatch = match;
                bestsize  = i;
            }
        }

        if (bestsize != -1)
        {
            // Report width & height of best matching mode
            *width = sizelist[bestsize].width;
            *height = sizelist[bestsize].height;
        }

        XRRFreeScreenConfigInfo(sc);

        if (bestsize != -1)
            return bestsize;
#endif /*_GLFW_HAS_XRANDR*/
    }

    // Default: Simply use the screen resolution
    *width = DisplayWidth(_glfw.x11.display, _glfw.x11.screen);
    *height = DisplayHeight(_glfw.x11.display, _glfw.x11.screen);

    return 0;
}


//========================================================================
// Change the current video mode
//========================================================================

void _glfwSetVideoModeMODE(_GLFWmonitor* monitor, int mode)
{
    if (_glfw.x11.randr.available)
    {
#if defined(_GLFW_HAS_XRANDR)
        XRRScreenConfiguration* sc;
        Window root;

        root = _glfw.x11.root;
        sc   = XRRGetScreenInfo(_glfw.x11.display, root);

        // Remember old size and flag that we have changed the mode
        if (!monitor->x11.modeChanged)
        {
            monitor->x11.oldSizeID = XRRConfigCurrentConfiguration(sc, &monitor->x11.oldRotation);
            monitor->x11.oldWidth  = DisplayWidth(_glfw.x11.display,
                                                  _glfw.x11.screen);
            monitor->x11.oldHeight = DisplayHeight(_glfw.x11.display,
                                                   _glfw.x11.screen);

            monitor->x11.modeChanged = GL_TRUE;
        }

        XRRSetScreenConfig(_glfw.x11.display,
                           sc,
                           root,
                           mode,
                           RR_Rotate_0,
                           CurrentTime);

        XRRFreeScreenConfigInfo(sc);
#endif /*_GLFW_HAS_XRANDR*/
    }
}


//========================================================================
// Change the current video mode
//========================================================================

void _glfwSetVideoMode(_GLFWmonitor* monitor, int* width, int* height)
{
    int bestmode;

    // Find a best match mode
    bestmode = _glfwGetClosestVideoMode(monitor, width, height);

    // Change mode
    _glfwSetVideoModeMODE(monitor, bestmode);
}


//========================================================================
// Restore the previously saved (original) video mode
//========================================================================

void _glfwRestoreVideoMode(_GLFWmonitor* monitor)
{
    if (!monitor->x11.modeChanged)
        return;

    if (_glfw.x11.randr.available)
    {
#if defined(_GLFW_HAS_XRANDR)
        XRRScreenConfiguration* sc;

        sc = XRRGetScreenInfo(_glfw.x11.display, _glfw.x11.root);

        XRRSetScreenConfig(_glfw.x11.display,
                           sc,
                           _glfw.x11.root,
                           monitor->x11.oldSizeID,
                           monitor->x11.oldRotation,
                           CurrentTime);

        XRRFreeScreenConfigInfo(sc);
#endif /*_GLFW_HAS_XRANDR*/
    }

    monitor->x11.modeChanged = GL_FALSE;
}


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

    if (_glfw.x11.randr.available)
    {
#if defined (_GLFW_HAS_XRANDR)
        int i;
        RROutput primary;
        XRRScreenResources* sr;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);

        primary = XRRGetOutputPrimary(_glfw.x11.display, _glfw.x11.root);

        monitors = (_GLFWmonitor**) calloc(sr->noutput, sizeof(_GLFWmonitor*));
        if (!monitors)
        {
            XRRFreeScreenResources(sr);

            _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
            return NULL;
        }

        for (i = 0;  i < sr->noutput;  i++)
        {
            XRROutputInfo* oi;
            XRRCrtcInfo* ci;
            int widthMM, heightMM;

            oi = XRRGetOutputInfo(_glfw.x11.display, sr, sr->outputs[i]);
            if (oi->connection != RR_Connected)
            {
                XRRFreeOutputInfo(oi);
                continue;
            }

            if (oi->mm_width && oi->mm_height)
            {
                widthMM = oi->mm_width;
                heightMM = oi->mm_height;
            }
            else
            {
                widthMM = DisplayWidthMM(_glfw.x11.display, _glfw.x11.screen);
                heightMM = DisplayHeightMM(_glfw.x11.display, _glfw.x11.screen);
            }

            ci = XRRGetCrtcInfo(_glfw.x11.display, sr, oi->crtc);

            monitors[found] = _glfwCreateMonitor(oi->name,
                                                 sr->outputs[i] == primary,
                                                 widthMM, heightMM,
                                                 ci->x, ci->y);

            XRRFreeCrtcInfo(ci);

            if (!monitors[found])
            {
                // TODO: wat
                return NULL;
            }

            // This is retained until the monitor object is destroyed
            monitors[found]->x11.output = oi;
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
    if (_glfw.x11.randr.available)
    {
#if defined (_GLFW_HAS_XRANDR)
        XRRFreeOutputInfo(monitor->x11.output);
#endif /*_GLFW_HAS_XRANDR*/
    }
}


//========================================================================
// List available video modes
//========================================================================

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
    GLFWvidmode* result;
    int depth, r, g, b;

    depth = DefaultDepth(_glfw.x11.display, _glfw.x11.screen);
    _glfwSplitBPP(depth, &r, &g, &b);

    *found = 0;

    // Build array of available resolutions

    if (_glfw.x11.randr.available)
    {
#if defined(_GLFW_HAS_XRANDR)
        XRRScreenResources* sr;
        int i, j, count = monitor->x11.output->nmode;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);

        result = (GLFWvidmode*) malloc(sizeof(GLFWvidmode) * count);
        if (!result)
        {
            _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
            return NULL;
        }

        for (i = 0;  i < count;  i++)
        {
            GLFWvidmode mode;

            for (j = 0;  j < sr->nmode;  j++)
            {
                if (sr->modes[j].id == monitor->x11.output->modes[i])
                    break;
            }

            if (j == sr->nmode)
                continue;

            mode.width  = sr->modes[j].width;
            mode.height = sr->modes[j].height;

            for (j = 0;  j < *found;  j++)
            {
                if (result[j].width == mode.width &&
                    result[j].height == mode.height)
                {
                    break;
                }
            }

            if (j < *found)
            {
                // This is a duplicate, so skip it
                continue;
            }

            mode.redBits = r;
            mode.greenBits = g;
            mode.blueBits = b;

            result[*found] = mode;
            (*found)++;
        }

        XRRFreeScreenResources(sr);
#endif /*_GLFW_HAS_XRANDR*/
    }

    if (result == NULL)
    {
        *found = 1;

        result = (GLFWvidmode*) malloc(sizeof(GLFWvidmode));
        if (!result)
        {
            _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
            return NULL;
        }

        result[0].width = DisplayWidth(_glfw.x11.display, _glfw.x11.screen);
        result[0].height = DisplayHeight(_glfw.x11.display, _glfw.x11.screen);
        result[0].redBits = r;
        result[0].greenBits = g;
        result[0].blueBits = b;
    }

    return result;
}


//========================================================================
// Get the current video mode for the specified monitor
//========================================================================

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
    if (_glfw.x11.randr.available)
    {
#if defined (_GLFW_HAS_XRANDR)
        XRRScreenResources* sr;
        XRRCrtcInfo* ci;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);
        if (!sr)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "X11: Failed to retrieve RandR screen resources");
            return;
        }

        ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.output->crtc);
        if (!ci)
        {
            XRRFreeScreenResources(sr);

            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "X11: Failed to retrieve RandR crtc info");
            return;
        }

        mode->width = ci->width;
        mode->height = ci->height;

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
#endif /*_GLFW_HAS_XRANDR*/
    }
    else
    {
        mode->width = DisplayWidth(_glfw.x11.display, _glfw.x11.screen);
        mode->height = DisplayHeight(_glfw.x11.display, _glfw.x11.screen);
    }

    _glfwSplitBPP(DefaultDepth(_glfw.x11.display, _glfw.x11.screen),
                  &mode->redBits, &mode->greenBits, &mode->blueBits);
}

