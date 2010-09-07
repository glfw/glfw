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

#include <GL/glfw.h>

#include <stdio.h>
#include <stdlib.h>

static int cursor_x = 0, cursor_y = 0;
static int window_width = 640, window_height = 480;

static void GLFWCALL window_size_callback(int width, int height)
{
    window_width = width;
    window_height = height;

    glViewport(0, 0, window_width, window_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.f, window_width, 0.f, window_height);
}

static void GLFWCALL mouse_position_callback(int x, int y)
{
    cursor_x = x;
    cursor_y = y;
}

int main(void)
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    if (!glfwOpenWindow(window_width, window_height, 0, 0, 0, 0, 0, 0, GLFW_WINDOW))
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window\n");
        exit(EXIT_FAILURE);
    }

    glfwSetWindowTitle("Cursor Inaccuracy Detector");
    glfwSetMousePosCallback(mouse_position_callback);
    glfwSetWindowSizeCallback(window_size_callback);
    glfwSwapInterval(1);

    glClearColor(0, 0, 0, 0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    while (glfwGetWindowParam(GLFW_OPENED))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glColor3f(1.f, 1.f, 1.f);

        glBegin(GL_LINES);
        glVertex2f(0.f, (GLfloat) window_height - cursor_y);
        glVertex2f((GLfloat) window_width, (GLfloat) window_height - cursor_y);
        glVertex2f((GLfloat) cursor_x, 0.f);
        glVertex2f((GLfloat) cursor_x, (GLfloat) window_height);
        glEnd();

        glfwSwapBuffers();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

