//========================================================================
// Multithreading test
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
// This test is intended to verify whether the OpenGL context part of
// the GLFW API is able to be used from multiple threads
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "tinycthread.h"

static volatile GLboolean running = GL_TRUE;

static int thread_start(void* data)
{
    GLFWwindow window = (GLFWwindow) data;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    while (running)
    {
        const float red = (float) sin(glfwGetTime()) / 2.f + 0.5f;

        glClearColor(red, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
    }

    glfwMakeContextCurrent(NULL);
    return 0;
}

int main(void)
{
    int result;
    GLFWwindow window;
    thrd_t thread;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(640, 480, GLFW_WINDOWED, "Multithreading", NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to open GLFW window: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    if (thrd_create(&thread, thread_start, window) != thrd_success)
    {
        fprintf(stderr, "Failed to create secondary thread\n");
        exit(EXIT_FAILURE);
    }

    while (!glfwGetWindowParam(window, GLFW_CLOSE_REQUESTED))
        glfwWaitEvents();

    running = GL_FALSE;
    thrd_join(thread, &result);

    exit(EXIT_SUCCESS);
}

