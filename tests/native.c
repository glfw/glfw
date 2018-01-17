//========================================================================
// Win32 native handle attachment test
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

#define UNICODE

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // This will only be used until glfwAttachWin32Window
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int main(void)
{
    GLFWwindow* window;
    WNDCLASSEX wc;
    HWND handle;

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC) windowProc;
    wc.hInstance     = GetModuleHandleW(NULL);
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.lpszClassName = L"SomeKindOfWindowClassName";

    if (!RegisterClassExW(&wc))
        exit(EXIT_FAILURE);

    handle = CreateWindowExW(WS_EX_APPWINDOW,
                             L"SomeKindOfWindowClassName",
                             L"HWND attachment test",
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             600, 400,
                             NULL,
                             NULL,
                             GetModuleHandleW(NULL),
                             NULL);

    if (!handle)
        exit(EXIT_FAILURE);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        DestroyWindow(handle);
        exit(EXIT_FAILURE);
    }

    window = glfwAttachWin32Window(handle, NULL);
    if (!window)
    {
        glfwTerminate();
        DestroyWindow(handle);
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    glfwTerminate();
    DestroyWindow(handle);
    exit(EXIT_SUCCESS);
}

