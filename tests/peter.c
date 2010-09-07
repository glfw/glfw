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

#include <GL/glfw.h>

#include <stdio.h>
#include <stdlib.h>

static GLboolean cursor_enabled = GL_TRUE;

static GLboolean open_window(void);

static void toggle_mouse_cursor(void)
{
    if (cursor_enabled)
        glfwDisable(GLFW_MOUSE_CURSOR);
    else
        glfwEnable(GLFW_MOUSE_CURSOR);

    cursor_enabled = !cursor_enabled;
}

static void GLFWCALL mouse_position_callback(int x, int y)
{
    printf("Mouse moved to: %i %i\n", x, y);
}

static void GLFWCALL key_callback(int key, int action)
{
    switch (key)
    {
        case GLFW_KEY_SPACE:
        {
            if (action == GLFW_PRESS)
                toggle_mouse_cursor();

            break;
        }

        case 'R':
        {
            if (action == GLFW_PRESS)
            {
                glfwCloseWindow();
                open_window();
            }

            break;
        }
    }
}

static void GLFWCALL window_size_callback(int width, int height)
{
    glViewport(0, 0, width, height);
}

static GLboolean open_window(void)
{
    int x, y;

    if (!glfwOpenWindow(0, 0, 0, 0, 0, 0, 0, 0, GLFW_WINDOW))
        return GL_FALSE;

    glfwSetWindowTitle("Peter Detector");

    glfwGetMousePos(&x, &y);
    printf("Mouse position: %i %i\n", x, y);

    glfwDisable(GLFW_AUTO_POLL_EVENTS);
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
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(1);
    }

    if (!open_window())
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window\n");
        exit(1);
    }

    glClearColor(0.f, 0.f, 0.f, 0.f);

    while (glfwGetWindowParam(GLFW_OPENED))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers();
        glfwWaitEvents();
    }

    glfwTerminate();
    exit(0);
}

