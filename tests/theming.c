//========================================================================
// Theming test program
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
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
// This program is used to test the theming features.
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const unsigned char theme_colors[6][4] =
{
    {   0,   0,   0, 255 }, // black
    { 255,   0,   0, 255 }, // red
    {   0, 255,   0, 255 }, // green
    {   0,   0, 255, 255 }, // blue
    { 255, 255, 255, 255 }, // white
    { 128, 128, 128, 255 }  // gray (no theme color)
};

static int cur_theme_color = 0;

static GLFWtheme theme;

static void set_theme(GLFWwindow* window, int theme_color)
{
    memcpy(theme.color, theme_colors[theme_color], 4);
    
    if (theme_color == 6)
    {
        theme.flags &= ~GLFW_THEME_FLAG_HAS_COLOR;
    } else
    {
        theme.flags |= GLFW_THEME_FLAG_HAS_COLOR;
    }

    const char* title;
    
    switch (theme.baseTheme) {
        case GLFW_BASE_THEME_DEFAULT:
            title = "Default theme";
            break;
        case GLFW_BASE_THEME_LIGHT:
            title = "Light theme";
            break;
        case GLFW_BASE_THEME_DARK:
            title = "Dark theme";
            break;
    }
    
    glfwSetWindowTitle(window, title);
    glfwSetTheme(window, &theme);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_0:
            glfwSetWindowTitle(window, "Default theme (NULL)");
            glfwSetTheme(window, NULL);
            break;
        case GLFW_KEY_1:
            theme.baseTheme = GLFW_BASE_THEME_DEFAULT;
            set_theme(window, cur_theme_color);
            break;
        case GLFW_KEY_2:
            theme.baseTheme = GLFW_BASE_THEME_LIGHT;
            set_theme(window, cur_theme_color);
            break;
        case GLFW_KEY_3:
            theme.baseTheme = GLFW_BASE_THEME_DARK;
            set_theme(window, cur_theme_color);
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_SPACE:
            cur_theme_color = (cur_theme_color + 1) % 6;
            set_theme(window, cur_theme_color);
            break;
    }
}

int main(int argc, char** argv)
{
    GLFWwindow* window;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(200, 200, "Window Icon", NULL, NULL);
    if (!window)
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window\n");
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    glfwSetKeyCallback(window, key_callback);
    
    theme.baseTheme = GLFW_BASE_THEME_DEFAULT;
    theme.flags = 0;
    theme.color[0] = 0;
    theme.color[1] = 0;
    theme.color[2] = 0;
    theme.color[3] = 0;
    set_theme(window, cur_theme_color);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(
                theme_colors[cur_theme_color][0] / 255.0f,
                theme_colors[cur_theme_color][1] / 255.0f,
                theme_colors[cur_theme_color][2] / 255.0f,
                theme_colors[cur_theme_color][3] / 255.0f
        );
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
