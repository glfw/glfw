//========================================================================
// Gamma correction test program
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
// This program is used to test the clipboard functionality.
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static void usage(void)
{
    printf("Usage: clipboard [-h]\n");
}

static void key_callback(GLFWwindow window, int key, int action)
{
    static int control = GL_FALSE;
    if (key == GLFW_KEY_LEFT_CONTROL)
    {
        control = (action == GLFW_PRESS);
        return;
    }

    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            glfwCloseWindow(window);
            break;
        case GLFW_KEY_V:
            if (control)
            {
                char buffer[4096];
                size_t size;
                printf("Paste test.\n");
                size = glfwGetClipboardData(buffer, sizeof(buffer), GLFW_CLIPBOARD_FORMAT_STRING);
                if (size >= sizeof(buffer))
                {
                    printf("Buffer wasn't big enough to hold clipboard data.\n");
                }
                printf("[%ld]: %s\n", size, buffer);
            }
            break;
        case GLFW_KEY_C:
            if (control)
            {
                glfwSetClipboardData("Hello GLFW World!", sizeof("Hello GLFW World!"),
                                     GLFW_CLIPBOARD_FORMAT_STRING);
                printf("Setting clipboard to: %s\n", "Hello GLFW World!");
            }
            break;
    }
}

static void size_callback(GLFWwindow window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(int argc, char** argv)
{
    int ch;
    GLFWwindow window;

    while ((ch = getopt(argc, argv, "h")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    window = glfwOpenWindow(0, 0, GLFW_WINDOWED, "Clipboard Test", NULL);
    if (!window)
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    glfwSwapInterval(1);
    glfwSetKeyCallback(key_callback);
    glfwSetWindowSizeCallback(size_callback);

    glMatrixMode(GL_PROJECTION);
    glOrtho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.5f, 0.5f, 0.5f, 0);

    while (glfwIsWindow(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glColor3f(0.8f, 0.2f, 0.4f);
        glRectf(-0.5f, -0.5f, 0.5f, 0.5f);

        glfwSwapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

