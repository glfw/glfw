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
    GLFWmonitor monitor;
    GLFWvidmode dtmode, modes[400];
    int modecount, i;

    if (!glfwInit(NULL))
    {
        fprintf(stderr, "Failed to initialize GLFW: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    // Show desktop video mode
    glfwGetDesktopMode(&dtmode);
    printf("Desktop mode: ");
    print_mode(&dtmode);

    monitor = NULL;

    while ((monitor = glfwGetNextMonitor(monitor)))
    {
        printf("Monitor name: %s\n"
               "Physical dimensions: %dmm x %dmm\n"
               "Logical position: (%d,%d)\n",
               glfwGetMonitorString(monitor, GLFW_MONITOR_NAME),
               glfwGetMonitorParam(monitor, GLFW_MONITOR_PHYSICAL_WIDTH),
               glfwGetMonitorParam(monitor, GLFW_MONITOR_PHYSICAL_HEIGHT),
               glfwGetMonitorParam(monitor, GLFW_MONITOR_SCREEN_POS_X),
               glfwGetMonitorParam(monitor, GLFW_MONITOR_SCREEN_POS_Y));

        // List available video modes
        modecount = glfwGetVideoModes(monitor, modes, sizeof(modes) / sizeof(GLFWvidmode));
        printf("Available modes:\n");

        for (i = 0;  i < modecount;  i++)
        {
            printf("%3i: ", i);
            print_mode(modes + i);
        }
    }

    exit(EXIT_SUCCESS);
}

