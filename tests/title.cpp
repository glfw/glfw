//========================================================================
// UTF-8 window title test (C++ version)
// Copyright (c) Matt Coster <matt@mtcoster.net>
// Based on the original C version by Camilla Löwy <elmindreda@glfw.org>
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
// This test sets a UTF-8 window title
//
//========================================================================

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.hpp>

#include <cstdlib>
#include <iostream>

static void error_callback(int error, const char* description) {
    std::cerr << "Error " << error << ": " << description << "\n";
}

int main() {
    try {
        auto glfwInst = glfw::GLFW(&error_callback);

        auto window = glfwInst.createWindow({400, 400}, "English 日本語 русский язык 官話");

        window.makeContextCurrent();
        gladLoadGL(glfwGetProcAddress);
        glfwInst.swapInterval(1);

        while (!window.shouldClose()) {
            glClear(GL_COLOR_BUFFER_BIT);
            window.swapBuffers();
            glfwInst.waitEvents();
        }
    } catch (glfw::Error const &err) {
        std::cerr << "GLFW error: " << err.what() << "\n";
        std::exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
