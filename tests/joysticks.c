//========================================================================
// Joystick input test
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
// This test displays the state of every button and axis of every connected
// joystick and/or gamepad
//
//========================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include <nuklear.h>

#define NK_GLFW_GL2_IMPLEMENTATION
#include <nuklear_glfw_gl2.h>

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

static void joystick_callback(int jid, int event)
{
    if (event == GLFW_CONNECTED)
        joysticks[joystick_count++] = jid;
    else if (event == GLFW_DISCONNECTED)
    {
        int i;

        for (i = 0;  i < joystick_count;  i++)
        {
            if (joysticks[i] == jid)
                break;
        }

        for (i = i + 1;  i < joystick_count;  i++)
            joysticks[i - 1] = joysticks[i];

        joystick_count--;
    }
}

static const char* joystick_label(int jid)
{
    static char label[1024];
    snprintf(label, sizeof(label), "%i: %s", jid + 1, glfwGetJoystickName(jid));
    return label;
}

int main(void)
{
    int jid;
    GLFWwindow* window;
    struct nk_context* nk;
    struct nk_font_atlas* atlas;

    memset(joysticks, 0, sizeof(joysticks));

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    for (jid = GLFW_JOYSTICK_1;  jid <= GLFW_JOYSTICK_LAST;  jid++)
    {
        if (glfwJoystickPresent(jid))
            joystick_callback(jid, GLFW_CONNECTED);
    }

    glfwSetJoystickCallback(joystick_callback);

    window = glfwCreateWindow(640, 480, "Joystick Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    nk = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();

    while (!glfwWindowShouldClose(window))
    {
        int i, width, height;

        glfwGetWindowSize(window, &width, &height);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_glfw3_new_frame();

        if (nk_begin(nk,
                     "Joysticks",
                     nk_rect(width - 200.f, 0.f, 200.f, (float) height),
                     NK_WINDOW_MINIMIZABLE |
                     NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(nk, 30, 1);

            if (joystick_count)
            {
                for (i = 0;  i < joystick_count;  i++)
                {
                    if (nk_button_label(nk, joystick_label(joysticks[i])))
                        nk_window_set_focus(nk, joystick_label(joysticks[i]));
                }
            }
            else
                nk_label(nk, "No joysticks connected", NK_TEXT_LEFT);
        }

        nk_end(nk);

        for (i = 0;  i < joystick_count;  i++)
        {
            if (nk_begin(nk,
                         joystick_label(joysticks[i]),
                         nk_rect(i * 20.f, i * 20.f, 400.f, 400.f),
                         NK_WINDOW_BORDER |
                         NK_WINDOW_MOVABLE |
                         NK_WINDOW_SCALABLE |
                         NK_WINDOW_MINIMIZABLE |
                         NK_WINDOW_TITLE))
            {
                int j, axis_count, button_count;
                const float* axes;
                const unsigned char* buttons;

                nk_layout_row_dynamic(nk, 30, 1);

                axes = glfwGetJoystickAxes(joysticks[i], &axis_count);
                if (axis_count)
                {
                    for (j = 0;  j < axis_count;  j++)
                        nk_slide_float(nk, -1.f, axes[j], 1.f, 0.1f);
                }

                nk_layout_row_dynamic(nk, 30, 8);

                buttons = glfwGetJoystickButtons(joysticks[i], &button_count);
                if (button_count)
                {
                    for (j = 0;  j < button_count;  j++)
                    {
                        char name[16];
                        snprintf(name, sizeof(name), "%i", j + 1);
                        nk_select_label(nk, name, NK_TEXT_CENTERED, buttons[j]);
                    }
                }

                nk_layout_row_end(nk);
            }

            nk_end(nk);
        }

        nk_glfw3_render(NK_ANTI_ALIASING_ON, 10000, 1000);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

