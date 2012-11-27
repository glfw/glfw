//========================================================================
// GLFW - An OpenGL library
// Platform:    EGL
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


//========================================================================
// Thread local storage attribute macro
//========================================================================
#if defined(_MSC_VER)
 #define _GLFW_TLS __declspec(thread)
#elif defined(__GNUC__)
 #define _GLFW_TLS __thread
#else
 #define _GLFW_TLS
#endif


//========================================================================
// The per-thread current context/window pointer
//========================================================================
static _GLFW_TLS _GLFWwindow* _glfwCurrentWindow = NULL;


//========================================================================
// Returns the specified attribute of the specified EGLConfig
//========================================================================

static int getConfigAttrib(EGLConfig config, int attrib)
{
    int value;
    eglGetConfigAttrib(_glfwLibrary.EGL.display, config, attrib, &value);
    return value;
}


//========================================================================
// Return a list of available and usable framebuffer configs
//========================================================================

static _GLFWfbconfig* getFBConfigs(_GLFWwindow* window,
                                   const _GLFWwndconfig* wndconfig,
                                   unsigned int* found)
{
    EGLConfig* configs;
    _GLFWfbconfig* result;
    int i, count = 0;

    *found = 0;

    eglGetConfigs(_glfwLibrary.EGL.display, NULL, 0, &count);

    configs = (EGLConfig*) malloc(sizeof(EGLConfig) * count);
    if (!configs)
    {
        _glfwSetError(GLFW_OUT_OF_MEMORY, NULL);
        return NULL;
    }

    eglGetConfigs(_glfwLibrary.EGL.display, configs, count, &count);
    if (!count)
    {
        free(configs);

        _glfwSetError(GLFW_API_UNAVAILABLE,
                      "EGL: No EGLConfigs returned");
        return NULL;
    }

    result = (_GLFWfbconfig*) malloc(sizeof(_GLFWfbconfig) * count);
    if (!result)
    {
        free(configs);

        _glfwSetError(GLFW_OUT_OF_MEMORY, NULL);
        return NULL;
    }

    for (i = 0;  i < count;  i++)
    {
        _GLFWfbconfig* f = result + *found;

        if (!getConfigAttrib(configs[i], EGL_NATIVE_VISUAL_ID))
        {
            // Only consider EGLConfigs with associated visuals
            continue;
        }

        if (!(getConfigAttrib(configs[i], EGL_COLOR_BUFFER_TYPE) & EGL_RGB_BUFFER))
        {
            // Only consider RGB(A) EGLConfigs
            continue;
        }

        if (!(getConfigAttrib(configs[i], EGL_SURFACE_TYPE) & EGL_WINDOW_BIT))
        {
            // Only consider window EGLConfigs
            continue;
        }

        if (wndconfig->clientAPI == GLFW_OPENGL_ES_API)
        {
            if (wndconfig->glMajor == 1)
            {
                if (!(getConfigAttrib(configs[i], EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES_BIT))
                    continue;
            }
            else
            {
                if (!(getConfigAttrib(configs[i], EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES2_BIT))
                    continue;
            }
        }
        else if (wndconfig->clientAPI == GLFW_OPENGL_API)
        {
            if (!(getConfigAttrib(configs[i], EGL_RENDERABLE_TYPE) & EGL_OPENGL_BIT))
                continue;
        }

        f->redBits = getConfigAttrib(configs[i], EGL_RED_SIZE);
        f->greenBits = getConfigAttrib(configs[i], EGL_GREEN_SIZE);
        f->blueBits = getConfigAttrib(configs[i], EGL_BLUE_SIZE);

        f->alphaBits = getConfigAttrib(configs[i], EGL_ALPHA_SIZE);
        f->depthBits = getConfigAttrib(configs[i], EGL_DEPTH_SIZE);
        f->stencilBits = getConfigAttrib(configs[i], EGL_STENCIL_SIZE);

        f->samples = getConfigAttrib(configs[i], EGL_SAMPLES);

        f->platformID = (GLFWintptr) getConfigAttrib(configs[i], EGL_CONFIG_ID);

        (*found)++;
    }

    free(configs);
    return result;
}


//========================================================================
// Create the actual OpenGL(|ES) context
//========================================================================

#define setEGLattrib(attribs, index, attribName, attribValue) \
{ \
    attribs[index++] = attribName; \
    attribs[index++] = attribValue; \
}

static int createContext(_GLFWwindow* window,
                         const _GLFWwndconfig* wndconfig,
                         EGLint fbconfigID)
{
    int attribs[40];
    EGLint count, index;
    EGLConfig config;
    EGLContext share = NULL;

    if (wndconfig->share)
        share = wndconfig->share->EGL.context;

    // Retrieve the previously selected EGLConfig
    {
        index = 0;

        setEGLattrib(attribs, index, EGL_CONFIG_ID, fbconfigID);
        setEGLattrib(attribs, index, EGL_NONE, EGL_NONE);

        eglChooseConfig(_glfwLibrary.EGL.display, attribs, &config, 1, &count);
        if (!count)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "EGL: Failed to retrieve the selected EGLConfig");
            return GL_FALSE;
        }
    }

    // Retrieve the corresponding visual
    // NOTE: This is the only non-portable code in this file.
    // Maybe it would not hurt too much to add #ifdefs for different platforms?
#if defined(_GLFW_X11)
    {
        int mask;
        EGLint redBits, greenBits, blueBits, alphaBits, visualID = 0;
        XVisualInfo info;

        eglGetConfigAttrib(_glfwLibrary.EGL.display, config,
                           EGL_NATIVE_VISUAL_ID, &visualID);

        info.screen = _glfwLibrary.X11.screen;
        mask = VisualScreenMask;

        if (visualID)
        {
            // The X window visual must match the EGL config
            info.visualid = visualID;
            mask |= VisualIDMask;
        }
        else
        {
            // some EGL drivers don't implement the EGL_NATIVE_VISUAL_ID
            // attribute, so attempt to find the closest match.

            eglGetConfigAttrib(_glfwLibrary.EGL.display, config,
                               EGL_RED_SIZE, &redBits);
            eglGetConfigAttrib(_glfwLibrary.EGL.display, config,
                               EGL_GREEN_SIZE, &greenBits);
            eglGetConfigAttrib(_glfwLibrary.EGL.display, config,
                               EGL_BLUE_SIZE, &blueBits);
            eglGetConfigAttrib(_glfwLibrary.EGL.display, config,
                               EGL_ALPHA_SIZE, &alphaBits);

            info.depth = redBits + greenBits + blueBits + alphaBits;
            mask |= VisualDepthMask;
        }

        window->EGL.visual = XGetVisualInfo(_glfwLibrary.X11.display,
                                            mask, &info, &count);

        if (window->EGL.visual == NULL)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "EGL: Failed to retrieve visual for EGLConfig");
            return GL_FALSE;
        }
    }
#endif

    if (wndconfig->clientAPI == GLFW_OPENGL_ES_API)
    {
        if (!eglBindAPI(EGL_OPENGL_ES_API))
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "EGL: OpenGL ES is not supported");
            return GL_FALSE;
        }
    }
    else
    {
        if (!eglBindAPI(EGL_OPENGL_API))
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "EGL: OpenGL is not supported");
            return GL_FALSE;
        }
    }

    index = 0;

    if (_glfwLibrary.EGL.KHR_create_context)
    {
        setEGLattrib(attribs, index, EGL_CONTEXT_MAJOR_VERSION_KHR, wndconfig->glMajor);
        setEGLattrib(attribs, index, EGL_CONTEXT_MINOR_VERSION_KHR, wndconfig->glMinor);

        if (wndconfig->glForward || wndconfig->glDebug || wndconfig->glRobustness)
        {
            int flags = 0;

            if (wndconfig->glForward)
                flags |= EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR;

            if (wndconfig->glDebug)
                flags |= EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;

            if (wndconfig->glRobustness)
                flags |= EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR;

            setEGLattrib(attribs, index, EGL_CONTEXT_FLAGS_KHR, flags);
        }

        if (wndconfig->glProfile)
        {
            int flags = 0;

            if (wndconfig->glProfile == GLFW_OPENGL_CORE_PROFILE)
                flags = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;
            else if (wndconfig->glProfile == GLFW_OPENGL_COMPAT_PROFILE)
                flags = EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR;

            setEGLattrib(attribs, index, EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, flags);
        }

        if (wndconfig->glRobustness)
        {
            int strategy;

            if (wndconfig->glRobustness == GLFW_OPENGL_NO_RESET_NOTIFICATION)
                strategy = EGL_NO_RESET_NOTIFICATION_KHR;
            else if (wndconfig->glRobustness == GLFW_OPENGL_LOSE_CONTEXT_ON_RESET)
                strategy = EGL_LOSE_CONTEXT_ON_RESET_KHR;

            setEGLattrib(attribs, index, EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR, strategy);
        }
    }
    else
    {
        if (wndconfig->clientAPI == GLFW_OPENGL_ES_API)
            setEGLattrib(attribs, index, EGL_CONTEXT_CLIENT_VERSION, wndconfig->glMajor);
    }

    setEGLattrib(attribs, index, EGL_NONE, EGL_NONE);

    window->EGL.context = eglCreateContext(_glfwLibrary.EGL.display,
                                           config, share, attribs);

    if (window->EGL.context == EGL_NO_CONTEXT)
    {
        // TODO: Handle all the various error codes here

        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "EGL: Failed to create context");
        return GL_FALSE;
    }

    window->EGL.config = config;

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
#ifdef _GLFW_DLOPEN_LIBEGL
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
        _glfwSetError(GLFW_PLATFORM_ERROR, "EGL: Failed to find libEGL");
        return GL_FALSE;
    }
#endif

    _glfwLibrary.EGL.display = eglGetDisplay(_GLFW_EGL_NATIVE_DISPLAY);
    if (_glfwLibrary.EGL.display == EGL_NO_DISPLAY)
    {
        _glfwSetError(GLFW_API_UNAVAILABLE,
                      "EGL: Failed to get EGL display");
        return GL_FALSE;
    }

    if (!eglInitialize(_glfwLibrary.EGL.display,
                       &_glfwLibrary.EGL.majorVersion,
                       &_glfwLibrary.EGL.minorVersion))
    {
        _glfwSetError(GLFW_API_UNAVAILABLE,
                      "EGL: Failed to initialize EGL");
        return GL_FALSE;
    }

    if (_glfwPlatformExtensionSupported("EGL_KHR_create_context"))
        _glfwLibrary.EGL.KHR_create_context = GL_TRUE;

    return GL_TRUE;
}


//========================================================================
// Terminate EGL
//========================================================================

void _glfwTerminateOpenGL(void)
{
#ifdef _GLFW_DLOPEN_LIBEGL
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

        fbconfigs = getFBConfigs(window, wndconfig, &fbcount);
        if (!fbconfigs)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "EGL: No usable EGLFBConfigs found");
            return GL_FALSE;
        }

        result = _glfwChooseFBConfig(fbconfig, fbconfigs, fbcount);
        if (!result)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "EGL: No EGLFBConfig matched the criteria");

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
    if (window->EGL.visual)
    {
       XFree(window->EGL.visual);
       window->EGL.visual = NULL;
    }

    if (window->EGL.surface)
    {
        eglDestroySurface(_glfwLibrary.EGL.display, window->EGL.surface);
        window->EGL.surface = EGL_NO_SURFACE;
    }

    if (window->EGL.context)
    {
        eglDestroyContext(_glfwLibrary.EGL.display, window->EGL.context);
        window->EGL.context = EGL_NO_CONTEXT;
    }
}


//========================================================================
// Make the OpenGL context associated with the specified window current
//========================================================================

void _glfwPlatformMakeContextCurrent(_GLFWwindow* window)
{
    if (window)
    {
        if (window->EGL.surface == EGL_NO_SURFACE)
        {
            window->EGL.surface = eglCreateWindowSurface(_glfwLibrary.EGL.display,
                                                         window->EGL.config,
                                                         _GLFW_EGL_NATIVE_WINDOW,
                                                         NULL);
            if (window->EGL.surface == EGL_NO_SURFACE)
            {
                _glfwSetError(GLFW_PLATFORM_ERROR,
                              "EGL: Failed to create window surface");
            }
        }

        eglMakeCurrent(_glfwLibrary.EGL.display,
                       window->EGL.surface,
                       window->EGL.surface,
                       window->EGL.context);
    }
    else
    {
        eglMakeCurrent(_glfwLibrary.EGL.display,
                       EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }

    _glfwCurrentWindow = window;
}


//========================================================================
// Return the window object whose context is current
//========================================================================

_GLFWwindow* _glfwPlatformGetCurrentContext(void)
{
    return _glfwCurrentWindow;
}


//========================================================================
// Swap OpenGL buffers
//========================================================================

void _glfwPlatformSwapBuffers(_GLFWwindow* window)
{
    eglSwapBuffers(_glfwLibrary.EGL.display, window->EGL.surface);
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

    extensions = eglQueryString(_glfwLibrary.EGL.display, EGL_EXTENSIONS);
    if (extensions != NULL)
    {
        if (_glfwStringInExtensionString(extension, (unsigned char*) extensions))
            return GL_TRUE;
    }

    return GL_FALSE;
}


//========================================================================
// Get the function pointer to an OpenGL function
//========================================================================

GLFWglproc _glfwPlatformGetProcAddress(const char* procname)
{
    return _glfw_eglGetProcAddress(procname);
}

