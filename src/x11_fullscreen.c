//========================================================================
// GLFW - An OpenGL library
// Platform:    X11/GLX
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


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Finds the video mode closest in size to the specified desired size
//========================================================================

int _glfwGetClosestVideoMode(int* width, int* height, int* rate)
{
    int i, match, bestmatch;

    if (_glfwLibrary.X11.RandR.available)
    {
#if defined(_GLFW_HAS_XRANDR)
        int sizecount, bestsize;
        int ratecount, bestrate;
        short* ratelist;
        XRRScreenConfiguration* sc;
        XRRScreenSize* sizelist;

        sc = XRRGetScreenInfo(_glfwLibrary.X11.display, _glfwLibrary.X11.root);

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

            if (*rate > 0)
            {
                ratelist = XRRConfigRates(sc, bestsize, &ratecount);

                bestrate = -1;
                bestmatch = INT_MAX;
                for (i = 0;  i < ratecount;  i++)
                {
                    match = abs(ratelist[i] - *rate);
                    if (match < bestmatch)
                    {
                        bestmatch = match;
                        bestrate = ratelist[i];
                    }
                }

                if (bestrate != -1)
                    *rate = bestrate;
            }
        }

        XRRFreeScreenConfigInfo(sc);

        if (bestsize != -1)
            return bestsize;
#endif /*_GLFW_HAS_XRANDR*/
    }
    else if (_glfwLibrary.X11.VidMode.available)
    {
#if defined(_GLFW_HAS_XF86VIDMODE)
        XF86VidModeModeInfo** modelist;
        int bestmode, modecount;

        // Get a list of all available display modes
        XF86VidModeGetAllModeLines(_glfwLibrary.X11.display,
                                   _glfwLibrary.X11.screen,
                                   &modecount, &modelist);

        // Find the best matching mode
        bestmode  = -1;
        bestmatch = INT_MAX;
        for (i = 0;  i < modecount;  i++)
        {
            match = (*width - modelist[i]->hdisplay) *
                    (*width - modelist[i]->hdisplay) +
                    (*height - modelist[i]->vdisplay) *
                    (*height - modelist[i]->vdisplay);
            if (match < bestmatch)
            {
                bestmatch = match;
                bestmode  = i;
            }
        }

        if (bestmode != -1)
        {
            // Report width & height of best matching mode
            *width = modelist[bestmode]->hdisplay;
            *height = modelist[bestmode]->vdisplay;
        }

        XFree(modelist);

        if (bestmode != -1)
            return bestmode;
#endif /*_GLFW_HAS_XF86VIDMODE*/
    }

    // Default: Simply use the screen resolution
    *width = DisplayWidth(_glfwLibrary.X11.display, _glfwLibrary.X11.screen);
    *height = DisplayHeight(_glfwLibrary.X11.display, _glfwLibrary.X11.screen);

    return 0;
}


//========================================================================
// Change the current video mode
//========================================================================

void _glfwSetVideoModeMODE(int mode, int rate)
{
    if (_glfwLibrary.X11.RandR.available)
    {
#if defined(_GLFW_HAS_XRANDR)
        XRRScreenConfiguration* sc;
        Window root;

        root = _glfwLibrary.X11.root;
        sc   = XRRGetScreenInfo(_glfwLibrary.X11.display, root);

        // Remember old size and flag that we have changed the mode
        if (!_glfwLibrary.X11.FS.modeChanged)
        {
            _glfwLibrary.X11.FS.oldSizeID = XRRConfigCurrentConfiguration(sc, &_glfwLibrary.X11.FS.oldRotation);
            _glfwLibrary.X11.FS.oldWidth  = DisplayWidth(_glfwLibrary.X11.display,
                                                         _glfwLibrary.X11.screen);
            _glfwLibrary.X11.FS.oldHeight = DisplayHeight(_glfwLibrary.X11.display,
                                                          _glfwLibrary.X11.screen);

            _glfwLibrary.X11.FS.modeChanged = GL_TRUE;
        }

        if (rate > 0)
        {
            // Set desired configuration
            XRRSetScreenConfigAndRate(_glfwLibrary.X11.display,
                                      sc,
                                      root,
                                      mode,
                                      RR_Rotate_0,
                                      (short) rate,
                                      CurrentTime);
        }
        else
        {
            // Set desired configuration
            XRRSetScreenConfig(_glfwLibrary.X11.display,
                               sc,
                               root,
                               mode,
                               RR_Rotate_0,
                               CurrentTime);
        }

        XRRFreeScreenConfigInfo(sc);
#endif /*_GLFW_HAS_XRANDR*/
    }
    else if (_glfwLibrary.X11.VidMode.available)
    {
#if defined(_GLFW_HAS_XF86VIDMODE)
        XF86VidModeModeInfo **modelist;
        int modecount;

        // Get a list of all available display modes
        XF86VidModeGetAllModeLines(_glfwLibrary.X11.display,
                                   _glfwLibrary.X11.screen,
                                   &modecount, &modelist);

        // Unlock mode switch if necessary
        if (_glfwLibrary.X11.FS.modeChanged)
        {
            XF86VidModeLockModeSwitch(_glfwLibrary.X11.display,
                                      _glfwLibrary.X11.screen,
                                      0);
        }

        // Change the video mode to the desired mode
        XF86VidModeSwitchToMode(_glfwLibrary.X11.display,
                                _glfwLibrary.X11.screen,
                                modelist[mode]);

        // Set viewport to upper left corner (where our window will be)
        XF86VidModeSetViewPort(_glfwLibrary.X11.display,
                               _glfwLibrary.X11.screen,
                               0, 0);

        // Lock mode switch
        XF86VidModeLockModeSwitch(_glfwLibrary.X11.display,
                                  _glfwLibrary.X11.screen,
                                  1);

        // Remember old mode and flag that we have changed the mode
        if (!_glfwLibrary.X11.FS.modeChanged)
        {
            _glfwLibrary.X11.FS.oldMode = *modelist[0];
            _glfwLibrary.X11.FS.modeChanged = GL_TRUE;
        }

        XFree(modelist);
#endif /*_GLFW_HAS_XF86VIDMODE*/
    }
}


//========================================================================
// Change the current video mode
//========================================================================

void _glfwSetVideoMode(int* width, int* height, int* rate)
{
    int bestmode;

    // Find a best match mode
    bestmode = _glfwGetClosestVideoMode(width, height, rate);

    // Change mode
    _glfwSetVideoModeMODE(bestmode, *rate);
}


//========================================================================
// Restore the previously saved (original) video mode
//========================================================================

void _glfwRestoreVideoMode(void)
{
    if (_glfwLibrary.X11.FS.modeChanged)
    {
        if (_glfwLibrary.X11.RandR.available)
        {
#if defined(_GLFW_HAS_XRANDR)
            XRRScreenConfiguration* sc;

            if (_glfwLibrary.X11.RandR.available)
            {
                sc = XRRGetScreenInfo(_glfwLibrary.X11.display,
                                      _glfwLibrary.X11.root);

                XRRSetScreenConfig(_glfwLibrary.X11.display,
                                   sc,
                                   _glfwLibrary.X11.root,
                                   _glfwLibrary.X11.FS.oldSizeID,
                                   _glfwLibrary.X11.FS.oldRotation,
                                   CurrentTime);

                XRRFreeScreenConfigInfo(sc);
            }
#endif /*_GLFW_HAS_XRANDR*/
        }
        else if (_glfwLibrary.X11.VidMode.available)
        {
#if defined(_GLFW_HAS_XF86VIDMODE)
            // Unlock mode switch
            XF86VidModeLockModeSwitch(_glfwLibrary.X11.display,
                                      _glfwLibrary.X11.screen,
                                      0);

            // Change the video mode back to the old mode
            XF86VidModeSwitchToMode(_glfwLibrary.X11.display,
                                    _glfwLibrary.X11.screen,
                                    &_glfwLibrary.X11.FS.oldMode);
#endif /*_GLFW_HAS_XF86VIDMODE*/
        }

        _glfwLibrary.X11.FS.modeChanged = GL_FALSE;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

struct _glfwResolution
{
    int width;
    int height;
};

//========================================================================
// List available video modes
//========================================================================

int _glfwPlatformGetVideoModes(GLFWvidmode* list, int maxcount)
{
    int count, k, l, r, g, b, rgba, gl;
    int depth;
    XVisualInfo* vislist;
    XVisualInfo dummy;
    int viscount, rgbcount, rescount;
    int* rgbarray;
    struct _glfwResolution* resarray;

    // Get list of visuals
    vislist = XGetVisualInfo(_glfwLibrary.X11.display, 0, &dummy, &viscount);
    if (vislist == NULL)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "X11/GLX: Failed to retrieve the available visuals");
        return 0;
    }

    rgbarray = (int*) malloc(sizeof(int) * viscount);
    rgbcount = 0;

    // Build RGB array
    for (k = 0;  k < viscount;  k++)
    {
        // Does the visual support OpenGL & true color?
        glXGetConfig(_glfwLibrary.X11.display, &vislist[k], GLX_USE_GL, &gl);
        glXGetConfig(_glfwLibrary.X11.display, &vislist[k], GLX_RGBA, &rgba);
        if (gl && rgba)
        {
            // Get color depth for this visual
            depth = vislist[k].depth;

            // Convert to RGB
            _glfwSplitBPP(depth, &r, &g, &b);
            depth = (r << 16) | (g << 8) | b;

            // Is this mode unique?
            for (l = 0;  l < rgbcount;  l++)
            {
                if (depth == rgbarray[l])
                    break;
            }

            if (l >= rgbcount)
            {
                rgbarray[rgbcount] = depth;
                rgbcount++;
            }
        }
    }

    XFree(vislist);

    rescount = 0;
    resarray = NULL;

    // Build resolution array

    if (_glfwLibrary.X11.RandR.available)
    {
#if defined(_GLFW_HAS_XRANDR)
        XRRScreenConfiguration* sc;
        XRRScreenSize* sizelist;
        int sizecount;

        sc = XRRGetScreenInfo(_glfwLibrary.X11.display, _glfwLibrary.X11.root);
        sizelist = XRRConfigSizes(sc, &sizecount);

        resarray = (struct _glfwResolution*) malloc(sizeof(struct _glfwResolution) * sizecount);

        for (k = 0;  k < sizecount;  k++)
        {
            resarray[rescount].width = sizelist[k].width;
            resarray[rescount].height = sizelist[k].height;
            rescount++;
        }

        XRRFreeScreenConfigInfo(sc);
#endif /*_GLFW_HAS_XRANDR*/
    }
    else if (_glfwLibrary.X11.VidMode.available)
    {
#if defined(_GLFW_HAS_XF86VIDMODE)
        XF86VidModeModeInfo** modelist;
        int modecount, width, height;

        XF86VidModeGetAllModeLines(_glfwLibrary.X11.display,
                                   _glfwLibrary.X11.screen,
                                   &modecount, &modelist);

        resarray = (struct _glfwResolution*) malloc(sizeof(struct _glfwResolution) * modecount);

        for (k = 0;  k < modecount;  k++)
        {
            width  = modelist[k]->hdisplay;
            height = modelist[k]->vdisplay;

            // Is this mode unique?
            for (l = 0;  l < rescount;  l++)
            {
                if (width == resarray[l].width && height == resarray[l].height)
                    break;
            }

            if (l >= rescount)
            {
                resarray[rescount].width = width;
                resarray[rescount].height = height;
                rescount++;
            }
        }

        XFree(modelist);
#endif /*_GLFW_HAS_XF86VIDMODE*/
    }

    if (!resarray)
    {
        rescount = 1;
        resarray = (struct _glfwResolution*) malloc(sizeof(struct _glfwResolution) * rescount);

        resarray[0].width = DisplayWidth(_glfwLibrary.X11.display,
                                         _glfwLibrary.X11.screen);
        resarray[0].height = DisplayHeight(_glfwLibrary.X11.display,
                                           _glfwLibrary.X11.screen);
    }

    // Build permutations of colors and resolutions
    count = 0;
    for (k = 0;  k < rgbcount && count < maxcount;  k++)
    {
        for (l = 0;  l < rescount && count < maxcount;  l++)
        {
            list[count].width     = resarray[l].width;
            list[count].height    = resarray[l].height;
            list[count].redBits   = (rgbarray[k] >> 16) & 255;
            list[count].greenBits = (rgbarray[k] >> 8) & 255;
            list[count].blueBits  = rgbarray[k] & 255;
            count++;
        }
    }

    free(resarray);
    free(rgbarray);

    return count;
}


//========================================================================
// Get the desktop video mode
//========================================================================

void _glfwPlatformGetDesktopMode(GLFWvidmode* mode)
{
    int bpp;

    // Get and split display depth
    bpp = DefaultDepth(_glfwLibrary.X11.display, _glfwLibrary.X11.screen);
    _glfwSplitBPP(bpp, &mode->redBits, &mode->greenBits, &mode->blueBits);

    if (_glfwLibrary.X11.FS.modeChanged)
    {
        if (_glfwLibrary.X11.RandR.available)
        {
#if defined(_GLFW_HAS_XRANDR)
            mode->width  = _glfwLibrary.X11.FS.oldWidth;
            mode->height = _glfwLibrary.X11.FS.oldHeight;
#endif /*_GLFW_HAS_XRANDR*/
        }
        else if (_glfwLibrary.X11.VidMode.available)
        {
#if defined(_GLFW_HAS_XF86VIDMODE)
            mode->width  = _glfwLibrary.X11.FS.oldMode.hdisplay;
            mode->height = _glfwLibrary.X11.FS.oldMode.vdisplay;
#endif /*_GLFW_HAS_XF86VIDMODE*/
        }
    }
    else
    {
        mode->width = DisplayWidth(_glfwLibrary.X11.display,
                                   _glfwLibrary.X11.screen);
        mode->height = DisplayHeight(_glfwLibrary.X11.display,
                                     _glfwLibrary.X11.screen);
    }
}

