//========================================================================
// GLFW - An OpenGL library
// Platform:    Any
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

#include <stdlib.h>


//========================================================================
// Lexical comparison function for GLFW video modes, used by qsort
//========================================================================

static int compareVideoModes(const void* firstPtr, const void* secondPtr)
{
    int firstBPP, secondBPP, firstSize, secondSize;
    GLFWvidmode* first = (GLFWvidmode*) firstPtr;
    GLFWvidmode* second = (GLFWvidmode*) secondPtr;

    // First sort on color bits per pixel

    firstBPP = first->redBits +
               first->greenBits +
               first->blueBits;
    secondBPP = second->redBits +
                second->greenBits +
                second->blueBits;

    if (firstBPP != secondBPP)
        return firstBPP - secondBPP;

    // Then sort on screen area, in pixels

    firstSize = first->width * first->height;
    secondSize = second->width * second->height;

    return firstSize - secondSize;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Convert BPP to RGB bits based on "best guess"
//========================================================================

void _glfwSplitBPP(int bpp, int* red, int* green, int* blue)
{
    int delta;

    // We assume that by 32 they really meant 24
    if (bpp == 32)
        bpp = 24;

    // Convert "bits per pixel" to red, green & blue sizes

    *red = *green = *blue = bpp / 3;
    delta = bpp - (*red * 3);
    if (delta >= 1)
        *green = *green + 1;

    if (delta == 2)
        *red = *red + 1;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Get a list of available video modes
//========================================================================

GLFWAPI int glfwGetVideoModes(GLFWvidmode* list, int maxcount)
{
    int count;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return 0;
    }

    if (maxcount <= 0)
    {
        _glfwSetError(GLFW_INVALID_VALUE,
                      "glfwGetVideoModes: Parameter 'maxcount' must be "
                      "greater than zero");
        return 0;
    }

    if (list == NULL)
    {
        _glfwSetError(GLFW_INVALID_VALUE,
                      "glfwGetVideoModes: Parameter 'list' cannot be NULL");
        return 0;
    }

    count = _glfwPlatformGetVideoModes(list, maxcount);
    if (count > 0)
        qsort(list, count, sizeof(GLFWvidmode), compareVideoModes);

    return count;
}


//========================================================================
// Get the desktop video mode
//========================================================================

GLFWAPI void glfwGetDesktopMode(GLFWvidmode* mode)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (mode == NULL)
    {
        _glfwSetError(GLFW_INVALID_VALUE,
                      "glfwGetDesktopMode: Parameter 'mode' cannot be NULL");
        return;
    }

    _glfwPlatformGetDesktopMode(mode);
}

