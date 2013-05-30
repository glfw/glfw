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

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

#define STEP_SIZE 0.1f

static GLfloat gamma_value = 1.0f;

static void usage(void)
{
    printf("Usage: gamma [-h] [-f]\n");
}

static void set_gamma(GLFWwindow* window, float value)
{
    GLFWmonitor* monitor = glfwGetWindowMonitor(window);
    if (!monitor)
        monitor = glfwGetPrimaryMonitor();

    gamma_value = value;
    printf("Gamma: %f\n", gamma_value);
    glfwSetGamma(monitor, gamma_value);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        }

        case GLFW_KEY_KP_ADD:
        case GLFW_KEY_Q:
        {
            set_gamma(window, gamma_value + STEP_SIZE);
            break;
        }

        case GLFW_KEY_KP_SUBTRACT:
        case GLFW_KEY_W:
        {
            if (gamma_value - STEP_SIZE > 0.f)
                set_gamma(window, gamma_value - STEP_SIZE);

            break;
        }
    }
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(int argc, char** argv)
{
    int width, height, ch;
    GLFWmonitor* monitor = NULL;
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    while ((ch = getopt(argc, argv, "fh")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                monitor = glfwGetPrimaryMonitor();
                break;

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (monitor)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        width = mode->width;
        height = mode->height;
    }
    else
    {
        width = 200;
        height = 200;
    }

    window = glfwCreateWindow(width, height, "Gamma Test", monitor, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    set_gamma(window, 1.f);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glMatrixMode(GL_PROJECTION);
    glOrtho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.5f, 0.5f, 0.5f, 0);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glColor3f(0.8f, 0.2f, 0.4f);
        glRectf(-0.5f, -0.5f, 0.5f, 0.5f);

        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

