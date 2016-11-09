/**
 * This code was originally derived from the threads test code that came with
 * the license below.  However, its purpose has changed; while it does show
 * how to use threads, its real purpose is to show how to draw images on a
 * window based on the location of a different window.
 */

//========================================================================
// Multi-threading test
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
// This test is intended to verify whether the OpenGL context part of
// the GLFW API is able to be used from multiple threads
//
//========================================================================

#include "tinycthread.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static const struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.72f, 0.f, 0.f, 1.f }
};

static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

typedef struct
{
    GLFWwindow* primary_window;
    GLFWwindow* window;
    const char* title;
    float r, g, b, a;
    thrd_t id;
} Thread;

static volatile int running = GLFW_TRUE;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static int thread_main(void* data)
{
    const Thread* thread = data;

    glfwMakeContextCurrent(thread->window);
    glfwSwapInterval(1);

    while (running)
    {
        const float v = (float) fabs(sin(glfwGetTime() * 2.f));
        glClearColor(thread->r * v, thread->g * v, thread->b * v, thread->a * v);

        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(thread->window);
    }

    glfwMakeContextCurrent(NULL);
    return 0;
}

static int other_thread_main(void* data)
{
    const Thread* thread = data;

    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    glfwMakeContextCurrent(thread->window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 5, (void*) 0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 5, (void*) (sizeof(float) * 2));

    while (running)
    {
        int primary_x_pos, primary_y_pos;
        int primary_width, primary_height;
        int my_x_pos, my_y_pos;
        int my_width, my_height;
        float delta_x, delta_y;
        float ratio, x_ratio, y_ratio;
        float x_offset, y_offset;
        mat4x4 m, p, mvp;

        glfwGetWindowPos(thread->primary_window, &primary_x_pos, &primary_y_pos);
        glfwGetFramebufferSize(thread->primary_window, &primary_width, &primary_height);
        glfwGetWindowPos(thread->window, &my_x_pos, &my_y_pos);
        glfwGetFramebufferSize(thread->window, &my_width, &my_height);

        const float v = (float) fabs(sin(glfwGetTime() * 2.f));
        ratio = my_width / (float) my_height;
        x_ratio = (my_width / (float) primary_width);
        y_ratio = (my_height / (float) primary_height);

        delta_x = (((float)(primary_x_pos - my_x_pos)) / ((float) my_width)) * x_ratio * 2 - x_ratio + 1;
        delta_y = (-((float)(primary_y_pos - my_y_pos)) / ((float) my_height)) * y_ratio * 2 + y_ratio - 1;

        glViewport(0, 0, my_width, my_height);

        glClearColor(thread->r * v, thread->g * v, thread->b * v, thread->a * v);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -x_ratio, x_ratio, -y_ratio, y_ratio, 1.f, -1.f);
        mat4x4_translate_in_place(p, delta_x, delta_y, 0.0f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(thread->window);
    }

    glfwMakeContextCurrent(NULL);
    return 0;
}

int main(void)
{
    int i, result;
    Thread threads[] =
    {
        { NULL, NULL, "Red",   1.0f, 0.0f, 0.0f, 0.5f, 0 },
        { NULL, NULL, "Green", 0.0f, 1.0f, 0.0f, 0.5f, 0 },
        { NULL, NULL, "Blue",  0.0f, 0.0f, 1.0f, 0.5f, 0 }
    };
    const int count = sizeof(threads) / sizeof(Thread);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_TRANSPARENT, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    for (i = 0;  i < count;  i++)
    {
        threads[i].window = glfwCreateWindow(200, 200,
                                             threads[i].title,
                                             NULL, NULL);
        if (!threads[i].window)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwSetWindowPos(threads[i].window, 200 + 250 * i, 200);
        glfwShowWindow(threads[i].window);
    }

    for (i = 0; i < count; i++)
    {
        threads[i].primary_window = threads[0].window;
    }

    glfwMakeContextCurrent(threads[0].window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwMakeContextCurrent(NULL);

    for (i = 0;  i < count;  i++)
    {
        result = thrd_create(&threads[i].id, other_thread_main, threads + i);

        if (result != thrd_success)
        {
            fprintf(stderr, "Failed to create secondary thread\n");

            glfwTerminate();
            exit(EXIT_FAILURE);
        }
    }

    while (running)
    {
        glfwWaitEvents();

        for (i = 0;  i < count;  i++)
        {
            if (glfwWindowShouldClose(threads[i].window))
                running = GLFW_FALSE;
        }
    }

    for (i = 0;  i < count;  i++)
        glfwHideWindow(threads[i].window);

    for (i = 0;  i < count;  i++)
        thrd_join(threads[i].id, &result);

    exit(EXIT_SUCCESS);
}

