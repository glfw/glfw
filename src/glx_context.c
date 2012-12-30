//========================================================================
// GLFW - An OpenGL library
// Platform:    X11/GLX
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
#include <stdlib.h>


// This is the only glXGetProcAddress variant not declared by glxext.h
void (*glXGetProcAddressEXT(const GLubyte* procName))();


#ifndef GLXBadProfileARB
 #define GLXBadProfileARB 13
#endif


//========================================================================
// Thread local storage attribute macro
//========================================================================
#if defined(__GNUC__)
 #define _GLFW_TLS __thread
#else
 #define _GLFW_TLS
#endif


//========================================================================
// The X error code as provided to the X error handler
//========================================================================
static unsigned long _glfwErrorCode = Success;


//========================================================================
// The per-thread current context/window pointer
//========================================================================
static _GLFW_TLS _GLFWwindow* _glfwCurrentWindow = NULL;


//========================================================================
// Returns the specified attribute of the specified GLXFBConfig
// NOTE: Do not call this unless we have found GLX 1.3+ or GLX_SGIX_fbconfig
//========================================================================

static int getFBConfigAttrib(_GLFWwindow* window, GLXFBConfig fbconfig, int attrib)
{
    int value;

    if (_glfwLibrary.GLX.SGIX_fbconfig)
    {
        _glfwLibrary.GLX.GetFBConfigAttribSGIX(_glfwLibrary.X11.display,
                                               fbconfig, attrib, &value);
    }
    else
        glXGetFBConfigAttrib(_glfwLibrary.X11.display, fbconfig, attrib, &value);

    return value;
}


//========================================================================
// Return a list of available and usable framebuffer configs
//========================================================================

static _GLFWfbconfig* getFBConfigs(_GLFWwindow* window, unsigned int* found)
{
    GLXFBConfig* fbconfigs;
    _GLFWfbconfig* result;
    int i, count = 0;
    const char* vendor;
    GLboolean trustWindowBit = GL_TRUE;

    *found = 0;

    if (_glfwLibrary.GLX.majorVersion == 1 && _glfwLibrary.GLX.minorVersion < 3)
    {
        if (!_glfwLibrary.GLX.SGIX_fbconfig)
        {
            _glfwSetError(GLFW_API_UNAVAILABLE,
                          "GLX: GLXFBConfig support not found");
            return NULL;
        }
    }

    vendor = glXGetClientString(_glfwLibrary.X11.display, GLX_VENDOR);

    if (strcmp(vendor, "Chromium") == 0)
    {
        // HACK: This is a (hopefully temporary) workaround for Chromium
        // (VirtualBox GL) not setting the window bit on any GLXFBConfigs
        trustWindowBit = GL_FALSE;
    }

    if (_glfwLibrary.GLX.SGIX_fbconfig)
    {
        fbconfigs = _glfwLibrary.GLX.ChooseFBConfigSGIX(_glfwLibrary.X11.display,
                                                        _glfwLibrary.X11.screen,
                                                        NULL,
                                                        &count);
        if (!count)
        {
            _glfwSetError(GLFW_API_UNAVAILABLE,
                          "GLX: No GLXFBConfigs returned");
            return NULL;
        }
    }
    else
    {
        fbconfigs = glXGetFBConfigs(_glfwLibrary.X11.display,
                                    _glfwLibrary.X11.screen,
                                    &count);
        if (!count)
        {
            _glfwSetError(GLFW_API_UNAVAILABLE,
                          "GLX: No GLXFBConfigs returned");
            return NULL;
        }
    }

    result = (_GLFWfbconfig*) malloc(sizeof(_GLFWfbconfig) * count);
    if (!result)
    {
        _glfwSetError(GLFW_OUT_OF_MEMORY, NULL);
        return NULL;
    }

    for (i = 0;  i < count;  i++)
    {
        _GLFWfbconfig* f = result + *found;

        if (!getFBConfigAttrib(window, fbconfigs[i], GLX_DOUBLEBUFFER) ||
            !getFBConfigAttrib(window, fbconfigs[i], GLX_VISUAL_ID))
        {
            // Only consider double-buffered GLXFBConfigs with associated visuals
            continue;
        }

        if (!(getFBConfigAttrib(window,
                                fbconfigs[i],
                                GLX_RENDER_TYPE) & GLX_RGBA_BIT))
        {
            // Only consider RGBA GLXFBConfigs
            continue;
        }

        if (!(getFBConfigAttrib(window, fbconfigs[i], GLX_DRAWABLE_TYPE) & GLX_WINDOW_BIT))
        {
            if (trustWindowBit)
            {
                // Only consider window GLXFBConfigs
                continue;
            }
        }

        f->redBits = getFBConfigAttrib(window, fbconfigs[i], GLX_RED_SIZE);
        f->greenBits = getFBConfigAttrib(window, fbconfigs[i], GLX_GREEN_SIZE);
        f->blueBits = getFBConfigAttrib(window, fbconfigs[i], GLX_BLUE_SIZE);

        f->alphaBits = getFBConfigAttrib(window, fbconfigs[i], GLX_ALPHA_SIZE);
        f->depthBits = getFBConfigAttrib(window, fbconfigs[i], GLX_DEPTH_SIZE);
        f->stencilBits = getFBConfigAttrib(window, fbconfigs[i], GLX_STENCIL_SIZE);

        f->accumRedBits = getFBConfigAttrib(window, fbconfigs[i], GLX_ACCUM_RED_SIZE);
        f->accumGreenBits = getFBConfigAttrib(window, fbconfigs[i], GLX_ACCUM_GREEN_SIZE);
        f->accumBlueBits = getFBConfigAttrib(window, fbconfigs[i], GLX_ACCUM_BLUE_SIZE);
        f->accumAlphaBits = getFBConfigAttrib(window, fbconfigs[i], GLX_ACCUM_ALPHA_SIZE);

        f->auxBuffers = getFBConfigAttrib(window, fbconfigs[i], GLX_AUX_BUFFERS);
        f->stereo = getFBConfigAttrib(window, fbconfigs[i], GLX_STEREO);

        if (_glfwLibrary.GLX.ARB_multisample)
            f->samples = getFBConfigAttrib(window, fbconfigs[i], GLX_SAMPLES);
        else
            f->samples = 0;

        if (_glfwLibrary.GLX.ARB_framebuffer_sRGB)
            f->sRGB = getFBConfigAttrib(window, fbconfigs[i], GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB);
        else
            f->sRGB = GL_FALSE;

        f->platformID = (GLFWintptr) getFBConfigAttrib(window, fbconfigs[i], GLX_FBCONFIG_ID);

        (*found)++;
    }

    XFree(fbconfigs);

    return result;
}


//========================================================================
// Error handler used when creating a context with the GLX_ARB_create_context
// extension set
//========================================================================

static int errorHandler(Display *display, XErrorEvent* event)
{
    _glfwErrorCode = event->error_code;
    return 0;
}


//========================================================================
// Create the OpenGL context using legacy API
//========================================================================

static void createLegacyContext(_GLFWwindow* window,
                                const _GLFWwndconfig* wndconfig,
                                GLXFBConfig fbconfig,
                                GLXContext share)
{
    if (_glfwLibrary.GLX.SGIX_fbconfig)
    {
        window->GLX.context =
            _glfwLibrary.GLX.CreateContextWithConfigSGIX(_glfwLibrary.X11.display,
                                                         fbconfig,
                                                         GLX_RGBA_TYPE,
                                                         share,
                                                         True);
    }
    else
    {
        window->GLX.context = glXCreateNewContext(_glfwLibrary.X11.display,
                                                  fbconfig,
                                                  GLX_RGBA_TYPE,
                                                  share,
                                                  True);
    }
}


//========================================================================
// Create the OpenGL context
//========================================================================

#define setGLXattrib(attribName, attribValue) \
    attribs[index++] = attribName; \
    attribs[index++] = attribValue;

static int createContext(_GLFWwindow* window,
                         const _GLFWwndconfig* wndconfig,
                         GLXFBConfigID fbconfigID)
{
    int attribs[40];
    GLXFBConfig* fbconfig;
    GLXContext share = NULL;

    if (wndconfig->share)
        share = wndconfig->share->GLX.context;

    // Retrieve the previously selected GLXFBConfig
    {
        int dummy, index = 0;

        setGLXattrib(GLX_FBCONFIG_ID, (int) fbconfigID);
        setGLXattrib(None, None);

        if (_glfwLibrary.GLX.SGIX_fbconfig)
        {
            fbconfig =
                _glfwLibrary.GLX.ChooseFBConfigSGIX(_glfwLibrary.X11.display,
                                                    _glfwLibrary.X11.screen,
                                                    attribs,
                                                    &dummy);
        }
        else
        {
            fbconfig = glXChooseFBConfig(_glfwLibrary.X11.display,
                                         _glfwLibrary.X11.screen,
                                         attribs,
                                         &dummy);
        }

        if (fbconfig == NULL)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "GLX: Failed to retrieve the selected GLXFBConfig");
            return GL_FALSE;
        }
    }

    // Retrieve the corresponding visual
    if (_glfwLibrary.GLX.SGIX_fbconfig)
    {
        window->GLX.visual =
            _glfwLibrary.GLX.GetVisualFromFBConfigSGIX(_glfwLibrary.X11.display,
                                                       *fbconfig);
    }
    else
    {
        window->GLX.visual = glXGetVisualFromFBConfig(_glfwLibrary.X11.display,
                                                      *fbconfig);
    }

    if (window->GLX.visual == NULL)
    {
        XFree(fbconfig);

        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "GLX: Failed to retrieve visual for GLXFBConfig");
        return GL_FALSE;
    }

    if (wndconfig->clientAPI == GLFW_OPENGL_ES_API)
    {
        if (!_glfwLibrary.GLX.ARB_create_context ||
            !_glfwLibrary.GLX.ARB_create_context_profile ||
            !_glfwLibrary.GLX.EXT_create_context_es2_profile)
        {
            _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                          "GLX: OpenGL ES requested but "
                          "GLX_EXT_create_context_es2_profile is unavailable");
            return GL_FALSE;
        }
    }

    if (wndconfig->glForward)
    {
        if (!_glfwLibrary.GLX.ARB_create_context)
        {
            _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                          "GLX: Forward compatibility requested but "
                          "GLX_ARB_create_context_profile is unavailable");
            return GL_FALSE;
        }
    }

    if (wndconfig->glProfile)
    {
        if (!_glfwLibrary.GLX.ARB_create_context ||
            !_glfwLibrary.GLX.ARB_create_context_profile)
        {
            _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                          "GLX: An OpenGL profile requested but "
                          "GLX_ARB_create_context_profile is unavailable");
            return GL_FALSE;
        }
    }

    if (_glfwLibrary.GLX.ARB_create_context)
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
            if (_glfwLibrary.GLX.ARB_create_context_robustness)
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
            // necessary, as explicitly requesting version 1.0 does not always
            // return the highest available version

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

        // This is the only place we set an Xlib error handler, and we only do
        // it because glXCreateContextAttribsARB generates a BadMatch error if
        // the requested OpenGL version is unavailable (instead of a civilized
        // response like returning NULL)
        _glfwErrorCode = Success;
        XSetErrorHandler(errorHandler);

        window->GLX.context =
            _glfwLibrary.GLX.CreateContextAttribsARB(_glfwLibrary.X11.display,
                                                     *fbconfig,
                                                     share,
                                                     True,
                                                     attribs);

        // We are done, so unset the error handler again (see above)
        XSetErrorHandler(NULL);

        if (window->GLX.context == NULL)
        {
            // HACK: This is a fallback for the broken Mesa implementation of
            // GLX_ARB_create_context_profile, which fails default 1.0 context
            // creation with a GLXBadProfileARB error in violation of the spec
            if (_glfwErrorCode == _glfwLibrary.GLX.errorBase + GLXBadProfileARB &&
                wndconfig->clientAPI == GLFW_OPENGL_API &&
                wndconfig->glProfile == GLFW_OPENGL_NO_PROFILE &&
                wndconfig->glForward == GL_FALSE)
            {
                createLegacyContext(window, wndconfig, *fbconfig, share);
            }
        }
    }
    else
        createLegacyContext(window, wndconfig, *fbconfig, share);

    XFree(fbconfig);

    if (window->GLX.context == NULL)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR, "GLX: Failed to create context");
        return GL_FALSE;
    }

    return GL_TRUE;
}

#undef setGLXattrib


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Initialize GLX
//========================================================================

int _glfwInitOpenGL(void)
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
        _glfwLibrary.GLX.libGL = dlopen(libGL_names[i], RTLD_LAZY | RTLD_GLOBAL);
        if (_glfwLibrary.GLX.libGL)
            break;
    }

    if (!_glfwLibrary.GLX.libGL)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR, "GLX: Failed to find libGL");
        return GL_FALSE;
    }
#endif

    // Check if GLX is supported on this display
    if (!glXQueryExtension(_glfwLibrary.X11.display,
                           &_glfwLibrary.GLX.errorBase,
                           &_glfwLibrary.GLX.eventBase))
    {
        _glfwSetError(GLFW_API_UNAVAILABLE, "GLX: GLX support not found");
        return GL_FALSE;
    }

    if (!glXQueryVersion(_glfwLibrary.X11.display,
                         &_glfwLibrary.GLX.majorVersion,
                         &_glfwLibrary.GLX.minorVersion))
    {
        _glfwSetError(GLFW_API_UNAVAILABLE, "GLX: Failed to query GLX version");
        return GL_FALSE;
    }

    if (_glfwPlatformExtensionSupported("GLX_EXT_swap_control"))
    {
        _glfwLibrary.GLX.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)
            _glfwPlatformGetProcAddress("glXSwapIntervalEXT");

        if (_glfwLibrary.GLX.SwapIntervalEXT)
            _glfwLibrary.GLX.EXT_swap_control = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("GLX_SGI_swap_control"))
    {
        _glfwLibrary.GLX.SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
            _glfwPlatformGetProcAddress("glXSwapIntervalSGI");

        if (_glfwLibrary.GLX.SwapIntervalSGI)
            _glfwLibrary.GLX.SGI_swap_control = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("GLX_MESA_swap_control"))
    {
        _glfwLibrary.GLX.SwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)
            _glfwPlatformGetProcAddress("glXSwapIntervalMESA");

        if (_glfwLibrary.GLX.SwapIntervalMESA)
            _glfwLibrary.GLX.MESA_swap_control = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("GLX_SGIX_fbconfig"))
    {
        _glfwLibrary.GLX.GetFBConfigAttribSGIX = (PFNGLXGETFBCONFIGATTRIBSGIXPROC)
            _glfwPlatformGetProcAddress("glXGetFBConfigAttribSGIX");
        _glfwLibrary.GLX.ChooseFBConfigSGIX = (PFNGLXCHOOSEFBCONFIGSGIXPROC)
            _glfwPlatformGetProcAddress("glXChooseFBConfigSGIX");
        _glfwLibrary.GLX.CreateContextWithConfigSGIX = (PFNGLXCREATECONTEXTWITHCONFIGSGIXPROC)
            _glfwPlatformGetProcAddress("glXCreateContextWithConfigSGIX");
        _glfwLibrary.GLX.GetVisualFromFBConfigSGIX = (PFNGLXGETVISUALFROMFBCONFIGSGIXPROC)
            _glfwPlatformGetProcAddress("glXGetVisualFromFBConfigSGIX");

        if (_glfwLibrary.GLX.GetFBConfigAttribSGIX &&
            _glfwLibrary.GLX.ChooseFBConfigSGIX &&
            _glfwLibrary.GLX.CreateContextWithConfigSGIX &&
            _glfwLibrary.GLX.GetVisualFromFBConfigSGIX)
        {
            _glfwLibrary.GLX.SGIX_fbconfig = GL_TRUE;
        }
    }

    if (_glfwPlatformExtensionSupported("GLX_ARB_multisample"))
        _glfwLibrary.GLX.ARB_multisample = GL_TRUE;

    if (_glfwPlatformExtensionSupported("GLX_ARB_framebuffer_sRGB"))
        _glfwLibrary.GLX.ARB_framebuffer_sRGB = GL_TRUE;

    if (_glfwPlatformExtensionSupported("GLX_ARB_create_context"))
    {
        _glfwLibrary.GLX.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            _glfwPlatformGetProcAddress("glXCreateContextAttribsARB");

        if (_glfwLibrary.GLX.CreateContextAttribsARB)
            _glfwLibrary.GLX.ARB_create_context = GL_TRUE;
    }

    if (_glfwPlatformExtensionSupported("GLX_ARB_create_context_robustness"))
        _glfwLibrary.GLX.ARB_create_context_robustness = GL_TRUE;

    if (_glfwPlatformExtensionSupported("GLX_ARB_create_context_profile"))
        _glfwLibrary.GLX.ARB_create_context_profile = GL_TRUE;

    if (_glfwPlatformExtensionSupported("GLX_EXT_create_context_es2_profile"))
        _glfwLibrary.GLX.EXT_create_context_es2_profile = GL_TRUE;

    return GL_TRUE;
}


//========================================================================
// Terminate GLX
//========================================================================

void _glfwTerminateOpenGL(void)
{
    // Unload libGL.so if necessary
#ifdef _GLFW_DLOPEN_LIBGL
    if (_glfwLibrary.GLX.libGL != NULL)
    {
        dlclose(_glfwLibrary.GLX.libGL);
        _glfwLibrary.GLX.libGL = NULL;
    }
#endif
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
            return GL_FALSE;

        result = _glfwChooseFBConfig(fbconfig, fbconfigs, fbcount);
        if (!result)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "GLX: No GLXFBConfig matched the criteria");

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
    if (window->GLX.visual)
    {
        XFree(window->GLX.visual);
        window->GLX.visual = NULL;
    }

    if (window->GLX.context)
    {
        glXDestroyContext(_glfwLibrary.X11.display, window->GLX.context);
        window->GLX.context = NULL;
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
    {
        glXMakeCurrent(_glfwLibrary.X11.display,
                       window->X11.handle,
                       window->GLX.context);
    }
    else
        glXMakeCurrent(_glfwLibrary.X11.display, None, NULL);

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
    glXSwapBuffers(_glfwLibrary.X11.display, window->X11.handle);
}


//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval(int interval)
{
    _GLFWwindow* window = _glfwCurrentWindow;

    if (_glfwLibrary.GLX.EXT_swap_control)
    {
        _glfwLibrary.GLX.SwapIntervalEXT(_glfwLibrary.X11.display,
                                         window->X11.handle,
                                         interval);
    }
    else if (_glfwLibrary.GLX.MESA_swap_control)
        _glfwLibrary.GLX.SwapIntervalMESA(interval);
    else if (_glfwLibrary.GLX.SGI_swap_control)
    {
        if (interval > 0)
            _glfwLibrary.GLX.SwapIntervalSGI(interval);
    }
}


//========================================================================
// Check if an OpenGL extension is available at runtime
//========================================================================

int _glfwPlatformExtensionSupported(const char* extension)
{
    const GLubyte* extensions;

    // Get list of GLX extensions
    extensions = (const GLubyte*) glXQueryExtensionsString(_glfwLibrary.X11.display,
                                                           _glfwLibrary.X11.screen);
    if (extensions != NULL)
    {
        if (_glfwStringInExtensionString(extension, extensions))
            return GL_TRUE;
    }

    return GL_FALSE;
}


//========================================================================
// Get the function pointer to an OpenGL function
//========================================================================

GLFWglproc _glfwPlatformGetProcAddress(const char* procname)
{
    return _glfw_glXGetProcAddress((const GLubyte*) procname);
}

