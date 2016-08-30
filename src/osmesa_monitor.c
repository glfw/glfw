
#include "internal.h"

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

_GLFWmonitor** _glfwPlatformGetMonitors(int* count)
{
  // OSMesa is headless, so no monitors.
  _GLFWmonitor** monitors = NULL;
  if (count != NULL) {
    *count = 0;
  }
  return monitors;
}

int _glfwPlatformIsSameMonitor(_GLFWmonitor* first, _GLFWmonitor* second)
{
  return GLFW_FALSE;
}

void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos) {}

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
  return NULL;
}

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode) {}

void _glfwPlatformGetGammaRamp(_GLFWmonitor* monitor, GLFWgammaramp* ramp) {}

void _glfwPlatformSetGammaRamp(_GLFWmonitor* monitor,
                               const GLFWgammaramp* ramp) {}

//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////
