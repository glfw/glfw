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

#include <GL/glfw.h>

#include <stdio.h>
#include <stdlib.h>

static GLFWwindow window_handle = NULL;
static GLboolean closed = GL_FALSE;

static const char* get_mode_name(int mode)
{
    switch (mode)
    {
        case GLFW_WINDOW:
            return "windowed";
        case GLFW_FULLSCREEN:
            return "fullscreen";
        default:
            return "unknown";
    }
}

static void window_size_callback(GLFWwindow window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static int window_close_callback(GLFWwindow window)
{
    printf("Close callback triggered\n");
    window_handle = NULL;
    return 0;
}

static void key_callback(GLFWwindow window, int key, int action)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case 'Q':
        case GLFW_KEY_ESC:
            closed = GL_TRUE;
            break;
    }
}

static int open_window(int width, int height, int mode)
{
    double base = glfwGetTime();

    window_handle = glfwOpenWindow(width, height, 0, 0, 0, 0, 16, 0, mode);
    if (!window_handle)
    {
        fprintf(stderr, "Failed to create %s mode GLFW window\n", get_mode_name(mode));
        return 0;
    }

    glfwSetWindowTitle(window_handle, "Window Re-opener");
    glfwSetWindowSizeCallback(window_handle, window_size_callback);
    glfwSetWindowCloseCallback(window_handle, window_close_callback);
    glfwSetKeyCallback(window_handle, key_callback);
    glfwSwapInterval(1);

    printf("Opening %s mode window took %0.3f seconds\n",
           get_mode_name(mode),
           glfwGetTime() - base);

    return 1;
}

static void close_window(void)
{
    double base = glfwGetTime();

    glfwCloseWindow(window_handle);
    window_handle = NULL;

    printf("Closing window took %0.3f seconds\n", glfwGetTime() - base);
}

int main(int argc, char** argv)
{
    int count = 0;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(1);
    }

    for (;;)
    {
        if (!open_window(640, 480, (count & 1) ? GLFW_FULLSCREEN : GLFW_WINDOW))
        {
            glfwTerminate();
            exit(1);
        }

        glMatrixMode(GL_PROJECTION);
        glOrtho(-1.f, 1.f, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);

        glClearColor(0.f, 0.f, 0.f, 0.f);
        glColor3f(1.f, 1.f, 1.f);

        glfwSetTime(0.0);

        while (glfwGetTime() < 5.0)
        {
            glClear(GL_COLOR_BUFFER_BIT);

            glPushMatrix();
            glRotatef((GLfloat) glfwGetTime() * 100.f, 0.f, 0.f, 1.f);
            glRectf(-0.5f, -0.5f, 1.f, 1.f);
            glPopMatrix();

            glfwSwapBuffers();

            if (closed)
                close_window();

            if (!glfwIsWindow(window_handle))
            {
                printf("User closed window\n");

                glfwTerminate();
                exit(0);
            }
        }

        printf("Closing window\n");
        close_window();

        count++;
    }
}

