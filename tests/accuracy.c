//========================================================================
// Mouse cursor accuracy test
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
// This test came about as the result of bug #1867804
//
// No sign of said bug has so far been detected
//
//========================================================================

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

static double cursor_x = 0.0, cursor_y = 0.0;
static int window_width = 640, window_height = 480;
static int swap_interval = 1;

static void set_swap_interval(GLFWwindow* window, int interval)
{
    char title[256];

    swap_interval = interval;
    glfwSwapInterval(swap_interval);

    sprintf(title, "Cursor Inaccuracy Detector (interval %i)", swap_interval);

    glfwSetWindowTitle(window, title);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    window_width = width;
    window_height = height;

    glViewport(0, 0, window_width, window_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, window_width, 0.f, window_height, 0.f, 1.f);
}

static void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    cursor_x = x;
    cursor_y = y;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        set_swap_interval(window, 1 - swap_interval);
}

int main(void)
{
    GLFWwindow* window;
    int width, height;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    window = glfwCreateWindow(window_width, window_height, "", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);

    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);

    set_swap_interval(window, swap_interval);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_LINES);
        glVertex2f(0.f, (GLfloat) (window_height - cursor_y));
        glVertex2f((GLfloat) window_width, (GLfloat) (window_height - cursor_y));
        glVertex2f((GLfloat) cursor_x, 0.f);
        glVertex2f((GLfloat) cursor_x, (GLfloat) window_height);
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

