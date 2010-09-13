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

#include <string.h>


#ifndef GL_VERSION_3_0
#define GL_NUM_EXTENSIONS 0x821D
#endif


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Parses the OpenGL version string and extracts the version number
//========================================================================

void _glfwParseGLVersion(int* major, int* minor, int* rev)
{
    GLuint _major, _minor = 0, _rev = 0;
    const GLubyte* version;
    const GLubyte* ptr;

    // Get OpenGL version string
    version = glGetString(GL_VERSION);
    if (!version)
        return;

    // Parse string
    ptr = version;
    for (_major = 0;  *ptr >= '0' && *ptr <= '9';  ptr++)
        _major = 10 * _major + (*ptr - '0');

    if (*ptr == '.')
    {
        ptr++;
        for (_minor = 0;  *ptr >= '0' && *ptr <= '9';  ptr++)
            _minor = 10*_minor + (*ptr - '0');

        if (*ptr == '.')
        {
            ptr++;
            for (_rev = 0;  *ptr >= '0' && *ptr <= '9';  ptr++)
                _rev = 10*_rev + (*ptr - '0');
        }
    }

    // Return parsed values
    *major = _major;
    *minor = _minor;
    *rev = _rev;
}

//========================================================================
// Check if a string can be found in an OpenGL extension string
//========================================================================

int _glfwStringInExtensionString(const char* string,
                                 const GLubyte* extensions)
{
    const GLubyte* start;
    GLubyte* where;
    GLubyte* terminator;

    // It takes a bit of care to be fool-proof about parsing the
    // OpenGL extensions string. Don't be fooled by sub-strings,
    // etc.
    start = extensions;
    for (;;)
    {
        where = (GLubyte*) strstr((const char*) start, string);
        if (!where)
            return GL_FALSE;

        terminator = where + strlen(string);
        if (where == start || *(where - 1) == ' ')
        {
            if (*terminator == ' ' || *terminator == '\0')
                break;
        }

        start = terminator;
    }

    return GL_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Check if an OpenGL extension is available at runtime
//========================================================================

GLFWAPI int glfwExtensionSupported(const char* extension)
{
    const GLubyte* extensions;
    _GLFWwindow* window;
    GLubyte* where;
    GLint count;
    int i;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return GL_FALSE;
    }

    window = _glfwLibrary.currentWindow;
    if (!window)
    {
        _glfwSetError(GLFW_NO_CURRENT_WINDOW);
        return GL_FALSE;
    }

    // Extension names should not have spaces
    where = (GLubyte*) strchr(extension, ' ');
    if (where || *extension == '\0')
        return GL_FALSE;

    if (window->glMajor < 3)
    {
        // Check if extension is in the old style OpenGL extensions string

        extensions = glGetString(GL_EXTENSIONS);
        if (extensions != NULL)
        {
            if (_glfwStringInExtensionString(extension, extensions))
                return GL_TRUE;
        }
    }
    else
    {
        // Check if extension is in the modern OpenGL extensions string list

        glGetIntegerv(GL_NUM_EXTENSIONS, &count);

        for (i = 0;  i < count;  i++)
        {
             if (strcmp((const char*) window->GetStringi(GL_EXTENSIONS, i),
                         extension) == 0)
             {
                 return GL_TRUE;
             }
        }
    }

    // Additional platform specific extension checking (e.g. WGL)
    if (_glfwPlatformExtensionSupported(extension))
        return GL_TRUE;

    return GL_FALSE;
}


//========================================================================
// Get the function pointer to an OpenGL function.
// This function can be used to get access to extended OpenGL functions.
//========================================================================

GLFWAPI void* glfwGetProcAddress(const char* procname)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return NULL;
    }

    if (!_glfwLibrary.currentWindow)
    {
        _glfwSetError(GLFW_NO_CURRENT_WINDOW);
        return NULL;
    }

    return _glfwPlatformGetProcAddress(procname);
}


//========================================================================
// Returns the OpenGL version
//========================================================================

GLFWAPI void glfwGetGLVersion(int* major, int* minor, int* rev)
{
    _GLFWwindow* window;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return;
    }

    window = _glfwLibrary.currentWindow;
    if (!window)
    {
        _glfwSetError(GLFW_NO_CURRENT_WINDOW);
        return;
    }

    if (major != NULL)
        *major = window->glMajor;

    if (minor != NULL)
        *minor = window->glMinor;

    if (rev != NULL)
        *rev = window->glRevision;
}

