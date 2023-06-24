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
#include <stdio.h>

#ifndef OGLES2_OGLES2_DEFS_H
// it would be better to have an include with only the CreateContextTags enum difed, to avoid conflict
//  of other typedef with full OpenGL header file...
#include <ogles2/ogles2_defs.h>
#endif

void* aglCreateContext2(ULONG * errcode, struct TagItem * tags);
void aglDestroyContext(void* context);
void aglMakeCurrent(void* context);
void aglSwapBuffers();
void aglSetBitmap(struct BitMap *bitmap);
void aglSetParams2(struct TagItem * tags);
void* aglGetProcAddress(const char* name);
#define GETPROCADDRESS aglGetProcAddress

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
        aglDestroyContext(window->context.gl.glContext);
    }
    window->context.gl.glContext = NULL;
}

static void swapBuffersGL(_GLFWwindow* window)
{
    // First flush the render pipeline, so that everything gets drawn
    glFinish();

    if (window->context.gl.vsyncEnabled) {
        IGraphics->WaitTOF();
    }
    
    // Swap the buffers (if any)
    aglSwapBuffers();
}

static GLFWglproc getProcAddressGL(const char* procname)
{
    dprintf("Searching for %s\n", procname);
    const GLFWglproc proc = (GLFWglproc) GETPROCADDRESS(procname);
    return proc;
}

static int extensionSupportedGL(const char* extension)
{
    // TODO - Implement this
    return GLFW_FALSE;
}

static void swapIntervalGL(int interval)
{
    _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);

     switch (interval) {
        case 0:
        case 1:
            window->context.gl.vsyncEnabled = interval ? TRUE : FALSE;
            dprintf("VSYNC %d\n", interval);
        default:
            dprintf("Unsupported interval %d\n", interval);
    }
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

    void *sharedContext = NULL;
    if (ctxconfig->share != NULL) {
        sharedContext = ctxconfig->share->context.gl.glContext;
    }
    
    dprintf("sharedContext = %p\n", sharedContext);
    struct TagItem contextparams[] =
    {
            {OGLES2_CCT_WINDOW, (ULONG)window->os4.handle},
            {OGLES2_CCT_DEPTH, fbconfig->depthBits},
            {OGLES2_CCT_STENCIL, fbconfig->stencilBits},
            {OGLES2_CCT_VSYNC, 0},
            {OGLES2_CCT_RESIZE_VIEWPORT, TRUE},
            {OGLES2_CCT_SINGLE_GET_ERROR_MODE, 1},
            {OGLES2_CCT_SHARE_WITH, (ULONG)sharedContext},
            {TAG_DONE, 0}
    };

    window->context.gl.glContext = (void *)aglCreateContext2(&errCode, contextparams);
    dprintf("firstContext = %p\n", window->context.gl.glContext);

    /* Set the context as current */
    if (window->context.gl.glContext) {
        window->context.client = GLFW_OPENGL_ES_API;

        dprintf("GL Extensions: %s\n", glGetString(GL_EXTENSIONS));
        aglMakeCurrent(window->context.gl.glContext);

        // Some games (like q3) doesn't clear the z-buffer prior to use. Since we're using a floating-point depth buffer in warp3dnova,
        // that means it may contain illegal floating-point values, which causes some pixels to fail the depth-test when they shouldn't,
        // so we clear the depth buffer to a constant value when it's first created.
        // Pandora may well use an integer depth-buffer, in which case this can't happen.
        // On MiniGL it didn't happens as there is workaround inside of old warp3d (and probabaly inside of MiniGL itself too).
        // in SDL1 with gl4es (so warp3dnova/ogles2, where no such workaround) it didn't happens probabaly because SDL1 doing something like that (but not glClear).

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, window->os4.width, window->os4.height);        
    }
    else {
        IIntuition->CloseWindow(window->os4.handle);
        window->os4.handle = NULL;
        return GLFW_FALSE;
    }
    dprintf("Creating context %p for window handle %p\n", window->context.gl.glContext, window->os4.handle);

    window->context.makeCurrent = makeContextCurrentGL;
    window->context.swapBuffers = swapBuffersGL;
    window->context.swapInterval = swapIntervalGL;
    window->context.extensionSupported = extensionSupportedGL;
    window->context.getProcAddress = getProcAddressGL;
    window->context.destroy = destroyContextGL;

    return GLFW_TRUE;
}

