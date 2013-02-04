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
#include <assert.h>


// Thread local storage attribute macro
//
#if defined(_MSC_VER)
 #define _GLFW_TLS __declspec(thread)
#elif defined(__GNUC__)
 #define _GLFW_TLS __thread
#else
 #define _GLFW_TLS
#endif


// The per-thread current context/window pointer
//
static _GLFW_TLS _GLFWwindow* _glfwCurrentWindow = NULL;


// Initialize WGL-specific extensions
// This function is called once before initial context creation, i.e. before
// any WGL extensions could be present.  This is done in order to have both
// extension variable clearing and loading in the same place, hopefully
// decreasing the possibility of forgetting to add one without the other.
//
static void initWGLExtensions(_GLFWwindow* window)
{
    // This needs to include every function pointer loaded below
    window->wgl.SwapIntervalEXT = NULL;
    window->wgl.ChoosePixelFormatARB = NULL;
    window->wgl.GetExtensionsStringARB = NULL;
    window->wgl.GetExtensionsStringEXT = NULL;
    window->wgl.CreateContextAttribsARB = NULL;

    // This needs to include every extension used below except for
    // WGL_ARB_extensions_string and WGL_EXT_extensions_string
    window->wgl.ARB_multisample = GL_FALSE;
    window->wgl.ARB_framebuffer_sRGB = GL_FALSE;
    window->wgl.ARB_create_context = GL_FALSE;
    window->wgl.ARB_create_context_profile = GL_FALSE;
    window->wgl.EXT_create_context_es2_profile = GL_FALSE;
    window->wgl.ARB_create_context_robustness = GL_FALSE;
    window->wgl.EXT_swap_control = GL_FALSE;
    window->wgl.ARB_pixel_format = GL_FALSE;

    window->wgl.GetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)
        wglGetProcAddress("wglGetExtensionsStringEXT");
    if (!window->wgl.GetExtensionsStringEXT)
    {
        window->wgl.GetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)
            wglGetProcAddress("wglGetExtensionsStringARB");
        if (!window->wgl.GetExtensionsStringARB)
            return;
    }

    if (_glfwPlatformExtensionSupported("WGL_ARB_multisample"))
        window->wgl.ARB_multisample = GL_TRUE;

    if (_glfwPlatformExtensionSupported("WGL_ARB_framebuffer_sRGB"))
        window->wgl.ARB_framebuffer_sRGB = GL_TRUE;

    if (_glfwPlatformExtensionSupported("WGL_ARB_create_context"))
    {
        window->wgl.CreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
            wglGetProcAddress("wglCreateContextAttribsARB");

        if (window->wgl.CreateContextAttribsARB)
            window->wgl.ARB_create_context = GL_TRUE;
    }

    if (window->wgl.ARB_create_context)
    {
        if (_glfwPlatformExtensionSupported("WGL_ARB_create_context_profile"))
            window->wgl.ARB_create_context_profile = GL_TRUE;
    }

    if (window->wgl.ARB_create_context &&
        window->wgl.ARB_create_context_profile)
    {
        if (_glfwPlatformExtensionSupported("WGL_EXT_create_context_es2_profile"))
            window->wgl.EXT_create_context_es2_profile = GL_TRUE;
    }

    if (window->wgl.ARB_create_context)
    {
        if (_glfwPlatformExtensionSupported("WGL_ARB_create_context_robustness"))
            window->wgl.ARB_create_context_robustness = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("WGL_EXT_swap_control"))
    {
        window->wgl.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
            wglGetProcAddress("wglSwapIntervalEXT");

        if (window->wgl.SwapIntervalEXT)
            window->wgl.EXT_swap_control = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("WGL_ARB_pixel_format"))
    {
        window->wgl.ChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)
            wglGetProcAddress("wglChoosePixelFormatARB");

        if (window->wgl.ChoosePixelFormatARB)
            window->wgl.ARB_pixel_format = GL_TRUE;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize WGL
//
int _glfwInitContextAPI(void)
{
    return GL_TRUE;
}

// Terminate WGL
//
void _glfwTerminateContextAPI(void)
{
}

#define setWGLattrib(attribName, attribValue) \
{ \
    attribs[index++] = attribName; \
    attribs[index++] = attribValue; \
    assert(index < sizeof(attribs) / sizeof(attribs[0])); \
}

// Prepare for creation of the OpenGL context
//
int _glfwCreateContext(_GLFWwindow* window,
                       const _GLFWwndconfig* wndconfig,
                       const _GLFWfbconfig* fbconfig)
{
    int attribs[40];
    int pixelFormat;
    PIXELFORMATDESCRIPTOR pfd;
    HGLRC share = NULL;

    if (wndconfig->share)
        share = wndconfig->share->wgl.context;

    window->wgl.dc = GetDC(window->win32.handle);
    if (!window->wgl.dc)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to retrieve DC for window");
        return GL_FALSE;
    }

    if (window->wgl.ARB_pixel_format)
    {
        int index = 0;
        UINT count;

        setWGLattrib(WGL_SUPPORT_OPENGL_ARB, TRUE);
        setWGLattrib(WGL_DRAW_TO_WINDOW_ARB, TRUE);
        setWGLattrib(WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB);
        setWGLattrib(WGL_DOUBLE_BUFFER_ARB, TRUE);

        if (fbconfig->redBits)
            setWGLattrib(WGL_RED_BITS_ARB, fbconfig->redBits);
        if (fbconfig->greenBits)
            setWGLattrib(WGL_GREEN_BITS_ARB, fbconfig->greenBits);
        if (fbconfig->blueBits)
            setWGLattrib(WGL_BLUE_BITS_ARB, fbconfig->blueBits);
        if (fbconfig->alphaBits)
            setWGLattrib(WGL_ALPHA_BITS_ARB, fbconfig->alphaBits);

        if (fbconfig->depthBits)
            setWGLattrib(WGL_DEPTH_BITS_ARB, fbconfig->depthBits);
        if (fbconfig->stencilBits)
            setWGLattrib(WGL_STENCIL_BITS_ARB, fbconfig->stencilBits);

        if (fbconfig->auxBuffers)
            setWGLattrib(WGL_AUX_BUFFERS_ARB, fbconfig->auxBuffers);

        if (fbconfig->accumRedBits)
            setWGLattrib(WGL_ACCUM_RED_BITS_ARB, fbconfig->accumRedBits);
        if (fbconfig->accumGreenBits)
            setWGLattrib(WGL_ACCUM_GREEN_BITS_ARB, fbconfig->accumGreenBits);
        if (fbconfig->accumBlueBits)
            setWGLattrib(WGL_ACCUM_BLUE_BITS_ARB, fbconfig->accumBlueBits);
        if (fbconfig->accumAlphaBits)
            setWGLattrib(WGL_ACCUM_BLUE_BITS_ARB, fbconfig->accumAlphaBits);

        if (fbconfig->stereo)
            setWGLattrib(WGL_STEREO_ARB, TRUE);

        if (window->wgl.ARB_multisample)
        {
            if (fbconfig->samples)
            {
                setWGLattrib(WGL_SAMPLE_BUFFERS_ARB, 1);
                setWGLattrib(WGL_SAMPLES_ARB, fbconfig->samples);
            }
        }

        if (window->wgl.ARB_framebuffer_sRGB)
        {
            if (fbconfig->sRGB)
                setWGLattrib(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, TRUE);
        }

        setWGLattrib(0, 0);

        if (!window->wgl.ChoosePixelFormatARB(window->wgl.dc,
                                              attribs,
                                              NULL,
                                              1,
                                              &pixelFormat,
                                              &count))
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "WGL: Failed to find a suitable pixel format");
            return GL_FALSE;
        }
    }
    else
    {
        ZeroMemory(&pfd, sizeof(pfd));
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;

        pfd.dwFlags |= PFD_DRAW_TO_WINDOW |
                       PFD_SUPPORT_OPENGL |
                       PFD_DOUBLEBUFFER;

        if (fbconfig->stereo)
            pfd.dwFlags |= PFD_STEREO;

        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = fbconfig->redBits +
                         fbconfig->greenBits +
                         fbconfig->blueBits;
        pfd.cAlphaBits = fbconfig->alphaBits;
        pfd.cAccumBits = fbconfig->accumRedBits +
                         fbconfig->accumGreenBits +
                         fbconfig->accumBlueBits;
        pfd.cDepthBits = fbconfig->depthBits;
        pfd.cStencilBits = fbconfig->stencilBits;
        pfd.cAuxBuffers = fbconfig->auxBuffers;

        pixelFormat = ChoosePixelFormat(window->wgl.dc, &pfd);
        if (!pixelFormat)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "WGL: Failed to find a suitable pixel format");
            return GL_FALSE;
        }
    }

    if (!DescribePixelFormat(window->wgl.dc, pixelFormat, sizeof(pfd), &pfd))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to retrieve PFD for selected pixel "
                        "format");
        return GL_FALSE;
    }

    if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) &&
        (pfd.dwFlags & PFD_GENERIC_FORMAT))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to find an accelerated pixel format");
        return GL_FALSE;
    }

    if (!SetPixelFormat(window->wgl.dc, pixelFormat, &pfd))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to set selected pixel format");
        return GL_FALSE;
    }

    if (window->wgl.ARB_create_context)
    {
        int index = 0, mask = 0, flags = 0, strategy = 0;

        if (wndconfig->clientAPI == GLFW_OPENGL_API)
        {
            if (wndconfig->glForward)
                flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

            if (wndconfig->glDebug)
                flags |= WGL_CONTEXT_DEBUG_BIT_ARB;

            if (wndconfig->glProfile)
            {
                if (wndconfig->glProfile == GLFW_OPENGL_CORE_PROFILE)
                    mask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
                else if (wndconfig->glProfile == GLFW_OPENGL_COMPAT_PROFILE)
                    mask |= WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            }
        }
        else
            mask |= WGL_CONTEXT_ES2_PROFILE_BIT_EXT;

        if (wndconfig->glRobustness)
        {
            if (window->wgl.ARB_create_context_robustness)
            {
                if (wndconfig->glRobustness == GLFW_NO_RESET_NOTIFICATION)
                    strategy = WGL_NO_RESET_NOTIFICATION_ARB;
                else if (wndconfig->glRobustness == GLFW_LOSE_CONTEXT_ON_RESET)
                    strategy = WGL_LOSE_CONTEXT_ON_RESET_ARB;

                flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;
            }
        }

        if (wndconfig->glMajor != 1 || wndconfig->glMinor != 0)
        {
            setWGLattrib(WGL_CONTEXT_MAJOR_VERSION_ARB, wndconfig->glMajor);
            setWGLattrib(WGL_CONTEXT_MINOR_VERSION_ARB, wndconfig->glMinor);
        }

        if (flags)
            setWGLattrib(WGL_CONTEXT_FLAGS_ARB, flags);

        if (mask)
            setWGLattrib(WGL_CONTEXT_PROFILE_MASK_ARB, mask);

        if (strategy)
            setWGLattrib(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, strategy);

        setWGLattrib(0, 0);

        window->wgl.context = window->wgl.CreateContextAttribsARB(window->wgl.dc,
                                                                  share,
                                                                  attribs);
        if (!window->wgl.context)
        {
            _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                            "WGL: Failed to create OpenGL context");
            return GL_FALSE;
        }
    }
    else
    {
        window->wgl.context = wglCreateContext(window->wgl.dc);
        if (!window->wgl.context)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "WGL: Failed to create OpenGL context");
            return GL_FALSE;
        }

        if (share)
        {
            if (!wglShareLists(share, window->wgl.context))
            {
                _glfwInputError(GLFW_PLATFORM_ERROR,
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

#undef setWGLattrib

// Destroy the OpenGL context
//
void _glfwDestroyContext(_GLFWwindow* window)
{
    if (window->wgl.context)
    {
        wglDeleteContext(window->wgl.context);
        window->wgl.context = NULL;
    }

    if (window->wgl.dc)
    {
        ReleaseDC(window->win32.handle, window->wgl.dc);
        window->wgl.dc = NULL;
    }
}

// Analyzes the specified context for possible recreation
//
int _glfwAnalyzeContext(const _GLFWwindow* window,
                        const _GLFWwndconfig* wndconfig,
                        const _GLFWfbconfig* fbconfig)
{
    GLboolean required = GL_FALSE;

    if (wndconfig->clientAPI == GLFW_OPENGL_API)
    {
        if (wndconfig->glForward)
        {
            if (!window->wgl.ARB_create_context)
            {
                _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                                "WGL: A forward compatible OpenGL context "
                                "requested but WGL_ARB_create_context is "
                                "unavailable");
                return _GLFW_RECREATION_IMPOSSIBLE;
            }

            required = GL_TRUE;
        }

        if (wndconfig->glProfile)
        {
            if (!window->wgl.ARB_create_context_profile)
            {
                _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                                "WGL: OpenGL profile requested but "
                                "WGL_ARB_create_context_profile is unavailable");
                return _GLFW_RECREATION_IMPOSSIBLE;
            }

            required = GL_TRUE;
        }
    }
    else
    {
        if (!window->wgl.ARB_create_context ||
            !window->wgl.ARB_create_context_profile ||
            !window->wgl.EXT_create_context_es2_profile)
        {
            _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                            "WGL: OpenGL ES requested but "
                            "WGL_ARB_create_context_es2_profile is unavailable");
            return _GLFW_RECREATION_IMPOSSIBLE;
        }

        required = GL_TRUE;
    }

    if (wndconfig->glMajor != 1 || wndconfig->glMinor != 0)
    {
        if (window->wgl.ARB_create_context)
            required = GL_TRUE;
    }

    if (wndconfig->glDebug)
    {
        if (window->wgl.ARB_create_context)
            required = GL_TRUE;
    }

    if (fbconfig->samples > 0)
    {
        // We want FSAA, but can we get it?
        // FSAA is not a hard constraint, so otherwise we just don't care

        if (window->wgl.ARB_multisample && window->wgl.ARB_pixel_format)
        {
            // We appear to have both the extension and the means to ask for it
            required = GL_TRUE;
        }
    }

    if (required)
        return _GLFW_RECREATION_REQUIRED;

    return _GLFW_RECREATION_NOT_NEEDED;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformMakeContextCurrent(_GLFWwindow* window)
{
    if (window)
        wglMakeCurrent(window->wgl.dc, window->wgl.context);
    else
        wglMakeCurrent(NULL, NULL);

    _glfwCurrentWindow = window;
}

_GLFWwindow* _glfwPlatformGetCurrentContext(void)
{
    return _glfwCurrentWindow;
}

void _glfwPlatformSwapBuffers(_GLFWwindow* window)
{
    SwapBuffers(window->wgl.dc);
}

void _glfwPlatformSwapInterval(int interval)
{
    _GLFWwindow* window = _glfwCurrentWindow;

    if (window->wgl.EXT_swap_control)
        window->wgl.SwapIntervalEXT(interval);
}

int _glfwPlatformExtensionSupported(const char* extension)
{
    const GLubyte* extensions;

    _GLFWwindow* window = _glfwCurrentWindow;

    if (window->wgl.GetExtensionsStringEXT != NULL)
    {
        extensions = (GLubyte*) window->wgl.GetExtensionsStringEXT();
        if (extensions != NULL)
        {
            if (_glfwStringInExtensionString(extension, extensions))
                return GL_TRUE;
        }
    }

    if (window->wgl.GetExtensionsStringARB != NULL)
    {
        extensions = (GLubyte*) window->wgl.GetExtensionsStringARB(window->wgl.dc);
        if (extensions != NULL)
        {
            if (_glfwStringInExtensionString(extension, extensions))
                return GL_TRUE;
        }
    }

    return GL_FALSE;
}

GLFWglproc _glfwPlatformGetProcAddress(const char* procname)
{
    return (GLFWglproc) wglGetProcAddress(procname);
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI HGLRC glfwGetWGLContext(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwInputError(GLFW_NOT_INITIALIZED, NULL);
        return NULL;
    }

    return window->wgl.context;
}

