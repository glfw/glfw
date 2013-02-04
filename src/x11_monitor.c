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

// Set the current video mode for the specified monitor
//
void _glfwSetVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* mode)
{
    if (_glfw.x11.randr.available)
    {
        int i, j, k;
        XRRScreenResources* sr;
        XRRCrtcInfo* ci;
        RRMode bestMode = 0;
        unsigned int leastSizeDiff = UINT_MAX;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);
        ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);

        for (i = 0;  i < sr->nmode;  i++)
        {
            GLboolean usable = GL_TRUE;
            XRRModeInfo* mi = sr->modes + i;

            for (j = 0;  j < ci->noutput;  j++)
            {
                XRROutputInfo* oi = XRRGetOutputInfo(_glfw.x11.display,
                                                     sr, ci->outputs[j]);

                for (k = 0;  k < oi->nmode;  k++)
                {
                    if (oi->modes[k] == mi->id)
                        break;
                }

                if (k == oi->nmode)
                    usable = GL_FALSE;

                XRRFreeOutputInfo(oi);
            }

            if (!usable)
                continue;

            if (mi->modeFlags & RR_Interlace)
                continue;

            unsigned int sizeDiff = (mi->width - mode->width) *
                                    (mi->width - mode->width) +
                                    (mi->height - mode->height) *
                                    (mi->height - mode->height);

            if (sizeDiff < leastSizeDiff)
            {
                bestMode = mi->id;
                leastSizeDiff = sizeDiff;
            }
        }

        monitor->x11.oldMode = ci->mode;

        XRRSetCrtcConfig(_glfw.x11.display,
                         sr, monitor->x11.crtc,
                         CurrentTime,
                         ci->x, ci->y,
                         bestMode,
                         ci->rotation,
                         ci->outputs,
                         ci->noutput);

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
}

// Restore the saved (original) video mode for the specified monitor
//
void _glfwRestoreVideoMode(_GLFWmonitor* monitor)
{
    if (_glfw.x11.randr.available)
    {
        XRRScreenResources* sr;
        XRRCrtcInfo* ci;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);
        ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);

        XRRSetCrtcConfig(_glfw.x11.display,
                         sr, monitor->x11.crtc,
                         CurrentTime,
                         ci->x, ci->y,
                         monitor->x11.oldMode,
                         ci->rotation,
                         ci->outputs,
                         ci->noutput);

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

_GLFWmonitor** _glfwPlatformGetMonitors(int* found)
{
    _GLFWmonitor** monitors = NULL;

    *found = 0;

    if (_glfw.x11.randr.available)
    {
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

        for (i = 0;  i < sr->ncrtc;  i++)
        {
            int j;
            XRROutputInfo* oi;
            XRRCrtcInfo* ci;
            RROutput output;

            ci = XRRGetCrtcInfo(_glfw.x11.display, sr, sr->crtcs[i]);
            output = ci->outputs[i];

            for (j = 0;  j < ci->noutput;  j++)
            {
                if (ci->outputs[i] == primary)
                {
                    output = primary;
                    break;
                }
            }

            oi = XRRGetOutputInfo(_glfw.x11.display, sr, output);
            if (oi->connection != RR_Connected)
            {
                XRRFreeOutputInfo(oi);
                XRRFreeCrtcInfo(ci);
                continue;
            }

            monitors[*found] = _glfwCreateMonitor(oi->name,
                                                  output == primary,
                                                  oi->mm_width, oi->mm_height,
                                                  ci->x, ci->y);

            monitors[*found]->x11.output = output;
            monitors[*found]->x11.crtc   = oi->crtc;

            XRRFreeOutputInfo(oi);
            XRRFreeCrtcInfo(ci);

            (*found)++;
        }

        XRRFreeScreenResources(sr);
    }
    else
    {
        int widthMM, heightMM;

        monitors = (_GLFWmonitor**) calloc(1, sizeof(_GLFWmonitor*));
        if (!monitors)
        {
            _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
            return NULL;
        }

        widthMM  = DisplayWidthMM(_glfw.x11.display, _glfw.x11.screen);
        heightMM = DisplayHeightMM(_glfw.x11.display, _glfw.x11.screen);

        monitors[0] = _glfwCreateMonitor("Display",
                                         GL_TRUE,
                                         widthMM, heightMM,
                                         0, 0);

        *found = 1;
    }

    return monitors;
}

void _glfwPlatformDestroyMonitor(_GLFWmonitor* monitor)
{
}

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
        int i, j;
        XRRScreenResources* sr;
        XRROutputInfo* oi;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);
        oi = XRRGetOutputInfo(_glfw.x11.display, sr, monitor->x11.output);

        result = (GLFWvidmode*) malloc(sizeof(GLFWvidmode) * oi->nmode);
        if (!result)
        {
            _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
            return NULL;
        }

        for (i = 0;  i < oi->nmode;  i++)
        {
            GLFWvidmode mode;

            for (j = 0;  j < sr->nmode;  j++)
            {
                if (sr->modes[j].id == oi->modes[i])
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

        XRRFreeOutputInfo(oi);
        XRRFreeScreenResources(sr);
    }
    else
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

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
    if (_glfw.x11.randr.available)
    {
        XRRScreenResources* sr;
        XRRCrtcInfo* ci;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);
        ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);

        mode->width = ci->width;
        mode->height = ci->height;

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        mode->width = DisplayWidth(_glfw.x11.display, _glfw.x11.screen);
        mode->height = DisplayHeight(_glfw.x11.display, _glfw.x11.screen);
    }

    _glfwSplitBPP(DefaultDepth(_glfw.x11.display, _glfw.x11.screen),
                  &mode->redBits, &mode->greenBits, &mode->blueBits);
}

