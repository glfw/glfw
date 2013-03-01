//========================================================================
// Fullscreen window (un)focus test
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
// This test is used to test window focusing and iconfication for
// fullscreen windows with a video mode differing from the desktop mode
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_focus_callback(GLFWwindow* window, int focused)
{
    printf("%0.3f: Window %s\n",
           glfwGetTime(),
           focused ? "focused" : "defocused");
}

static void window_key_callback(GLFWwindow* window, int key, int action)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
        {
            printf("%0.3f: User pressed Escape\n", glfwGetTime());
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        }

        case GLFW_KEY_SPACE:
        {
            printf("%0.3f: User pressed Space\n", glfwGetTime());
            glfwIconifyWindow(window);
            break;
        }
    }
}

int main(void)
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    window = glfwCreateWindow(640, 480, "Fullscreen focus", glfwGetPrimaryMonitor(), NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetInputMode(window, GLFW_CURSOR_MODE, GLFW_CURSOR_NORMAL);

    glfwSetWindowFocusCallback(window, window_focus_callback);
    glfwSetKeyCallback(window, window_key_callback);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

