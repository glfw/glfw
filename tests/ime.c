//========================================================================
// IME test
// Copyright (c) Daijiro Fukuda <fukuda@clear-code.com>
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
// This test hooks IME callbacks.
// Left-click clears preedit and toggles IME status.
// Right-click updates preedit cursor position to current cursor position.
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

void usage(void)
{
    printf("Usage: inputlag [-h] [-f]\n");
    printf("Options:\n");
    printf("  -f create full screen window\n");
    printf("  -h show this help\n");
}

static size_t encode_utf8(char* s, unsigned int ch)
{
    size_t count = 0;

    if (ch < 0x80)
        s[count++] = (char) ch;
    else if (ch < 0x800)
    {
        s[count++] = (ch >> 6) | 0xc0;
        s[count++] = (ch & 0x3f) | 0x80;
    }
    else if (ch < 0x10000)
    {
        s[count++] = (ch >> 12) | 0xe0;
        s[count++] = ((ch >> 6) & 0x3f) | 0x80;
        s[count++] = (ch & 0x3f) | 0x80;
    }
    else if (ch < 0x110000)
    {
        s[count++] = (ch >> 18) | 0xf0;
        s[count++] = ((ch >> 12) & 0x3f) | 0x80;
        s[count++] = ((ch >> 6) & 0x3f) | 0x80;
        s[count++] = (ch & 0x3f) | 0x80;
    }

    return count;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            int currentIMEstatus = glfwGetInputMode(window, GLFW_IME);
            glfwSetInputMode(window, GLFW_IME, !currentIMEstatus);
            glfwResetPreeditText(window);
            printf("Reset preedit text and IME status -> %s\n", currentIMEstatus ? "OFF" : "ON");
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            int preX, preY;
            double curX, curY;
            glfwGetPreeditCursorPos(window, &preX, &preY, NULL);
            glfwGetCursorPos(window, &curX, &curY);
            glfwSetPreeditCursorPos(window, curX, curY, 20);
            printf("Move preedit text cursor position (%d, %d) -> (%d, %d)\n",
                   preX, preY, (int)curX, (int)curY);
        }
    }
}

static void char_callback(GLFWwindow* window, unsigned int codepoint)
{
    char string[5] = "";

    encode_utf8(string, codepoint);
    printf("Character 0x%08x (%s) input\n", codepoint, string);
}

static void preedit_callback(GLFWwindow* window, int strLength, unsigned int* string, int blockLength, int* blocks, int focusedBlock) {
    int i, blockIndex = -1, blockCount = 0;
    int width, height;
    char encoded[5] = "";
    printf("Preedit text ");
    if (strLength == 0 || blockLength == 0) {
        printf("(empty)\n");
    } else {
        for (i = 0; i < strLength; i++) {
            if (blockCount == 0) {
                if (blockIndex == focusedBlock) {
                    printf("]");
                }
                blockIndex++;
                blockCount = blocks[blockIndex];
                printf("\n   block %d: ", blockIndex);
                if (blockIndex == focusedBlock) {
                    printf("[");
                }
            }
            encode_utf8(encoded, string[i]);
            printf("%s", encoded);
            blockCount--;
        }
        if (blockIndex == focusedBlock) {
            printf("]");
        }
        printf("\n");
    }
}

static void ime_callback(GLFWwindow* window) {
    int currentIMEstatus = glfwGetInputMode(window, GLFW_IME);
    printf("IME switched: %s\n", currentIMEstatus ? "ON" : "OFF");
}

int main(int argc, char** argv)
{
    int ch, width, height;
    int fullscreen = GLFW_FALSE;
    GLFWmonitor* monitor = NULL;
    GLFWwindow* window;

    while ((ch = getopt(argc, argv, "fh")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                fullscreen = GLFW_TRUE;
                break;
        }
    }

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if (fullscreen)
    {
        const GLFWvidmode* mode;

        monitor = glfwGetPrimaryMonitor();
        mode = glfwGetVideoMode(monitor);

        width = mode->width;
        height = mode->height;
    }
    else
    {
        width = 640;
        height = 480;
    }

    window = glfwCreateWindow(width, height, "IME test", monitor, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetPreeditCallback(window, preedit_callback);
    glfwSetIMEStatusCallback(window, ime_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwWaitEvents();
        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

