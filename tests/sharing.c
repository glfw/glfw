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

static GLFWwindow open_window(const char* title, GLFWwindow share)
{
    GLFWwindow window;

    window = glfwOpenWindow(400, 400, GLFW_WINDOWED, title, share);
    if (!window)
        return NULL;

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

    texture = create_texture();

    windows[1] = open_window("Second", windows[0]);
    if (!windows[1])
    {
        fprintf(stderr, "Failed to open second GLFW window: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    while (glfwIsWindow(windows[0]) && glfwIsWindow(windows[1]))
    {
        glfwMakeWindowCurrent(windows[0]);
        glClear(GL_COLOR_BUFFER_BIT);
        draw_quad(texture);
        glfwSwapBuffers();

        glfwMakeWindowCurrent(windows[1]);
        glClear(GL_COLOR_BUFFER_BIT);
        draw_quad(texture);
        glfwSwapBuffers();

        glfwWaitEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

