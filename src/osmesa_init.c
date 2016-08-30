
#include "internal.h"

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformInit(void)
{
  if (!_glfwInitThreadLocalStoragePOSIX()) {
    return GLFW_FALSE;
  }

  _glfwInitTimerPOSIX();

  return GLFW_TRUE;
}

void _glfwPlatformTerminate(void)
{
  _glfwTerminateOSMesa();
  _glfwTerminateThreadLocalStoragePOSIX();
}

const char* _glfwPlatformGetVersionString(void)
{
    const char* version = _GLFW_VERSION_NUMBER " OSMESA";
    return version;
}

