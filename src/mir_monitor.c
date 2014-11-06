//========================================================================
// GLFW 3.1 Mir - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2014 Brandon Schaefer <brandon.schaefer@canonical.com>
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

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

_GLFWmonitor** _glfwPlatformGetMonitors(int* count)
{
    int d, found = 0;
    MirDisplayConfiguration* display_config = mir_connection_create_display_config(_glfw.mir.connection);

    _GLFWmonitor** monitors = NULL;

    for (d = 0; d < display_config->num_outputs; d++)
    {
        MirDisplayOutput const* out = display_config->outputs + d;

        if (out->used &&
            out->connected &&
            out->num_modes &&
            out->current_mode < out->num_modes)
        {
            found++;
            monitors = realloc(monitors, sizeof(_GLFWmonitor*) * found);

            _GLFWmonitor* monitor = calloc(1, sizeof(_GLFWmonitor));

            monitor->mir.x         = out->position_x;
            monitor->mir.y         = out->position_y;
            monitor->mir.output_id = out->output_id;
            monitor->mir.cur_mode  = out->current_mode;
            monitor->modeCount     = out->num_modes;
            monitor->widthMM       = out->physical_width_mm;
            monitor->heightMM      = out->physical_height_mm;

            monitor->modes         = calloc(out->num_modes, sizeof(GLFWvidmode));

            int n_mode;
            for (n_mode = 0; n_mode < out->num_modes; n_mode++)
            {
                monitor->modes[n_mode].width       = out->modes[n_mode].horizontal_resolution;
                monitor->modes[n_mode].height      = out->modes[n_mode].vertical_resolution;
                monitor->modes[n_mode].refreshRate = out->modes[n_mode].refresh_rate;
            }

            _glfwPlatformGetVideoMode(monitor, &monitor->currentMode);

            monitors[d] = monitor;
        }
    }

    *count = found;
    mir_display_config_destroy(display_config);

    return monitors;
}

GLboolean _glfwPlatformIsSameMonitor(_GLFWmonitor* first, _GLFWmonitor* second)
{
    return first->mir.output_id == second->mir.output_id;
}

void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = monitor->mir.x;
    if (ypos)
        *ypos = monitor->mir.y;
}

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
    GLFWvidmode* modes = NULL;
    int i, count = monitor->modeCount;

    modes = calloc(count, sizeof(GLFWvidmode));
    for (i = 0; i < count; i++)
      modes[i] = monitor->modes[i];

    *found = count;
    return modes;
}

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
  *mode = monitor->modes[monitor->mir.cur_mode];
}

void _glfwPlatformGetGammaRamp(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported Function %s!\n", __PRETTY_FUNCTION__);
}

void _glfwPlatformSetGammaRamp(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported Function %s!\n", __PRETTY_FUNCTION__);
}
