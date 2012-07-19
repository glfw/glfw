//========================================================================
// GLFW - An OpenGL library
// Platform:    X11/EGL
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

#ifndef _x11_egl_platform_h_
#define _x11_egl_platform_h_

#include <EGL/egl.h>

// Do we have support for dlopen/dlsym?
#if defined(_GLFW_HAS_DLOPEN)
 #include <dlfcn.h>
#endif

// We support two different ways for getting addresses for EGL
// extension functions: eglGetProcAddress and dlsym
#if defined(_GLFW_HAS_EGLGETPROCADDRESS)
 #define _glfw_eglGetProcAddress(x) eglGetProcAddress(x)
#elif defined(_GLFW_HAS_DLOPEN)
 #define _glfw_eglGetProcAddress(x) dlsym(_glfwLibrary.EGL.libEGL, x)
 #define _GLFW_DLOPEN_LIBEGL
#else
 #error "No OpenGL entry point retrieval mechanism was enabled"
#endif

#define _GLFW_PLATFORM_CONTEXT_STATE _GLFWcontextEGL EGL
#define _GLFW_PLATFORM_LIBRARY_OPENGL_STATE _GLFWlibraryEGL EGL


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
   XVisualInfo*   visual;
} _GLFWcontextEGL;


//------------------------------------------------------------------------
// Platform-specific library global data for EGL
//------------------------------------------------------------------------
typedef struct _GLFWlibraryEGL
{
    EGLDisplay      display;
    EGLint          majorVersion, minorVersion;

#if defined(_GLFW_DLOPEN_LIBEGL)
    void*           libEGL;  // dlopen handle for libEGL.so
#endif
} _GLFWlibraryEGL;


#endif // _x11_egl_platform_h_
