//========================================================================
// GLFW 3.5 OGC - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2024 Alberto Mardegan <info@mardy.it>
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

#if defined(_GLFW_OGC)

#include <ogc/color.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_OGC_MODES 4 /* - VIDEO_GetPreferredMode()
                           - 240p progressive mode
                           - 480p progressive mode
                           - 528p PAL progressive mode
                         */

static GXRModeObj *s_videoModes[MAX_OGC_MODES + 1]; // +1 for the NULL entry

// Inverse of the VI_TVMODE macro
#define VI_FORMAT_FROM_MODE(tvmode) (tvmode >> 2)

static void setupXfb(_GLFWmonitor* monitor)
{
    if (_glfw.ogc.xfb[0]) {
        free(MEM_K1_TO_K0(_glfw.ogc.xfb[0]));
        _glfw.ogc.xfb[0] = NULL;
    }
    if (_glfw.ogc.xfb[1]) {
        free(MEM_K1_TO_K0(_glfw.ogc.xfb[1]));
        _glfw.ogc.xfb[1] = NULL;
    }

    if (!_glfw.ogc.xfb[0]) {
        _glfw.ogc.xfb[0] =
            MEM_K0_TO_K1(SYS_AllocateFramebuffer(monitor->ogc.ogcMode));
    }
    if (monitor->window && monitor->window->doublebuffer) {
        _glfw.ogc.xfb[1] =
            MEM_K0_TO_K1(SYS_AllocateFramebuffer(monitor->ogc.ogcMode));
    }

    _glfw.ogc.fbIndex = 0;
}

static void ogcVideoModeToGlfw(const GXRModeObj *in, GLFWvidmode *out)
{
    out->width = in->fbWidth;
    out->height = in->efbHeight;
    out->redBits = out->greenBits = out->blueBits = 8;
    u32 format = VI_FORMAT_FROM_MODE(in->viTVMode);
    switch (format) {
    case VI_DEBUG:
    case VI_NTSC:
    case VI_EURGB60:
    case VI_MPAL:
        out->refreshRate = 60;
        break;
    case VI_PAL:
    case VI_DEBUG_PAL:
        out->refreshRate = 50;
        break;
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwCreateMonitorOgc()
{
    _GLFWmonitor* monitor = _glfwAllocMonitor("", 0, 0);
    _glfwInputMonitor(monitor, GLFW_CONNECTED, _GLFW_INSERT_FIRST);
}

void _glfwSetVideoModeOgc(_GLFWmonitor* monitor, const GLFWvidmode* desired)
{
    int i;
    const GLFWvidmode* best = _glfwChooseVideoMode(monitor, desired);

    fprintf(stderr, "%s:%d %s chosen %dx%d window %p\n", __FILE__, __LINE__, __func__,
            best->width, best->height, monitor->window);
    for (i = 0; i < monitor->modeCount; i++) {
        GLFWvidmode tmp;
        ogcVideoModeToGlfw(s_videoModes[i], &tmp);
        if (_glfwCompareVideoModes(best, &tmp) == 0)
            break;
    }
    /* In the unlikely case that this is not one of our modes, just pick our
     * preferred one */
    if (i >= monitor->modeCount) i = 0;

    GXRModeObj *vmode = s_videoModes[i];
    monitor->ogc.currentMode = i;
    monitor->ogc.ogcMode = vmode;

    setupXfb(monitor);

    VIDEO_Configure(vmode);
    VIDEO_ClearFrameBuffer(vmode, _glfw.ogc.xfb[0], COLOR_BLACK);
    VIDEO_SetNextFramebuffer(_glfw.ogc.xfb[0]);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();

    VIDEO_WaitVSync();
    if (vmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    float yscale = GX_GetYScaleFactor(vmode->efbHeight, vmode->xfbHeight);
    GX_SetDispCopyYScale(yscale);
    GX_SetDispCopySrc(0, 0, vmode->fbWidth, vmode->efbHeight);
    GX_SetDispCopyDst(vmode->fbWidth, vmode->xfbHeight);
    GX_SetCopyFilter(vmode->aa, vmode->sample_pattern, GX_TRUE, vmode->vfilter);
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwFreeMonitorOgc(_GLFWmonitor* monitor)
{
}

void _glfwGetMonitorPosOgc(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;
}

void _glfwGetMonitorContentScaleOgc(_GLFWmonitor* monitor,
                                    float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.0f;
    if (yscale)
        *yscale = 1.0f;
}

void _glfwGetMonitorWorkareaOgc(_GLFWmonitor* monitor,
                                int* xpos, int* ypos,
                                int* width, int* height)
{
    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;
    if (width)
        *width = monitor->modes[monitor->ogc.currentMode].width;
    if (height)
        *height = monitor->modes[monitor->ogc.currentMode].height;
}

GLFWvidmode* _glfwGetVideoModesOgc(_GLFWmonitor* monitor, int* found)
{
    int i, count;

    fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__);

    i = 0;
    s_videoModes[i++] = VIDEO_GetPreferredMode(NULL);
    /* Add more video modes, depending on the TV mode of the preferred mode
     * (NTSC, PAL, etc.) */
    switch (VI_FORMAT_FROM_MODE(s_videoModes[0]->viTVMode))
    {
    case VI_DEBUG:
    case VI_NTSC:
        s_videoModes[i++] = &TVNtsc240Ds;
        s_videoModes[i++] = &TVNtsc480Prog;
        break;
    case VI_MPAL:
        s_videoModes[i++] = &TVMpal240Ds;
        s_videoModes[i++] = &TVMpal480Prog;
        break;
    case VI_EURGB60:
        s_videoModes[i++] = &TVEurgb60Hz240Ds;
        s_videoModes[i++] = &TVEurgb60Hz480Prog;
        s_videoModes[i++] = &TVPal528Prog;
        break;
    case VI_PAL:
    case VI_DEBUG_PAL:
        s_videoModes[i++] = &TVPal264Ds;
        s_videoModes[i++] = &TVPal576ProgScale; // EFB height is 480
        s_videoModes[i++] = &TVPal528Prog;
        break;
    default:
        return NULL;
    }
    s_videoModes[i] = NULL;

    GLFWvidmode* modes = _glfw_calloc(i, sizeof(GLFWvidmode));

    count = i;
    for (int i = 0; i < count; i++)
    {
        ogcVideoModeToGlfw(s_videoModes[i], &modes[i]);
    }

    *found = count;
    return modes;
}

GLFWbool _glfwGetVideoModeOgc(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
    fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__);
    *mode = monitor->modes[monitor->ogc.currentMode];
    return GLFW_TRUE;
}

GLFWbool _glfwGetGammaRampOgc(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "gamma ramp");
    return GLFW_FALSE;
}

void _glfwSetGammaRampOgc(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    _glfwInputError(GLFW_FEATURE_UNAVAILABLE, _glfwUnimplementedFmt,
                    "gamma ramp");
}

#endif // _GLFW_OGC
