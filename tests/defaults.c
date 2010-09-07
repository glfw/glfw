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

#include <GL/glfw.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    int param;
    char* name;
} Param;

static Param parameters[] =
{
    { GLFW_ACCELERATED, "accelerated" },
    { GLFW_RED_BITS, "red bits" },
    { GLFW_GREEN_BITS, "green bits" },
    { GLFW_BLUE_BITS, "blue bits" },
    { GLFW_ALPHA_BITS, "alpha bits" },
    { GLFW_DEPTH_BITS, "depth bits" },
    { GLFW_STENCIL_BITS, "stencil bits" },
    { GLFW_REFRESH_RATE, "refresh rate" },
    { GLFW_ACCUM_RED_BITS, "accum red bits" },
    { GLFW_ACCUM_GREEN_BITS, "accum green bits" },
    { GLFW_ACCUM_BLUE_BITS, "accum blue bits" },
    { GLFW_ACCUM_ALPHA_BITS, "accum alpha bits" },
    { GLFW_AUX_BUFFERS, "aux buffers" },
    { GLFW_STEREO, "stereo" },
    { GLFW_FSAA_SAMPLES, "FSAA samples" },
    { GLFW_OPENGL_VERSION_MAJOR, "OpenGL major" },
    { GLFW_OPENGL_VERSION_MINOR, "OpenGL minor" },
    { GLFW_OPENGL_FORWARD_COMPAT, "OpenGL forward compatible" },
    { GLFW_OPENGL_DEBUG_CONTEXT, "OpenGL debug context" },
    { GLFW_OPENGL_PROFILE, "OpenGL profile" },
};

int main(void)
{
    int i, width, height;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(1);
    }

    if (!glfwOpenWindow(0, 0, 0, 0, 0, 0, 0, 0, GLFW_WINDOW))
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW default window\n");
        exit(1);
    }

    glfwGetWindowSize(&width, &height);

    printf("window size: %ix%i\n", width, height);

    for (i = 0;  (size_t) i < sizeof(parameters) / sizeof(parameters[0]);  i++)
    {
        printf("%s: %i\n", parameters[i].name, glfwGetWindowParam(parameters[i].param));
    }

    glfwCloseWindow();
    glfwTerminate();
    exit(0);
}

