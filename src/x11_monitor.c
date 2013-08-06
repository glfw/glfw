//========================================================================
// GLFW 3.0 X11 - www.glfw.org
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


static int calculateRefreshRate(const XRRModeInfo* mi)
{
    if (!mi->hTotal || !mi->vTotal)
        return 0;

    return (int) ((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal));
}

static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id)
{
    int i;

    for (i = 0;  i < sr->nmode;  i++)
    {
        if (sr->modes[i].id == id)
            return sr->modes + i;
    }

    return NULL;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Set the current video mode for the specified monitor
//
void _glfwSetVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* desired)
{
    if (_glfw.x11.randr.available)
    {
        int i, j;
        XRRScreenResources* sr;
        XRRCrtcInfo* ci;
        XRROutputInfo* oi;
        RRMode bestMode = 0;
        unsigned int sizeDiff, leastSizeDiff = UINT_MAX;
        unsigned int rateDiff, leastRateDiff = UINT_MAX;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);
        ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);
        oi = XRRGetOutputInfo(_glfw.x11.display, sr, monitor->x11.output);

        for (i = 0;  i < sr->nmode;  i++)
        {
            const XRRModeInfo* mi = sr->modes + i;

            if (mi->modeFlags & RR_Interlace)
                continue;

            for (j = 0;  j < oi->nmode;  j++)
            {
                if (oi->modes[j] == mi->id)
                    break;
            }

            if (j == oi->nmode)
                continue;

            sizeDiff = (mi->width - desired->width) *
                       (mi->width - desired->width) +
                       (mi->height - desired->height) *
                       (mi->height - desired->height);

            if (desired->refreshRate)
                rateDiff = abs(calculateRefreshRate(mi) - desired->refreshRate);
            else
                rateDiff = UINT_MAX - calculateRefreshRate(mi);

            if ((sizeDiff < leastSizeDiff) ||
                (sizeDiff == leastSizeDiff && rateDiff < leastRateDiff))
            {
                bestMode = mi->id;
                leastSizeDiff = sizeDiff;
                leastRateDiff = rateDiff;
            }
        }

        if (monitor->x11.oldMode == None)
            monitor->x11.oldMode = ci->mode;

        XRRSetCrtcConfig(_glfw.x11.display,
                         sr, monitor->x11.crtc,
                         CurrentTime,
                         ci->x, ci->y,
                         bestMode,
                         ci->rotation,
                         ci->outputs,
                         ci->noutput);

        XRRFreeOutputInfo(oi);
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

        if (monitor->x11.oldMode == None)
            return;

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

        monitor->x11.oldMode = None;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

_GLFWmonitor** _glfwPlatformGetMonitors(int* count)
{
    _GLFWmonitor** monitors = NULL;

    *count = 0;

    if (_glfw.x11.randr.available)
    {
        int i, found = 0;
        RROutput primary;
        XRRScreenResources* sr;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);
        primary = XRRGetOutputPrimary(_glfw.x11.display, _glfw.x11.root);

        monitors = calloc(sr->ncrtc, sizeof(_GLFWmonitor*));

        for (i = 0;  i < sr->ncrtc;  i++)
        {
            int j;
            XRROutputInfo* oi;
            XRRCrtcInfo* ci;
            RROutput output;

            ci = XRRGetCrtcInfo(_glfw.x11.display, sr, sr->crtcs[i]);
            if (ci->noutput == 0)
            {
                XRRFreeCrtcInfo(ci);
                continue;
            }

            output = ci->outputs[0];

            for (j = 0;  j < ci->noutput;  j++)
            {
                if (ci->outputs[j] == primary)
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

            monitors[found] = _glfwCreateMonitor(oi->name,
                                                 oi->mm_width, oi->mm_height);

            monitors[found]->x11.output = output;
            monitors[found]->x11.crtc   = oi->crtc;

            XRRFreeOutputInfo(oi);
            XRRFreeCrtcInfo(ci);

            found++;
        }

        XRRFreeScreenResources(sr);

        for (i = 0;  i < found;  i++)
        {
            if (monitors[i]->x11.output == primary)
            {
                _GLFWmonitor* temp = monitors[0];
                monitors[0] = monitors[i];
                monitors[i] = temp;
                break;
            }
        }

        if (found == 0)
        {
            free(monitors);
            monitors = NULL;
        }

        *count = found;
    }
    else
    {
        monitors = calloc(1, sizeof(_GLFWmonitor*));
        monitors[0] = _glfwCreateMonitor("Display",
                                         DisplayWidthMM(_glfw.x11.display,
                                                        _glfw.x11.screen),
                                         DisplayHeightMM(_glfw.x11.display,
                                                         _glfw.x11.screen));
        *count = 1;
    }

    return monitors;
}

GLboolean _glfwPlatformIsSameMonitor(_GLFWmonitor* first, _GLFWmonitor* second)
{
    return first->x11.crtc == second->x11.crtc;
}

void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    if (_glfw.x11.randr.available)
    {
        XRRScreenResources* sr;
        XRRCrtcInfo* ci;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);
        ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);

        if (xpos)
            *xpos = ci->x;
        if (ypos)
            *ypos = ci->y;

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        if (xpos)
            *xpos = 0;
        if (ypos)
            *ypos = 0;
    }
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

        result = calloc(oi->nmode, sizeof(GLFWvidmode));

        for (i = 0;  i < oi->nmode;  i++)
        {
            GLFWvidmode mode;
            const XRRModeInfo* mi = getModeInfo(sr, oi->modes[i]);

            mode.width  = mi->width;
            mode.height = mi->height;
            mode.refreshRate = calculateRefreshRate(mi);

            for (j = 0;  j < *found;  j++)
            {
                if (result[j].width == mode.width &&
                    result[j].height == mode.height &&
                    result[j].refreshRate == mode.refreshRate)
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

        result = calloc(1, sizeof(GLFWvidmode));

        result[0].width = DisplayWidth(_glfw.x11.display, _glfw.x11.screen);
        result[0].height = DisplayHeight(_glfw.x11.display, _glfw.x11.screen);
        result[0].redBits = r;
        result[0].greenBits = g;
        result[0].blueBits = b;
        result[0].refreshRate = 0;
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

        mode->refreshRate = calculateRefreshRate(getModeInfo(sr, ci->mode));

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        mode->width = DisplayWidth(_glfw.x11.display, _glfw.x11.screen);
        mode->height = DisplayHeight(_glfw.x11.display, _glfw.x11.screen);
        mode->refreshRate = 0;
    }

    _glfwSplitBPP(DefaultDepth(_glfw.x11.display, _glfw.x11.screen),
                  &mode->redBits, &mode->greenBits, &mode->blueBits);
}

