//========================================================================
// GLFW 3.4 WGL - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
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
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#include "internal.h"

#include <stdlib.h>

#ifndef GL4ES
#include <proto/ogles2.h>
#include <GLES2/gl2.h>
#define GETPROCADDRESS getProcAddressGL
#else
enum CreateContextTags {
        OGLES2_CCT_MIN=(1UL<<31),
        OGLES2_CCT_WINDOW,
        OGLES2_CCT_MODEID,
        OGLES2_CCT_DEPTH,
        OGLES2_CCT_STENCIL,
        OGLES2_CCT_VSYNC,
        OGLES2_CCT_SINGLE_GET_ERROR_MODE,
        OGLES2_CCT_GET_WIDTH,
        OGLES2_CCT_GET_HEIGHT,
        OGLES2_CCT_BITMAP,
        OGLES2_CCT_SHADER_COMPAT_PATCH,
        OGLES2_CCT_CONTEXT_FOR_MODEID,
        OGLES2_CCT_RESIZE_VIEWPORT,
        OGLES2_CCT_DEBUG_SHADER_LOG,
};
typedef int GLint;
typedef int GLsizei;

extern void  aglMakeCurrent(void* context);
extern void  aglSetParams2(struct TagItem * tags);
extern void  aglDestroyContext(void* context);
extern void  aglMakeCurrent(void* context);
extern void  aglSwapBuffers();
extern void  glFinish();
extern void  glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
extern void *aglCreateContext2(ULONG * errcode, struct TagItem * tags);
extern void *gl4es_aglGetProcAddress(const char *name);
extern void gl4es_glXSwapInterval(int interval);
#define GETPROCADDRESS gl4es_aglGetProcAddress

extern struct OGLES2IFace *IOGLES2;
#endif

static void makeContextCurrentGL(_GLFWwindow* window)
{
    if (window)
    {
        aglMakeCurrent(window->context.gl.glContext);

        _glfwPlatformSetTls(&_glfw.contextSlot, window);
    }
}

static void destroyContextGL(_GLFWwindow* window) {
    if (window->context.gl.glContext) {
        printf("Destroying context %p for window handle %p\n", window->context.gl.glContext, window->os4.handle);
        aglDestroyContext(window->context.gl.glContext);
    }
    window->context.gl.glContext = NULL;
}

static void swapBuffersGL(_GLFWwindow* window)
{
    // First flush the render pipeline, so that everything gets drawn
    glFinish();

    // Swap the buffers (if any)
    aglSwapBuffers();
}

static GLFWglproc getProcAddressGL(const char* procname)
{
    const GLFWglproc proc = (GLFWglproc) aglGetProcAddress(procname);
    return proc;
}

static int extensionSupportedGL(const char* extension)
{
    // TODO - Implement this
    return GLFW_FALSE;
}

static void swapIntervalGL(int interval)
{
    // TODO - Should we implement this?
}

// Create the OpenGL or OpenGL ES context
//
GLFWbool _glfwCreateContextGL(_GLFWwindow* window,
                               const _GLFWctxconfig* ctxconfig,
                               const _GLFWfbconfig* fbconfig)
{
    ULONG errCode;

    dprintf("redBits=%d\n", fbconfig->redBits);
    dprintf("greenBits=%d\n", fbconfig->greenBits);
    dprintf("blueBits=%d\n", fbconfig->blueBits);
    dprintf("alphaBits=%d\n", fbconfig->alphaBits);
    dprintf("depthBits=%d\n", fbconfig->depthBits);
    dprintf("stencilBits=%d\n", fbconfig->stencilBits);
    dprintf("accumRedBits=%d\n", fbconfig->accumRedBits);
    dprintf("accumGreenBits=%d\n", fbconfig->accumGreenBits);
    dprintf("accumBlueBits=%d\n", fbconfig->accumBlueBits);
    dprintf("accumAlphaBits=%d\n", fbconfig->accumAlphaBits);
    dprintf("auxBuffers=%d\n", fbconfig->auxBuffers);

    struct TagItem contextparams[] =
    {
            {OGLES2_CCT_WINDOW, (ULONG)window->os4.handle},
            {OGLES2_CCT_DEPTH, fbconfig->depthBits},
            {OGLES2_CCT_STENCIL, fbconfig->stencilBits},
            {OGLES2_CCT_VSYNC, 0},
            {OGLES2_CCT_RESIZE_VIEWPORT, TRUE},
            {TAG_DONE, 0}
    };

    window->context.gl.glContext = (void *)aglCreateContext2(&errCode, contextparams);

    /* Set the context as current */
    if (window->context.gl.glContext) {
        aglMakeCurrent(window->context.gl.glContext);
    }
    else {
        IIntuition->CloseWindow(window->os4.handle);
        window->os4.handle = NULL;
        return GLFW_FALSE;
    }
    printf("Creating context %p for window handle %p\n", window->context.gl.glContext, window->os4.handle);

    window->context.makeCurrent = makeContextCurrentGL;
    window->context.swapBuffers = swapBuffersGL;
    window->context.swapInterval = swapIntervalGL;
    window->context.extensionSupported = extensionSupportedGL;
    window->context.getProcAddress = GETPROCADDRESS;
    window->context.destroy = destroyContextGL;

    return GLFW_TRUE;
}

