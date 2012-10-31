//========================================================================
// Cursor input bug test
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
// This test came about as the result of bugs #1262764, #1726540 and
// #1726592, all reported by the user peterpp, hence the name
//
// The utility of this test outside of these bugs is uncertain
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

static GLboolean reopen = GL_FALSE;
static GLFWwindow window_handle = NULL;
static int cursor_x;
static int cursor_y;

static GLboolean open_window(void);

static void toggle_cursor(GLFWwindow window)
{
    if (glfwGetInputMode(window, GLFW_CURSOR_MODE) == GLFW_CURSOR_CAPTURED)
    {
        printf("Released cursor\n");
        glfwSetInputMode(window, GLFW_CURSOR_MODE, GLFW_CURSOR_NORMAL);
    }
    else
    {
        printf("Captured cursor\n");
        glfwSetInputMode(window, GLFW_CURSOR_MODE, GLFW_CURSOR_CAPTURED);
    }
}

static void cursor_position_callback(GLFWwindow window, int x, int y)
{
    printf("Cursor moved to: %i %i (%i %i)\n", x, y, x - cursor_x, y - cursor_y);
    cursor_x = x;
    cursor_y = y;
}

static void key_callback(GLFWwindow window, int key, int action)
{
    switch (key)
    {
        case GLFW_KEY_SPACE:
        {
            if (action == GLFW_PRESS)
                toggle_cursor(window);

            break;
        }

        case GLFW_KEY_R:
        {
            if (action == GLFW_PRESS)
                reopen = GL_TRUE;

            break;
        }
    }
}

static void window_size_callback(GLFWwindow window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static GLboolean open_window(void)
{
    window_handle = glfwCreateWindow(0, 0, "Peter Detector", NULL, NULL);
    if (!window_handle)
        return GL_FALSE;

    glfwMakeContextCurrent(window_handle);
    glfwSwapInterval(1);

    glfwGetCursorPos(window_handle, &cursor_x, &cursor_y);
    printf("Cursor position: %i %i\n", cursor_x, cursor_y);

    glfwSetWindowSizeCallback(window_handle, window_size_callback);
    glfwSetCursorPosCallback(window_handle, cursor_position_callback);
    glfwSetKeyCallback(window_handle, key_callback);

    return GL_TRUE;
}

int main(void)
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    if (!open_window())
    {
        fprintf(stderr, "Failed to open GLFW window: %s\n", glfwErrorString(glfwGetError()));

        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glClearColor(0.f, 0.f, 0.f, 0.f);

    while (!glfwGetWindowParam(window_handle, GLFW_CLOSE_REQUESTED))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window_handle);
        glfwWaitEvents();

        if (reopen)
        {
            glfwDestroyWindow(window_handle);
            if (!open_window())
            {
                fprintf(stderr, "Failed to open GLFW window: %s\n", glfwErrorString(glfwGetError()));

                glfwTerminate();
                exit(EXIT_FAILURE);
            }

            reopen = GL_FALSE;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

