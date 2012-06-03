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
            _glfwSetError(GLFW_OPENGL_UNAVAILABLE,
                          "X11/GLX: GLXFBConfig support not found");
            return NULL;
        }
    }

    vendor = glXGetClientString(_glfwLibrary.X11.display, GLX_VENDOR);

    if (strcmp(vendor, "Chromium") == 0)
    {
        // This is a (hopefully temporary) workaround for Chromium (VirtualBox
        // GL) not setting the window bit on any GLXFBConfigs
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
            _glfwSetError(GLFW_OPENGL_UNAVAILABLE,
                          "X11/GLX: No GLXFBConfigs returned");
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
            _glfwSetError(GLFW_OPENGL_UNAVAILABLE,
                          "X11/GLX: No GLXFBConfigs returned");
            return NULL;
        }
    }

    result = (_GLFWfbconfig*) malloc(sizeof(_GLFWfbconfig) * count);
    if (!result)
    {
        _glfwSetError(GLFW_OUT_OF_MEMORY,
                      "X11/GLX: Failed to allocate _GLFWfbconfig array");
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

        f->platformID = (GLFWintptr) getFBConfigAttrib(window, fbconfigs[i], GLX_FBCONFIG_ID);

        (*found)++;
    }

    XFree(fbconfigs);

    return result;
}


//========================================================================
// Error handler for BadMatch errors when requesting context with
// unavailable OpenGL versions using the GLX_ARB_create_context extension
//========================================================================

static int errorHandler(Display *display, XErrorEvent* event)
{
    return 0;
}


//========================================================================
// Read back framebuffer parameters from the context
//========================================================================

static void refreshContextParams(_GLFWwindow* window, GLXFBConfigID fbconfigID)
{
    int dummy;
    GLXFBConfig* fbconfig;

    int attribs[] = { GLX_FBCONFIG_ID, fbconfigID, None };

    if (_glfwLibrary.GLX.SGIX_fbconfig)
    {
        fbconfig = _glfwLibrary.GLX.ChooseFBConfigSGIX(_glfwLibrary.X11.display,
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
        // This should never ever happen
        // TODO: Flag this as an error and propagate up
        _glfwSetError(GLFW_PLATFORM_ERROR, "X11/GLX: Cannot find known "
                                           "GLXFBConfig by ID. This cannot "
                                           "happen. Have a nice day.\n");
        abort();
    }

    // There is no clear definition of an "accelerated" context on X11/GLX, and
    // true sounds better than false, so we hardcode true here
    window->accelerated = GL_TRUE;

    window->redBits = getFBConfigAttrib(window, *fbconfig, GLX_RED_SIZE);
    window->greenBits = getFBConfigAttrib(window, *fbconfig, GLX_GREEN_SIZE);
    window->blueBits = getFBConfigAttrib(window, *fbconfig, GLX_BLUE_SIZE);

    window->alphaBits = getFBConfigAttrib(window, *fbconfig, GLX_ALPHA_SIZE);
    window->depthBits = getFBConfigAttrib(window, *fbconfig, GLX_DEPTH_SIZE);
    window->stencilBits = getFBConfigAttrib(window, *fbconfig, GLX_STENCIL_SIZE);

    window->accumRedBits = getFBConfigAttrib(window, *fbconfig, GLX_ACCUM_RED_SIZE);
    window->accumGreenBits = getFBConfigAttrib(window, *fbconfig, GLX_ACCUM_GREEN_SIZE);
    window->accumBlueBits = getFBConfigAttrib(window, *fbconfig, GLX_ACCUM_BLUE_SIZE);
    window->accumAlphaBits = getFBConfigAttrib(window, *fbconfig, GLX_ACCUM_ALPHA_SIZE);

    window->auxBuffers = getFBConfigAttrib(window, *fbconfig, GLX_AUX_BUFFERS);
    window->stereo = getFBConfigAttrib(window, *fbconfig, GLX_STEREO) ? GL_TRUE : GL_FALSE;

    // Get FSAA buffer sample count
    if (_glfwLibrary.GLX.ARB_multisample)
        window->samples = getFBConfigAttrib(window, *fbconfig, GLX_SAMPLES);
    else
        window->samples = 0;

    XFree(fbconfig);
}


//========================================================================
// Create the actual OpenGL context
//========================================================================

#define setGLXattrib(attribs, index, attribName, attribValue) \
    attribs[index++] = attribName; \
    attribs[index++] = attribValue;

static int createContext(_GLFWwindow* window,
                         const _GLFWwndconfig* wndconfig,
                         GLXFBConfigID fbconfigID)
{
    int attribs[40];
    int dummy, index;
    GLXFBConfig* fbconfig;
    GLXContext share = NULL;

    if (wndconfig->share)
        share = wndconfig->share->GLX.context;

    // Retrieve the previously selected GLXFBConfig
    {
        index = 0;

        setGLXattrib(attribs, index, GLX_FBCONFIG_ID, (int) fbconfigID);
        setGLXattrib(attribs, index, None, None);

        if (_glfwLibrary.GLX.SGIX_fbconfig)
        {
            fbconfig = _glfwLibrary.GLX.ChooseFBConfigSGIX(_glfwLibrary.X11.display,
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
                          "X11/GLX: Failed to retrieve the selected GLXFBConfig");
            return GL_FALSE;
        }
    }

    // Retrieve the corresponding visual
    if (_glfwLibrary.GLX.SGIX_fbconfig)
    {
        window->GLX.visual = _glfwLibrary.GLX.GetVisualFromFBConfigSGIX(_glfwLibrary.X11.display,
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
                      "X11/GLX: Failed to retrieve visual for GLXFBConfig");
        return GL_FALSE;
    }

    if (_glfwLibrary.GLX.ARB_create_context)
    {
        index = 0;

        if (wndconfig->glMajor != 1 || wndconfig->glMinor != 0)
        {
            // Request an explicitly versioned context

            setGLXattrib(attribs, index, GLX_CONTEXT_MAJOR_VERSION_ARB, wndconfig->glMajor);
            setGLXattrib(attribs, index, GLX_CONTEXT_MINOR_VERSION_ARB, wndconfig->glMinor);
        }

        if (wndconfig->glForward || wndconfig->glDebug || wndconfig->glRobustness)
        {
            int flags = 0;

            if (wndconfig->glForward)
                flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

            if (wndconfig->glDebug)
                flags |= GLX_CONTEXT_DEBUG_BIT_ARB;

            if (wndconfig->glRobustness)
                flags |= GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB;

            setGLXattrib(attribs, index, GLX_CONTEXT_FLAGS_ARB, flags);
        }

        if (wndconfig->glProfile)
        {
            int flags = 0;

            if (!_glfwLibrary.GLX.ARB_create_context_profile)
            {
                _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                              "X11/GLX: An OpenGL profile requested but "
                              "GLX_ARB_create_context_profile is unavailable");
                return GL_FALSE;
            }

            if (wndconfig->glProfile == GLFW_OPENGL_ES2_PROFILE &&
                !_glfwLibrary.GLX.EXT_create_context_es2_profile)
            {
                _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                              "X11/GLX: OpenGL ES 2.x profile requested but "
                              "GLX_EXT_create_context_es2_profile is unavailable");
                return GL_FALSE;
            }

            if (wndconfig->glProfile == GLFW_OPENGL_CORE_PROFILE)
                flags = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
            else if (wndconfig->glProfile == GLFW_OPENGL_COMPAT_PROFILE)
                flags = GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            else if (wndconfig->glProfile == GLFW_OPENGL_ES2_PROFILE)
                flags = GLX_CONTEXT_ES2_PROFILE_BIT_EXT;

            setGLXattrib(attribs, index, GLX_CONTEXT_PROFILE_MASK_ARB, flags);
        }

        if (wndconfig->glRobustness)
        {
            int strategy;

            if (!_glfwLibrary.GLX.ARB_create_context_robustness)
            {
                _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                              "X11/GLX: An OpenGL robustness strategy was "
                              "requested but GLX_ARB_create_context_robustness "
                              "is unavailable");
                return GL_FALSE;
            }

            if (wndconfig->glRobustness == GLFW_OPENGL_NO_RESET_NOTIFICATION)
                strategy = GLX_NO_RESET_NOTIFICATION_ARB;
            else if (wndconfig->glRobustness == GLFW_OPENGL_LOSE_CONTEXT_ON_RESET)
                strategy = GLX_LOSE_CONTEXT_ON_RESET_ARB;

            setGLXattrib(attribs,
                         index,
                         GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
                         strategy);
        }

        setGLXattrib(attribs, index, None, None);

        // This is the only place we set an Xlib error handler, and we only do
        // it because glXCreateContextAttribsARB generates a BadMatch error if
        // the requested OpenGL version is unavailable (instead of a civilized
        // response like returning NULL)
        XSetErrorHandler(errorHandler);

        window->GLX.context =
            _glfwLibrary.GLX.CreateContextAttribsARB(_glfwLibrary.X11.display,
                                                     *fbconfig,
                                                     share,
                                                     True,
                                                     attribs);

        // We are done, so unset the error handler again (see above)
        XSetErrorHandler(NULL);
    }
    else
    {
        if (_glfwLibrary.GLX.SGIX_fbconfig)
        {
            window->GLX.context =
                _glfwLibrary.GLX.CreateContextWithConfigSGIX(_glfwLibrary.X11.display,
                                                             *fbconfig,
                                                             GLX_RGBA_TYPE,
                                                             share,
                                                             True);
        }
        else
        {
            window->GLX.context = glXCreateNewContext(_glfwLibrary.X11.display,
                                                      *fbconfig,
                                                      GLX_RGBA_TYPE,
                                                      share,
                                                      True);
        }
    }

    XFree(fbconfig);

    if (window->GLX.context == NULL)
    {
        // TODO: Handle all the various error codes here

        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "X11/GLX: Failed to create OpenGL context");
        return GL_FALSE;
    }

    refreshContextParams(window, fbconfigID);

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
        _glfwSetError(GLFW_PLATFORM_ERROR, "X11/GLX: Failed to find libGL");
        return GL_FALSE;
    }
#endif

    // Check if GLX is supported on this display
    if (!glXQueryExtension(_glfwLibrary.X11.display, NULL, NULL))
    {
        _glfwSetError(GLFW_OPENGL_UNAVAILABLE, "X11/GLX: GLX supported not found");
        return GL_FALSE;
    }

    if (!glXQueryVersion(_glfwLibrary.X11.display,
                         &_glfwLibrary.GLX.majorVersion,
                         &_glfwLibrary.GLX.minorVersion))
    {
        _glfwSetError(GLFW_OPENGL_UNAVAILABLE,
                      "X11/GLX: Failed to query GLX version");
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
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "X11/GLX: No usable GLXFBConfigs found");
            return GL_FALSE;
        }

        result = _glfwChooseFBConfig(fbconfig, fbconfigs, fbcount);
        if (!result)
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "X11/GLX: No GLXFBConfig matched the criteria");

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
        // Release and destroy the context
        glXMakeCurrent(_glfwLibrary.X11.display, None, NULL);
        glXDestroyContext(_glfwLibrary.X11.display, window->GLX.context);
        window->GLX.context = NULL;
    }
}


//========================================================================
// Return the X visual associated with the specified context
//========================================================================

XVisualInfo* _glfwGetContextVisual(_GLFWwindow* window)
{
    return window->GLX.visual;
}


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
}


//========================================================================
// Swap OpenGL buffers
//========================================================================

void _glfwPlatformSwapBuffers(void)
{
    glXSwapBuffers(_glfwLibrary.X11.display,
                   _glfwLibrary.currentWindow->X11.handle);
}


//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval(int interval)
{
    _GLFWwindow* window = _glfwLibrary.currentWindow;

    if (_glfwLibrary.GLX.EXT_swap_control)
    {
        _glfwLibrary.GLX.SwapIntervalEXT(_glfwLibrary.X11.display,
                                         window->X11.handle,
                                         interval);
    }
    else if (_glfwLibrary.GLX.SGI_swap_control)
        _glfwLibrary.GLX.SwapIntervalSGI(interval);
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

void* _glfwPlatformGetProcAddress(const char* procname)
{
    return (void*) _glfw_glXGetProcAddress((const GLubyte*) procname);
}


//========================================================================
// Copies the specified OpenGL state categories from src to dst
//========================================================================

void _glfwPlatformCopyContext(_GLFWwindow* src, _GLFWwindow* dst, unsigned long mask)
{
    glXCopyContext(_glfwLibrary.X11.display,
                   src->GLX.context,
                   dst->GLX.context,
                   mask);
}

