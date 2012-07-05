//========================================================================
// GLFW - An OpenGL library
// Platform:    All
// API version: 3.0
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2008-2010 Camilla Berglund <elmindreda@elmindreda.org>
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
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// The current error value and callback
// These are not in _glfwLibrary since they need to be initialized and
// accessible before glfwInit so it can report errors
//========================================================================

static int _glfwError = GLFW_NO_ERROR;
static GLFWerrorfun _glfwErrorCallback = NULL;


//========================================================================
// Sets the current error value
// This function may be called without GLFW having been initialized
//========================================================================

void _glfwSetError(int error, const char* description)
{
    if (_glfwErrorCallback)
    {
        if (!description)
            description = glfwErrorString(error);

        _glfwErrorCallback(error, description);
    }
    else
        _glfwError = error;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Returns the current error value
// This function may be called without GLFW having been initialized
//========================================================================

GLFWAPI int glfwGetError(void)
{
    int error = _glfwError;
    _glfwError = GLFW_NO_ERROR;
    return error;
}


//========================================================================
// Returns a string representation of the specified error value
// This function may be called without GLFW having been initialized
//========================================================================

GLFWAPI const char* glfwErrorString(int error)
{
    switch (error)
    {
        case GLFW_NO_ERROR:
            return "No error";
        case GLFW_NOT_INITIALIZED:
            return "The GLFW library is not initialized";
        case GLFW_NO_CURRENT_WINDOW:
            return "There is no current GLFW window";
        case GLFW_INVALID_ENUM:
            return "Invalid argument for enum parameter";
        case GLFW_INVALID_VALUE:
            return "Invalid value for parameter";
        case GLFW_OUT_OF_MEMORY:
            return "Out of memory";
        case GLFW_OPENGL_UNAVAILABLE:
            return "OpenGL is not available on this machine";
        case GLFW_VERSION_UNAVAILABLE:
            return "The requested OpenGL version is unavailable";
        case GLFW_PLATFORM_ERROR:
            return "A platform-specific error occurred";
        case GLFW_WINDOW_NOT_ACTIVE:
            return "The specified window is not active";
        case GLFW_FORMAT_UNAVAILABLE:
            return "The requested format is unavailable";
    }

    return "ERROR: UNKNOWN ERROR TOKEN PASSED TO glfwErrorString";
}


//========================================================================
// Sets the callback function for GLFW errors
// This function may be called without GLFW having been initialized
//========================================================================

GLFWAPI void glfwSetErrorCallback(GLFWerrorfun cbfun)
{
    _glfwErrorCallback = cbfun;
}

