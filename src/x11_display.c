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

#include <stdlib.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

#if defined (_GLFW_HAS_XRANDR)
_GLFWmonitor** _glfwCreateMonitor(_GLFWmonitor** current, XRROutputInfo* outputInfo, XRRCrtcInfo* crtcInfo)
{
    *current = _glfwMalloc(sizeof(_GLFWmonitor));
    memset(*current, 0, sizeof(_GLFWmonitor));

    (*current)->physicalWidth  = outputInfo->mm_width;
    (*current)->physicalHeight = outputInfo->mm_height;

    memcpy((*current)->deviceName, outputInfo->name, GLFW_MONITOR_PARAM_S_NAME_LEN+1);
    (*current)->deviceName[GLFW_MONITOR_PARAM_S_NAME_LEN] = '\0';

    (*current)->screenXPosition = crtcInfo->x;
    (*current)->screenYPosition = crtcInfo->y;

    (*current)->X11.output = outputInfo;
    return &((*current)->next);
}
#endif /*_GLFW_HAS_XRANDR*/

_GLFWmonitor* _glfwDestroyMonitor(_GLFWmonitor* monitor)
{
    _GLFWmonitor* result;

    result = monitor->next;

#if defined (_GLFW_HAS_XRANDR)
    XRRFreeOutputInfo(monitor->X11.output);    
#endif /*_GLFW_HAS_XRANDR*/

    _glfwFree(monitor);

    return result;
}

void _glfwInitMonitors(void)
{
    _glfwLibrary.monitorListHead = NULL;

    if(_glfwLibrary.X11.RandR.available == GL_TRUE)
    {
#if defined (_GLFW_HAS_XRANDR)
        XRRScreenResources* resources;
        int outputIDX;
        _GLFWmonitor** curMonitor;

        curMonitor = &_glfwLibrary.monitorListHead;

        resources = XRRGetScreenResources(_glfwLibrary.X11.display,
                                          _glfwLibrary.X11.root);

        for(outputIDX = 0; outputIDX < resources->noutput; outputIDX++)
        {
            // physical device
            XRROutputInfo* outputInfo = NULL;
            // logical surface
            XRRCrtcInfo* crtcInfo = NULL;
            int crtcIDX;

            outputInfo = XRRGetOutputInfo(_glfwLibrary.X11.display,
                                          resources,
                                          resources->outputs[outputIDX]);

            if(outputInfo->connection == RR_Connected)
            {
                for(crtcIDX = 0; crtcIDX < outputInfo->ncrtc; crtcIDX++)
                {
                    if(outputInfo->crtc == outputInfo->crtcs[crtcIDX])
                    {
                        crtcInfo = XRRGetCrtcInfo(_glfwLibrary.X11.display,
                                                  resources,
                                                  outputInfo->crtcs[crtcIDX]);
                        break;
                    }
                }

                curMonitor = _glfwCreateMonitor(curMonitor, outputInfo, crtcInfo);

                // Freeing of the outputInfo is done in _glfwDestroyMonitor
                XRRFreeCrtcInfo(crtcInfo);
            }
        }
#endif /*_GLFW_HAS_XRANDR*/
    }
}

void _glfwTerminateMonitors(void)
{
    while(_glfwLibrary.monitorListHead)
        _glfwLibrary.monitorListHead = _glfwDestroyMonitor(_glfwLibrary.monitorListHead);
}

