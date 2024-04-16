//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2019 Camilla Löwy <elmindreda@glfw.org>
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// The the sole (fake) video mode of our (sole) fake monitor
//
static GLFWvidmode getVideoMode(void)
{
    APTR handle;
    struct DimensionInfo diminfo;
    struct DisplayInfo dispinfo;
    GLFWvidmode mode;
    ULONG modeid;

    IIntuition->GetScreenAttrs(_glfw.os4.publicScreen, SA_DisplayID, &modeid, TAG_DONE);

    handle = IGraphics->FindDisplayInfo(modeid);
    if (!handle) {
        goto out;
    }

    if (!IGraphics->GetDisplayInfoData(handle, (UBYTE *)&diminfo, sizeof(diminfo), DTAG_DIMS, 0)) {
        dprintf("Failed to get dim info\n");
        goto out;
    }

    if (!IGraphics->GetDisplayInfoData(handle, (UBYTE *)&dispinfo, sizeof(dispinfo), DTAG_DISP, 0)) {
        dprintf("Failed to get disp info\n");
        goto out;
    }

    mode.width = diminfo.Nominal.MaxX - diminfo.Nominal.MinX + 1;
    mode.height = diminfo.Nominal.MaxY - diminfo.Nominal.MinY + 1;
    mode.refreshRate = 60; // grab DTAG_MNTR?
    if (dispinfo.PropertyFlags & DIPF_IS_RTG) {
        dprintf("RTG mode %d: w=%d, h=%d, bits=%d\n", modeid, mode.width, mode.height, diminfo.MaxDepth);

        switch (diminfo.MaxDepth) {
        case 32:
            mode.redBits = 8;
            mode.greenBits = 8;
            mode.blueBits = 8;
            break;
        case 24:
            mode.redBits = 8;
            mode.greenBits = 8;
            mode.blueBits = 8;
            break;
        case 16:
            mode.redBits = 5;
            mode.greenBits = 6;
            mode.blueBits = 5;
            break;
        case 15:
            mode.redBits = 5;
            mode.greenBits = 5;
            mode.blueBits = 5;
            break;
        default:
            // TODO - What we have to use for 8?
            mode.redBits = 8;
            mode.greenBits = 8;
            mode.blueBits = 8;
            break;
        }

        return mode;
    }

out:
    // TODO - Change this fallback
    mode.refreshRate = 60;
    mode.redBits = 8;
    mode.greenBits = 8;
    mode.blueBits = 8;

    return mode;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPollMonitorsOS4(void)
{
    const float dpi = 141.f;
    const GLFWvidmode mode = getVideoMode();
    struct DimensionInfo diminfo;
    APTR handle;
    ULONG modeid;
    _glfw.os4.publicScreen = IIntuition->LockPubScreen(NULL);
    if (_glfw.os4.publicScreen != NULL) {
        IIntuition->GetScreenAttrs(_glfw.os4.publicScreen, SA_DisplayID, &modeid, TAG_DONE);

        handle = IGraphics->FindDisplayInfo(modeid);
        if (handle) {
            if (IGraphics->GetDisplayInfoData(handle, (UBYTE *)&diminfo, sizeof(diminfo), DTAG_DIMS, 0)) {
                GLFW_DisplayModeData *data;
                data = (GLFW_DisplayModeData *) malloc(sizeof(*data));
                if (data) {
                    data->modeid = modeid;
                    data->x = diminfo.Nominal.MinX;
                    data->y = diminfo.Nominal.MinY;
                    data->depth = diminfo.MaxDepth;

                    _GLFWmonitor* monitor = _glfwAllocMonitor("OS4 Monitor 0",
                                                            (int) (mode.width * 25.4f / dpi),
                                                            (int) (mode.height * 25.4f / dpi));
                    
                    monitor->userPointer = data;
                    
                    _glfwInputMonitor(monitor, GLFW_CONNECTED, _GLFW_INSERT_FIRST);
                }
            }
        }
        IIntuition->UnlockPubScreen(NULL, _glfw.os4.publicScreen);
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwFreeMonitorOS4(_GLFWmonitor* monitor)
{
    if (monitor->userPointer)
        free(monitor->userPointer);

    _glfwFreeGammaArrays(&monitor->os4.ramp);
}

void _glfwGetMonitorPosOS4(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;
}

void _glfwGetMonitorContentScaleOS4(_GLFWmonitor* monitor,
                                     float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _glfwGetMonitorWorkareaOS4(_GLFWmonitor* monitor,
                                 int* xpos, int* ypos,
                                 int* width, int* height)
{
    const GLFWvidmode mode = getVideoMode();

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 10;
    if (width)
        *width = mode.width;
    if (height)
        *height = mode.height - 10;
}

GLFWvidmode* _glfwGetVideoModesOS4(_GLFWmonitor* monitor, int* found)
{
    GLFWvidmode* mode = _glfw_calloc(1, sizeof(GLFWvidmode));
    *mode = getVideoMode();
    *found = 1;
    return mode;
}

GLFWbool _glfwGetVideoModeOS4(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
    *mode = getVideoMode();
    return GLFW_TRUE;
}

GLFWbool _glfwGetGammaRampOS4(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    if (!monitor->os4.ramp.size)
    {
        unsigned int i;

        _glfwAllocGammaArrays(&monitor->os4.ramp, 256);

        for (i = 0;  i < monitor->os4.ramp.size;  i++)
        {
            const float gamma = 2.2f;
            float value;
            value = i / (float) (monitor->os4.ramp.size - 1);
            value = powf(value, 1.f / gamma) * 65535.f + 0.5f;
            value = fminf(value, 65535.f);

            monitor->os4.ramp.red[i]   = (unsigned short) value;
            monitor->os4.ramp.green[i] = (unsigned short) value;
            monitor->os4.ramp.blue[i]  = (unsigned short) value;
        }
    }

    _glfwAllocGammaArrays(ramp, monitor->os4.ramp.size);
    memcpy(ramp->red,   monitor->os4.ramp.red,   sizeof(short) * ramp->size);
    memcpy(ramp->green, monitor->os4.ramp.green, sizeof(short) * ramp->size);
    memcpy(ramp->blue,  monitor->os4.ramp.blue,  sizeof(short) * ramp->size);
    return GLFW_TRUE;
}

void _glfwSetGammaRampOS4(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    if (monitor->os4.ramp.size != ramp->size)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "OS4: Gamma ramp size must match current ramp size");
        return;
    }

    memcpy(monitor->os4.ramp.red,   ramp->red,   sizeof(short) * ramp->size);
    memcpy(monitor->os4.ramp.green, ramp->green, sizeof(short) * ramp->size);
    memcpy(monitor->os4.ramp.blue,  ramp->blue,  sizeof(short) * ramp->size);
}

/**********************************************************************************************/
/******************************************** PRIVATE METHODS *********************************/
/**********************************************************************************************/
