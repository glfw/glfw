//========================================================================
// Simple multi-window test
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
// This test creates four windows and clears each in a different color
//
//========================================================================

#include <GL/glfw.h>

#include <stdio.h>
#include <stdlib.h>

static const char* titles[] =
{
    "Foo",
    "Bar",
    "Baz",
    "Quux"
};

static GLFWwindow open_window(int width, int height, const char* title)
{
    GLFWwindow window = glfwOpenWindow(width, height, GLFW_WINDOW);
    if (!window)
    {
        fprintf(stderr, "Failed to open GLFW default window\n");
        return NULL;
    }

    glfwSetWindowTitle(window, title);

    return window;
}

int main(void)
{
    int i;
    GLboolean running = GL_TRUE;
    GLFWwindow windows[4];

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0;  i < 4;  i++)
    {
        windows[i] = open_window(200, 200, titles[i]);
        if (!windows[i])
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glClearColor((i & 0x01) ? 1.0 : 0.0,
                        (i & 0x02) ? 1.0 : 0.0,
                        0.0,
                        0.0);
    }

    while (running)
    {
        for (i = 0;  i < 4;  i++)
        {
            glfwMakeWindowCurrent(windows[i]);
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers();
        }

        glfwPollEvents();

        for (i = 0;  i < 4;  i++)
        {
            if (!glfwIsWindow(windows[i]))
                running = GL_FALSE;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

