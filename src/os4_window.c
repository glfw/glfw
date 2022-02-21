//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2019 Camilla Löwy <elmindreda@glfw.org>
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <proto/intuition.h>
#include <intuition/pointerclass.h>

#define MIN_WINDOW_SIZE 100

static void OS4_CopyIdcmpMessage(struct IntuiMessage * src, struct MyIntuiMessage * dst);
static _GLFWwindow *OS4_FindWindow(struct Window * syswin);
static uint32_t *OS4_CopyImageData(const GLFWimage *surface);
static int OS4_GetButtonState(uint16_t code);
static int OS4_GetButton(uint16_t code);
static char OS4_TranslateUnicode(uint16_t code, uint32_t qualifier);
static int OS4_TranslateState(int state);
static uint32_t OS4_GetWindowFlags(_GLFWwindow* window, BOOL fullscreen);
static void OS4_SetWindowLimits(_GLFWwindow * window, struct Window * syswin);
static void OS4_CreateIconifyGadget(_GLFWwindow * window);
static struct DiskObject *OS4_GetDiskObject();
static void OS4_HandleAppIcon(struct AppMessage * msg);
static ULONG OS4_BackFill(const struct Hook *hook, struct RastPort *rastport, struct BackFillMessage *message);



static UWORD fallbackPointerData[2 * 16] = { 0 };

static struct BitMap fallbackPointerBitMap = { 2, 16, 0, 2, 0,
    { (PLANEPTR)fallbackPointerData, (PLANEPTR)(fallbackPointerData + 16) } };

static struct Hook OS4_BackFillHook = {
    {0, 0},       /* h_MinNode */
    (void *)OS4_BackFill, /* h_Entry */
    0,            /* h_SubEntry */
    0             /* h_Data */
};

static void applySizeLimits(_GLFWwindow* window, int* width, int* height)
{
    if (window->numer != GLFW_DONT_CARE && window->denom != GLFW_DONT_CARE)
    {
        const float ratio = (float) window->numer / (float) window->denom;
        *height = (int) (*width / ratio);
    }

    if (window->minwidth != GLFW_DONT_CARE && *width < window->minwidth)
        *width = window->minwidth;
    else if (window->maxwidth != GLFW_DONT_CARE && *width > window->maxwidth)
        *width = window->maxwidth;

    if (window->minheight != GLFW_DONT_CARE && *height < window->minheight)
        *height = window->minheight;
    else if (window->maxheight != GLFW_DONT_CARE && *height > window->maxheight)
        *height = window->maxheight;
}

static void fitToMonitor(_GLFWwindow* window)
{
    GLFWvidmode mode;
    _glfwGetVideoModeOS4(window->monitor, &mode);
    _glfwGetMonitorPosOS4(window->monitor,
                           &window->os4.xpos,
                           &window->os4.ypos);
    window->os4.width = mode.width;
    window->os4.height = mode.height;
}

static void acquireMonitor(_GLFWwindow* window)
{
    _glfwInputMonitorWindow(window->monitor, window);
}

static void releaseMonitor(_GLFWwindow* window)
{
    if (window->monitor->window != window)
        return;

    _glfwInputMonitorWindow(window->monitor, NULL);
}

static int createNativeWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWfbconfig* fbconfig,
                              int windowType)
{
    if (window->monitor) {
        printf("fitToMonitor\n");
        fitToMonitor(window);
    }
    else
    {
        window->os4.xpos = 17;
        window->os4.ypos = 17;
        window->os4.width = wndconfig->width;
        window->os4.height = wndconfig->height;
    }

    window->os4.visible = wndconfig->visible;
    window->os4.decorated = wndconfig->decorated;
    window->os4.maximized = wndconfig->maximized;
    window->os4.floating = wndconfig->floating;
    if (fbconfig != NULL)
        window->os4.transparent = fbconfig->transparent;
    else
        window->os4.transparent = 0;
    window->os4.opacity = 1.f;

    window->os4.windowType = windowType;
    uint32_t windowFlags = OS4_GetWindowFlags(window, FALSE);

    OS4_BackFillHook.h_Data = IGraphics; // Smuggle interface ptr for the hook

    window->os4.handle = IIntuition->OpenWindowTags(NULL,
            WA_Left,              window->os4.xpos,
            WA_Top,               window->os4.ypos,
            WA_InnerWidth,        wndconfig->width,
            WA_InnerHeight,       wndconfig->height,
            WA_Title,             wndconfig->title,
            WA_ScreenTitle,       wndconfig->title,
            WA_Flags,             windowFlags,
            WA_IDCMP,             IDCMP_CLOSEWINDOW |
                                  IDCMP_MOUSEMOVE |
                                  IDCMP_MOUSEBUTTONS |
                                  IDCMP_EXTENDEDMOUSE |
                                  IDCMP_RAWKEY |
                                  IDCMP_NEWSIZE |
                                  IDCMP_DELTAMOVE |
                                  IDCMP_ACTIVEWINDOW |
                                  IDCMP_INACTIVEWINDOW |
                                  IDCMP_INTUITICKS |
                                  IDCMP_GADGETUP |
                                  IDCMP_CHANGEWINDOW,
            WA_Hidden,            !wndconfig->visible,
            WA_UserPort,          _glfw.os4.userPort,
            WA_BackFill,          &OS4_BackFillHook,
            TAG_DONE);

    /* If we have a valid handle return GLFW_TRUE */
    if (window->os4.handle) {
        window->os4.title = wndconfig->title;

        _glfwGetWindowPosOS4(window, &window->os4.xpos, &window->os4.ypos);
        _glfwGetWindowSizeOS4(window, &window->os4.width, &window->os4.height);        
        window->os4.lastCursorPosX = window->os4.xpos;
        window->os4.lastCursorPosY = window->os4.ypos;

        if (wndconfig->decorated && wndconfig->width > 99 && wndconfig->height) {
            OS4_CreateIconifyGadget(window);
        }
        window->os4.appWin = IWorkbench->AddAppWindow(0, (ULONG)window, window->os4.handle, _glfw.os4.appMsgPort, TAG_DONE);
        
        if (wndconfig->autoIconify) {
            OS4_IconifyWindow(window);
        }

        return GLFW_TRUE;
    }
    else
        return GLFW_FALSE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwCreateWindowOS4(_GLFWwindow* window,
                          const _GLFWwndconfig* wndconfig,
                          const _GLFWctxconfig* ctxconfig,
                          const _GLFWfbconfig* fbconfig)
{
    if (!createNativeWindow(window, wndconfig, fbconfig, ctxconfig->client))
        return GLFW_FALSE;
    
    if (ctxconfig->client != GLFW_NO_API)
    {
        if (!_glfwCreateContextGL(window, ctxconfig, fbconfig))
            return GLFW_FALSE;
    }

    if (window->monitor)
    {
        _glfwShowWindowOS4(window);
        _glfwFocusWindowOS4(window);
        acquireMonitor(window);
    }

    return GLFW_TRUE;
}

void _glfwDestroyWindowOS4(_GLFWwindow* window)
{
    if (window->context.destroy)
        window->context.destroy(window);

    if (window->os4.appWin) {
        IWorkbench->RemoveAppWindow(window->os4.appWin);
        window->os4.appWin = NULL;
    }

    if (window->os4.appIcon) {
        IWorkbench->RemoveAppIcon(window->os4.appIcon);
        window->os4.appIcon = NULL;
    }

    IIntuition->CloseWindow(window->os4.handle);

    if (window->os4.gadget) {
        IIntuition->DisposeObject((Object *)window->os4.gadget);
        window->os4.gadget = NULL;
    }

    if (window->os4.image) {
        IIntuition->DisposeObject((Object *)window->os4.image);
        window->os4.image = NULL;
    }

    if (window->monitor)
        releaseMonitor(window);

    if (_glfw.os4.focusedWindow == window)
        _glfw.os4.focusedWindow = NULL;
}

void _glfwSetWindowTitleOS4(_GLFWwindow* window, const char* title)
{
    IIntuition->SetWindowTitles(window->os4.handle, title, title);
}

void _glfwSetWindowIconOS4(_GLFWwindow* window, int count, const GLFWimage* images)
{
    // We don't have window's icon
}

void _glfwSetWindowMonitorOS4(_GLFWwindow* window,
                               _GLFWmonitor* monitor,
                               int xpos, int ypos,
                               int width, int height,
                               int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (!monitor)
        {
            _glfwSetWindowPosOS4(window, xpos, ypos);
            _glfwSetWindowSizeOS4(window, width, height);
        }

        return;
    }

    if (window->monitor)
        releaseMonitor(window);

    _glfwInputWindowMonitor(window, monitor);

    if (window->monitor)
    {
        window->os4.visible = GLFW_TRUE;
        acquireMonitor(window);
        fitToMonitor(window);
    }
    else
    {
        _glfwSetWindowPosOS4(window, xpos, ypos);
        _glfwSetWindowSizeOS4(window, width, height);
    }
}

void _glfwGetWindowPosOS4(_GLFWwindow* window, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = window->os4.xpos;
    if (ypos)
        *ypos = window->os4.ypos;
}

void _glfwSetWindowPosOS4(_GLFWwindow* window, int xpos, int ypos)
{
    if (window->monitor)
        return;

    if (window->os4.xpos != xpos || window->os4.ypos != ypos)
    {
        window->os4.xpos = xpos;
        window->os4.ypos = ypos;
        _glfwInputWindowPos(window, xpos, ypos);

        IIntuition->SetWindowAttrs(window->os4.handle,
            WA_Left, xpos,
            WA_Top, ypos,
            TAG_DONE);

        window->os4.lastCursorPosX = xpos;
        window->os4.lastCursorPosY = ypos;
    }
}

void _glfwGetWindowSizeOS4(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->os4.width;
    if (height)
        *height = window->os4.height;
}

void _glfwSetWindowSizeOS4(_GLFWwindow* window, int width, int height)
{
    if (window->monitor)
        return;

    if (window->os4.width != width || window->os4.height != height)
    {
        window->os4.width = width;
        window->os4.height = height;
        _glfwInputWindowSize(window, width, height);
        _glfwInputFramebufferSize(window, width, height);

        IIntuition->SetWindowAttrs(window->os4.handle,
            WA_Width, width,
            WA_Height, height,
            TAG_DONE);
    }
}

void _glfwSetWindowSizeLimitsOS4(_GLFWwindow* window,
                                  int minwidth, int minheight,
                                  int maxwidth, int maxheight)
{
    int width = window->os4.width;
    int height = window->os4.height;
    applySizeLimits(window, &width, &height);
    _glfwSetWindowSizeOS4(window, width, height);
}

void _glfwSetWindowAspectRatioOS4(_GLFWwindow* window, int n, int d)
{
    int width = window->os4.width;
    int height = window->os4.height;
    applySizeLimits(window, &width, &height);
    _glfwSetWindowSizeOS4(window, width, height);
}

void _glfwGetFramebufferSizeOS4(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width = window->os4.width;
    if (height)
        *height = window->os4.height;
}

void _glfwGetWindowFrameSizeOS4(_GLFWwindow* window,
                                 int* left, int* top,
                                 int* right, int* bottom)
{
    if (window->os4.decorated && !window->monitor)
    {
        if (left)
            *left = 1;
        if (top)
            *top = 10;
        if (right)
            *right = 1;
        if (bottom)
            *bottom = 1;
    }
    else
    {
        if (left)
            *left = 0;
        if (top)
            *top = 0;
        if (right)
            *right = 0;
        if (bottom)
            *bottom = 0;
    }
}

void _glfwGetWindowContentScaleOS4(_GLFWwindow* window, float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _glfwIconifyWindowOS4(_GLFWwindow* window)
{
    if (_glfw.os4.focusedWindow == window)
    {
        _glfw.os4.focusedWindow = NULL;
        _glfwInputWindowFocus(window, GLFW_FALSE);
    }

    if (!window->os4.iconified)
    {
        window->os4.iconified = GLFW_TRUE;
        _glfwInputWindowIconify(window, GLFW_TRUE);

        if (window->monitor)
            releaseMonitor(window);
    }
}

void _glfwRestoreWindowOS4(_GLFWwindow* window)
{
    if (window->os4.iconified)
    {
        window->os4.iconified = GLFW_FALSE;
        _glfwInputWindowIconify(window, GLFW_FALSE);

        if (window->monitor)
            acquireMonitor(window);
    }
    else if (window->os4.maximized)
    {
        window->os4.maximized = GLFW_FALSE;
        _glfwInputWindowMaximize(window, GLFW_FALSE);
    }
}

void _glfwMaximizeWindowOS4(_GLFWwindow* window)
{
    if (!window->os4.maximized)
    {
        window->os4.maximized = GLFW_TRUE;
        _glfwInputWindowMaximize(window, GLFW_TRUE);
    }
}

int _glfwWindowMaximizedOS4(_GLFWwindow* window)
{
    return window->os4.maximized;
}

int _glfwWindowHoveredOS4(_GLFWwindow* window)
{
    return _glfw.os4.xcursor >= window->os4.xpos &&
           _glfw.os4.ycursor >= window->os4.ypos &&
           _glfw.os4.xcursor <= window->os4.xpos + window->os4.width - 1 &&
           _glfw.os4.ycursor <= window->os4.ypos + window->os4.height - 1;
}

int _glfwFramebufferTransparentOS4(_GLFWwindow* window)
{
    return window->os4.transparent;
}

void _glfwSetWindowResizableOS4(_GLFWwindow* window, GLFWbool enabled)
{
    window->os4.resizable = enabled;
}

void _glfwSetWindowDecoratedOS4(_GLFWwindow* window, GLFWbool enabled)
{
    window->os4.decorated = enabled;
}

void _glfwSetWindowFloatingOS4(_GLFWwindow* window, GLFWbool enabled)
{
    window->os4.floating = enabled;
}

void _glfwSetWindowMousePassthroughOS4(_GLFWwindow* window, GLFWbool enabled)
{
}

float _glfwGetWindowOpacityOS4(_GLFWwindow* window)
{
    return window->os4.opacity;
}

void _glfwSetWindowOpacityOS4(_GLFWwindow* window, float opacity)
{
    window->os4.opacity = opacity;

    int iOpacity = (int)opacity * 255;

    IIntuition->SetWindowAttrs(window->os4.handle, WA_Opaqueness, iOpacity, TAG_DONE);

}

void _glfwSetRawMouseMotionOS4(_GLFWwindow *window, GLFWbool enabled)
{
}

GLFWbool _glfwRawMouseMotionSupportedOS4(void)
{
    return GLFW_TRUE;
}

void _glfwShowWindowOS4(_GLFWwindow* window)
{
    window->os4.visible = GLFW_TRUE;

    IIntuition->ShowWindow(window->os4.handle, NULL);
}

void _glfwRequestWindowAttentionOS4(_GLFWwindow* window)
{
}

void _glfwHideWindowOS4(_GLFWwindow* window)
{
    if (_glfw.os4.focusedWindow == window)
    {
        _glfw.os4.focusedWindow = NULL;
        _glfwInputWindowFocus(window, GLFW_FALSE);
    }

    window->os4.visible = GLFW_FALSE;

    IIntuition->HideWindow(window->os4.handle);
}

void _glfwFocusWindowOS4(_GLFWwindow* window)
{
    _GLFWwindow* previous;

    if (_glfw.os4.focusedWindow == window)
        return;

    if (!window->os4.visible)
        return;

    previous = _glfw.os4.focusedWindow;
    _glfw.os4.focusedWindow = window;

    if (previous)
    {
        _glfwInputWindowFocus(previous, GLFW_FALSE);
        if (previous->monitor && previous->autoIconify)
            _glfwIconifyWindowOS4(previous);
    }

    _glfwInputWindowFocus(window, GLFW_TRUE);
}

int _glfwWindowFocusedOS4(_GLFWwindow* window)
{
    return _glfw.os4.focusedWindow == window;
}

int _glfwWindowIconifiedOS4(_GLFWwindow* window)
{
    return window->os4.iconified;
}

int _glfwWindowVisibleOS4(_GLFWwindow* window)
{
    return window->os4.visible;
}

void _glfwPollEventsOS4(void)
{
    struct IntuiMessage *imsg;
    struct MyIntuiMessage msg;

    while ((imsg = (struct IntuiMessage *)IExec->GetMsg(_glfw.os4.userPort))) {

        OS4_CopyIdcmpMessage(imsg, &msg);
        _GLFWwindow* window = OS4_FindWindow(imsg->IDCMPWindow);

        IExec->ReplyMsg((struct Message *) imsg);

        switch (msg.Class) {
            case IDCMP_MOUSEMOVE:
                _glfwInputCursorPos(window, msg.WindowMouseX, msg.WindowMouseY);
                break;

            case IDCMP_RAWKEY:
                uint8_t rawkey = msg.Code & 0x7F;
                //printf("RAWKEY = 0x%x\n", rawkey);
                int key = _glfw.os4.keycodes[rawkey];
                int mods = OS4_TranslateState(msg.Qualifier);
                const int plain = !(mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT));

                if (imsg->Code < 0x80) {
                    char text[2];

                    text[0] = OS4_TranslateUnicode(msg.Code, msg.Qualifier);
                    text[1] = '\0';

                    _glfwInputKey(window, key, rawkey, GLFW_PRESS, mods);                          

                    if (text[0] && text[0] < 0x80) {
                        _glfwInputChar(window, text, mods, plain);
                    }
                } else {
                    _glfwInputKey(window, key, rawkey, GLFW_RELEASE, mods);                          
                }
                break;

            case IDCMP_MOUSEBUTTONS:
                //OS4_HandleMouseButtons(_this, &msg);
                int button = OS4_GetButton(imsg->Code);
                int state = OS4_GetButtonState(imsg->Code);

                _glfwInputMouseClick(window,
                                     button,
                                     state,
                                     0);
                break;

            case IDCMP_EXTENDEDMOUSE:
                struct IntuiWheelData *data = (struct IntuiWheelData *)msg.Gadget;
                if (data->WheelY < 0) {
                    _glfwInputScroll(window, 0.0, 1.0);
                }
                else if (data->WheelY > 0) {
                    _glfwInputScroll(window, 0.0, -1.0);
                }
                if (data->WheelX < 0) {
                    _glfwInputScroll(window, 1.0, 0.0);
                } else if (data->WheelX > 0) {
                    _glfwInputScroll(window, -1.0, 0.0);
                }
                break;

            case IDCMP_NEWSIZE:
                printf("w: %d - h: %d\n", msg.Width, msg.Height);
                //_glfwInputWindowSize(window, msg.Width, msg.Height);
                //OS4_HandleResize(_this, &msg);
                break;

            case IDCMP_CHANGEWINDOW:
                if (window != NULL) {
                    window->os4.xpos = imsg->IDCMPWindow->LeftEdge;
                    window->os4.ypos = imsg->IDCMPWindow->TopEdge;
                }

                //OS4_HandleMove(_this, &msg);
                //OS4_HandleResize(_this, &msg);
                break;

            case IDCMP_ACTIVEWINDOW:
                //if (window->cursorMode == GLFW_CURSOR_DISABLED)
                //    disableCursor(window);

                _glfwInputWindowFocus(window, GLFW_TRUE);
                //OS4_HandleActivation(_this, &msg, SDL_TRUE);
                break;

            case IDCMP_INACTIVEWINDOW:
                //OS4_HandleActivation(_this, &msg, SDL_FALSE);
                //if (window->cursorMode == GLFW_CURSOR_DISABLED)
                //    enableCursor(window);                

                _glfwInputWindowFocus(window, GLFW_FALSE);
                break;

            case IDCMP_CLOSEWINDOW:
                if (window != NULL) {
                    _glfwInputWindowCloseRequest(window);
                }

                break;

            case IDCMP_INTUITICKS:
                //OS4_HandleTicks(_this, &msg);
                break;

            case IDCMP_GADGETUP:
                OS4_IconifyWindow(window);
                break;

            default:
                dprintf("Unknown event received class %d, code %d\n", msg.Class, msg.Code);
                break;
        }
    }

    struct AppMessage *amsg;

    while ((amsg = (struct AppMessage *)IExec->GetMsg(_glfw.os4.appMsgPort))) {
        switch (amsg->am_Type) {
            case AMTYPE_APPWINDOW:
                //OS4_HandleAppWindow(msg);
                break;
            case AMTYPE_APPICON:
                OS4_HandleAppIcon(amsg);
                break;
            default:
                dprintf("Unknown AppMsg %d %p\n", amsg->am_Type, (APTR)amsg->am_UserData);
                break;
        }

        IExec->ReplyMsg((struct Message *) amsg);
    }
}

void _glfwWaitEventsOS4(void)
{
    _glfwPollEventsOS4();
}

void _glfwWaitEventsTimeoutOS4(double timeout)
{
    _glfwPollEventsOS4();
}

void _glfwPostEmptyEventOS4(void)
{
}

void _glfwGetCursorPosOS4(_GLFWwindow* window, double* xpos, double* ypos)
{
    if (xpos)
        *xpos = _glfw.os4.xcursor - window->os4.xpos;
    if (ypos)
        *ypos = _glfw.os4.ycursor - window->os4.ypos;
}

void _glfwSetCursorPosOS4(_GLFWwindow* window, double x, double y)
{
    _glfw.os4.xcursor = window->os4.xpos + (int) x;
    _glfw.os4.ycursor = window->os4.ypos + (int) y;
}

void _glfwSetCursorModeOS4(_GLFWwindow* window, int mode)
{
}

int _glfwCreateCursorOS4(_GLFWcursor* cursor,
                          const GLFWimage* image,
                          int xhot, int yhot)
{
    uint32_t *buffer = OS4_CopyImageData(image);

    /* We need to pass some compatibility parameters even though we are going to use just ARGB pointer */
    Object *object = IIntuition->NewObject(
        NULL,
        POINTERCLASS,
        POINTERA_BitMap, &fallbackPointerBitMap,
        POINTERA_XOffset, xhot,
        POINTERA_YOffset, yhot,
        POINTERA_WordWidth, 1,
        POINTERA_XResolution, POINTERXRESN_SCREENRES,
        POINTERA_YResolution, POINTERYRESN_SCREENRES,
        POINTERA_ImageData, buffer,
        POINTERA_Width, image->width,
        POINTERA_Height, image->height,
        TAG_DONE);
    if (object) {
        printf("cursor created\n");
        cursor->os4.handle = object;
        cursor->os4.imageData = buffer;
        return GLFW_TRUE;
    }
    printf("error creating cursor\n");
    return GLFW_FALSE;
}

int _glfwCreateStandardCursorOS4(_GLFWcursor* cursor, int shape)
{
    return GLFW_TRUE;
}

void _glfwDestroyCursorOS4(_GLFWcursor* cursor)
{
    if (cursor->os4.handle) {
        IIntuition->DisposeObject(cursor->os4.handle);
        if (cursor->os4.imageData)
            free(cursor->os4.imageData);
        cursor->os4.imageData = NULL;
        cursor->os4.handle = NULL;
    }
}

void _glfwSetCursorOS4(_GLFWwindow* window, _GLFWcursor* cursor)
{
}

void _glfwSetClipboardStringOS4(const char* string)
{
    char* copy = _glfw_strdup(string);
    _glfw_free(_glfw.os4.clipboardString);
    _glfw.os4.clipboardString = copy;

    ITextClip->WriteClipVector(string, strlen(string));
}

const char* _glfwGetClipboardStringOS4(void)
{
    STRPTR from;
    ULONG size;

    LONG result = ITextClip->ReadClipVector(&from, &size);

    if (result) {

        if (size) {
            _glfw.os4.clipboardString = _glfw_calloc( ++size, 1 );

            if (_glfw.os4.clipboardString) {
                strlcpy(_glfw.os4.clipboardString, from, size);
            } else {
                dprintf("Failed to allocate memory\n");
            }
        } else {
            _glfw.os4.clipboardString = strdup("");
        }

        ITextClip->DisposeClipVector(from);
    }

    return _glfw.os4.clipboardString;
}

const char* _glfwGetScancodeNameOS4(int scancode)
{
    if (scancode < GLFW_KEY_SPACE || scancode > GLFW_KEY_LAST)
    {
        _glfwInputError(GLFW_INVALID_VALUE, "Invalid OS4 scancode %i", scancode);
        return NULL;
    }
    return _glfw.os4.keynames[_glfw.os4.keycodes[scancode]];
}

int _glfwGetKeyScancodeOS4(int key)
{
    return key;
}

void _glfwGetRequiredInstanceExtensionsOS4(char** extensions)
{
}

int _glfwGetPhysicalDevicePresentationSupportOS4(VkInstance instance,
                                                  VkPhysicalDevice device,
                                                  uint32_t queuefamily)
{
    return GLFW_FALSE;
}

VkResult _glfwCreateWindowSurfaceOS4(VkInstance instance,
                                      _GLFWwindow* window,
                                      const VkAllocationCallbacks* allocator,
                                      VkSurfaceKHR* surface)
{
    // This seems like the most appropriate error to return here
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

/**********************************************************************************************/
/******************************************** PRIVATE METHODS *********************************/
/**********************************************************************************************/

static _GLFWwindow *
OS4_FindWindow(struct Window * syswin)
{
    _GLFWwindow *glfwWin;

    glfwWin = _glfw.windowListHead;
    while (glfwWin)
    {
        if (glfwWin->os4.handle == syswin)
            return glfwWin;
        glfwWin = glfwWin->next;
    }

    dprintf("No window found\n");

    return NULL;
}

static void
OS4_CopyIdcmpMessage(struct IntuiMessage * src, struct MyIntuiMessage * dst)
{
    // Copy relevant fields. This makes it safer if the window goes away during
    // this loop (re-open due to keystroke)
    dst->Class           = src->Class;
    dst->Code            = src->Code;
    dst->Qualifier       = src->Qualifier;

    dst->Gadget          = (struct Gadget *) src->IAddress;

    dst->RelativeMouseX  = src->MouseX;
    dst->RelativeMouseY  = src->MouseY;

    dst->IDCMPWindow     = src->IDCMPWindow;

    if (src->IDCMPWindow) {
        dst->WindowMouseX = src->IDCMPWindow->MouseX - src->IDCMPWindow->BorderLeft;
        dst->WindowMouseY = src->IDCMPWindow->MouseY - src->IDCMPWindow->BorderTop;

        dst->ScreenMouseX = src->IDCMPWindow->WScreen->MouseX;
        dst->ScreenMouseY = src->IDCMPWindow->WScreen->MouseY;

        dst->Width        = src->IDCMPWindow->Width  - src->IDCMPWindow->BorderLeft - src->IDCMPWindow->BorderRight;
        dst->Height       = src->IDCMPWindow->Height - src->IDCMPWindow->BorderTop  - src->IDCMPWindow->BorderBottom;
    }
}

static uint32_t*
OS4_CopyImageData(const GLFWimage *surface)
{
    const size_t bytesPerRow = surface->width * sizeof(uint32_t);
    uint32_t* buffer = malloc(bytesPerRow * surface->height);

    dprintf("Copying cursor data %d*%d from surface %p to buffer %p\n", surface->width, surface->height, surface, buffer);

    if (buffer) {
        uint8_t* source = surface->pixels;
        uint32_t* destination = buffer;
        int y;

        for (y = 0; y < surface->height; y++) {
            memcpy(destination, source, bytesPerRow);
            destination += surface->width;
            source += 4;
        }
    } else {
        dprintf("Failed to allocate memory\n");
    }

    return buffer;
}

static int
OS4_GetButtonState(uint16_t code)
{
    return (code & IECODE_UP_PREFIX) ? GLFW_RELEASE : GLFW_PRESS;
}

static int
OS4_GetButton(uint16_t code)
{
    switch (code & ~IECODE_UP_PREFIX) {
        case IECODE_LBUTTON:
            return GLFW_MOUSE_BUTTON_LEFT;
        case IECODE_RBUTTON:
            return GLFW_MOUSE_BUTTON_RIGHT;
        case IECODE_MBUTTON:
            return GLFW_MOUSE_BUTTON_MIDDLE;
        default:
            return 0;
    }
}

static char
OS4_TranslateUnicode(uint16_t code, uint32_t qualifier)
{
    struct InputEvent ie;
    uint16_t res;
    char buffer[10];

    ie.ie_Class = IECLASS_RAWKEY;
    ie.ie_SubClass = 0;
    ie.ie_Code  = code & ~(IECODE_UP_PREFIX);
    ie.ie_Qualifier = qualifier;
    ie.ie_EventAddress = NULL;

    res = IKeymap->MapRawKey(&ie, buffer, sizeof(buffer), 0);

    if (res != 1)
        return 0;
    else
        return buffer[0];
}

static int OS4_TranslateState(int state)
{
    int mods = 0;

    if (state & IEQUALIFIER_LSHIFT || state & IEQUALIFIER_RSHIFT)
        mods |= GLFW_MOD_SHIFT;
    if (state & IEQUALIFIER_CONTROL)
        mods |= GLFW_MOD_CONTROL;
    if (state & IEQUALIFIER_LALT || state & IEQUALIFIER_RALT)
        mods |= GLFW_MOD_ALT;
    if (state & IEQUALIFIER_LCOMMAND || state & IEQUALIFIER_RCOMMAND)
        mods |= GLFW_MOD_SUPER;
    if (state & IEQUALIFIER_CAPSLOCK)
        mods |= GLFW_MOD_CAPS_LOCK;

    return mods;
}

static uint32_t
OS4_GetWindowFlags(_GLFWwindow* window, BOOL fullscreen)
{
    uint32_t windowFlags = WFLG_REPORTMOUSE | WFLG_RMBTRAP | WFLG_SMART_REFRESH | WFLG_NOCAREREFRESH;

    if (fullscreen) {
        windowFlags |= WFLG_BORDERLESS | WFLG_BACKDROP;
    } else {
        windowFlags |= WFLG_NEWLOOKMENUS;

        if (!window->decorated) {
            windowFlags |= WFLG_BORDERLESS;
        } else {
            windowFlags |= WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET;

            if (window->resizable) {
                windowFlags |= WFLG_SIZEGADGET | WFLG_SIZEBBOTTOM;
            }
        }
    }

    return windowFlags;
}

static void
OS4_SetWindowLimits(_GLFWwindow * window, struct Window * syswin)
{
    const int minW = window->minwidth ? max(MIN_WINDOW_SIZE, window->minwidth) : MIN_WINDOW_SIZE;
    const int minH = window->minheight ? max(MIN_WINDOW_SIZE, window->minheight) : MIN_WINDOW_SIZE;
    const int maxW = window->maxwidth;
    const int maxH = window->maxheight;

    dprintf("Window min size %d*%d, max size %d*%d\n", minW, minH, maxW, maxH);

    const int borderWidth = syswin->BorderLeft + syswin->BorderRight;
    const int borderHeight = syswin->BorderTop + syswin->BorderBottom;

    BOOL ret = IIntuition->WindowLimits(syswin,
        minW + borderWidth,
        minH + borderHeight,
        maxW ? (maxW + borderWidth) : -1,
        maxH ? (maxH + borderHeight) : -1);

    if (!ret) {
        dprintf("Setting window limits failed\n");
    }
}

static void
OS4_CreateIconifyGadget(_GLFWwindow * window)
{
    struct DrawInfo *di = IIntuition->GetScreenDrawInfo(_glfw.os4.publicScreen);

    if (di) {
        window->os4.image = (struct Image *)IIntuition->NewObject(NULL, SYSICLASS,
            SYSIA_Which, ICONIFYIMAGE,
            SYSIA_DrawInfo, di,
            TAG_DONE);

        if (window->os4.image) {

            window->os4.gadget = (struct Gadget *)IIntuition->NewObject(NULL, BUTTONGCLASS,
                GA_Image, window->os4.image,
                GA_ID, GID_ICONIFY,
                GA_TopBorder, TRUE,
                GA_RelRight, TRUE,
                GA_Titlebar, TRUE,
                GA_RelVerify, TRUE,
                TAG_DONE);

            if (window->os4.gadget) {
                struct Window *syswin = window->os4.handle;

                IIntuition->AddGadget(syswin, window->os4.gadget, -1);
            } else {
                dprintf("Failed to create button class\n");
            }
        } else {
           dprintf("Failed to create image class\n");
        }

        IIntuition->FreeScreenDrawInfo(_glfw.os4.publicScreen, di);

    } else {
        dprintf("Failed to get screen draw info\n");
    }
}

static struct DiskObject*
OS4_GetDiskObject()
{
    struct DiskObject *diskObject = NULL;

    if (_glfw.os4.appName) {
        BPTR oldDir = IDOS->SetCurrentDir(IDOS->GetProgramDir());
        diskObject = IIcon->GetDiskObject(_glfw.os4.appName);
        IDOS->SetCurrentDir(oldDir);
    }

    if (!diskObject) {
        CONST_STRPTR fallbackIconName = "ENVARC:Sys/def_window";

        dprintf("Falling back to '%s'\n", fallbackIconName);
        diskObject = IIcon->GetDiskObjectNew(fallbackIconName);
    }

    return diskObject;
}

void
OS4_IconifyWindow(_GLFWwindow *window)
{
    if (window->os4.iconified) {
        dprintf("Window '%s' is already iconified\n", window->os4.title);
    } else {
        struct DiskObject *diskObject = OS4_GetDiskObject();

        if (diskObject) {
            diskObject->do_CurrentX = NO_ICON_POSITION;
            diskObject->do_CurrentY = NO_ICON_POSITION;

            window->os4.appIcon = IWorkbench->AddAppIcon(
                0,
                (ULONG)window,
                _glfw.os4.appName,
                _glfw.os4.appMsgPort,
                0,
                diskObject,
                TAG_DONE);

            if (!window->os4.appIcon) {
                dprintf("Failed to add AppIcon\n");
            } else {
                dprintf("Iconifying '%s'\n", window->os4.title);

                IIntuition->HideWindow(window->os4.handle);
                window->os4.iconified = TRUE;
                //SDL_SendWindowEvent(window, SDL_WINDOWEVENT_MINIMIZED, 0, 0);
            }

            IIcon->FreeDiskObject(diskObject);
        } else {
            dprintf("Failed to load icon\n");
        }
    }
}

void
OS4_UniconifyWindow(_GLFWwindow* window)
{
    if (window->os4.iconified) {
        dprintf("Restoring '%s'\n", window->os4.title);

        if (window->os4.appIcon) {
            IWorkbench->RemoveAppIcon(window->os4.appIcon);
            window->os4.appIcon == NULL;
        }
        IIntuition->SetWindowAttrs(window->os4.handle,
            WA_Hidden, FALSE,
            TAG_DONE);

        window->os4.iconified = FALSE;
        //SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESTORED, 0, 0);
    }
}

static void
OS4_HandleAppIcon(struct AppMessage * msg)
{
    _GLFWwindow *window = (_GLFWwindow *)msg->am_UserData;
    dprintf("Window ptr = %p\n", window);

    OS4_UniconifyWindow(window);
}

static ULONG
OS4_BackFill(const struct Hook *hook, struct RastPort *rastport, struct BackFillMessage *message)
{
    struct Rectangle *rect = &message->Bounds;
    struct GraphicsIFace *igfx = hook->h_Data;

    struct RastPort bfRastport;

    igfx->InitRastPort(&bfRastport);
    bfRastport.BitMap = rastport->BitMap;
    igfx->RectFillColor(&bfRastport, rect->MinX, rect->MinY, rect->MaxX, rect->MaxY, 0xFF000000);

    return 0;
}