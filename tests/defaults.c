//========================================================================
// Default window/context test
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
// This test creates a windowed mode window with all parameters set to
// default values and then reports the actual parameters of the created
// window and context
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#ifndef GL_ARB_multisample
 #define GL_SAMPLES_ARB 0x80A9
#endif

typedef struct
{
    int param;
    char* ext;
    char* name;
} ParamGL;

typedef struct
{
    int param;
    char* name;
} ParamGLFW;

static ParamGL gl_params[] =
{
    { GL_RED_BITS, NULL, "red bits" },
    { GL_GREEN_BITS, NULL, "green bits" },
    { GL_BLUE_BITS, NULL, "blue bits" },
    { GL_ALPHA_BITS, NULL, "alpha bits" },
    { GL_DEPTH_BITS, NULL, "depth bits" },
    { GL_STENCIL_BITS, NULL, "stencil bits" },
    { GL_STEREO, NULL, "stereo" },
    { GL_SAMPLES_ARB, "GL_ARB_multisample", "FSAA samples" },
    { 0, NULL, NULL }
};

static ParamGLFW glfw_params[] =
{
    { GLFW_REFRESH_RATE, "refresh rate" },
    { GLFW_OPENGL_VERSION_MAJOR, "OpenGL major" },
    { GLFW_OPENGL_VERSION_MINOR, "OpenGL minor" },
    { GLFW_OPENGL_FORWARD_COMPAT, "OpenGL forward compatible" },
    { GLFW_OPENGL_DEBUG_CONTEXT, "OpenGL debug context" },
    { GLFW_OPENGL_PROFILE, "OpenGL profile" },
    { 0, NULL }
};

int main(void)
{
    int i, width, height;
    GLFWwindow window;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(0, 0, GLFW_WINDOWED, "Defaults", NULL);
    if (!window)
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window: %s\n", glfwErrorString(glfwGetError()));
        exit(EXIT_FAILURE);
    }

    glfwGetWindowSize(window, &width, &height);

    printf("window size: %ix%i\n", width, height);

    for (i = 0;  glfw_params[i].name;   i++)
    {
        printf("%s: %i\n",
               glfw_params[i].name,
               glfwGetWindowParam(window, glfw_params[i].param));
    }

    for (i = 0;  gl_params[i].name;   i++)
    {
        GLint value = 0;

        if (gl_params[i].ext)
        {
            if (!glfwExtensionSupported(gl_params[i].ext))
                continue;
        }

        glGetIntegerv(gl_params[i].param, &value);

        printf("%s: %i\n", gl_params[i].name, value);
    }

    glfwDestroyWindow(window);
    window = NULL;

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

