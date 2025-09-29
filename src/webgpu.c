//========================================================================
// GLFW 3.5 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) Sebastian Dawid <sebdawid@gmail.com>
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

#include <assert.h>
#include <stdlib.h>

GLFWAPI WGPUSurface glfwCreateWindowWGPUSurface(WGPUInstance instance, GLFWwindow* handle)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL)

    _GLFWwindow* window = (_GLFWwindow*)handle;

    assert(window != NULL);
    assert(instance != NULL);
    assert(&wgpuInstanceCreateSurface != NULL);

    if (window->context.client != GLFW_NO_API)
    {
        _glfwInputError(GLFW_INVALID_VALUE,
                        "WebGPU: Window surface creation requires the window to have the client API set to GLFW_NO_API");
        return NULL;
    };

    return _glfw.platform.createWindowWGPUSurface(instance, window);
}
