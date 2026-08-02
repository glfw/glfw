//========================================================================
// User context test
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
// This test is intended to verify whether the OpenGL user context part of
// the GLFW API is able to be used from multiple threads
//
//========================================================================

#include "tinycthread.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static int thread_main(void* data)
{
    GLFWusercontext* usercontext = (GLFWusercontext*)data;

    /* set the user context current */
    glfwMakeUserContextCurrent(usercontext);

    if (glfwGetCurrentContext() != NULL)
    {
        fprintf(stderr, "Current glfw window context not NULL after glfwMakeUserContextCurrent\n");
        glfwTerminate();
        return -1;
    }
    if (glfwGetCurrentUserContext() != usercontext)
    {
        fprintf(stderr, "Current user context not correct after glfwMakeUserContextCurrent\n");
        glfwTerminate();
        return -1;
    }

    /* set the user context to NULL */
    glfwMakeUserContextCurrent(NULL);
    if (glfwGetCurrentUserContext() != NULL)
    {
        fprintf(stderr, "Current user context not NULL after glfwMakeContextCurrent\n");
        glfwTerminate();
        return -1;
    }

    return 0;
}

int main(void)
{
    GLFWwindow* window;
    GLFWusercontext* usercontext;
    thrd_t thread_id;
    int result, count;

    glfwSetErrorCallback(error_callback);

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "User Context", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    /* make a new context */
    usercontext = glfwCreateUserContext(window);
    if (!usercontext)
    {
        fprintf(stderr, "Failed to create user context\n");
        glfwTerminate();
        return -1;
    }

    /* set the user context current */
    glfwMakeUserContextCurrent(usercontext);

    if (glfwGetCurrentContext() != NULL)
    {
        fprintf(stderr, "Current glfw window context not NULL after glfwMakeUserContextCurrent\n");
        glfwTerminate();
        return -1;
    }
    if (glfwGetCurrentUserContext() != usercontext)
    {
        fprintf(stderr, "Current user context not correct after glfwMakeUserContextCurrent\n");
        glfwTerminate();
        return -1;
    }

    /* set the window context current */
    glfwMakeContextCurrent(window);

    if (glfwGetCurrentUserContext() != NULL)
    {
        fprintf(stderr, "Current user context not NULL after glfwMakeContextCurrent\n");
        glfwTerminate();
        return -1;
    }
    if (glfwGetCurrentContext() != window)
    {
        fprintf(stderr, "Current glfw window context not correct after glfwMakeContextCurrent\n");
        glfwTerminate();
        return -1;
    }

    glClearColor( 0.4f, 0.3f, 0.4f, 1.0f );

    // Launch a thread which should create and use the usercontext
    if (thrd_create(&thread_id, thread_main, usercontext ) !=
            thrd_success)
    {
        fprintf(stderr, "Failed to create secondary thread\n");

        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    /* Loop 60 times or until the user closes the window */
    count = 0;
    while (!glfwWindowShouldClose(window) && count++ < 60)
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    thrd_join(thread_id, &result);

    /* One more test now the thread has joined */

    /* set the user context current */
    glfwMakeUserContextCurrent(usercontext);

    if (glfwGetCurrentContext() != NULL)
    {
        fprintf(stderr, "Current glfw window context not NULL after glfwMakeUserContextCurrent\n");
        glfwTerminate();
        return -1;
    }
    if (glfwGetCurrentUserContext() != usercontext)
    {
        fprintf(stderr, "Current user context not correct after glfwMakeUserContextCurrent\n");
        glfwTerminate();
        return -1;
    }

    /* set the user context to NULL */
    glfwMakeUserContextCurrent(NULL);
    if (glfwGetCurrentUserContext() != NULL)
    {
        fprintf(stderr, "Current user context not NULL after glfwMakeContextCurrent\n");
        glfwTerminate();
        return -1;
    }

    glfwDestroyUserContext(usercontext);
    glfwTerminate();
    return 0;
}