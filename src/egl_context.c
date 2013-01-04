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
    eglGetConfigAttrib(_glfw.egl.display, config, attrib, &value);
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

    eglGetConfigs(_glfw.egl.display, NULL, 0, &count);

    configs = (EGLConfig*) malloc(sizeof(EGLConfig) * count);
    if (!configs)
    {
        _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
        return NULL;
    }

    eglGetConfigs(_glfw.egl.display, configs, count, &count);
    if (!count)
    {
        free(configs);

        _glfwInputError(GLFW_API_UNAVAILABLE, "EGL: No EGLConfigs returned");
        return NULL;
    }

    result = (_GLFWfbconfig*) malloc(sizeof(_GLFWfbconfig) * count);
    if (!result)
    {
        free(configs);

        _glfwInputError(GLFW_OUT_OF_MEMORY, NULL);
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

        // NOTE: There does not appear to be any way to request sRGB
        // framebuffers for OpenGL or GLES contexts; only for OpenVG ones
        f->sRGB = GL_FALSE;

        f->platformID = (GLFWintptr) getConfigAttrib(configs[i], EGL_CONFIG_ID);

        (*found)++;
    }

    free(configs);
    return result;
}


//========================================================================
// Create the actual OpenGL(|ES) context
//========================================================================

#define setEGLattrib(attribName, attribValue) \
{ \
    attribs[index++] = attribName; \
    attribs[index++] = attribValue; \
}

static int createContext(_GLFWwindow* window,
                         const _GLFWwndconfig* wndconfig,
                         EGLint fbconfigID)
{
    int attribs[40];
    EGLint count;
    EGLConfig config;
    EGLContext share = NULL;

    if (wndconfig->share)
        share = wndconfig->share->egl.context;

    // Retrieve the previously selected EGLConfig
    {
        int index = 0;

        setEGLattrib(EGL_CONFIG_ID, fbconfigID);
        setEGLattrib(EGL_NONE, EGL_NONE);

        eglChooseConfig(_glfw.egl.display, attribs, &config, 1, &count);
        if (!count)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "EGL: Failed to retrieve the selected EGLConfig");
            return GL_FALSE;
        }
    }

#if defined(_GLFW_X11)
    // Retrieve the visual corresponding to the chosen EGL config
    {
        int mask;
        EGLint redBits, greenBits, blueBits, alphaBits, visualID = 0;
        XVisualInfo info;

        eglGetConfigAttrib(_glfw.egl.display, config,
                           EGL_NATIVE_VISUAL_ID, &visualID);

        info.screen = _glfw.x11.screen;
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

            eglGetConfigAttrib(_glfw.egl.display, config,
                               EGL_RED_SIZE, &redBits);
            eglGetConfigAttrib(_glfw.egl.display, config,
                               EGL_GREEN_SIZE, &greenBits);
            eglGetConfigAttrib(_glfw.egl.display, config,
                               EGL_BLUE_SIZE, &blueBits);
            eglGetConfigAttrib(_glfw.egl.display, config,
                               EGL_ALPHA_SIZE, &alphaBits);

            info.depth = redBits + greenBits + blueBits + alphaBits;
            mask |= VisualDepthMask;
        }

        window->egl.visual = XGetVisualInfo(_glfw.x11.display,
                                            mask, &info, &count);

        if (window->egl.visual == NULL)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "EGL: Failed to retrieve visual for EGLConfig");
            return GL_FALSE;
        }
    }
#endif

    if (wndconfig->clientAPI == GLFW_OPENGL_ES_API)
    {
        if (!eglBindAPI(EGL_OPENGL_ES_API))
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "EGL: OpenGL ES is not supported");
            return GL_FALSE;
        }
    }
    else
    {
        if (!eglBindAPI(EGL_OPENGL_API))
        {
            _glfwInputError(GLFW_PLATFORM_ERROR, "EGL: OpenGL is not supported");
            return GL_FALSE;
        }
    }

    if (_glfw.egl.KHR_create_context)
    {
        int index = 0, mask = 0, flags = 0, strategy = 0;

        if (wndconfig->clientAPI == GLFW_OPENGL_API)
        {
            if (wndconfig->glProfile == GLFW_OPENGL_CORE_PROFILE)
                mask |= EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;
            else if (wndconfig->glProfile == GLFW_OPENGL_COMPAT_PROFILE)
                mask |= EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR;

            if (wndconfig->glForward)
                flags |= EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR;

            if (wndconfig->glDebug)
                flags |= EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
        }

        if (wndconfig->glRobustness != GLFW_NO_ROBUSTNESS)
        {
            if (wndconfig->glRobustness == GLFW_NO_RESET_NOTIFICATION)
                strategy = EGL_NO_RESET_NOTIFICATION_KHR;
            else if (wndconfig->glRobustness == GLFW_LOSE_CONTEXT_ON_RESET)
                strategy = EGL_LOSE_CONTEXT_ON_RESET_KHR;

            flags |= EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR;
        }

        if (wndconfig->glMajor != 1 || wndconfig->glMinor != 0)
        {
            setEGLattrib(EGL_CONTEXT_MAJOR_VERSION_KHR, wndconfig->glMajor);
            setEGLattrib(EGL_CONTEXT_MINOR_VERSION_KHR, wndconfig->glMinor);
        }

        if (mask)
            setEGLattrib(EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, mask);

        if (flags)
            setEGLattrib(EGL_CONTEXT_FLAGS_KHR, flags);

        if (strategy)
            setEGLattrib(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR, strategy);

        setEGLattrib(EGL_NONE, EGL_NONE);
    }
    else
    {
        int index = 0;

        if (wndconfig->clientAPI == GLFW_OPENGL_ES_API)
            setEGLattrib(EGL_CONTEXT_CLIENT_VERSION, wndconfig->glMajor);

        setEGLattrib(EGL_NONE, EGL_NONE);
    }

    window->egl.context = eglCreateContext(_glfw.egl.display,
                                           config, share, attribs);

    if (window->egl.context == EGL_NO_CONTEXT)
    {
        // TODO: Handle all the various error codes here

        _glfwInputError(GLFW_PLATFORM_ERROR, "EGL: Failed to create context");
        return GL_FALSE;
    }

    window->egl.config = config;

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
    _glfw.egl.display = eglGetDisplay(_GLFW_EGL_NATIVE_DISPLAY);
    if (_glfw.egl.display == EGL_NO_DISPLAY)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE, "EGL: Failed to get EGL display");
        return GL_FALSE;
    }

    if (!eglInitialize(_glfw.egl.display,
                       &_glfw.egl.versionMajor,
                       &_glfw.egl.versionMinor))
    {
        _glfwInputError(GLFW_API_UNAVAILABLE, "EGL: Failed to initialize EGL");
        return GL_FALSE;
    }

    if (_glfwPlatformExtensionSupported("EGL_KHR_create_context"))
        _glfw.egl.KHR_create_context = GL_TRUE;

    return GL_TRUE;
}


//========================================================================
// Terminate EGL
//========================================================================

void _glfwTerminateOpenGL(void)
{
    eglTerminate(_glfw.egl.display);
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
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "EGL: No usable EGLFBConfigs found");
            return GL_FALSE;
        }

        result = _glfwChooseFBConfig(fbconfig, fbconfigs, fbcount);
        if (!result)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
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
    if (window->egl.visual)
    {
       XFree(window->egl.visual);
       window->egl.visual = NULL;
    }

    if (window->egl.surface)
    {
        eglDestroySurface(_glfw.egl.display, window->egl.surface);
        window->egl.surface = EGL_NO_SURFACE;
    }

    if (window->egl.context)
    {
        eglDestroyContext(_glfw.egl.display, window->egl.context);
        window->egl.context = EGL_NO_CONTEXT;
    }
}


//========================================================================
// Analyzes the specified context for possible recreation
//========================================================================

int _glfwAnalyzeContext(const _GLFWwindow* window,
                        const _GLFWwndconfig* wndconfig,
                        const _GLFWfbconfig* fbconfig)
{
#if _GLFW_WIN32
    return _GLFW_RECREATION_NOT_NEEDED;
#else
    return 0;
#endif
}


//========================================================================
// Make the OpenGL context associated with the specified window current
//========================================================================

void _glfwPlatformMakeContextCurrent(_GLFWwindow* window)
{
    if (window)
    {
        if (window->egl.surface == EGL_NO_SURFACE)
        {
            window->egl.surface = eglCreateWindowSurface(_glfw.egl.display,
                                                         window->egl.config,
                                                         _GLFW_EGL_NATIVE_WINDOW,
                                                         NULL);
            if (window->egl.surface == EGL_NO_SURFACE)
            {
                _glfwInputError(GLFW_PLATFORM_ERROR,
                                "EGL: Failed to create window surface");
            }
        }

        eglMakeCurrent(_glfw.egl.display,
                       window->egl.surface,
                       window->egl.surface,
                       window->egl.context);
    }
    else
    {
        eglMakeCurrent(_glfw.egl.display,
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
    eglSwapBuffers(_glfw.egl.display, window->egl.surface);
}


//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval(int interval)
{
    eglSwapInterval(_glfw.egl.display, interval);
}


//========================================================================
// Check if an OpenGL extension is available at runtime
//========================================================================

int _glfwPlatformExtensionSupported(const char* extension)
{
    const char* extensions;

    extensions = eglQueryString(_glfw.egl.display, EGL_EXTENSIONS);
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
    return eglGetProcAddress(procname);
}

