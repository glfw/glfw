//========================================================================
// GLFW 3.0 WGL - www.glfw.org
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

#ifndef _wgl_platform_h_
#define _wgl_platform_h_

// This path may need to be changed if you build GLFW using your own setup
// We ship and use our own copy of wglext.h since GLFW uses fairly new
// extensions and not all operating systems come with an up-to-date version
#include "../deps/GL/wglext.h"


#define _GLFW_PLATFORM_FBCONFIG             int             wgl
#define _GLFW_PLATFORM_CONTEXT_STATE        _GLFWcontextWGL wgl
#define _GLFW_PLATFORM_LIBRARY_OPENGL_STATE _GLFWlibraryWGL wgl


//========================================================================
// GLFW platform specific types
//========================================================================

//------------------------------------------------------------------------
// Platform-specific OpenGL context structure
//------------------------------------------------------------------------
typedef struct _GLFWcontextWGL
{
    // Platform specific window resources
    HDC       dc;              // Private GDI device context
    HGLRC     context;         // Permanent rendering context

    // Platform specific extensions (context specific)
    PFNWGLSWAPINTERVALEXTPROC           SwapIntervalEXT;
    PFNWGLGETPIXELFORMATATTRIBIVARBPROC GetPixelFormatAttribivARB;
    PFNWGLGETEXTENSIONSSTRINGEXTPROC    GetExtensionsStringEXT;
    PFNWGLGETEXTENSIONSSTRINGARBPROC    GetExtensionsStringARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC   CreateContextAttribsARB;
    GLboolean                           EXT_swap_control;
    GLboolean                           ARB_multisample;
    GLboolean                           ARB_framebuffer_sRGB;
    GLboolean                           ARB_pixel_format;
    GLboolean                           ARB_create_context;
    GLboolean                           ARB_create_context_profile;
    GLboolean                           EXT_create_context_es2_profile;
    GLboolean                           ARB_create_context_robustness;
} _GLFWcontextWGL;


//------------------------------------------------------------------------
// Platform-specific library global data for WGL
//------------------------------------------------------------------------
typedef struct _GLFWlibraryWGL
{
    GLboolean       hasTLS;
    DWORD           current;

    // opengl32.dll
    struct {
        HINSTANCE   instance;
    } opengl32;

} _GLFWlibraryWGL;


#endif // _wgl_platform_h_
