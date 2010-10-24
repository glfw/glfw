//========================================================================
// Mouse cursor bug test
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

static GLboolean cursor_enabled = GL_TRUE;
static GLFWwindow window_handle = NULL;

static GLboolean open_window(void);

static void toggle_mouse_cursor(GLFWwindow window)
{
    if (cursor_enabled)
        glfwDisable(window, GLFW_MOUSE_CURSOR);
    else
        glfwEnable(window, GLFW_MOUSE_CURSOR);

    cursor_enabled = !cursor_enabled;
}

static void mouse_position_callback(GLFWwindow window, int x, int y)
{
    printf("Mouse moved to: %i %i\n", x, y);
}

static void key_callback(GLFWwindow window, int key, int action)
{
    switch (key)
    {
        case GLFW_KEY_SPACE:
        {
            if (action == GLFW_PRESS)
                toggle_mouse_cursor(window);

            break;
        }

        case 'R':
        {
            if (action == GLFW_PRESS)
            {
                glfwCloseWindow(window);
                open_window();
            }

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
    int x, y;

    window_handle = glfwOpenWindow(0, 0, GLFW_WINDOWED, "Peter Detector", NULL);
    if (!window_handle)
        return GL_FALSE;

    glfwGetMousePos(window_handle, &x, &y);
    printf("Mouse position: %i %i\n", x, y);

    glfwSetWindowSizeCallback(window_size_callback);
    glfwSetMousePosCallback(mouse_position_callback);
    glfwSetKeyCallback(key_callback);
    glfwSwapInterval(1);

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
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    glClearColor(0.f, 0.f, 0.f, 0.f);

    while (glfwIsWindow(window_handle))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers();
        glfwWaitEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

