//========================================================================
// Joystick input test
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
// This test displays the state of every button and axis of every connected
// joystick and/or gamepad
//
//========================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define strdup(x) _strdup(x)
#endif

static int joysticks[GLFW_JOYSTICK_LAST + 1];
static int joystick_count = 0;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void draw_joystick(int index, int x, int y, int width, int height)
{
    int i;
    int axis_count, button_count;
    const float* axes;
    const unsigned char* buttons;
    const int axis_height = 3 * height / 4;
    const int button_height = height / 4;

    axes = glfwGetJoystickAxes(joysticks[index], &axis_count);
    if (axis_count)
    {
        const int axis_width = width / axis_count;

        for (i = 0;  i < axis_count;  i++)
        {
            float value = axes[i] / 2.f + 0.5f;

            glColor3f(0.3f, 0.3f, 0.3f);
            glRecti(x + i * axis_width,
                    y,
                    x + (i + 1) * axis_width,
                    y + axis_height);

            glColor3f(1.f, 1.f, 1.f);
            glRecti(x + i * axis_width,
                    y + (int) (value * (axis_height - 5)),
                    x + (i + 1) * axis_width,
                    y + 5 + (int) (value * (axis_height - 5)));
        }
    }

    buttons = glfwGetJoystickButtons(joysticks[index], &button_count);
    if (button_count)
    {
        const int button_width = width / button_count;

        for (i = 0;  i < button_count;  i++)
        {
            if (buttons[i])
                glColor3f(1.f, 1.f, 1.f);
            else
                glColor3f(0.3f, 0.3f, 0.3f);

            glRecti(x + i * button_width,
                    y + axis_height,
                    x + (i + 1) * button_width,
                    y + axis_height + button_height);
        }
    }
}

static void draw_joysticks(GLFWwindow* window)
{
    int i, width, height, offset = 0;

    glfwGetFramebufferSize(window, &width, &height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, width, height, 0.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);

    for (i = 0;  i < joystick_count;  i++)
    {
        draw_joystick(i,
                      0, offset * height / joystick_count,
                      width, height / joystick_count);
        offset++;
    }
}

static void joystick_callback(int joy, int event)
{
    if (event == GLFW_CONNECTED)
    {
        int axis_count, button_count;

        glfwGetJoystickAxes(joy, &axis_count);
        glfwGetJoystickButtons(joy, &button_count);

        printf("Found joystick %i named \'%s\' with %i axes, %i buttons\n",
                joy + 1,
                glfwGetJoystickName(joy),
                axis_count,
                button_count);

        joysticks[joystick_count++] = joy;
    }
    else if (event == GLFW_DISCONNECTED)
    {
        int i;

        for (i = 0;  i < joystick_count;  i++)
        {
            if (joysticks[i] == joy)
                break;
        }

        for (i = i + 1;  i < joystick_count;  i++)
            joysticks[i - 1] = joysticks[i];

        printf("Lost joystick %i\n", joy + 1);
        joystick_count--;
    }
}

static void find_joysticks(void)
{
    int joy;

    for (joy = GLFW_JOYSTICK_1;  joy <= GLFW_JOYSTICK_LAST;  joy++)
    {
        if (glfwJoystickPresent(joy))
            joystick_callback(joy, GLFW_CONNECTED);
    }
}

int main(void)
{
    GLFWwindow* window;

    memset(joysticks, 0, sizeof(joysticks));

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    find_joysticks();
    glfwSetJoystickCallback(joystick_callback);

    window = glfwCreateWindow(640, 480, "Joystick Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        draw_joysticks(window);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

