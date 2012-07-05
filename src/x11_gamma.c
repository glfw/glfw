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

#include <limits.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Detect gamma ramp support and save original gamma ramp, if available
//========================================================================

void _glfwInitGammaRamp(void)
{
#ifdef _GLFW_HAS_XRANDR
    // RandR gamma support is only available with version 1.2 and above
    if (_glfwLibrary.X11.RandR.available &&
        (_glfwLibrary.X11.RandR.majorVersion > 1 ||
         (_glfwLibrary.X11.RandR.majorVersion == 1 &&
          _glfwLibrary.X11.RandR.minorVersion >= 2)))
    {
        // FIXME: Assumes that all monitors have the same size gamma tables
        // This is reasonable as I suspect the that if they did differ, it
        // would imply that setting the gamma size to an arbitary size is
        // possible as well.
        XRRScreenResources* rr = XRRGetScreenResources(_glfwLibrary.X11.display,
                                                       _glfwLibrary.X11.root);

        _glfwLibrary.originalRampSize = XRRGetCrtcGammaSize(_glfwLibrary.X11.display,
                                                            rr->crtcs[0]);
        if (!_glfwLibrary.originalRampSize)
        {
            // This is probably Nvidia RandR with broken gamma support
            // Flag it as useless and try Xf86VidMode below, if available
            _glfwLibrary.X11.RandR.gammaBroken = GL_TRUE;
        }

        XRRFreeScreenResources(rr);
    }
#endif /*_GLFW_HAS_XRANDR*/

#if defined(_GLFW_HAS_XF86VIDMODE)
    if (_glfwLibrary.X11.VidMode.available &&
        !_glfwLibrary.originalRampSize)
    {
        // Get the gamma size using XF86VidMode
        XF86VidModeGetGammaRampSize(_glfwLibrary.X11.display,
                                    _glfwLibrary.X11.screen,
                                    &_glfwLibrary.originalRampSize);
    }
#endif /*_GLFW_HAS_XF86VIDMODE*/

    if (_glfwLibrary.originalRampSize)
    {
        // Save the original gamma ramp
        _glfwPlatformGetGammaRamp(&_glfwLibrary.originalRamp);
        _glfwLibrary.currentRamp = _glfwLibrary.originalRamp;
    }
}


//========================================================================
// Restore original gamma ramp if necessary
//========================================================================

void _glfwTerminateGammaRamp(void)
{
    if (_glfwLibrary.originalRampSize && _glfwLibrary.rampChanged)
        _glfwPlatformSetGammaRamp(&_glfwLibrary.originalRamp);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Retrieve the currently set gamma ramp
//========================================================================

void _glfwPlatformGetGammaRamp(GLFWgammaramp* ramp)
{
    // For now, don't support anything that is not GLFW_GAMMA_RAMP_SIZE
    if (_glfwLibrary.originalRampSize != GLFW_GAMMA_RAMP_SIZE)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "X11/GLX: Failed to get gamma ramp due to size "
                      "incompatibility");
        return;
    }

    if (_glfwLibrary.X11.RandR.available &&
        !_glfwLibrary.X11.RandR.gammaBroken)
    {
#if defined (_GLFW_HAS_XRANDR)
        size_t size = GLFW_GAMMA_RAMP_SIZE * sizeof(unsigned short);

        XRRScreenResources* rr = XRRGetScreenResources(_glfwLibrary.X11.display,
                                                       _glfwLibrary.X11.root);

        XRRCrtcGamma* gamma = XRRGetCrtcGamma(_glfwLibrary.X11.display,
                                              rr->crtcs[0]);

        // TODO: Handle case of original ramp size having a size other than 256

        memcpy(ramp->red, gamma->red, size);
        memcpy(ramp->green, gamma->green, size);
        memcpy(ramp->blue, gamma->blue, size);

        XRRFreeGamma(gamma);
        XRRFreeScreenResources(rr);
#endif /*_GLFW_HAS_XRANDR*/
    }
    else if (_glfwLibrary.X11.VidMode.available)
    {
#if defined (_GLFW_HAS_XF86VIDMODE)
        XF86VidModeGetGammaRamp(_glfwLibrary.X11.display,
                                _glfwLibrary.X11.screen,
                                GLFW_GAMMA_RAMP_SIZE,
                                ramp->red,
                                ramp->green,
                                ramp->blue);
#endif /*_GLFW_HAS_XF86VIDMODE*/
    }
}


//========================================================================
// Push the specified gamma ramp to the monitor
//========================================================================

void _glfwPlatformSetGammaRamp(const GLFWgammaramp* ramp)
{
    // For now, don't support anything that is not GLFW_GAMMA_RAMP_SIZE
    if (_glfwLibrary.originalRampSize != GLFW_GAMMA_RAMP_SIZE)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "X11/GLX: Failed to set gamma ramp due to size "
                      "incompatibility");
        return;
    }

    if (_glfwLibrary.X11.RandR.available &&
        !_glfwLibrary.X11.RandR.gammaBroken)
    {
#if defined (_GLFW_HAS_XRANDR)
        int i;
        size_t size = GLFW_GAMMA_RAMP_SIZE * sizeof(unsigned short);

        XRRScreenResources* rr = XRRGetScreenResources(_glfwLibrary.X11.display,
                                                       _glfwLibrary.X11.root);

        // Update gamma per monitor
        for (i = 0;  i < rr->ncrtc;  i++)
        {
            XRRCrtcGamma* gamma = XRRAllocGamma(GLFW_GAMMA_RAMP_SIZE);

            memcpy(gamma->red, ramp->red, size);
            memcpy(gamma->green, ramp->green, size);
            memcpy(gamma->blue, ramp->blue, size);

            XRRSetCrtcGamma(_glfwLibrary.X11.display, rr->crtcs[i], gamma);
            XRRFreeGamma(gamma);
        }

        XRRFreeScreenResources(rr);
#endif /*_GLFW_HAS_XRANDR*/
    }
    else if (_glfwLibrary.X11.VidMode.available)
    {
#if defined (_GLFW_HAS_XF86VIDMODE)
        XF86VidModeSetGammaRamp(_glfwLibrary.X11.display,
                                _glfwLibrary.X11.screen,
                                GLFW_GAMMA_RAMP_SIZE,
                                (unsigned short*) ramp->red,
                                (unsigned short*) ramp->green,
                                (unsigned short*) ramp->blue);
#endif /*_GLFW_HAS_XF86VIDMODE*/
    }
}

