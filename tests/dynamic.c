//========================================================================
// Dynamic linking test
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
// This test came about as the result of bug #3060461
//
//========================================================================

#define GLFW_DLL
#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

static void window_size_callback(GLFWwindow window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(void)
{
    GLFWwindow window;
    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);

    if (major != GLFW_VERSION_MAJOR ||
        minor != GLFW_VERSION_MINOR ||
        rev != GLFW_VERSION_REVISION)
    {
        fprintf(stderr, "GLFW library version mismatch\n");
        exit(EXIT_FAILURE);
    }

    printf("GLFW version: %i.%i.%i\n", major, minor, rev);
    printf("GLFW version string: %s\n", glfwGetVersionString());

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    window = glfwOpenWindow(0, 0, GLFW_WINDOWED, "Dynamic Linking Test", NULL);
    if (!window)
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window\n");
        exit(EXIT_FAILURE);
    }

    glfwSetWindowSizeCallback(window_size_callback);
    glfwSwapInterval(1);

    while (glfwIsWindow(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

