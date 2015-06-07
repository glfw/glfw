//========================================================================
// GLFW 3.1 EGL - www.glfw.org
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

#ifndef _glfw3_egl_context_h_
#define _glfw3_egl_context_h_

#include <EGL/egl.h>

// This path may need to be changed if you build GLFW using your own setup
// We ship and use our own copy of eglext.h since GLFW uses fairly new
// extensions and not all operating systems come with an up-to-date version
#include "../deps/EGL/eglext.h"

#define _GLFW_PLATFORM_FBCONFIG                 EGLConfig       egl
#define _GLFW_PLATFORM_CONTEXT_STATE            _GLFWcontextEGL egl
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE    _GLFWlibraryEGL egl


// EGL-specific per-context data
//
typedef struct _GLFWcontextEGL
{
   EGLConfig      config;
   EGLContext     context;
   EGLSurface     surface;

#if defined(_GLFW_X11)
   XVisualInfo*   visual;
#endif

} _GLFWcontextEGL;


// EGL-specific global data
//
typedef struct _GLFWlibraryEGL
{
    EGLDisplay      display;
    EGLint          major, minor;

    GLboolean       KHR_create_context;

} _GLFWlibraryEGL;


int _glfwInitContextAPI(void);
void _glfwTerminateContextAPI(void);
int _glfwCreateContext(_GLFWwindow* window,
                       const _GLFWctxconfig* ctxconfig,
                       const _GLFWfbconfig* fbconfig);
void _glfwDestroyContext(_GLFWwindow* window);
int _glfwAnalyzeContext(const _GLFWwindow* window,
                        const _GLFWctxconfig* ctxconfig,
                        const _GLFWfbconfig* fbconfig);

#endif // _glfw3_egl_context_h_
