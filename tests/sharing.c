//========================================================================
// Context sharing test program
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
// This program is used to test sharing of objects between contexts
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#define WIDTH  400
#define HEIGHT 400

static void key_callback(GLFWwindow window, int key, int action)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_ESC)
        glfwCloseWindow(window);
}

static GLFWwindow open_window(const char* title, GLFWwindow share)
{
    GLFWwindow window;

    window = glfwOpenWindow(WIDTH, HEIGHT, GLFW_WINDOWED, title, share);
    if (!window)
        return NULL;

    glfwSetKeyCallback(key_callback);
    glfwSwapInterval(1);

    return window;
}

static GLuint create_texture(void)
{
    int x, y;
    char pixels[256 * 256];
    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    for (y = 0;  y < 256;  y++)
    {
        for (x = 0;  x < 256;  x++)
            pixels[y * 256 + x] = rand() % 256;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

static void draw_quad(GLuint texture)
{
    int width, height;
    glfwGetWindowSize(glfwGetCurrentWindow(), &width, &height);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.f, 1.f, 0.f, 1.f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glColor3f(0.6f, 0.f, 0.6f);

    glBegin(GL_QUADS);

    glTexCoord2f(0.f, 0.f);
    glVertex2f(0.f, 0.f);

    glTexCoord2f(1.f, 0.f);
    glVertex2f(1.f, 0.f);

    glTexCoord2f(1.f, 1.f);
    glVertex2f(1.f, 1.f);

    glTexCoord2f(0.f, 1.f);
    glVertex2f(0.f, 1.f);

    glEnd();
}

int main(int argc, char** argv)
{
    GLFWwindow windows[2];
    GLuint texture;
    int x, y;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    windows[0] = open_window("First", NULL);
    if (!windows[0])
    {
        fprintf(stderr, "Failed to open first GLFW window: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    // This is the one and only time we create a texture
    // It is created inside the first context, created above
    // It will then be shared with the second context, created below
    texture = create_texture();

    windows[1] = open_window("Second", windows[0]);
    if (!windows[1])
    {
        fprintf(stderr, "Failed to open second GLFW window: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    glfwGetWindowPos(windows[0], &x, &y);
    glfwSetWindowPos(windows[1], x + WIDTH + 50, y);

    while (glfwIsWindow(windows[0]) && glfwIsWindow(windows[1]))
    {
        glfwMakeWindowCurrent(windows[0]);
        draw_quad(texture);
        glfwSwapBuffers();

        glfwMakeWindowCurrent(windows[1]);
        draw_quad(texture);
        glfwSwapBuffers();

        glfwWaitEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

