//========================================================================
// GLFW 3.1 X11 - www.glfw.org
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
    if (_glfw.x11.randr.available)
    {
        XRRScreenResources* sr = XRRGetScreenResources(_glfw.x11.display,
                                                       _glfw.x11.root);

        if (!sr->ncrtc || !XRRGetCrtcGammaSize(_glfw.x11.display, sr->crtcs[0]))
        {
            // This is either a headless system or an older Nvidia binary driver
            // with broken gamma support
            // Flag it as useless and fall back to Xf86VidMode, if available
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "X11: RandR gamma ramp support seems broken");
            _glfw.x11.randr.gammaBroken = GL_TRUE;
        }

        XRRFreeScreenResources(sr);
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformGetGammaRamp(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    if (_glfw.x11.randr.available && !_glfw.x11.randr.gammaBroken)
    {
        const size_t size = XRRGetCrtcGammaSize(_glfw.x11.display,
                                                monitor->x11.crtc);
        XRRCrtcGamma* gamma = XRRGetCrtcGamma(_glfw.x11.display,
                                              monitor->x11.crtc);

        _glfwAllocGammaArrays(ramp, size);

        memcpy(ramp->red, gamma->red, size * sizeof(unsigned short));
        memcpy(ramp->green, gamma->green, size * sizeof(unsigned short));
        memcpy(ramp->blue, gamma->blue, size * sizeof(unsigned short));

        XRRFreeGamma(gamma);
    }
    else if (_glfw.x11.vidmode.available)
    {
        int size;
        XF86VidModeGetGammaRampSize(_glfw.x11.display, _glfw.x11.screen, &size);

        _glfwAllocGammaArrays(ramp, size);

        XF86VidModeGetGammaRamp(_glfw.x11.display,
                                _glfw.x11.screen,
                                ramp->size, ramp->red, ramp->green, ramp->blue);
    }
}

void _glfwPlatformSetGammaRamp(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    if (_glfw.x11.randr.available && !_glfw.x11.randr.gammaBroken)
    {
        XRRCrtcGamma* gamma = XRRAllocGamma(ramp->size);

        memcpy(gamma->red, ramp->red, ramp->size * sizeof(unsigned short));
        memcpy(gamma->green, ramp->green, ramp->size * sizeof(unsigned short));
        memcpy(gamma->blue, ramp->blue, ramp->size * sizeof(unsigned short));

        XRRSetCrtcGamma(_glfw.x11.display, monitor->x11.crtc, gamma);
        XRRFreeGamma(gamma);
    }
    else if (_glfw.x11.vidmode.available)
    {
        XF86VidModeSetGammaRamp(_glfw.x11.display,
                                _glfw.x11.screen,
                                ramp->size,
                                (unsigned short*) ramp->red,
                                (unsigned short*) ramp->green,
                                (unsigned short*) ramp->blue);
    }
}

