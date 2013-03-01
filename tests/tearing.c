//========================================================================
// Vsync enabling test
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
// This test renders a high contrast, horizontally moving bar, allowing for
// visual verification of whether the set swap interval is indeed obeyed
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static int swap_interval;

static void set_swap_interval(GLFWwindow* window, int interval)
{
    char title[256];

    swap_interval = interval;
    glfwSwapInterval(swap_interval);

    sprintf(title, "Tearing detector (interval %i)", swap_interval);

    glfwSetWindowTitle(window, title);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int action)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        set_swap_interval(window, 1 - swap_interval);
}

int main(void)
{
    float position;
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    window = glfwCreateWindow(640, 480, "", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    set_swap_interval(window, swap_interval);

    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glMatrixMode(GL_PROJECTION);
    glOrtho(-1.f, 1.f, -1.f, 1.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        position = cosf(glfwGetTime() * 4.f) * 0.75f;
        glRectf(position - 0.25f, -1.f, position + 0.25f, 1.f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

