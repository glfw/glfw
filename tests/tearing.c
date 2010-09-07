//========================================================================
// Vsync enabling test
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
// This test renders a high contrast, horizontally moving bar, allowing for
// visual verification of whether the set swap interval is indeed obeyed
//
//========================================================================

#include <GL/glfw.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static void GLFWCALL window_size_callback(int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(void)
{
    float position;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(1);
    }

    if (!glfwOpenWindow(0, 0, 0, 0, 0, 0, 0, 0, GLFW_WINDOW))
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window\n");
        exit(1);
    }

    glfwSetWindowTitle("Tearing Detector");
    glfwSetWindowSizeCallback(window_size_callback);
    glfwSwapInterval(1);

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glColor3f(1.f, 1.f, 1.f);

    glMatrixMode(GL_PROJECTION);
    glOrtho(-1.f, 1.f, -1.f, 1.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);

    while (glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
    {
        glClear(GL_COLOR_BUFFER_BIT);

        position = cosf(glfwGetTime() * 4.f) * 0.75f;
        glRectf(position - 0.25f, -1.f, position + 0.25f, 1.f);

        glfwSwapBuffers();
    }

    glfwTerminate();
    exit(0);
}

