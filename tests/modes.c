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

static void print_mode(GLFWvidmode* mode)
{
    printf("%i x %i x %i (%i %i %i)",
           mode->width, mode->height,
           mode->redBits + mode->greenBits + mode->blueBits,
           mode->redBits, mode->greenBits, mode->blueBits);
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

static void list_modes(GLFWvidmode* modes, int count)
{
    int i;
    GLFWvidmode mode;

    glfwGetDesktopMode(&mode);
    printf("Desktop mode: ");
    print_mode(&mode);
    putchar('\n');

    printf("Available modes:\n");

    for (i = 0;  i < count;  i++)
    {
        printf("%3i: ", i);
        print_mode(modes + i);
        putchar('\n');
    }
}

static void test_modes(GLFWvidmode* modes, int count)
{
    int i, width, height;

    glfwSetWindowSizeCallback(window_size_callback);
    glfwSetWindowCloseCallback(window_close_callback);

    for (i = 0;  i < count;  i++)
    {
        glfwOpenWindowHint(GLFW_RED_BITS, modes[i].redBits);
        glfwOpenWindowHint(GLFW_GREEN_BITS, modes[i].greenBits);
        glfwOpenWindowHint(GLFW_BLUE_BITS, modes[i].blueBits);

        printf("Opening ");
        print_mode(modes + i);
        printf(" window\n");

        window = glfwOpenWindow(modes[i].width, modes[i].height,
                                GLFW_FULLSCREEN, "Video Mode Test",
                                NULL);
        if (!window)
        {
            printf("Failed to enter mode %i: ", i);
            print_mode(modes + i);
            putchar('\n');
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

        if (glfwGetWindowParam(window, GLFW_RED_BITS) != modes[i].redBits ||
            glfwGetWindowParam(window, GLFW_GREEN_BITS) != modes[i].greenBits ||
            glfwGetWindowParam(window, GLFW_BLUE_BITS) != modes[i].blueBits)
        {
            printf("*** Color bit mismatch: (%i %i %i) instead of (%i %i %i)\n",
                   glfwGetWindowParam(window, GLFW_RED_BITS),
                   glfwGetWindowParam(window, GLFW_GREEN_BITS),
                   glfwGetWindowParam(window, GLFW_BLUE_BITS),
                   modes[i].redBits,
                   modes[i].greenBits,
                   modes[i].blueBits);
        }

        glfwGetWindowSize(window, &width, &height);

        if (width != modes[i].width || height != modes[i].height)
        {
            printf("*** Size mismatch: %ix%i instead of %ix%i\n",
                   width, height,
                   modes[i].width, modes[i].height);
        }

        printf("Closing window\n");

        glfwCloseWindow(window);
        glfwPollEvents();
        window = NULL;
    }
}

int main(int argc, char** argv)
{
    int ch, found, count = 0, mode = LIST_MODE;
    GLFWvidmode* modes = NULL;

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

    for (;;)
    {
        count += 256;
        modes = realloc(modes, sizeof(GLFWvidmode) * count);

        found = glfwGetVideoModes(modes, count);
        if (found < count)
            break;
    }

    if (mode == LIST_MODE)
        list_modes(modes, found);
    else if (mode == TEST_MODE)
        test_modes(modes, found);

    free(modes);
    modes = NULL;

    exit(EXIT_SUCCESS);
}

