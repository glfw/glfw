//========================================================================
// Cursor animation
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
// Cursor animation test.
//
//========================================================================

#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <math.h>

#ifdef min
 #undef min
#endif
#ifdef max
 #undef max
#endif

#define SIZE 64  // cursor size (width & height)
#define N    60  // number of frames

unsigned char buffer[4 * SIZE * SIZE];

static float max(float a, float b) { return a > b ? a : b; }
static float min(float a, float b) { return a < b ? a : b; }

static float star(int x, int y, float t)
{
    float c = SIZE / 2.0f;

    float i = (0.25f * (float)sin(2.0f * 3.1415926f * t) + 0.75f);
    float k = SIZE * 0.046875f * i;

    float dist = (float)sqrt((x - c) * (x - c) + (y - c) * (y - c));

    float salpha = 1.0f - dist / c;
    float xalpha = (float)x == c ? c : k / (float)fabs(x - c);
    float yalpha = (float)y == c ? c : k / (float)fabs(y - c);

    return max(0.0f, min(1.0f, i * salpha * 0.2f + salpha * xalpha * yalpha));
}

static GLFWcursor* load_frame(float t)
{
    int i = 0, x, y;
    const GLFWimage image = { SIZE, SIZE, buffer };

    for (y = 0;  y < image.width;  y++)
    {
        for (x = 0;  x < image.height;  x++)
        {
            buffer[i++] = 255;
            buffer[i++] = 255;
            buffer[i++] = 255;
            buffer[i++] = (unsigned char)(255 * star(x, y, t));
        }
    }

    return glfwCreateCursor(&image, image.width / 2, image.height / 2);
}

int main(void)
{
    int i;
    double t0, t1, frameTime = 0.0;

    GLFWwindow* window;
    GLFWcursor* frames[N];

    if (!glfwInit())
        exit(EXIT_FAILURE);

    window = glfwCreateWindow(640, 480, "Cursor animation", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    for (i = 0; i < N; i++)
        frames[i] = load_frame(i / (float)N);

    i = 0;

    t0 = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSetCursor(window, frames[i]);
        glfwSwapBuffers(window);
        glfwPollEvents();

        t1 = glfwGetTime();
        frameTime += t1 - t0;
        t0 = t1;

        while (frameTime > 1.0 / (double)N)
        {
            i = (i + 1) % N;
            frameTime -= 1.0 / (double)N;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

