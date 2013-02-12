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

#include <limits.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Detect gamma ramp support
//
void _glfwInitGammaRamp(void)
{
    // RandR gamma support is only available with version 1.2 and above
    if (_glfw.x11.randr.available &&
        (_glfw.x11.randr.versionMajor > 1 ||
         (_glfw.x11.randr.versionMajor == 1 &&
          _glfw.x11.randr.versionMinor >= 2)))
    {
        // FIXME: Assumes that all monitors have the same size gamma tables
        // This is reasonable as I suspect the that if they did differ, it
        // would imply that setting the gamma size to an arbitary size is
        // possible as well.
        XRRScreenResources* rr = XRRGetScreenResources(_glfw.x11.display,
                                                       _glfw.x11.root);

        if (XRRGetCrtcGammaSize(_glfw.x11.display, rr->crtcs[0]))
        {
            // This is probably older Nvidia RandR with broken gamma support
            // Flag it as useless and try Xf86VidMode below, if available
            _glfw.x11.randr.gammaBroken = GL_TRUE;
        }

        XRRFreeScreenResources(rr);
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformGetGammaRamp(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    // TODO: Support ramp sizes other than 256

    if (_glfw.x11.randr.available && !_glfw.x11.randr.gammaBroken)
    {
        XRRCrtcGamma* gamma;
        size_t size = GLFW_GAMMA_RAMP_SIZE * sizeof(unsigned short);

        if (XRRGetCrtcGammaSize(_glfw.x11.display, monitor->x11.crtc) !=
            GLFW_GAMMA_RAMP_SIZE)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "X11: Only gamma ramps of size 256 supported");
            return;
        }

        gamma = XRRGetCrtcGamma(_glfw.x11.display, monitor->x11.crtc);

        memcpy(ramp->red, gamma->red, size);
        memcpy(ramp->green, gamma->green, size);
        memcpy(ramp->blue, gamma->blue, size);

        XRRFreeGamma(gamma);
    }
    else if (_glfw.x11.vidmode.available)
    {
        int size;
        XF86VidModeGetGammaRampSize(_glfw.x11.display, _glfw.x11.screen, &size);

        if (size != GLFW_GAMMA_RAMP_SIZE)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "X11: Only gamma ramps of size 256 supported");
            return;
        }

        XF86VidModeGetGammaRamp(_glfw.x11.display,
                                _glfw.x11.screen,
                                GLFW_GAMMA_RAMP_SIZE,
                                ramp->red, ramp->green, ramp->blue);
    }
}

void _glfwPlatformSetGammaRamp(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    if (_glfw.x11.randr.available && !_glfw.x11.randr.gammaBroken)
    {
        size_t size = GLFW_GAMMA_RAMP_SIZE * sizeof(unsigned short);
        XRRCrtcGamma* gamma;

        if (XRRGetCrtcGammaSize(_glfw.x11.display, monitor->x11.crtc) !=
            GLFW_GAMMA_RAMP_SIZE)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "X11: Only gamma ramps of size 256 supported");
            return;
        }

        gamma = XRRAllocGamma(GLFW_GAMMA_RAMP_SIZE);

        memcpy(gamma->red, ramp->red, size);
        memcpy(gamma->green, ramp->green, size);
        memcpy(gamma->blue, ramp->blue, size);

        XRRSetCrtcGamma(_glfw.x11.display, monitor->x11.crtc, gamma);
    }
    else if (_glfw.x11.vidmode.available)
    {
        int size;
        XF86VidModeGetGammaRampSize(_glfw.x11.display, _glfw.x11.screen, &size);

        if (size != GLFW_GAMMA_RAMP_SIZE)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "X11: Only gamma ramps of size 256 supported");
            return;
        }

        XF86VidModeSetGammaRamp(_glfw.x11.display,
                                _glfw.x11.screen,
                                GLFW_GAMMA_RAMP_SIZE,
                                (unsigned short*) ramp->red,
                                (unsigned short*) ramp->green,
                                (unsigned short*) ramp->blue);
    }
}

