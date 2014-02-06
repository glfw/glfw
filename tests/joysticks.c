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

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define strdup(x) _strdup(x)
#endif

typedef struct Joystick
{
    GLboolean present;
    char* name;
    float* axes;
    unsigned char* buttons;
    int axis_count;
    int button_count;
} Joystick;

static Joystick joysticks[GLFW_JOYSTICK_LAST - GLFW_JOYSTICK_1 + 1];
static int joystick_count = 0;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void draw_joystick(Joystick* j, int x, int y, int width, int height)
{
    int i;
    const int axis_height = 3 * height / 4;
    const int button_height = height / 4;

    if (j->axis_count)
    {
        const int axis_width = width / j->axis_count;

        for (i = 0;  i < j->axis_count;  i++)
        {
            float value = j->axes[i] / 2.f + 0.5f;

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

    if (j->button_count)
    {
        const int button_width = width / j->button_count;

        for (i = 0;  i < j->button_count;  i++)
        {
            if (j->buttons[i])
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
    int i, width, height;

    glfwGetFramebufferSize(window, &width, &height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, width, height, 0.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);

    for (i = 0;  i < sizeof(joysticks) / sizeof(Joystick);  i++)
    {
        Joystick* j = joysticks + i;

        if (j->present)
        {
            draw_joystick(j,
                          0, i * height / joystick_count,
                          width, height / joystick_count);
        }
    }
}

static void refresh_joysticks(void)
{
    int i;

    for (i = 0;  i < sizeof(joysticks) / sizeof(Joystick);  i++)
    {
        Joystick* j = joysticks + i;

        if (glfwJoystickPresent(GLFW_JOYSTICK_1 + i))
        {
            const float* axes;
            const unsigned char* buttons;
            int axis_count, button_count;

            free(j->name);
            j->name = strdup(glfwGetJoystickName(GLFW_JOYSTICK_1 + i));

            axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1 + i, &axis_count);
            if (axis_count != j->axis_count)
            {
                j->axis_count = axis_count;
                j->axes = realloc(j->axes, j->axis_count * sizeof(float));
            }

            memcpy(j->axes, axes, axis_count * sizeof(float));

            buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1 + i, &button_count);
            if (button_count != j->button_count)
            {
                j->button_count = button_count;
                j->buttons = realloc(j->buttons, j->button_count);
            }

            memcpy(j->buttons, buttons, button_count * sizeof(unsigned char));

            if (!j->present)
            {
                printf("Found joystick %i named \'%s\' with %i axes, %i buttons\n",
                       i + 1, j->name, j->axis_count, j->button_count);

                joystick_count++;
            }

            j->present = GL_TRUE;
        }
        else
        {
            if (j->present)
            {
                printf("Lost joystick %i named \'%s\'\n", i + 1, j->name);

                free(j->name);
                free(j->axes);
                free(j->buttons);
                memset(j, 0, sizeof(Joystick));

                joystick_count--;
            }
        }
    }
}

int main(void)
{
    GLFWwindow* window;

    memset(joysticks, 0, sizeof(joysticks));

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    window = glfwCreateWindow(640, 480, "Joystick Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        refresh_joysticks();
        draw_joysticks(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

