//========================================================================
// Iconify/restore test program
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
// This program is used to test the iconify/restore functionality for
// both fullscreen and windowed mode windows
//
//========================================================================

#include <GL/glfw.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static void usage(void)
{
    printf("iconify [-h] [-f]\n");
}

static void GLFWCALL key_callback(int key, int action)
{
    printf("%0.2f Key %s\n",
           glfwGetTime(),
           action == GLFW_PRESS ? "pressed" : "released");

    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_SPACE:
            glfwIconifyWindow();
            break;
        case GLFW_KEY_ESC:
            glfwCloseWindow();
            break;
    }
}

static void GLFWCALL size_callback(int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(int argc, char** argv)
{
    int width, height, ch;
    int mode = GLFW_WINDOW;
    GLboolean active = -1, iconified = -1;

    while ((ch = getopt(argc, argv, "fh")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                mode = GLFW_FULLSCREEN;
                break;

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    if (mode == GLFW_FULLSCREEN)
    {
        GLFWvidmode mode;
        glfwGetDesktopMode(&mode);
        width = mode.Width;
        height = mode.Height;
    }
    else
    {
        width = 0;
        height = 0;
    }

    if (!glfwOpenWindow(width, height, 0, 0, 0, 0, 0, 0, mode))
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window\n");
        exit(EXIT_FAILURE);
    }

    glfwSetWindowTitle("Iconify");
    glfwSwapInterval(1);
    glfwSetKeyCallback(key_callback);
    glfwSetWindowSizeCallback(size_callback);

    glEnable(GL_SCISSOR_TEST);

    while (glfwGetWindowParam(GLFW_OPENED))
    {
        int width, height;

        if (iconified != glfwGetWindowParam(GLFW_ICONIFIED) ||
            active != glfwGetWindowParam(GLFW_ACTIVE))
        {
            iconified = glfwGetWindowParam(GLFW_ICONIFIED);
            active = glfwGetWindowParam(GLFW_ACTIVE);

            printf("%0.2f %s %s\n",
                   glfwGetTime(),
                   iconified ? "Iconified" : "Restored",
                   active ? "Active" : "Inactive");
        }

        glfwGetWindowSize(&width, &height);

        glScissor(0, 0, width, height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glScissor(0, 0, 640, 480);
        glClearColor(1, 1, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

