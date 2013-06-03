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
// This test creates a windowed mode window with all window hints set to
// default values and then reports the actual attributes of the created
// window and context
//
//========================================================================

#include <GLFW/glfw3.h>
#include <GL/glext.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    int attrib;
    const char* ext;
    const char* name;
} AttribGL;

typedef struct
{
    int attrib;
    const char* name;
} AttribGLFW;

static AttribGL gl_attribs[] =
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

static AttribGLFW glfw_attribs[] =
{
    { GLFW_CONTEXT_VERSION_MAJOR, "Context version major" },
    { GLFW_CONTEXT_VERSION_MINOR, "Context version minor" },
    { GLFW_OPENGL_FORWARD_COMPAT, "OpenGL forward compatible" },
    { GLFW_OPENGL_DEBUG_CONTEXT, "OpenGL debug context" },
    { GLFW_OPENGL_PROFILE, "OpenGL profile" },
    { 0, NULL }
};

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(void)
{
    int i, width, height;
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    window = glfwCreateWindow(640, 480, "Defaults", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &width, &height);

    printf("framebuffer size: %ix%i\n", width, height);

    for (i = 0;  glfw_attribs[i].name;   i++)
    {
        printf("%s: %i\n",
               glfw_attribs[i].name,
               glfwGetWindowAttrib(window, glfw_attribs[i].attrib));
    }

    for (i = 0;  gl_attribs[i].name;   i++)
    {
        GLint value = 0;

        if (gl_attribs[i].ext)
        {
            if (!glfwExtensionSupported(gl_attribs[i].ext))
                continue;
        }

        glGetIntegerv(gl_attribs[i].attrib, &value);

        printf("%s: %i\n", gl_attribs[i].name, value);
    }

    glfwDestroyWindow(window);
    window = NULL;

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

