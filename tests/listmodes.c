//========================================================================
// This is a small test application for GLFW.
// The program lists all available fullscreen video modes.
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

static void print_mode(GLFWvidmode* mode)
{
    printf("%i x %i x %i (%i %i %i)\n",
           mode->width, mode->height,
           mode->redBits + mode->greenBits + mode->blueBits,
           mode->redBits, mode->greenBits, mode->blueBits);
}

int main(void)
{
    GLFWmonitor monitorHandle;
    GLFWvidmode dtmode, modes[400];
    int modecount, i;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    // Show desktop video mode
    glfwGetDesktopMode(&dtmode);
    printf("Desktop mode: ");
    print_mode(&dtmode);

    monitorHandle = GLFW_MONITOR_INVALID_HANDLE;

    while( GLFW_MONITOR_INVALID_HANDLE != ( monitorHandle = glfwGetNextMonitor( monitorHandle )))
    {
        printf( "Monitor name: %s\n"
                "Physical dimensions: %dmm x %dmm\n"
                "Logical position: (%d,%d)\n",
                glfwGetMonitorStringParam( monitorHandle, GLFW_MONITOR_PARAM_S_NAME ),
                glfwGetMonitorIntegerParam( monitorHandle, GLFW_MONITOR_PARAM_I_PHYS_WIDTH ),
                glfwGetMonitorIntegerParam( monitorHandle, GLFW_MONITOR_PARAM_I_PHYS_HEIGHT ),
                glfwGetMonitorIntegerParam( monitorHandle, GLFW_MONITOR_PARAM_I_SCREEN_X_POS ),
                glfwGetMonitorIntegerParam( monitorHandle, GLFW_MONITOR_PARAM_I_SCREEN_Y_POS )
        );
        // List available video modes
        modecount = glfwGetVideoModes(monitorHandle, modes, sizeof(modes) / sizeof(GLFWvidmode));
        printf( "Available modes:\n" );
        for( i = 0; i < modecount; i ++ )
        {
            printf("%3i: ", i);
            print_mode(modes + i);
        }
    }
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

