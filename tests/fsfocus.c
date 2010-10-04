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
// This test is used to test window activation and iconfication for
// fullscreen windows with a video mode differing from the desktop mode
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

static GLboolean running = GL_TRUE;

static void window_focus_callback(GLFWwindow window, int activated)
{
    printf("%0.3f: Window %s\n",
           glfwGetTime(),
           activated ? "activated" : "deactivated");
}

static void window_key_callback(GLFWwindow window, int key, int action)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_ESC:
        {
            printf("%0.3f: User pressed Escape\n", glfwGetTime());
            running = GL_FALSE;
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

static int window_close_callback(GLFWwindow window)
{
    printf("%0.3f: User closed window\n", glfwGetTime());
    return GL_TRUE;
}

int main(void)
{
    GLFWwindow window;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    window = glfwOpenWindow(640, 480, GLFW_FULLSCREEN, "Fullscreen focus", NULL);
    if (!window)
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    glfwSwapInterval(1);
    glfwEnable(window, GLFW_MOUSE_CURSOR);

    glfwSetWindowFocusCallback(window, window_focus_callback);
    glfwSetKeyCallback(window, window_key_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);

    while (running && glfwIsWindow(window) == GL_TRUE)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers();
        glfwWaitEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

