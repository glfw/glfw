//========================================================================
// Full screen anti-aliasing test
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
// This test renders two high contrast, slowly rotating quads, one aliased
// and one (hopefully) anti-aliased, thus allowing for visual verification
// of whether FSAA is indeed enabled
//
//========================================================================

#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_SPACE:
            glfwSetTime(0.0);
            break;
    }
}

static void usage(void)
{
    printf("Usage: fsaa [-h] [-s SAMPLES]\n");
}

int main(int argc, char** argv)
{
    int ch, samples = 4;
    GLFWwindow* window;

    while ((ch = getopt(argc, argv, "hs:")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 's':
                samples = atoi(optarg);
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if (samples)
        printf("Requesting FSAA with %i samples\n", samples);
    else
        printf("Requesting that FSAA not be available\n");

    glfwWindowHint(GLFW_SAMPLES, samples);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    window = glfwCreateWindow(800, 400, "Aliasing Detector", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!glfwExtensionSupported("GL_ARB_multisample"))
    {
        printf("GL_ARB_multisample extension not supported\n");

        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwShowWindow(window);

    glGetIntegerv(GL_SAMPLES_ARB, &samples);
    if (samples)
        printf("Context reports FSAA is available with %i samples\n", samples);
    else
        printf("Context reports FSAA is unavailable\n");

    glMatrixMode(GL_PROJECTION);
    glOrtho(0.f, 1.f, 0.f, 0.5f, 0.f, 1.f);
    glMatrixMode(GL_MODELVIEW);

    while (!glfwWindowShouldClose(window))
    {
        GLfloat time = (GLfloat) glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);

        glLoadIdentity();
        glTranslatef(0.25f, 0.25f, 0.f);
        glRotatef(time, 0.f, 0.f, 1.f);

        glDisable(GL_MULTISAMPLE_ARB);
        glRectf(-0.15f, -0.15f, 0.15f, 0.15f);

        glLoadIdentity();
        glTranslatef(0.75f, 0.25f, 0.f);
        glRotatef(time, 0.f, 0.f, 1.f);

        glEnable(GL_MULTISAMPLE_ARB);
        glRectf(-0.15f, -0.15f, 0.15f, 0.15f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

