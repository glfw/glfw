//========================================================================
// Window icon animation test program
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
// This program is used to test the icon feature.
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define ALPHA 220
#define WIDTH 3
#define HEIGHT 3
#define SPEED 0.3

const unsigned char iconColors[5][3] =
{
    { 1, 0, 0 }, // Red
    { 0, 1, 0 }, // Green
    { 0, 0, 1 }, // Blue
    { 1, 1, 1 }  // All
};

static int currentIconColor = -1;
static int animate = GLFW_FALSE;
static int smooth = GLFW_FALSE;
static int useOptimalSize = GLFW_FALSE;
static int width = WIDTH, height = HEIGHT;

static void error_callback(int error_code, const char* description)
{
    fprintf(stderr, "Error %i: %s\n", error_code, description);
}

static void set_icon(GLFWwindow* window, int iconColor)
{
    GLFWimage image;
    int x, y;
    double time = glfwGetTime();
    
    double red   = (cos(SPEED * time * M_PI      ) + 1.0) / 2.0;
    double green = (cos(SPEED * time * M_PI * 1.5) + 1.0) / 2.0;
    double blue  = (cos(SPEED * time * M_PI * 2.0) + 1.0) / 2.0;
    
    unsigned char pixels[width * height * 4];

    for (x = 0; x < width; ++x)
    {
        for (y = 0; y < height; ++y)
        {
            int offset = x * height * 4 + y * 4;
        
            pixels[offset + 0] = iconColors[currentIconColor][0] * (char) (255.0 * red);
            pixels[offset + 1] = iconColors[currentIconColor][1] * (char) (255.0 * green);
            pixels[offset + 2] = iconColors[currentIconColor][2] * (char) (255.0 * blue);
            pixels[offset + 3] = (char) (255.0 * ((double) (x + y) / (double) (width + height)));
        }
    }
    
    image.width = width;
    image.height = height;
    image.pixels = pixels;
    
    glfwSetErrorCallback(NULL);
    
    glfwSetWindowIcon(window, 1, &image);
    glfwSetWindowIcon(0, 1, &image);
    
    glfwSetErrorCallback(error_callback);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_SPACE:
            currentIconColor = (currentIconColor + 1) % 4;
            animate = GLFW_TRUE;
            break;
        case GLFW_KEY_S:
            smooth = !smooth;
            break;
        case GLFW_KEY_O:
        {
            GLFWimage image = { GLFW_DONT_CARE, GLFW_DONT_CARE, NULL };
            glfwSetWindowIcon(NULL, 0, &image);
            printf("Optimal size: [%i, %i]\n", image.width, image.height);
            useOptimalSize = !useOptimalSize;
            break;
        }
        case GLFW_KEY_X:
            glfwSetWindowIcon(window, 0, NULL);
            glfwSetWindowIcon(0, 0, NULL);
            currentIconColor = -1;
            animate = GLFW_FALSE;
            break;
    }
}

int main(int argc, char** argv)
{
    GLFWwindow* window;
    double lastTime = 0.0;
    double time;
    
    glfwSetErrorCallback(error_callback);

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
    glfwSwapInterval(60);

    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window))
    {
        
        if (useOptimalSize)
        {
            GLFWimage image = { GLFW_DONT_CARE, GLFW_DONT_CARE, NULL };
            glfwSetWindowIcon(NULL, 0, &image);
        
            if (image.width != GLFW_DONT_CARE)
            {
                assert(image.height != GLFW_DONT_CARE);
                
                width = image.width;
                height = image.height;
            }
        }
        else
        {
            width = WIDTH;
            height = HEIGHT;
        }
    
        time = glfwGetTime();
        if (animate && (time - lastTime) > (smooth ? 0.01 : 0.15))
        {
            set_icon(window, currentIconColor);
            lastTime = time;
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
    
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
