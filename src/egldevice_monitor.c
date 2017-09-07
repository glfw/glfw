//========================================================================
// GLFW 3.3 EGLDevice - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
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

static GLFWbool pickConnector(_GLFWmonitor* monitor, int drm_fd, drmModeRes* res_info)
{
    int i, j;

    // Getting suitable connector, encoder and crtc
    for (i = 0; i < res_info->count_connectors; i++)
    {
        monitor->egldevice.connId = res_info->connectors[i];

        drmModeConnector* conn_info;

        conn_info  =
            drmModeGetConnector(drm_fd, monitor->egldevice.connId);
        if (!conn_info)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "EGLDevice: Unable to obtain info for connector (%d)",
                            monitor->egldevice.connId);
            return GLFW_FALSE;
        }
        if (conn_info->connection == DRM_MODE_CONNECTED &&
            conn_info->count_modes > 0 &&
            conn_info->count_encoders > 0)
        {
                monitor->egldevice.encId = conn_info->encoders[0];

                drmModeEncoder* enc_info;

                enc_info =
                    drmModeGetEncoder(drm_fd, monitor->egldevice.encId);
                if (!enc_info) {
                    _glfwInputError(GLFW_PLATFORM_ERROR,
                                    "EGLDevice: Unable to query DRM-KMS information for connector index %d", i);
                }

                // Select the modesize
                monitor->currentMode.width = (int)conn_info->modes[0].hdisplay;
                monitor->currentMode.height = (int)conn_info->modes[0].vdisplay;
                monitor->currentMode.refreshRate = (int)conn_info->modes[0].vrefresh;
                monitor->modeCount = (int)conn_info->count_modes;
                monitor->name = conn_info->modes[0].name;

                for (j = 0; j < res_info->count_crtcs; j++)
                {
                    if ((enc_info->possible_crtcs & (1 << j)) == 0)
                        continue;

                    monitor->egldevice.crtcId = res_info->crtcs[j];
                    monitor->egldevice.crtcIndex = j;

                    if (res_info->crtcs[i] == enc_info->crtc_id)
                        break;
                }
                if (monitor->egldevice.crtcId == 0)
                {
                    _glfwInputError(GLFW_PLATFORM_ERROR,
                                    "EGLDevice: Unable to select suitable CRTC");
                    return GLFW_FALSE;
                }
                drmModeFreeEncoder(enc_info);
        }
        drmModeFreeConnector(conn_info);
    }
    return GLFW_TRUE;
}

static GLFWbool initDRMResources(_GLFWmonitor* monitor, int drm_fd)
{
    drmModeRes* res_info;

    res_info = drmModeGetResources(drm_fd);
    if (!res_info)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Couldn't obtain DRM-KMS resources");
        return GLFW_FALSE;
    }

    if(!pickConnector(monitor, drm_fd, res_info))
        return GLFW_FALSE;

    drmModeFreeResources(res_info);

    return GLFW_TRUE;
}

/////////////////////////////////////////////////////////////////////////////
///////////               GLFW platform API                    //////////////
/////////////////////////////////////////////////////////////////////////////

void _glfwPollMonitorsEGLDevice(void)
{
    _GLFWmonitor* monitor = _glfwAllocMonitor("Monitor", 0, 0);

     // Obtain DRM resource info
    if (!initDRMResources(monitor, _glfw.egldevice.drmFd))
    {
        _glfwFreeMonitor(monitor);
        return;
    }

    _glfwInputMonitor(monitor, GLFW_CONNECTED, _GLFW_INSERT_FIRST);
}

void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "EGLDevice: _glfwPlatformGetMonitorPos not implemented");
}

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "EGLDevice: _glfwPlatformGetVideoModes not implemented");
    *found = 0;

    return NULL;
}

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "EGLDevice: _glfwPlatformGetVideoMode not implemented");
}

void _glfwPlatformGetGammaRamp(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "EGLDevice: _glfwPlatformGetGammaRamp not implemented");
}

void _glfwPlatformSetGammaRamp(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "EGLDevice: _glfwPlatformSetGammaRamp not implemented");
}

