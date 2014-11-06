#include "internal.h"

#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

_GLFWmonitor** _glfwPlatformGetMonitors(int* count)
{
  // FIXME Work out the best way to get this from mir, as we'll end up looping
  // through all of that info... best to store it before we get here.
  _GLFWmonitor** monitors = calloc(1, sizeof(_GLFWmonitor*));
  return monitors;
}

GLboolean _glfwPlatformIsSameMonitor(_GLFWmonitor* first, _GLFWmonitor* second)
{
  return 0;
}

void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
}

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
  return NULL;
}

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
}

void _glfwPlatformGetGammaRamp(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    // TODO
    fprintf(stderr, "_glfwPlatformGetGammaRamp not implemented yet\n");
}

void _glfwPlatformSetGammaRamp(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    // TODO
    fprintf(stderr, "_glfwPlatformSetGammaRamp not implemented yet\n");
}
