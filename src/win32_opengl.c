//========================================================================
// GLFW - An OpenGL library
// Platform:    Win32/WGL
// API version: 3.0
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
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

#include "internal.h"


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Make the OpenGL context associated with the specified window current
//========================================================================

void _glfwPlatformMakeContextCurrent(_GLFWwindow* window)
{
    if (window)
        wglMakeCurrent(window->WGL.DC, window->WGL.context);
    else
        wglMakeCurrent(NULL, NULL);
}


//========================================================================
// Swap buffers (double-buffering)
//========================================================================

void _glfwPlatformSwapBuffers(void)
{
    _GLFWwindow* window = _glfwLibrary.currentWindow;

    SwapBuffers(window->WGL.DC);
}


//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval(int interval)
{
    _GLFWwindow* window = _glfwLibrary.currentWindow;

    if (window->WGL.EXT_swap_control)
        window->WGL.SwapIntervalEXT(interval);
}


//========================================================================
// Check if the current context supports the specified WGL extension
//========================================================================

int _glfwPlatformExtensionSupported(const char* extension)
{
    const GLubyte* extensions;

    _GLFWwindow* window = _glfwLibrary.currentWindow;

    if (window->WGL.GetExtensionsStringEXT != NULL)
    {
        extensions = (GLubyte*) window->WGL.GetExtensionsStringEXT();
        if (extensions != NULL)
        {
            if (_glfwStringInExtensionString(extension, extensions))
                return GL_TRUE;
        }
    }

    if (window->WGL.GetExtensionsStringARB != NULL)
    {
        extensions = (GLubyte*) window->WGL.GetExtensionsStringARB(window->WGL.DC);
        if (extensions != NULL)
        {
            if (_glfwStringInExtensionString(extension, extensions))
                return GL_TRUE;
        }
    }

    return GL_FALSE;
}


//========================================================================
// Get the function pointer to an OpenGL function
//========================================================================

void* _glfwPlatformGetProcAddress(const char* procname)
{
    return (void*) wglGetProcAddress(procname);
}


//========================================================================
// Copies the specified OpenGL state categories from src to dst
//========================================================================

void _glfwPlatformCopyContext(_GLFWwindow* src, _GLFWwindow* dst, unsigned long mask)
{
    if (!wglCopyContext(src->WGL.context, dst->WGL.context, mask))
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32/WGL: Failed to copy OpenGL context attributes");
    }
}

