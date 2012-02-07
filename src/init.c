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

#define _init_c_
#include "internal.h"

#include <string.h>
#include <stdlib.h>


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Initialize various GLFW state
//========================================================================

GLFWAPI int glfwInit(void)
{
    if (_glfwInitialized)
        return GL_TRUE;

    memset(&_glfwLibrary, 0, sizeof(_glfwLibrary));

    // Not all window hints have zero as their default value, so this
    // needs to be here despite the memset above
    _glfwSetDefaultWindowHints();

    if (!_glfwPlatformInit())
    {
        _glfwPlatformTerminate();
        return GL_FALSE;
    }

    atexit(glfwTerminate);

    _glfwInitialized = GL_TRUE;

    return GL_TRUE;
}


//========================================================================
// Close window and shut down library
//========================================================================

GLFWAPI void glfwTerminate(void)
{
    if (!_glfwInitialized)
        return;

    // Close all remaining windows
    while (_glfwLibrary.windowListHead)
        glfwCloseWindow(_glfwLibrary.windowListHead);

    if (!_glfwPlatformTerminate())
        return;

    _glfwInitialized = GL_FALSE;
}


//========================================================================
// Get GLFW version
//========================================================================

GLFWAPI void glfwGetVersion(int* major, int* minor, int* rev)
{
    if (major != NULL)
        *major = GLFW_VERSION_MAJOR;

    if (minor != NULL)
        *minor = GLFW_VERSION_MINOR;

    if (rev != NULL)
        *rev = GLFW_VERSION_REVISION;
}


//========================================================================
// Get the GLFW version string
//========================================================================

GLFWAPI const char* glfwGetVersionString(void)
{
    return _glfwPlatformGetVersionString();
}

