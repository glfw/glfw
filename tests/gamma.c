//========================================================================
// Gamma correction test program
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
// This program is used to test the gamma correction functionality for
// both fullscreen and windowed mode windows
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static GLfloat ggamma = 1.0f;
static GLfloat ggain = 1.0f;
static GLfloat gblacklevel = 0.0f;

static void usage(void)
{
    printf("Usage: gammatest [-h] [-f]\n");
}

static void key_callback(GLFWwindow window, int key, int action)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_ESC:
            glfwCloseWindow(window);
            break;
        case 'Q':
            ggamma += 0.1f;
            printf("Gamma: %f\n", ggamma);
            glfwSetGammaFormula( ggamma, gblacklevel, ggain );
            break;
        case 'W':
            ggamma -= 0.1f;
            printf("Gamma: %f\n", ggamma);
            glfwSetGammaFormula( ggamma, gblacklevel, ggain );
            break;
        case 'A':
            ggain += 0.1f;
            printf("Gain: %f\n", ggain);
            glfwSetGammaFormula( ggamma, gblacklevel, ggain );
            break;
        case 'S':
            ggain -= 0.1f;
            printf("Gain: %f\n", ggain);
            glfwSetGammaFormula( ggamma, gblacklevel, ggain );
            break;
        case 'Z':
            gblacklevel += 0.1f;
            printf("Black Level: %f\n", gblacklevel);
            glfwSetGammaFormula( ggamma, gblacklevel, ggain );
            break;
        case 'X':
            gblacklevel -= 0.1f;
            printf("Black Level: %f\n", gblacklevel);
            glfwSetGammaFormula( ggamma, gblacklevel, ggain );
            break;

    }
}

static void size_callback(GLFWwindow window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(int argc, char** argv)
{
    int width, height, ch;
    int mode = GLFW_WINDOWED;
    GLFWwindow window;

    while ((ch = getopt(argc, argv, "fh")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                mode = GLFW_FULLSCREEN;
                break;

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    if (mode == GLFW_FULLSCREEN)
    {
        GLFWvidmode mode;
        glfwGetDesktopMode(&mode);
        width = mode.width;
        height = mode.height;
    }
    else
    {
        width = 0;
        height = 0;
    }

    window = glfwOpenWindow(width, height, mode, "Gamma Test", NULL);
    if (!window)
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    printf("Gamma: %f\nGain: %f\nBlack Level: %f\n",
           ggamma, ggain, gblacklevel);

    glfwSwapInterval(1);
    glfwSetKeyCallback(key_callback);
    glfwSetWindowSizeCallback(size_callback);

    glEnable(GL_SCISSOR_TEST);

    while (glfwIsWindow(window))
    {
        int width, height;

        glfwGetWindowSize(window, &width, &height);

        glScissor(0, 0, width, height);
        glClearColor(0.5f, 0.5f, 0.5f, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glScissor(0, 0, 640, 480);
        glClearColor(0.8f, 0.2f, 0.4f, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

