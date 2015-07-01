//========================================================================
// GLFW 3.1 Wayland - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2015 Anthony Smith <asmith@anthonycodes.com>
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

#include <malloc.h>

// initialize the global memory state to use
// the standard malloc, calloc and free functions
_GLFWmemory _memory = { malloc, calloc, free };

//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI int glfwSetMemoryFuncs(GLFWmallocfun mallocfun, GLFWcallocfun callocfun, GLFWfreefun freefun)
{
    if(_glfwInitialized)
    {
        _glfwInputError(GLFW_INITIALIZED, NULL);
        return GL_FALSE;
    }
    if(!mallocfun || !callocfun || !freefun)
    {
        _glfwInputError(GLFW_INVALID_VALUE, NULL);
        return GL_FALSE;
    }
    _memory.malloc = mallocfun;
    _memory.calloc = callocfun;
    _memory.free   = freefun;
    return GL_TRUE;
}
