//========================================================================
// GLFW 3.5 OGC - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2024 Alberto Mardegan <info@mardy.it>
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opengx.h>
#ifdef __wii__
#include <wiiuse/wpad.h>
#endif

#ifdef __wii__
static void drawCursorRect(_GLFWcursor* cursor)
{
    short xhot = cursor->ogc.xhot;
    short yhot = cursor->ogc.yhot;
    u16 width = GX_GetTexObjWidth(&cursor->ogc.texobj);
    u16 height = GX_GetTexObjHeight(&cursor->ogc.texobj);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position2s16(-xhot, -yhot);
    GX_TexCoord2u8(0, 0);
    GX_Position2s16(width - xhot, -yhot);
    GX_TexCoord2u8(1, 0);
    GX_Position2s16(width - xhot, height - yhot);
    GX_TexCoord2u8(1, 1);
    GX_Position2s16(-xhot, height - yhot);
    GX_TexCoord2u8(0, 1);
    GX_End();
}

static void drawCursor(_GLFWwindow* window)
{
    Mtx44 proj;
    Mtx mv;
    _GLFWcursor* cursor = window->ogc.currentCursor;

    if (!cursor) return;

    GX_LoadTexObj(&cursor->ogc.texobj, GX_TEXMAP0);

    int width = window->ogc.width;
    int height = window->ogc.height;

    guOrtho(proj, 0, height, 0, width, 0, 1);
    GX_LoadProjectionMtx(proj, GX_ORTHOGRAPHIC);

    guMtxIdentity(mv);
    guMtxScaleApply(mv, mv,
                    width / 640.0f,
                    height / 480.0f, 1.0f);
    if (cursor->ogc.canRotate) {
        Mtx rot;
        float angle;
        WPADData *data = WPAD_Data(0);

        angle = data->ir.angle;
        guMtxRotDeg(rot, 'z', angle);
        guMtxConcat(mv, rot, mv);
    }
    guMtxTransApply(mv, mv,
                    window->virtualCursorPosX,
                    window->virtualCursorPosY, 0);
    GX_LoadPosMtxImm(mv, GX_PNMTX1);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetNumTevStages(1);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

    GX_SetNumTexGens(1);
    GX_SetCurrentMtx(GX_PNMTX1);
    GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);
    GX_SetScissor(0, 0, width, height);
    drawCursorRect(cursor);
    GX_SetCurrentMtx(GX_PNMTX0);
    GX_DrawDone();
}
#endif // __wii__

static void makeContextCurrentOgc(_GLFWwindow* window)
{
    _glfwPlatformSetTls(&_glfw.contextSlot, window);
}

static GLFWglproc getProcAddressOgc(const char* procname)
{
    return (GLFWglproc) ogx_get_proc_address(procname);
}

static void destroyContextOgc(_GLFWwindow* window)
{
}

static void swapBuffersOgc(_GLFWwindow* window)
{
    void *xfb;
    u8 mustClear, mustWait;

    if (ogx_prepare_swap_buffers() < 0) return;

#ifdef __wii__
    if (window->ogc.hovered &&
        (window->callbacks.mouseButton || window->callbacks.cursorPos))
        drawCursor(window);
#endif // __wii__

    if (window->doublebuffer) {
        mustClear = GX_TRUE;
        mustWait = GX_TRUE;
        xfb = _glfw.ogc.xfb[_glfw.ogc.fbIndex];
        _glfw.ogc.fbIndex ^= 1;
    } else {
        mustClear = GX_FALSE;
        mustWait = GX_FALSE;
        xfb = _glfw.ogc.xfb[0];
    }
    GX_CopyDisp(xfb, mustClear);
    GX_DrawDone();
    GX_Flush();

    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_Flush();
    if (mustWait)
        VIDEO_WaitVSync();
}

static void swapIntervalOgc(int interval)
{
    // No swap interval on Ogc
}

static int extensionSupportedOgc(const char* extension)
{
    const char *extensions = (const char*)glGetString(GL_EXTENSIONS);
    return strstr(extensions, extensions) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwCreateContextOgc(_GLFWwindow* window,
                               const _GLFWctxconfig* ctxconfig,
                               const _GLFWfbconfig* fbconfig)
{
    if (ctxconfig->client == GLFW_OPENGL_ES_API)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "Ogc: OpenGL ES is not available on Ogc");
        return GLFW_FALSE;
    }

    ogx_enable_double_buffering(fbconfig->doublebuffer);
    if (fbconfig->stencilBits > 0) {
        OgxStencilFlags stencilFlags = OGX_STENCIL_NONE;
        if (fbconfig->stencilBits > 4) stencilFlags |= OGX_STENCIL_8BIT;
        ogx_stencil_create(stencilFlags);
    }

    window->context.makeCurrent = makeContextCurrentOgc;
    window->context.swapBuffers = swapBuffersOgc;
    window->context.swapInterval = swapIntervalOgc;
    window->context.extensionSupported = extensionSupportedOgc;
    window->context.getProcAddress = getProcAddressOgc;
    window->context.destroy = destroyContextOgc;

    return GLFW_TRUE;
}

