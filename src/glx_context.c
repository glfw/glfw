//========================================================================
// GLFW 3.0 GLX - www.glfw.org
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
#include <stdlib.h>
#include <assert.h>


// This is the only glXGetProcAddress variant not declared by glxext.h
void (*glXGetProcAddressEXT(const GLubyte* procName))();


#ifndef GLXBadProfileARB
 #define GLXBadProfileARB 13
#endif


// Returns the specified attribute of the specified GLXFBConfig
// NOTE: Do not call this unless we have found GLX 1.3+ or GLX_SGIX_fbconfig
//
static int getFBConfigAttrib(GLXFBConfig fbconfig, int attrib)
{
    int value;

    if (_glfw.glx.SGIX_fbconfig)
    {
        _glfw.glx.GetFBConfigAttribSGIX(_glfw.x11.display,
                                        fbconfig, attrib, &value);
    }
    else
        glXGetFBConfigAttrib(_glfw.x11.display, fbconfig, attrib, &value);

    return value;
}

// Return a list of available and usable framebuffer configs
//
static GLboolean chooseFBConfig(const _GLFWfbconfig* desired, GLXFBConfig* result)
{
    GLXFBConfig* nativeConfigs;
    _GLFWfbconfig* usableConfigs;
    const _GLFWfbconfig* closest;
    int i, nativeCount, usableCount;
    const char* vendor;
    GLboolean trustWindowBit = GL_TRUE;

    vendor = glXGetClientString(_glfw.x11.display, GLX_VENDOR);
    if (strcmp(vendor, "Chromium") == 0)
    {
        // HACK: This is a (hopefully temporary) workaround for Chromium
        //       (VirtualBox GL) not setting the window bit on any GLXFBConfigs
        trustWindowBit = GL_FALSE;
    }

    if (_glfw.glx.SGIX_fbconfig)
    {
        nativeConfigs = _glfw.glx.ChooseFBConfigSGIX(_glfw.x11.display,
                                                     _glfw.x11.screen,
                                                     NULL,
                                                     &nativeCount);
    }
    else
    {
        nativeConfigs = glXGetFBConfigs(_glfw.x11.display,
                                        _glfw.x11.screen,
                                        &nativeCount);
    }

    if (!nativeCount)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "GLX: No GLXFBConfigs returned");
        return GL_FALSE;
    }

    usableConfigs = calloc(nativeCount, sizeof(_GLFWfbconfig));
    usableCount = 0;

    for (i = 0;  i < nativeCount;  i++)
    {
        const GLXFBConfig n = nativeConfigs[i];
        _GLFWfbconfig* u = usableConfigs + usableCount;

        if (!getFBConfigAttrib(n, GLX_DOUBLEBUFFER) ||
            !getFBConfigAttrib(n, GLX_VISUAL_ID))
        {
            // Only consider double-buffered GLXFBConfigs with associated visuals
            continue;
        }

        if (!(getFBConfigAttrib(n, GLX_RENDER_TYPE) & GLX_RGBA_BIT))
        {
            // Only consider RGBA GLXFBConfigs
            continue;
        }

        if (!(getFBConfigAttrib(n, GLX_DRAWABLE_TYPE) & GLX_WINDOW_BIT))
        {
            if (trustWindowBit)
            {
                // Only consider window GLXFBConfigs
                continue;
            }
        }

        u->redBits = getFBConfigAttrib(n, GLX_RED_SIZE);
        u->greenBits = getFBConfigAttrib(n, GLX_GREEN_SIZE);
        u->blueBits = getFBConfigAttrib(n, GLX_BLUE_SIZE);

        u->alphaBits = getFBConfigAttrib(n, GLX_ALPHA_SIZE);
        u->depthBits = getFBConfigAttrib(n, GLX_DEPTH_SIZE);
        u->stencilBits = getFBConfigAttrib(n, GLX_STENCIL_SIZE);

        u->accumRedBits = getFBConfigAttrib(n, GLX_ACCUM_RED_SIZE);
        u->accumGreenBits = getFBConfigAttrib(n, GLX_ACCUM_GREEN_SIZE);
        u->accumBlueBits = getFBConfigAttrib(n, GLX_ACCUM_BLUE_SIZE);
        u->accumAlphaBits = getFBConfigAttrib(n, GLX_ACCUM_ALPHA_SIZE);

        u->auxBuffers = getFBConfigAttrib(n, GLX_AUX_BUFFERS);
        u->stereo = getFBConfigAttrib(n, GLX_STEREO);

        if (_glfw.glx.ARB_multisample)
            u->samples = getFBConfigAttrib(n, GLX_SAMPLES);

        if (_glfw.glx.ARB_framebuffer_sRGB)
            u->sRGB = getFBConfigAttrib(n, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB);

        u->glx = n;
        usableCount++;
    }

    closest = _glfwChooseFBConfig(desired, usableConfigs, usableCount);
    if (closest)
        *result = closest->glx;

    XFree(nativeConfigs);
    free(usableConfigs);

    return closest ? GL_TRUE : GL_FALSE;
}

// Create the OpenGL context using legacy API
//
static GLXContext createLegacyContext(_GLFWwindow* window,
                                      GLXFBConfig fbconfig,
                                      GLXContext share)
{
    if (_glfw.glx.SGIX_fbconfig)
    {
        return _glfw.glx.CreateContextWithConfigSGIX(_glfw.x11.display,
                                                     fbconfig,
                                                     GLX_RGBA_TYPE,
                                                     share,
                                                     True);
    }

    return glXCreateNewContext(_glfw.x11.display,
                               fbconfig,
                               GLX_RGBA_TYPE,
                               share,
                               True);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize GLX
//
int _glfwInitContextAPI(void)
{
#ifdef _GLFW_DLOPEN_LIBGL
    int i;
    char* libGL_names[ ] =
    {
        "libGL.so",
        "libGL.so.1",
        "/usr/lib/libGL.so",
        "/usr/lib/libGL.so.1",
        NULL
    };

    for (i = 0;  libGL_names[i] != NULL;  i++)
    {
        _glfw.glx.libGL = dlopen(libGL_names[i], RTLD_LAZY | RTLD_GLOBAL);
        if (_glfw.glx.libGL)
            break;
    }

    if (!_glfw.glx.libGL)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "GLX: Failed to find libGL");
        return GL_FALSE;
    }
#endif

    if (pthread_key_create(&_glfw.glx.current, NULL) != 0)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "GLX: Failed to create context TLS");
        return GL_FALSE;
    }

    // Check if GLX is supported on this display
    if (!glXQueryExtension(_glfw.x11.display,
                           &_glfw.glx.errorBase,
                           &_glfw.glx.eventBase))
    {
        _glfwInputError(GLFW_API_UNAVAILABLE, "GLX: GLX support not found");
        return GL_FALSE;
    }

    if (!glXQueryVersion(_glfw.x11.display,
                         &_glfw.glx.versionMajor,
                         &_glfw.glx.versionMinor))
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "GLX: Failed to query GLX version");
        return GL_FALSE;
    }

    if (_glfwPlatformExtensionSupported("GLX_EXT_swap_control"))
    {
        _glfw.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)
            _glfwPlatformGetProcAddress("glXSwapIntervalEXT");

        if (_glfw.glx.SwapIntervalEXT)
            _glfw.glx.EXT_swap_control = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("GLX_SGI_swap_control"))
    {
        _glfw.glx.SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
            _glfwPlatformGetProcAddress("glXSwapIntervalSGI");

        if (_glfw.glx.SwapIntervalSGI)
            _glfw.glx.SGI_swap_control = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("GLX_MESA_swap_control"))
    {
        _glfw.glx.SwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)
            _glfwPlatformGetProcAddress("glXSwapIntervalMESA");

        if (_glfw.glx.SwapIntervalMESA)
            _glfw.glx.MESA_swap_control = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("GLX_SGIX_fbconfig"))
    {
        _glfw.glx.GetFBConfigAttribSGIX = (PFNGLXGETFBCONFIGATTRIBSGIXPROC)
            _glfwPlatformGetProcAddress("glXGetFBConfigAttribSGIX");
        _glfw.glx.ChooseFBConfigSGIX = (PFNGLXCHOOSEFBCONFIGSGIXPROC)
            _glfwPlatformGetProcAddress("glXChooseFBConfigSGIX");
        _glfw.glx.CreateContextWithConfigSGIX = (PFNGLXCREATECONTEXTWITHCONFIGSGIXPROC)
            _glfwPlatformGetProcAddress("glXCreateContextWithConfigSGIX");
        _glfw.glx.GetVisualFromFBConfigSGIX = (PFNGLXGETVISUALFROMFBCONFIGSGIXPROC)
            _glfwPlatformGetProcAddress("glXGetVisualFromFBConfigSGIX");

        if (_glfw.glx.GetFBConfigAttribSGIX &&
            _glfw.glx.ChooseFBConfigSGIX &&
            _glfw.glx.CreateContextWithConfigSGIX &&
            _glfw.glx.GetVisualFromFBConfigSGIX)
        {
            _glfw.glx.SGIX_fbconfig = GL_TRUE;
        }
    }

    if (_glfwPlatformExtensionSupported("GLX_ARB_multisample"))
        _glfw.glx.ARB_multisample = GL_TRUE;

    if (_glfwPlatformExtensionSupported("GLX_ARB_framebuffer_sRGB"))
        _glfw.glx.ARB_framebuffer_sRGB = GL_TRUE;

    if (_glfwPlatformExtensionSupported("GLX_ARB_create_context"))
    {
        _glfw.glx.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            _glfwPlatformGetProcAddress("glXCreateContextAttribsARB");

        if (_glfw.glx.CreateContextAttribsARB)
            _glfw.glx.ARB_create_context = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("GLX_ARB_create_context_robustness"))
        _glfw.glx.ARB_create_context_robustness = GL_TRUE;

    if (_glfwPlatformExtensionSupported("GLX_ARB_create_context_profile"))
        _glfw.glx.ARB_create_context_profile = GL_TRUE;

    if (_glfwPlatformExtensionSupported("GLX_EXT_create_context_es2_profile"))
        _glfw.glx.EXT_create_context_es2_profile = GL_TRUE;

    return GL_TRUE;
}

// Terminate GLX
//
void _glfwTerminateContextAPI(void)
{
    // Unload libGL.so if necessary
#ifdef _GLFW_DLOPEN_LIBGL
    if (_glfw.glx.libGL != NULL)
    {
        dlclose(_glfw.glx.libGL);
        _glfw.glx.libGL = NULL;
    }
#endif

    pthread_key_delete(_glfw.glx.current);
}

#define setGLXattrib(attribName, attribValue) \
{ \
    attribs[index++] = attribName; \
    attribs[index++] = attribValue; \
    assert((size_t) index < sizeof(attribs) / sizeof(attribs[0])); \
}

// Prepare for creation of the OpenGL context
//
int _glfwCreateContext(_GLFWwindow* window,
                       const _GLFWwndconfig* wndconfig,
                       const _GLFWfbconfig* fbconfig)
{
    int attribs[40];
    GLXFBConfig native;
    GLXContext share = NULL;

    if (wndconfig->share)
        share = wndconfig->share->glx.context;

    if (!chooseFBConfig(fbconfig, &native))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "GLX: Failed to find a suitable GLXFBConfig");
        return GL_FALSE;
    }

    // Retrieve the corresponding visual
    if (_glfw.glx.SGIX_fbconfig)
    {
        window->glx.visual =
            _glfw.glx.GetVisualFromFBConfigSGIX(_glfw.x11.display, native);
    }
    else
        window->glx.visual = glXGetVisualFromFBConfig(_glfw.x11.display, native);

    if (window->glx.visual == NULL)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "GLX: Failed to retrieve visual for GLXFBConfig");
        return GL_FALSE;
    }

    if (wndconfig->clientAPI == GLFW_OPENGL_ES_API)
    {
        if (!_glfw.glx.ARB_create_context ||
            !_glfw.glx.ARB_create_context_profile ||
            !_glfw.glx.EXT_create_context_es2_profile)
        {
            _glfwInputError(GLFW_API_UNAVAILABLE,
                            "GLX: OpenGL ES requested but "
                            "GLX_EXT_create_context_es2_profile is unavailable");
            return GL_FALSE;
        }
    }

    if (wndconfig->glForward)
    {
        if (!_glfw.glx.ARB_create_context)
        {
            _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                            "GLX: Forward compatibility requested but "
                            "GLX_ARB_create_context_profile is unavailable");
            return GL_FALSE;
        }
    }

    if (wndconfig->glProfile)
    {
        if (!_glfw.glx.ARB_create_context ||
            !_glfw.glx.ARB_create_context_profile)
        {
            _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                            "GLX: An OpenGL profile requested but "
                            "GLX_ARB_create_context_profile is unavailable");
            return GL_FALSE;
        }
    }

    _glfwGrabXErrorHandler();

    if (_glfw.glx.ARB_create_context)
    {
        int index = 0, mask = 0, flags = 0, strategy = 0;

        if (wndconfig->clientAPI == GLFW_OPENGL_API)
        {
            if (wndconfig->glForward)
                flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

            if (wndconfig->glDebug)
                flags |= GLX_CONTEXT_DEBUG_BIT_ARB;

            if (wndconfig->glProfile)
            {
                if (wndconfig->glProfile == GLFW_OPENGL_CORE_PROFILE)
                    mask |= GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
                else if (wndconfig->glProfile == GLFW_OPENGL_COMPAT_PROFILE)
                    mask |= GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            }
        }
        else
            mask |= GLX_CONTEXT_ES2_PROFILE_BIT_EXT;

        if (wndconfig->glRobustness != GLFW_NO_ROBUSTNESS)
        {
            if (_glfw.glx.ARB_create_context_robustness)
            {
                if (wndconfig->glRobustness == GLFW_NO_RESET_NOTIFICATION)
                    strategy = GLX_NO_RESET_NOTIFICATION_ARB;
                else if (wndconfig->glRobustness == GLFW_LOSE_CONTEXT_ON_RESET)
                    strategy = GLX_LOSE_CONTEXT_ON_RESET_ARB;

                flags |= GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB;
            }
        }

        if (wndconfig->glMajor != 1 || wndconfig->glMinor != 0)
        {
            // NOTE: Only request an explicitly versioned context when
            //       necessary, as explicitly requesting version 1.0 does not
            //       always return the highest available version

            setGLXattrib(GLX_CONTEXT_MAJOR_VERSION_ARB, wndconfig->glMajor);
            setGLXattrib(GLX_CONTEXT_MINOR_VERSION_ARB, wndconfig->glMinor);
        }

        if (mask)
            setGLXattrib(GLX_CONTEXT_PROFILE_MASK_ARB, mask);

        if (flags)
            setGLXattrib(GLX_CONTEXT_FLAGS_ARB, flags);

        if (strategy)
            setGLXattrib(GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, strategy);

        setGLXattrib(None, None);

        window->glx.context =
            _glfw.glx.CreateContextAttribsARB(_glfw.x11.display,
                                              native,
                                              share,
                                              True,
                                              attribs);

        if (window->glx.context == NULL)
        {
            // HACK: This is a fallback for the broken Mesa implementation of
            //       GLX_ARB_create_context_profile, which fails default 1.0
            //       context creation with a GLXBadProfileARB error in violation
            //       of the extension spec
            if (_glfw.x11.errorCode == _glfw.glx.errorBase + GLXBadProfileARB &&
                wndconfig->clientAPI == GLFW_OPENGL_API &&
                wndconfig->glProfile == GLFW_OPENGL_ANY_PROFILE &&
                wndconfig->glForward == GL_FALSE)
            {
                window->glx.context = createLegacyContext(window, native, share);
            }
        }
    }
    else
        window->glx.context = createLegacyContext(window, native, share);

    _glfwReleaseXErrorHandler();

    if (window->glx.context == NULL)
    {
        _glfwInputXError(GLFW_PLATFORM_ERROR, "GLX: Failed to create context");
        return GL_FALSE;
    }

    return GL_TRUE;
}

#undef setGLXattrib

// Destroy the OpenGL context
//
void _glfwDestroyContext(_GLFWwindow* window)
{
    if (window->glx.visual)
    {
        XFree(window->glx.visual);
        window->glx.visual = NULL;
    }

    if (window->glx.context)
    {
        glXDestroyContext(_glfw.x11.display, window->glx.context);
        window->glx.context = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformMakeContextCurrent(_GLFWwindow* window)
{
    if (window)
    {
        glXMakeCurrent(_glfw.x11.display,
                       window->x11.handle,
                       window->glx.context);
    }
    else
        glXMakeCurrent(_glfw.x11.display, None, NULL);

    pthread_setspecific(_glfw.glx.current, window);
}

_GLFWwindow* _glfwPlatformGetCurrentContext(void)
{
    return (_GLFWwindow*) pthread_getspecific(_glfw.glx.current);
}

void _glfwPlatformSwapBuffers(_GLFWwindow* window)
{
    glXSwapBuffers(_glfw.x11.display, window->x11.handle);
}

void _glfwPlatformSwapInterval(int interval)
{
    _GLFWwindow* window = _glfwPlatformGetCurrentContext();

    if (_glfw.glx.EXT_swap_control)
    {
        _glfw.glx.SwapIntervalEXT(_glfw.x11.display,
                                  window->x11.handle,
                                  interval);
    }
    else if (_glfw.glx.MESA_swap_control)
        _glfw.glx.SwapIntervalMESA(interval);
    else if (_glfw.glx.SGI_swap_control)
    {
        if (interval > 0)
            _glfw.glx.SwapIntervalSGI(interval);
    }
}

int _glfwPlatformExtensionSupported(const char* extension)
{
    const GLubyte* extensions;

    // Get list of GLX extensions
    extensions = (const GLubyte*) glXQueryExtensionsString(_glfw.x11.display,
                                                           _glfw.x11.screen);
    if (extensions != NULL)
    {
        if (_glfwStringInExtensionString(extension, extensions))
            return GL_TRUE;
    }

    return GL_FALSE;
}

GLFWglproc _glfwPlatformGetProcAddress(const char* procname)
{
    return _glfw_glXGetProcAddress((const GLubyte*) procname);
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI GLXContext glfwGetGLXContext(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    return window->glx.context;
}

