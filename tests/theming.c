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

const float theme_colors[6][4] =
{
    {   0,   0,   0, 1.0 }, // black
    { 1.0,   0,   0, 1.0 }, // red
    {   0, 1.0,   0, 1.0 }, // green
    {   0,   0, 1.0, 1.0 }, // blue
    { 1.0, 1.0, 1.0, 1.0 }, // white
    { 0.5, 0.5, 0.5, 1.0 }  // gray (no theme color)
};

static int cur_theme_color = 0;

static GLFWtheme* theme;

static void print_theme(GLFWtheme* theme, const char* title)
{
    int n = 0;
    
    printf("%s: {\n", title);
    printf("    Variation: %s\n", glfwThemeGetVariation(theme) == GLFW_THEME_LIGHT ? "light" : "dark");
    printf("    Attributes: [");
    if (glfwThemeGetAttribute(theme, GLFW_THEME_COLOR_MAIN))
    {
        printf(n++ > 0 ? ", %s" : "%s", "COLOR_MAIN");
    }
    if (glfwThemeGetAttribute(theme, GLFW_THEME_ATTRIBUTE_VIBRANT))
    {
        printf(n++ > 0 ? ", %s" : "%s", "VIBRANT");
    }
    if (glfwThemeGetAttribute(theme, GLFW_THEME_ATTRIBUTE_HIGH_CONTRAST))
    {
        printf(n++ > 0 ? ", %s" : "%s", "HIGH_CONTRAST");
    }
    printf("]\n");
    if (glfwThemeGetAttribute(theme, GLFW_THEME_COLOR_MAIN))
    {
        float r, g, b, a;
        glfwThemeGetColor(theme, GLFW_THEME_COLOR_MAIN, &r, &g, &b, &a);
        printf("    Main color: [%f, %f, %f, %f]\n", r, g, b, a);
    }
    printf("}\n");
}

static void set_theme(GLFWwindow* window, int theme_color)
{
    glfwThemeSetColor(
            theme,
            GLFW_THEME_COLOR_MAIN,
            theme_colors[theme_color][0],
            theme_colors[theme_color][1],
            theme_colors[theme_color][2],
            theme_colors[theme_color][3]
    );
    
    if (theme_color == 6)
        glfwThemeSetAttribute(theme, GLFW_THEME_COLOR_MAIN, GLFW_FALSE);
    else
        glfwThemeSetAttribute(theme, GLFW_THEME_COLOR_MAIN, GLFW_TRUE);

    const char* title;
    
    switch (glfwThemeGetVariation(theme)) {
        case GLFW_THEME_DEFAULT:
            title = "Default theme";
            break;
        case GLFW_THEME_LIGHT:
            title = "Light theme";
            break;
        case GLFW_THEME_DARK:
            title = "Dark theme";
            break;
    }
    
    glfwSetWindowTitle(window, title);
    glfwSetTheme(window, theme);
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
            glfwThemeSetVariation(theme, GLFW_THEME_DEFAULT);
            set_theme(window, cur_theme_color);
            break;
        case GLFW_KEY_2:
            glfwThemeSetVariation(theme, GLFW_THEME_LIGHT);
            set_theme(window, cur_theme_color);
            break;
        case GLFW_KEY_3:
            glfwThemeSetVariation(theme, GLFW_THEME_DARK);
            set_theme(window, cur_theme_color);
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_SPACE:
            cur_theme_color = (cur_theme_color + 1) % 6;
            set_theme(window, cur_theme_color);
            break;
        case GLFW_KEY_P:
            print_theme(glfwGetTheme(window), "Window theme");
            break;
    }
}

static void theme_callback(GLFWtheme* theme)
{
    print_theme(theme, "System theme changed to");
}

int main(int argc, char** argv)
{
    GLFWwindow* window;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }
    
    print_theme(glfwGetSystemDefaultTheme(), "System theme");

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
    
    theme = glfwCreateTheme();
    set_theme(window, cur_theme_color);
    
    glfwSetSystemThemeCallback(theme_callback);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(
                theme_colors[cur_theme_color][0],
                theme_colors[cur_theme_color][1],
                theme_colors[cur_theme_color][2],
                theme_colors[cur_theme_color][3]
        );
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    glfwDestroyTheme(theme);
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
