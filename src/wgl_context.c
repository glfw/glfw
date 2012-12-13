//========================================================================
// GLFW - An OpenGL library
// Platform:    Win32/WGL
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
#include <malloc.h>


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
// Initialize WGL-specific extensions
// This function is called once before initial context creation, i.e. before
// any WGL extensions could be present.  This is done in order to have both
// extension variable clearing and loading in the same place, hopefully
// decreasing the possibility of forgetting to add one without the other.
//========================================================================

static void initWGLExtensions(_GLFWwindow* window)
{
    // This needs to include every function pointer loaded below
    window->WGL.SwapIntervalEXT = NULL;
    window->WGL.GetPixelFormatAttribivARB = NULL;
    window->WGL.GetExtensionsStringARB = NULL;
    window->WGL.GetExtensionsStringEXT = NULL;
    window->WGL.CreateContextAttribsARB = NULL;

    // This needs to include every extension used below except for
    // WGL_ARB_extensions_string and WGL_EXT_extensions_string
    window->WGL.ARB_multisample = GL_FALSE;
    window->WGL.ARB_framebuffer_sRGB = GL_FALSE;
    window->WGL.ARB_create_context = GL_FALSE;
    window->WGL.ARB_create_context_profile = GL_FALSE;
    window->WGL.EXT_create_context_es2_profile = GL_FALSE;
    window->WGL.ARB_create_context_robustness = GL_FALSE;
    window->WGL.EXT_swap_control = GL_FALSE;
    window->WGL.ARB_pixel_format = GL_FALSE;

    window->WGL.GetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)
        wglGetProcAddress("wglGetExtensionsStringEXT");
    if (!window->WGL.GetExtensionsStringEXT)
    {
        window->WGL.GetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)
            wglGetProcAddress("wglGetExtensionsStringARB");
        if (!window->WGL.GetExtensionsStringARB)
            return;
    }

    if (_glfwPlatformExtensionSupported("WGL_ARB_multisample"))
        window->WGL.ARB_multisample = GL_TRUE;

    if (_glfwPlatformExtensionSupported("WGL_ARB_framebuffer_sRGB"))
        window->WGL.ARB_framebuffer_sRGB = GL_TRUE;

    if (_glfwPlatformExtensionSupported("WGL_ARB_create_context"))
    {
        window->WGL.CreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
            wglGetProcAddress("wglCreateContextAttribsARB");

        if (window->WGL.CreateContextAttribsARB)
            window->WGL.ARB_create_context = GL_TRUE;
    }

    if (window->WGL.ARB_create_context)
    {
        if (_glfwPlatformExtensionSupported("WGL_ARB_create_context_profile"))
            window->WGL.ARB_create_context_profile = GL_TRUE;
    }

    if (window->WGL.ARB_create_context &&
        window->WGL.ARB_create_context_profile)
    {
        if (_glfwPlatformExtensionSupported("WGL_EXT_create_context_es2_profile"))
            window->WGL.EXT_create_context_es2_profile = GL_TRUE;
    }

    if (window->WGL.ARB_create_context)
    {
        if (_glfwPlatformExtensionSupported("WGL_ARB_create_context_robustness"))
            window->WGL.ARB_create_context_robustness = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("WGL_EXT_swap_control"))
    {
        window->WGL.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
            wglGetProcAddress("wglSwapIntervalEXT");

        if (window->WGL.SwapIntervalEXT)
            window->WGL.EXT_swap_control = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("WGL_ARB_pixel_format"))
    {
        window->WGL.GetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)
            wglGetProcAddress("wglGetPixelFormatAttribivARB");

        if (window->WGL.GetPixelFormatAttribivARB)
            window->WGL.ARB_pixel_format = GL_TRUE;
    }
}


//========================================================================
// Returns the specified attribute of the specified pixel format
// NOTE: Do not call this unless we have found WGL_ARB_pixel_format
//========================================================================

static int getPixelFormatAttrib(_GLFWwindow* window, int pixelFormat, int attrib)
{
    int value = 0;

    if (!window->WGL.GetPixelFormatAttribivARB(window->WGL.DC,
                                               pixelFormat,
                                               0, 1, &attrib, &value))
    {
        // NOTE: We should probably handle this error somehow
        return 0;
    }

    return value;
}


//========================================================================
// Return a list of available and usable framebuffer configs
//========================================================================

static _GLFWfbconfig* getFBConfigs(_GLFWwindow* window, unsigned int* found)
{
    _GLFWfbconfig* fbconfigs;
    PIXELFORMATDESCRIPTOR pfd;
    int i, available;

    *found = 0;

    if (window->WGL.ARB_pixel_format)
    {
        available = getPixelFormatAttrib(window,
                                         1,
                                         WGL_NUMBER_PIXEL_FORMATS_ARB);
    }
    else
    {
        available = DescribePixelFormat(window->WGL.DC,
                                        1,
                                        sizeof(PIXELFORMATDESCRIPTOR),
                                        NULL);
    }

    if (!available)
    {
        _glfwSetError(GLFW_API_UNAVAILABLE, "WGL: No pixel formats found");
        return NULL;
    }

    fbconfigs = (_GLFWfbconfig*) malloc(sizeof(_GLFWfbconfig) * available);
    if (!fbconfigs)
    {
        _glfwSetError(GLFW_OUT_OF_MEMORY, NULL);
        return NULL;
    }

    for (i = 1;  i <= available;  i++)
    {
        _GLFWfbconfig* f = fbconfigs + *found;

        if (window->WGL.ARB_pixel_format)
        {
            // Get pixel format attributes through WGL_ARB_pixel_format
            if (!getPixelFormatAttrib(window, i, WGL_SUPPORT_OPENGL_ARB) ||
                !getPixelFormatAttrib(window, i, WGL_DRAW_TO_WINDOW_ARB) ||
                !getPixelFormatAttrib(window, i, WGL_DOUBLE_BUFFER_ARB))
            {
                continue;
            }

            if (getPixelFormatAttrib(window, i, WGL_PIXEL_TYPE_ARB) !=
                WGL_TYPE_RGBA_ARB)
            {
                continue;
            }

            if (getPixelFormatAttrib(window, i, WGL_ACCELERATION_ARB) ==
                 WGL_NO_ACCELERATION_ARB)
            {
                continue;
            }

            f->redBits = getPixelFormatAttrib(window, i, WGL_RED_BITS_ARB);
            f->greenBits = getPixelFormatAttrib(window, i, WGL_GREEN_BITS_ARB);
            f->blueBits = getPixelFormatAttrib(window, i, WGL_BLUE_BITS_ARB);
            f->alphaBits = getPixelFormatAttrib(window, i, WGL_ALPHA_BITS_ARB);

            f->depthBits = getPixelFormatAttrib(window, i, WGL_DEPTH_BITS_ARB);
            f->stencilBits = getPixelFormatAttrib(window, i, WGL_STENCIL_BITS_ARB);

            f->accumRedBits = getPixelFormatAttrib(window, i, WGL_ACCUM_RED_BITS_ARB);
            f->accumGreenBits = getPixelFormatAttrib(window, i, WGL_ACCUM_GREEN_BITS_ARB);
            f->accumBlueBits = getPixelFormatAttrib(window, i, WGL_ACCUM_BLUE_BITS_ARB);
            f->accumAlphaBits = getPixelFormatAttrib(window, i, WGL_ACCUM_ALPHA_BITS_ARB);

            f->auxBuffers = getPixelFormatAttrib(window, i, WGL_AUX_BUFFERS_ARB);
            f->stereo = getPixelFormatAttrib(window, i, WGL_STEREO_ARB);

            if (window->WGL.ARB_multisample)
                f->samples = getPixelFormatAttrib(window, i, WGL_SAMPLES_ARB);
            else
                f->samples = 0;

            if (window->WGL.ARB_framebuffer_sRGB)
                f->sRGB = getPixelFormatAttrib(window, i, WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB);
            else
                f->sRGB = GL_FALSE;
        }
        else
        {
            // Get pixel format attributes through old-fashioned PFDs

            if (!DescribePixelFormat(window->WGL.DC,
                                     i,
                                     sizeof(PIXELFORMATDESCRIPTOR),
                                     &pfd))
            {
                continue;
            }

            if (!(pfd.dwFlags & PFD_DRAW_TO_WINDOW) ||
                !(pfd.dwFlags & PFD_SUPPORT_OPENGL) ||
                !(pfd.dwFlags & PFD_DOUBLEBUFFER))
            {
                continue;
            }

            if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) &&
                (pfd.dwFlags & PFD_GENERIC_FORMAT))
            {
                continue;
            }

            if (pfd.iPixelType != PFD_TYPE_RGBA)
                continue;

            f->redBits = pfd.cRedBits;
            f->greenBits = pfd.cGreenBits;
            f->blueBits = pfd.cBlueBits;
            f->alphaBits = pfd.cAlphaBits;

            f->depthBits = pfd.cDepthBits;
            f->stencilBits = pfd.cStencilBits;

            f->accumRedBits = pfd.cAccumRedBits;
            f->accumGreenBits = pfd.cAccumGreenBits;
            f->accumBlueBits = pfd.cAccumBlueBits;
            f->accumAlphaBits = pfd.cAccumAlphaBits;

            f->auxBuffers = pfd.cAuxBuffers;
            f->stereo = (pfd.dwFlags & PFD_STEREO) ? GL_TRUE : GL_FALSE;

            // PFD pixel formats do not support FSAA
            f->samples = 0;

            // PFD pixel formats do not support sRGB
            f->sRGB = GL_FALSE;
        }

        f->platformID = i;

        (*found)++;
    }

    if (*found == 0)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32/WGL: No usable pixel formats found");

        free(fbconfigs);
        return NULL;
    }

    return fbconfigs;
}


//========================================================================
// Creates an OpenGL context on the specified device context
//========================================================================

static GLboolean createContext(_GLFWwindow* window,
                               const _GLFWwndconfig* wndconfig,
                               int pixelFormat)
{
    PIXELFORMATDESCRIPTOR pfd;
    int i = 0, attribs[40];
    HGLRC share = NULL;

    if (wndconfig->share)
        share = wndconfig->share->WGL.context;

    if (!DescribePixelFormat(window->WGL.DC, pixelFormat, sizeof(pfd), &pfd))
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32: Failed to retrieve PFD for selected pixel format");
        return GL_FALSE;
    }

    if (!SetPixelFormat(window->WGL.DC, pixelFormat, &pfd))
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32: Failed to set selected pixel format");
        return GL_FALSE;
    }

    if (window->WGL.ARB_create_context)
    {
        // Use the newer wglCreateContextAttribsARB creation method

        if (wndconfig->glMajor != 1 || wndconfig->glMinor != 0)
        {
            // Request an explicitly versioned context

            attribs[i++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
            attribs[i++] = wndconfig->glMajor;
            attribs[i++] = WGL_CONTEXT_MINOR_VERSION_ARB;
            attribs[i++] = wndconfig->glMinor;
        }

        if (wndconfig->clientAPI == GLFW_OPENGL_ES_API)
        {
            if (!window->WGL.ARB_create_context_profile ||
                !window->WGL.EXT_create_context_es2_profile)
            {
                _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                            "Win32/WGL: OpenGL ES 2.x requested but "
                            "WGL_EXT_create_context_es2_profile is unavailable");
                return GL_FALSE;
            }

            attribs[i++] = WGL_CONTEXT_PROFILE_MASK_ARB;
            attribs[i++] = WGL_CONTEXT_ES2_PROFILE_BIT_EXT;
        }

        if (wndconfig->glForward || wndconfig->glDebug || wndconfig->glRobustness)
        {
            int flags = 0;

            if (wndconfig->glForward)
                flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

            if (wndconfig->glDebug)
                flags |= WGL_CONTEXT_DEBUG_BIT_ARB;

            if (wndconfig->glRobustness)
                flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;

            attribs[i++] = WGL_CONTEXT_FLAGS_ARB;
            attribs[i++] = flags;
        }

        if (wndconfig->glProfile)
        {
            int flags = 0;

            if (!window->WGL.ARB_create_context_profile)
            {
                _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                              "WGL: OpenGL profile requested but "
                              "WGL_ARB_create_context_profile is unavailable");
                return GL_FALSE;
            }

            if (wndconfig->glProfile == GLFW_OPENGL_CORE_PROFILE)
                flags = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
            else if (wndconfig->glProfile == GLFW_OPENGL_COMPAT_PROFILE)
                flags = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;

            attribs[i++] = WGL_CONTEXT_PROFILE_MASK_ARB;
            attribs[i++] = flags;
        }

        if (wndconfig->glRobustness)
        {
            int strategy = 0;

            if (!window->WGL.ARB_create_context_robustness)
            {
                _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                              "WGL: An OpenGL robustness strategy was "
                              "requested but WGL_ARB_create_context_robustness "
                              "is unavailable");
                return GL_FALSE;
            }

            if (wndconfig->glRobustness == GLFW_NO_RESET_NOTIFICATION)
                strategy = WGL_NO_RESET_NOTIFICATION_ARB;
            else if (wndconfig->glRobustness == GLFW_LOSE_CONTEXT_ON_RESET)
                strategy = WGL_LOSE_CONTEXT_ON_RESET_ARB;

            attribs[i++] = WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB;
            attribs[i++] = strategy;
        }

        attribs[i++] = 0;

        window->WGL.context = window->WGL.CreateContextAttribsARB(window->WGL.DC,
                                                                  share,
                                                                  attribs);
        if (!window->WGL.context)
        {
            _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                          "WGL: Failed to create OpenGL context");
            return GL_FALSE;
        }
    }
    else
    {
        window->WGL.context = wglCreateContext(window->WGL.DC);
        if (!window->WGL.context)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "WGL: Failed to create OpenGL context");
            return GL_FALSE;
        }

        if (share)
        {
            if (!wglShareLists(share, window->WGL.context))
            {
                _glfwSetError(GLFW_PLATFORM_ERROR,
                              "WGL: Failed to enable sharing with specified "
                              "OpenGL context");
                return GL_FALSE;
            }
        }
    }

    _glfwPlatformMakeContextCurrent(window);
    initWGLExtensions(window);

    return GL_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Prepare for creation of the OpenGL context
//========================================================================

int _glfwCreateContext(_GLFWwindow* window,
                       const _GLFWwndconfig* wndconfig,
                       const _GLFWfbconfig* fbconfig)
{
    _GLFWfbconfig closest;

    window->WGL.DC = GetDC(window->Win32.handle);
    if (!window->WGL.DC)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32: Failed to retrieve DC for window");
        return GL_FALSE;
    }

    // Choose the best available fbconfig
    {
        unsigned int fbcount;
        _GLFWfbconfig* fbconfigs;
        const _GLFWfbconfig* result;

        fbconfigs = getFBConfigs(window, &fbcount);
        if (!fbconfigs)
            return GL_FALSE;

        result = _glfwChooseFBConfig(fbconfig, fbconfigs, fbcount);
        if (!result)
        {
            _glfwSetError(GLFW_FORMAT_UNAVAILABLE,
                          "Win32/WGL: No pixel format matched the criteria");

            free(fbconfigs);
            return GL_FALSE;
        }

        closest = *result;
        free(fbconfigs);
    }

    return createContext(window, wndconfig, (int) closest.platformID);
}


//========================================================================
// Destroy the OpenGL context
//========================================================================

void _glfwDestroyContext(_GLFWwindow* window)
{
    if (window->WGL.context)
    {
        wglDeleteContext(window->WGL.context);
        window->WGL.context = NULL;
    }

    if (window->WGL.DC)
    {
        ReleaseDC(window->Win32.handle, window->WGL.DC);
        window->WGL.DC = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Make the OpenGL context associated with the specified window current
//========================================================================

void _glfwPlatformMakeContextCurrent(_GLFWwindow* window)
{
    if (window)
        wglMakeCurrent(window->WGL.DC, window->WGL.context);
    else
        wglMakeCurrent(NULL, NULL);

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
// Swap buffers (double-buffering)
//========================================================================

void _glfwPlatformSwapBuffers(_GLFWwindow* window)
{
    SwapBuffers(window->WGL.DC);
}


//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval(int interval)
{
    _GLFWwindow* window = _glfwCurrentWindow;

    if (window->WGL.EXT_swap_control)
        window->WGL.SwapIntervalEXT(interval);
}


//========================================================================
// Check if the current context supports the specified WGL extension
//========================================================================

int _glfwPlatformExtensionSupported(const char* extension)
{
    const GLubyte* extensions;

    _GLFWwindow* window = _glfwCurrentWindow;

    if (window->WGL.GetExtensionsStringEXT != NULL)
    {
        extensions = (GLubyte*) window->WGL.GetExtensionsStringEXT();
        if (extensions != NULL)
        {
            if (_glfwStringInExtensionString(extension, extensions))
                return GL_TRUE;
        }
    }

    if (window->WGL.GetExtensionsStringARB != NULL)
    {
        extensions = (GLubyte*) window->WGL.GetExtensionsStringARB(window->WGL.DC);
        if (extensions != NULL)
        {
            if (_glfwStringInExtensionString(extension, extensions))
                return GL_TRUE;
        }
    }

    return GL_FALSE;
}


//========================================================================
// Get the function pointer to an OpenGL function
//========================================================================

GLFWglproc _glfwPlatformGetProcAddress(const char* procname)
{
    return (GLFWglproc) wglGetProcAddress(procname);
}

