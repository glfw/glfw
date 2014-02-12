//========================================================================
// Window icon test program
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
// This program is used to test the icon feature.
//
//========================================================================

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// a simple glfw logo
const char * const logo[] = {
    "................",
    "................",
    "...0000..0......",
    "...0.....0......",
    "...0.00..0......",
    "...0..0..0......",
    "...0000..0000...",
    "................",
    "................",
    "...000..0...0...",
    "...0....0...0...",
    "...000..0.0.0...",
    "...0....0.0.0...",
    "...0....00000...",
    "................",
    "................"
};

const unsigned char icon_colors[5][4] = {
    {0x00, 0x00, 0x00, 0xff}, // black
    {0xff, 0x00, 0x00, 0xff}, // red
    {0x00, 0xff, 0x00, 0xff}, // green
    {0xff, 0x00, 0xff, 0xff}, // blue
    {0xff, 0xff, 0xff, 0xff} // white
};

static int cur_icon_color = 0;

static void set_icon(GLFWwindow* window, int icon_color) {
    GLFWimage img;
    int x, y;
    char* pixels;

    // create image
    img.width = 16;
    img.height = 16;
    pixels = malloc(img.width * img.height * 4);

    if (!pixels)
    {
        glfwTerminate();

        fprintf(stderr, "Failed to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for (x = 0; x < 16; ++x)
    {
        for (y = 0; y < 16; ++y)
        {
            // 15 - y because we need to flip the icon
            if (logo[15 - y][x] == '0')
                memcpy(pixels + 4 * (x + y * img.width), icon_colors[icon_color], 4);
            else
                memset(pixels + 4 * (x + y * img.width), 0, 4); // transparency
        }
    }

    img.pixels = pixels;

    glfwSetWindowIcons(window, &img, 1);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            glfwDestroyWindow(window);
            break;
        case GLFW_KEY_SPACE:
            cur_icon_color = (cur_icon_color + 1) % 5;
            set_icon(window, cur_icon_color);
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

    window = glfwCreateWindow(800, 600, "Icons", NULL, NULL);
    if (!window)
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window\n");
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    set_icon(window, cur_icon_color);

    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    
    return EXIT_SUCCESS;
}
