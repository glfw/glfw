//========================================================================
// GLFW 3.1 X11 - www.glfw.org
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


// Check whether the display mode should be included in enumeration
//
static GLboolean modeIsGood(const XRRModeInfo* mi)
{
    return (mi->modeFlags & RR_Interlace) == 0;
}

// Calculates the refresh rate, in Hz, from the specified RandR mode info
//
static int calculateRefreshRate(const XRRModeInfo* mi)
{
    if (mi->hTotal && mi->vTotal)
        return (int) ((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal));
    else
        return 0;
}

// Returns the mode info for a RandR mode XID
//
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

// Convert RandR mode info to GLFW video mode
//
static GLFWvidmode vidmodeFromModeInfo(const XRRModeInfo* mi)
{
    GLFWvidmode mode;
    mode.width  = mi->width;
    mode.height = mi->height;
    mode.refreshRate = calculateRefreshRate(mi);

    _glfwSplitBPP(DefaultDepth(_glfw.x11.display, _glfw.x11.screen),
                  &mode.redBits, &mode.greenBits, &mode.blueBits);

    return mode;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Set the current video mode for the specified monitor
//
GLboolean _glfwSetVideoMode(_GLFWmonitor* monitor, const GLFWvidmode* desired)
{
    if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr;
        XRRCrtcInfo* ci;
        XRROutputInfo* oi;
        GLFWvidmode current;
        const GLFWvidmode* best;
        RRMode native = None;
        int i;

        best = _glfwChooseVideoMode(monitor, desired);
        _glfwPlatformGetVideoMode(monitor, &current);
        if (_glfwCompareVideoModes(&current, best) == 0)
            return GL_TRUE;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);
        ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);
        oi = XRRGetOutputInfo(_glfw.x11.display, sr, monitor->x11.output);

        for (i = 0;  i < oi->nmode;  i++)
        {
            const XRRModeInfo* mi = getModeInfo(sr, oi->modes[i]);
            if (!modeIsGood(mi))
                continue;

            const GLFWvidmode mode = vidmodeFromModeInfo(mi);
            if (_glfwCompareVideoModes(best, &mode) == 0)
            {
                native = mi->id;
                break;
            }
        }

        if (native)
        {
            if (monitor->x11.oldMode == None)
                monitor->x11.oldMode = ci->mode;

            XRRSetCrtcConfig(_glfw.x11.display,
                             sr, monitor->x11.crtc,
                             CurrentTime,
                             ci->x, ci->y,
                             native,
                             ci->rotation,
                             ci->outputs,
                             ci->noutput);
        }

        XRRFreeOutputInfo(oi);
        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);

        if (!native)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "X11: Monitor mode list changed");
            return GL_FALSE;
        }
    }

    return GL_TRUE;
}

// Restore the saved (original) video mode for the specified monitor
//
void _glfwRestoreVideoMode(_GLFWmonitor* monitor)
{
    if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken)
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
    int i, found = 0;
    _GLFWmonitor** monitors = NULL;

    *count = 0;

    if (_glfw.x11.randr.available)
    {
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

            monitors[found] = _glfwAllocMonitor(oi->name,
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
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "X11: RandR monitor support seems broken");
            _glfw.x11.randr.monitorBroken = GL_TRUE;

            free(monitors);
            monitors = NULL;
        }
    }

    if (!monitors)
    {
        monitors = calloc(1, sizeof(_GLFWmonitor*));
        monitors[0] = _glfwAllocMonitor("Display",
                                        DisplayWidthMM(_glfw.x11.display,
                                                       _glfw.x11.screen),
                                        DisplayHeightMM(_glfw.x11.display,
                                                        _glfw.x11.screen));
        found = 1;
    }

    *count = found;
    return monitors;
}

GLboolean _glfwPlatformIsSameMonitor(_GLFWmonitor* first, _GLFWmonitor* second)
{
    return first->x11.crtc == second->x11.crtc;
}

void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken)
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
}

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
    GLFWvidmode* result;

    *found = 0;

    // Build array of available resolutions

    if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken)
    {
        int i, j;
        XRRScreenResources* sr;
        XRROutputInfo* oi;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);
        oi = XRRGetOutputInfo(_glfw.x11.display, sr, monitor->x11.output);

        result = calloc(oi->nmode, sizeof(GLFWvidmode));

        for (i = 0;  i < oi->nmode;  i++)
        {
            const XRRModeInfo* mi = getModeInfo(sr, oi->modes[i]);
            if (!modeIsGood(mi))
                continue;

            const GLFWvidmode mode = vidmodeFromModeInfo(mi);

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
        _glfwPlatformGetVideoMode(monitor, result);
    }

    return result;
}

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
    if (_glfw.x11.randr.available && !_glfw.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr;
        XRRCrtcInfo* ci;

        sr = XRRGetScreenResources(_glfw.x11.display, _glfw.x11.root);
        ci = XRRGetCrtcInfo(_glfw.x11.display, sr, monitor->x11.crtc);

        *mode = vidmodeFromModeInfo(getModeInfo(sr, ci->mode));

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        mode->width = DisplayWidth(_glfw.x11.display, _glfw.x11.screen);
        mode->height = DisplayHeight(_glfw.x11.display, _glfw.x11.screen);
        mode->refreshRate = 0;

        _glfwSplitBPP(DefaultDepth(_glfw.x11.display, _glfw.x11.screen),
                      &mode->redBits, &mode->greenBits, &mode->blueBits);
    }
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI RRCrtc glfwGetX11Monitor(GLFWmonitor* handle)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(None);
    return monitor->x11.crtc;
}

