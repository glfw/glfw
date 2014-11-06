#include "internal.h"

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformInit(void)
{
    _glfw.mir.connection = mir_connect_sync(NULL, __PRETTY_FUNCTION__);

    if (!mir_connection_is_valid(_glfw.mir.connection))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Mir: Unable to connect to Server\n");
        return GL_FALSE;
    }

    _glfw.mir.native_display = mir_connection_get_egl_native_display(_glfw.mir.connection);

    // TODO Add in bits to get the correct monitors and screen sizes...
    // Ill just hard code in my own right now to jump ahead to surface and events.

    return GL_TRUE;
}

void _glfwPlatformTerminate(void)
{
    mir_connection_release(_glfw.mir.connection);
}

const char* _glfwPlatformGetVersionString(void)
{
    return "MIR // FIXME (<0_0>)";
}

