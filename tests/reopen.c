//========================================================================
// Window re-opener (open/close stress test)
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
// This test came about as the result of bug #1262773
//
// It closes and re-opens the GLFW window every five seconds, alternating
// between windowed and fullscreen mode
//
// It also times and logs opening and closing actions and attempts to separate
// user initiated window closing from its own
//
//========================================================================

#include <GLFW/glfw3.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void window_close_callback(GLFWwindow* window)
{
    printf("Close callback triggered\n");
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
    }
}

static GLFWwindow* open_window(int width, int height, GLFWmonitor* monitor)
{
    double base;
    GLFWwindow* window;

    base = glfwGetTime();

    window = glfwCreateWindow(width, height, "Window Re-opener", monitor, NULL);
    if (!window)
        return NULL;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetKeyCallback(window, key_callback);

    if (monitor)
    {
        printf("Opening full screen window on monitor %s took %0.3f seconds\n",
               glfwGetMonitorName(monitor),
               glfwGetTime() - base);
    }
    else
    {
        printf("Opening regular window took %0.3f seconds\n",
               glfwGetTime() - base);
    }

    return window;
}

static void close_window(GLFWwindow* window)
{
    double base = glfwGetTime();
    glfwDestroyWindow(window);
    printf("Closing window took %0.3f seconds\n", glfwGetTime() - base);
}

int main(int argc, char** argv)
{
    int count = 0;
    GLFWwindow* window;

    srand((unsigned int) time(NULL));

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    for (;;)
    {
        GLFWmonitor* monitor = NULL;

        if (count % 2 == 0)
        {
            int monitorCount;
            GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
            monitor = monitors[rand() % monitorCount];
        }

        window = open_window(640, 480, monitor);
        if (!window)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glMatrixMode(GL_PROJECTION);
        glOrtho(-1.f, 1.f, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);

        glfwSetTime(0.0);

        while (glfwGetTime() < 5.0)
        {
            glClear(GL_COLOR_BUFFER_BIT);

            glPushMatrix();
            glRotatef((GLfloat) glfwGetTime() * 100.f, 0.f, 0.f, 1.f);
            glRectf(-0.5f, -0.5f, 1.f, 1.f);
            glPopMatrix();

            glfwSwapBuffers(window);
            glfwPollEvents();

            if (glfwWindowShouldClose(window))
            {
                close_window(window);
                printf("User closed window\n");

                glfwTerminate();
                exit(EXIT_SUCCESS);
            }
        }

        printf("Closing window\n");
        close_window(window);

        count++;
    }

    glfwTerminate();
}

