//========================================================================
// Monitor information tool
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
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
//
// This test prints monitor and video mode information or verifies video
// modes
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "getopt.h"

enum Mode
{
    LIST_MODE_OLD,
    LIST_MODE,
    TEST_MODE
};

static void usage(void)
{
    printf("Usage: monitors [-t]\n");
    printf("       monitors -h\n");
}

static int euclid(int a, int b)
{
    return b ? euclid(b, a % b) : a;
}

static const char* format_mode(const GLFWvidmode* mode)
{
    static char buffer[512];
    const int gcd = euclid(mode->width, mode->height);

    snprintf(buffer,
             sizeof(buffer),
             "%i x %i x %i (%i:%i) (%i %i %i) %i Hz",
             mode->width, mode->height,
             mode->redBits + mode->greenBits + mode->blueBits,
             mode->width / gcd, mode->height / gcd,
             mode->redBits, mode->greenBits, mode->blueBits,
             mode->refreshRate);

    buffer[sizeof(buffer) - 1] = '\0';
    return buffer;
}

static const char* format_mode2(const GLFWvideoMode* mode)
{
    int width, height;
    int r, g, b;
    float rate;
    
    glfwVideoModeGetSize(mode, &width, &height);
    glfwVideoModeGetColorDepth(mode, &r, &g, &b);
    rate = glfwVideoModeGetRefreshRate(mode);
    
    static char buffer[512];
    const int gcd = euclid(width, height);

    snprintf(buffer,
             sizeof(buffer),
             "%i x %i x %i (%i:%i) (%i %i %i) %f Hz",
             width, height,
             r + g + b,
             width / gcd, height / gcd,
             r, g, b,
             rate);

    buffer[sizeof(buffer) - 1] = '\0';
    return buffer;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    printf("Framebuffer resized to %ix%i\n", width, height);

    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void list_modes(GLFWmonitor* monitor)
{
    int count, x, y, width_mm, height_mm, i;
    int workarea_x, workarea_y, workarea_width, workarea_height;
    float xscale, yscale;

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    const GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);

    glfwGetMonitorPos(monitor, &x, &y);
    glfwGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    glfwGetMonitorWorkarea(monitor, &workarea_x, &workarea_y, &workarea_width, &workarea_height);

    printf("Name: %s (%s)\n",
           glfwGetMonitorName(monitor),
           glfwGetPrimaryMonitor() == monitor ? "primary" : "secondary");
    printf("Current mode: %s\n", format_mode(mode));
    printf("Virtual position: %i, %i\n", x, y);
    printf("Content scale: %f x %f\n", xscale, yscale);

    printf("Physical size: %i x %i mm (%0.2f dpi at %i x %i)\n",
           width_mm, height_mm, mode->width * 25.4f / width_mm, mode->width, mode->height);
    printf("Monitor work area: %i x %i starting at %i, %i\n",
            workarea_width, workarea_height, workarea_x, workarea_y);

    printf("Modes:\n");

    for (i = 0;  i < count;  i++)
    {
        printf("%3u: %s", (unsigned int) i, format_mode(modes + i));

        if (memcmp(mode, modes + i, sizeof(GLFWvidmode)) == 0)
            printf(" (current mode)");

        putchar('\n');
    }
}

static void list_modes2(GLFWmonitor* monitor)
{
    int width, height;
    int count, x, y, width_mm, height_mm, i;
    int workarea_x, workarea_y, workarea_width, workarea_height;
    float xscale, yscale;

    const GLFWvideoMode* mode = glfwGetVideoMode2(monitor);
    const GLFWvideoMode* const* modes = glfwGetVideoModes2(monitor, &count);

    glfwGetMonitorPos(monitor, &x, &y);
    glfwGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    glfwGetMonitorWorkarea(monitor, &workarea_x, &workarea_y, &workarea_width, &workarea_height);
    glfwVideoModeGetSize(mode, &width, &height);

    printf("Name: %s (%s)\n",
           glfwGetMonitorName(monitor),
           glfwGetPrimaryMonitor() == monitor ? "primary" : "secondary");
    printf("Current mode: %s\n", format_mode2(mode));
    printf("Virtual position: %i, %i\n", x, y);
    printf("Content scale: %f x %f\n", xscale, yscale);

    printf("Physical size: %i x %i mm (%0.2f dpi at %i x %i)\n",
           width_mm, height_mm, width * 25.4f / width_mm, width, height);
    printf("Monitor work area: %i x %i starting at %i, %i\n",
            workarea_width, workarea_height, workarea_x, workarea_y);

    printf("Modes:\n");

    for (i = 0;  i < count;  i++)
    {
        printf("%3u: %s", (unsigned int) i, format_mode2(modes[i]));

        if (glfwVideoModesEqual(mode, modes[i]))
            printf(" (current mode)");

        putchar('\n');
    }
}

static void test_modes(GLFWmonitor* monitor)
{
    int i, count;
    GLFWwindow* window;
    const GLFWvideoMode* const* modes = glfwGetVideoModes2(monitor, &count);

    for (i = 0;  i < count;  i++)
    {
        const GLFWvideoMode* mode = modes[i];
        int width, height, currentWidth, currentHeight;
        int red, green, blue, currentRed, currentGreen, currentBlue;
        double refreshRate, currentRefreshRate;
    
        glfwVideoModeGetSize(mode, &width, &height);
        glfwVideoModeGetColorDepth(mode, &red, &green, &blue);
        refreshRate = glfwVideoModeGetRefreshRate(mode);

        glfwWindowHint(GLFW_RED_BITS, red);
        glfwWindowHint(GLFW_GREEN_BITS, green);
        glfwWindowHint(GLFW_BLUE_BITS, blue);
        glfwWindowHintDouble(GLFW_REFRESH_RATE, refreshRate);

        printf("Testing mode %u on monitor %s: %s\n",
               (unsigned int) i,
               glfwGetMonitorName(monitor),
               format_mode2(mode));

        window = glfwCreateWindow(width, height,
                                  "Video Mode Test",
                                  monitor,
                                  NULL);
        if (!window)
        {
            printf("Failed to enter mode %u: %s\n",
                   (unsigned int) i,
                   format_mode2(mode));
            continue;
        }

        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetKeyCallback(window, key_callback);

        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(1);

        glfwSetTime(0.0);

        while (glfwGetTime() < 5.0)
        {
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(window);
            glfwPollEvents();

            if (glfwWindowShouldClose(window))
            {
                printf("User terminated program\n");

                glfwTerminate();
                exit(EXIT_SUCCESS);
            }
        }

        glGetIntegerv(GL_RED_BITS, &currentRed);
        glGetIntegerv(GL_GREEN_BITS, &currentGreen);
        glGetIntegerv(GL_BLUE_BITS, &currentBlue);

        glfwGetWindowSize(window, &currentWidth, &currentHeight);

        if (currentRed != red ||
            currentGreen != green ||
            currentBlue != blue)
        {
            printf("*** Color bit mismatch: (%i %i %i) instead of (%i %i %i)\n",
                   currentRed, currentGreen, currentBlue,
                   red, green, blue);
        }

        if (currentWidth != width || currentHeight != height)
        {
            printf("*** Size mismatch: %ix%i instead of %ix%i\n",
                   currentWidth, currentHeight,
                   width, height);
        }

        printf("Closing window\n");

        glfwDestroyWindow(window);
        window = NULL;

        glfwPollEvents();
    }
}

int main(int argc, char** argv)
{
    int ch, i, count, mode = LIST_MODE;
    GLFWmonitor** monitors;

    while ((ch = getopt(argc, argv, "tho")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 'o':
                mode = LIST_MODE_OLD;
                break;
            case 't':
                mode = TEST_MODE;
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    glfwSetErrorCallback(error_callback);

    glfwInitHint(GLFW_COCOA_MENUBAR, GLFW_FALSE);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    monitors = glfwGetMonitors(&count);

    for (i = 0;  i < count;  i++)
    {
        if (mode == LIST_MODE)
            list_modes2(monitors[i]);
        else if (mode == LIST_MODE_OLD)
            list_modes(monitors[i]);
        else if (mode == TEST_MODE)
            test_modes(monitors[i]);
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

