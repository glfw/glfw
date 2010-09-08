//========================================================================
// GLFW - An OpenGL framework
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


//************************************************************************
//****                    GLFW user functions                         ****
//************************************************************************

//========================================================================
// Get a list of available video modes
//========================================================================

GLFWAPI int glfwGetVideoModes(GLFWvidmode* list, int maxcount)
{
    int count, i, swap, res1, res2, depth1, depth2;
    GLFWvidmode vm;

    if (!_glfwInitialized || maxcount <= 0 || list == NULL)
        return 0;

    // Get list of video modes
    count = _glfwPlatformGetVideoModes(list, maxcount);

    // Sort list (bubble sort)
    do
    {
        swap = 0;
        for (i = 0;  i < count - 1;  i++)
        {
            res1   = list[i].width*list[i].height;
            depth1 = list[i].redBits+list[i].greenBits+list[i].blueBits;
            res2   = list[i + 1].width*list[i + 1].height;
            depth2 = list[i + 1].redBits+list[i + 1].greenBits+
                     list[i + 1].blueBits;

            if ((depth2 < depth1) || ((depth2 == depth1) && (res2 < res1)))
            {
                vm = list[i];
                list[i] = list[i + 1];
                list[i + 1] = vm;
                swap = 1;
            }
        }
    }
    while (swap);

    return count;
}


//========================================================================
// Get the desktop video mode
//========================================================================

GLFWAPI void glfwGetDesktopMode(GLFWvidmode* mode)
{
    if (!_glfwInitialized || mode == NULL)
        return;

    _glfwPlatformGetDesktopMode(mode);
}

