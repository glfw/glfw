//========================================================================
// GLFW 3.0 EGL - www.glfw.org
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

#ifndef _egl_platform_h_
#define _egl_platform_h_

#include <EGL/egl.h>

// This path may need to be changed if you build GLFW using your own setup
// We ship and use our own copy of eglext.h since GLFW uses fairly new
// extensions and not all operating systems come with an up-to-date version
#include "../deps/EGL/eglext.h"

// Do we have support for dlopen/dlsym?
#if defined(_GLFW_HAS_DLOPEN)
 #include <dlfcn.h>
#endif

#define _GLFW_PLATFORM_FBCONFIG             EGLConfig       egl
#define _GLFW_PLATFORM_CONTEXT_STATE        _GLFWcontextEGL egl
#define _GLFW_PLATFORM_LIBRARY_OPENGL_STATE _GLFWlibraryEGL egl


//========================================================================
// GLFW platform specific types
//========================================================================

//------------------------------------------------------------------------
// Platform-specific OpenGL context structure
//------------------------------------------------------------------------
typedef struct _GLFWcontextEGL
{
   EGLConfig      config;
   EGLContext     context;
   EGLSurface     surface;

#if defined(_GLFW_X11)
   XVisualInfo*   visual;
#endif
} _GLFWcontextEGL;


//------------------------------------------------------------------------
// Platform-specific library global data for EGL
//------------------------------------------------------------------------
typedef struct _GLFWlibraryEGL
{
    EGLDisplay      display;
    EGLint          versionMajor, versionMinor;

    GLboolean       KHR_create_context;

} _GLFWlibraryEGL;


#endif // _egl_platform_h_
