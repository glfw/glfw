//========================================================================
// Video mode test
// Copyright (c) Camilla Berglund <elmindreda@elmindreda.org>
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
// This test enumerates or verifies video modes
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "getopt.h"

static GLFWwindow window = NULL;

enum Mode
{
    LIST_MODE,
    TEST_MODE
};

static void usage(void)
{
    printf("Usage: modes [-t]\n");
    printf("       modes -h\n");
}

static const char* format_mode(GLFWvidmode* mode)
{
    static char buffer[512];

    snprintf(buffer, sizeof(buffer),
             "%i x %i x %i (%i %i %i)",
             mode->width, mode->height,
             mode->redBits + mode->greenBits + mode->blueBits,
             mode->redBits, mode->greenBits, mode->blueBits);

    buffer[sizeof(buffer) - 1] = '\0';
    return buffer;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_size_callback(GLFWwindow window, int width, int height)
{
    printf("Window resized to %ix%i\n", width, height);

    glViewport(0, 0, width, height);
}

static int window_close_callback(GLFWwindow dummy)
{
    window = NULL;
    return GL_TRUE;
}

static void key_callback(GLFWwindow dummy, int key, int action)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        glfwDestroyWindow(window);
        window = NULL;
    }
}

static void list_modes(void)
{
    int count, i;
    GLFWvidmode desktop_mode;
    GLFWvidmode* modes = glfwGetVideoModes(&count);

    glfwGetDesktopMode(&desktop_mode);
    printf("Desktop mode: %s\n", format_mode(&desktop_mode));

    printf("Available modes:\n");

    for (i = 0;  i < count;  i++)
    {
        printf("%3u: %s", (unsigned int) i, format_mode(modes + i));

        if (memcmp(&desktop_mode, modes + i, sizeof(GLFWvidmode)) == 0)
            printf(" (desktop mode)");

        putchar('\n');
    }
}

static void test_modes(void)
{
    int i, count, width, height;
    GLFWvidmode* modes = glfwGetVideoModes(&count);

    glfwSetWindowSizeCallback(window_size_callback);
    glfwSetWindowCloseCallback(window_close_callback);
    glfwSetKeyCallback(key_callback);

    for (i = 0;  i < count;  i++)
    {
        GLFWvidmode* mode = modes + i;

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);

        printf("Testing mode %u: %s", (unsigned int) i, format_mode(mode));

        window = glfwCreateWindow(mode->width, mode->height,
                                  GLFW_FULLSCREEN, "Video Mode Test",
                                  NULL);
        if (!window)
        {
            printf("Failed to enter mode %u: %s\n",
                   (unsigned int) i,
                   format_mode(mode));
            continue;
        }

        glfwSetTime(0.0);
        glfwSwapInterval(1);

        while (glfwGetTime() < 5.0)
        {
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers();
            glfwPollEvents();

            if (!window)
            {
                printf("User terminated program\n");
                exit(EXIT_SUCCESS);
            }
        }

        if (glfwGetWindowParam(window, GLFW_RED_BITS) != mode->redBits ||
            glfwGetWindowParam(window, GLFW_GREEN_BITS) != mode->greenBits ||
            glfwGetWindowParam(window, GLFW_BLUE_BITS) != mode->blueBits)
        {
            printf("*** Color bit mismatch: (%i %i %i) instead of (%i %i %i)\n",
                   glfwGetWindowParam(window, GLFW_RED_BITS),
                   glfwGetWindowParam(window, GLFW_GREEN_BITS),
                   glfwGetWindowParam(window, GLFW_BLUE_BITS),
                   mode->redBits,
                   mode->greenBits,
                   mode->blueBits);
        }

        glfwGetWindowSize(window, &width, &height);

        if (width != mode->width || height != mode->height)
        {
            printf("*** Size mismatch: %ix%i instead of %ix%i\n",
                   width, height,
                   mode->width, mode->height);
        }

        printf("Closing window\n");

        glfwDestroyWindow(window);
        glfwPollEvents();
        window = NULL;
    }
}

int main(int argc, char** argv)
{
    int ch, mode = LIST_MODE;

    while ((ch = getopt(argc, argv, "th")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 't':
                mode = TEST_MODE;
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    argc -= optind;
    argv += optind;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if (mode == LIST_MODE)
        list_modes();
    else if (mode == TEST_MODE)
        test_modes();

    exit(EXIT_SUCCESS);
}

