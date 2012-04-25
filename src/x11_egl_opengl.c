//========================================================================
// GLFW - An OpenGL library
// Platform:    X11/EGL/GLES
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

#include <stdio.h>
#include <stdlib.h>

// Max number of EGL configuration we handle
#define _GLFW_EGL_CONFIG_IN 15

//========================================================================
// Returns the specified attribute of the specified EGLConfig
//========================================================================

static int getFBConfigAttrib(_GLFWwindow* window, EGLConfig fbconfig, int attrib)
{
    int value;
    eglGetConfigAttrib(_glfwLibrary.EGL.display, fbconfig, attrib, &value);
    return value;
}


//========================================================================
// Return a list of available and usable framebuffer configs
//========================================================================

static _GLFWfbconfig* getFBConfigs(_GLFWwindow* window, unsigned int* found)
{
    EGLConfig fbconfigs[_GLFW_EGL_CONFIG_IN];
    _GLFWfbconfig* result;
    int i, count = 0;

    *found = 0;


    eglGetConfigs(_glfwLibrary.EGL.display, fbconfigs,
            _GLFW_EGL_CONFIG_IN, &count);
    if (!count)
    {
        _glfwSetError(GLFW_OPENGL_UNAVAILABLE,
                      "X11/EGL: No EGLConfigs returned");
        return NULL;
    }

    result = (_GLFWfbconfig*) malloc(sizeof(_GLFWfbconfig) * count);
    if (!result)
    {
        _glfwSetError(GLFW_OUT_OF_MEMORY,
                      "X11/EGL: Failed to allocate _GLFWfbconfig array");
        return NULL;
    }

    for (i = 0;  i < count;  i++)
    {
        if (!getFBConfigAttrib(window, fbconfigs[i], EGL_NATIVE_VISUAL_ID))
        {
            // Only consider EGLConfigs with associated visuals
            continue;
        }

        if (!(getFBConfigAttrib(window,
                                fbconfigs[i],
                                EGL_COLOR_BUFFER_TYPE) & EGL_RGB_BUFFER))
        {
            // Only consider RGB(A) EGLConfigs
            continue;
        }

        if (!(getFBConfigAttrib(window, fbconfigs[i], EGL_RENDERABLE_TYPE) & EGL_WINDOW_BIT))
        {
            // Only consider window EGLConfigs
            continue;
        }

        result[*found].redBits = getFBConfigAttrib(window, fbconfigs[i], EGL_RED_SIZE);
        result[*found].greenBits = getFBConfigAttrib(window, fbconfigs[i], EGL_GREEN_SIZE);
        result[*found].blueBits = getFBConfigAttrib(window, fbconfigs[i], EGL_BLUE_SIZE);

        result[*found].alphaBits = getFBConfigAttrib(window, fbconfigs[i], EGL_ALPHA_SIZE);
        result[*found].depthBits = getFBConfigAttrib(window, fbconfigs[i], EGL_DEPTH_SIZE);
        result[*found].stencilBits = getFBConfigAttrib(window, fbconfigs[i], EGL_STENCIL_SIZE);

        result[*found].samples = getFBConfigAttrib(window, fbconfigs[i], EGL_SAMPLES);

        result[*found].platformID = (GLFWintptr) getFBConfigAttrib(window, fbconfigs[i], EGL_CONFIG_ID);

        (*found)++;
    }

    return result;
}

//========================================================================
// Read back framebuffer parameters from the context
//========================================================================

static void refreshContextParams(_GLFWwindow* window, EGLint fbconfigID)
{
    EGLint dummy;
    EGLConfig fbconfig[_GLFW_EGL_CONFIG_IN];

    int attribs[] = { EGL_CONFIG_ID, fbconfigID, None };

    eglChooseConfig(_glfwLibrary.EGL.display,
            attribs,
            fbconfig,
            _GLFW_EGL_CONFIG_IN,
            &dummy);
    if (!dummy)
    {
        // This should never ever happen
        // TODO: Flag this as an error and propagate up
        _glfwSetError(GLFW_PLATFORM_ERROR, "X11/EGL: Cannot find known "
                                           "EGLConfig by ID. This cannot "
                                           "happen. Have a nice day.\n");
        abort();
    }

    // There is no clear definition of an "accelerated" context on X11/EGL, and
    // true sounds better than false, so we hardcode true here
    window->accelerated = GL_TRUE;

    window->redBits = getFBConfigAttrib(window, *fbconfig, EGL_RED_SIZE);
    window->greenBits = getFBConfigAttrib(window, *fbconfig, EGL_GREEN_SIZE);
    window->blueBits = getFBConfigAttrib(window, *fbconfig, EGL_BLUE_SIZE);

    window->alphaBits = getFBConfigAttrib(window, *fbconfig, EGL_ALPHA_SIZE);
    window->depthBits = getFBConfigAttrib(window, *fbconfig, EGL_DEPTH_SIZE);
    window->stencilBits = getFBConfigAttrib(window, *fbconfig, EGL_STENCIL_SIZE);

    // Get FSAA buffer sample count
    window->samples = getFBConfigAttrib(window, *fbconfig, EGL_SAMPLES);
}


//========================================================================
// Create the actual OpenGL(|ES) context
//========================================================================

#define setEGLattrib(attribs, index, attribName, attribValue) \
    attribs[index++] = attribName; \
    attribs[index++] = attribValue;

static int createContext(_GLFWwindow* window,
                         const _GLFWwndconfig* wndconfig,
                         EGLint fbconfigID)
{
    int attribs[40];
    EGLint dummy, index, vid;
    EGLConfig fbconfig[_GLFW_EGL_CONFIG_IN];
    EGLContext share = NULL;
    XVisualInfo visTemplate;

    if (wndconfig->share)
        share = wndconfig->share->EGL.context;

    // Retrieve the previously selected EGLConfig
    {
        index = 0;

        setEGLattrib(attribs, index, EGL_CONFIG_ID, fbconfigID);
        setEGLattrib(attribs, index, None, None);

        eglChooseConfig(_glfwLibrary.EGL.display,
                attribs,
                fbconfig,
                _GLFW_EGL_CONFIG_IN,
                &dummy);

        if (!dummy)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "X11/EGL: Failed to retrieve the selected EGLConfig");
            return GL_FALSE;
        }
    }

    // Retrieve the corresponding visual
    if (!eglGetConfigAttrib(_glfwLibrary.EGL.display, *fbconfig, EGL_NATIVE_VISUAL_ID, &vid)) {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                "X11/EGL: Failed to retrieve visual for EGLConfig");
        return GL_FALSE;
    }

    // The X window visual must match the EGL config
    visTemplate.visualid = vid;
    window->EGL.visual = XGetVisualInfo(_glfwLibrary.X11.display, VisualIDMask, &visTemplate, &dummy);
    if (window->EGL.visual == NULL) {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "X11/GLX: Failed to retrieve visual for EGLConfig");
        return GL_FALSE;
    }

    if (wndconfig->glProfile == GLFW_OPENGL_ES2_PROFILE)
    {
        setEGLattrib(attribs, index, EGL_CONTEXT_CLIENT_VERSION, 2);
    }
    else
    {
        setEGLattrib(attribs, index, EGL_CONTEXT_CLIENT_VERSION, 1);
    }

    setEGLattrib(attribs, index, EGL_NONE, EGL_NONE);

    eglBindAPI(EGL_OPENGL_ES_API);

    window->EGL.context = eglCreateContext(_glfwLibrary.EGL.display, *fbconfig, share, attribs);
    if (window->EGL.context == EGL_NO_CONTEXT)
    {
        // TODO: Handle all the various error codes here

        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "X11/EGL: Failed to create OpenGL(|ES) context");
        return GL_FALSE;
    }

    refreshContextParams(window, fbconfigID);

    return GL_TRUE;
}

#undef setEGLattrib


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Initialize EGL
//========================================================================

int _glfwInitOpenGL(void)
{
#ifdef _GLFW_DLOPEN_LIBGL
    int i;
    char* libEGL_names[ ] =
    {
        "libEGL.so",
        "libEGL.so.1",
        "/usr/lib/libEGL.so",
        "/usr/lib/libEGL.so.1",
        NULL
    };

    for (i = 0;  libEGL_names[i] != NULL;  i++)
    {
        _glfwLibrary.EGL.libEGL = dlopen(libEGL_names[i], RTLD_LAZY | RTLD_GLOBAL);
        if (_glfwLibrary.EGL.libEGL)
            break;
    }

    if (!_glfwLibrary.EGL.libEGL)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR, "X11/EGL: Failed to find libEGL");
        return GL_FALSE;
    }
#endif

    _glfwLibrary.EGL.display = eglGetDisplay((EGLNativeDisplayType)_glfwLibrary.X11.display);
    if (_glfwLibrary.EGL.display == EGL_NO_DISPLAY)
    {
        _glfwSetError(GLFW_OPENGL_UNAVAILABLE,
                      "X11/EGL: Failed to get EGL display");
        return GL_FALSE;
    }

    if (!eglInitialize(_glfwLibrary.EGL.display,
                &_glfwLibrary.EGL.majorVersion,
                &_glfwLibrary.EGL.minorVersion))
    {
        _glfwSetError(GLFW_OPENGL_UNAVAILABLE,
                      "X11/EGL: Failed to initialize EGL");
        return GL_FALSE;
    }

    return GL_TRUE;
}


//========================================================================
// Terminate EGL
//========================================================================

void _glfwTerminateOpenGL(void)
{
    // Unload libEGL.so if necessary
#ifdef _GLFW_DLOPEN_LIBGL
    if (_glfwLibrary.EGL.libEGL != NULL)
    {
        dlclose(_glfwLibrary.EGL.libEGL);
        _glfwLibrary.EGL.libEGL = NULL;
    }
#endif
    eglTerminate(_glfwLibrary.EGL.display);
}


//========================================================================
// Prepare for creation of the OpenGL context
//========================================================================

int _glfwCreateContext(_GLFWwindow* window,
                       const _GLFWwndconfig* wndconfig,
                       const _GLFWfbconfig* fbconfig)
{
    _GLFWfbconfig closest;

    // Choose the best available fbconfig
    {
        unsigned int fbcount;
        _GLFWfbconfig* fbconfigs;
        const _GLFWfbconfig* result;

        fbconfigs = getFBConfigs(window, &fbcount);
        if (!fbconfigs)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "X11/EGL: No usable EGLFBConfigs found");
            return GL_FALSE;
        }

        result = _glfwChooseFBConfig(fbconfig, fbconfigs, fbcount);
        if (!result)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "X11/EGL: No EGLFBConfig matched the criteria");

            free(fbconfigs);
            return GL_FALSE;
        }

        closest = *result;
        free(fbconfigs);
    }

    return createContext(window, wndconfig, closest.platformID);
}


//========================================================================
// Destroy the OpenGL context
//========================================================================

void _glfwDestroyContext(_GLFWwindow* window)
{
    if (window->EGL.context)
    {
        // Release and destroy the context
        eglMakeCurrent(_glfwLibrary.EGL.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        window->EGL.context = NULL;
    }
}


//========================================================================
// Make the OpenGL context associated with the specified window current
//========================================================================

void _glfwPlatformMakeContextCurrent(_GLFWwindow* window)
{
    if (window)
    {
        eglMakeCurrent(_glfwLibrary.EGL.display,
                       window->EGL.surface,
                       window->EGL.surface,
                       window->EGL.context);
    }
    else
        eglMakeCurrent(_glfwLibrary.EGL.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}


//========================================================================
// Swap OpenGL buffers
//========================================================================

void _glfwPlatformSwapBuffers(void)
{
    eglSwapBuffers(_glfwLibrary.EGL.display,
                   _glfwLibrary.currentWindow->EGL.surface);
}


//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval(int interval)
{
    eglSwapInterval(_glfwLibrary.EGL.display, interval);
}


//========================================================================
// Check if an OpenGL extension is available at runtime
//========================================================================

int _glfwPlatformExtensionSupported(const char* extension)
{
    const char* extensions;

    // Get list of GLX extensions
    extensions = eglQueryString(_glfwLibrary.EGL.display,
                                EGL_EXTENSIONS);
    if (extensions != NULL)
    {
        if (_glfwStringInExtensionString(extension, (unsigned char*)extensions))
            return GL_TRUE;
    }

    return GL_FALSE;
}


//========================================================================
// Get the function pointer to an OpenGL function
//========================================================================

void* _glfwPlatformGetProcAddress(const char* procname)
{
    return (void*) _glfw_eglGetProcAddress(procname);
}


//========================================================================
// Copies the specified OpenGL state categories from src to dst
//========================================================================

void _glfwPlatformCopyContext(_GLFWwindow* src, _GLFWwindow* dst, unsigned long mask)
{
    // AFAIK, EGL doesn't have this
}

