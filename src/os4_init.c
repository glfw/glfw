//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2017 Camilla Löwy <elmindreda@glfw.org>
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

extern struct Library *DOSBase;
extern struct DOSIFace *IDOS;

struct GraphicsIFace *IGraphics = NULL;
struct Library *GfxBase = NULL;

struct IntuitionIFace *IIntuition = NULL;
struct Library *IntuitionBase = NULL;

struct KeymapIFace *IKeymap = NULL;
struct Library *KeymapBase = NULL;

struct UtilityIFace *IUtility = NULL;
struct Library *UtilityBase = NULL;

struct Library *AIN_Base = NULL;
struct AIN_IFace *IAIN = NULL;

struct Device *TimerBase = NULL;
struct TimerIFace *ITimer = NULL;

struct TextClipIFace *ITextClip = NULL;
struct Library *TextClipBase = NULL;

struct IconIFace *IIcon = NULL;
struct Library *IconBase = NULL;

struct WorkbenchIFace *IWorkbench = NULL;
struct Library *WorkbenchBase = NULL;

#ifndef GL4ES
struct Library *OGLES2Base = NULL;
struct OGLES2IFace *IOGLES2 = NULL;
#else
extern struct OGLES2IFace *IOGLES2;
#endif

#define MIN_LIB_VERSION 51

static void OS4_FindApplicationName();

//************************************************************************
//****                  OS4 internal functions                        ****
//************************************************************************

BOOL VARARGS68K showErrorRequester(const char *errMsgRaw, ...)
{
    va_list ap;
    va_startlinear(ap, errMsgRaw);
    ULONG *errMsgArgs = va_getlinearva(ap, ULONG *);

    Object *object = NULL;
    if (IIntuition)
    {
        object = IIntuition->NewObject(
            NULL, "requester.class",
            REQ_Type, REQTYPE_INFO,
            REQ_TitleText, "GLFW: FATAL ERROR",
            REQ_BodyText, errMsgRaw,
            REQ_VarArgs, errMsgArgs,
            REQ_Image, REQIMAGE_ERROR,
            REQ_GadgetText, "_Ok",
            TAG_DONE);
    }
    if (object)
    {
        IIntuition->IDoMethod(object, RM_OPENREQ, NULL, NULL, NULL, TAG_DONE);
        IIntuition->DisposeObject(object);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static struct Library *openLib(const char *libName, unsigned int minVers, struct Interface **iFacePtr)
{
    struct Library *libBase = IExec->OpenLibrary(libName, minVers);
    if (libBase)
    {
        struct Interface *iFace = IExec->GetInterface(libBase, "main", 1, NULL);
        if (!iFace)
        {
            // Failed
            //CloseLibrary(libBase);
            libBase = NULL; // Lets the code below know we've failed
        }

        if (iFacePtr)
        {
            // Write the interface pointer
            *iFacePtr = iFace;
        }
    }
    else
    {
        // Opening the library failed. Show the error requester
        const char errMsgRaw[] = "Couldn't open %s version %u+.\n";
        if (!showErrorRequester(errMsgRaw, libName, minVers))
        {
            // Use printf() as a backup
            printf(errMsgRaw, libName, minVers);
        }
    }

    return libBase;
}

static int loadLibraries(void)
{
    // graphics.library
    DOSBase = openLib("dos.library", MIN_LIB_VERSION, (struct Interface **)&IDOS);
    if (!DOSBase)
    {
        return 0;
    }

    // graphics.library
    GfxBase = openLib("graphics.library", 54, (struct Interface **)&IGraphics);
    if (!GfxBase)
    {
        return 0;
    }

    // intuition.library
    IntuitionBase = openLib("intuition.library", MIN_LIB_VERSION, (struct Interface **)&IIntuition);
    if (!IntuitionBase)
    {
        return 0;
    }

    // keymap.library
    KeymapBase = openLib("keymap.library", MIN_LIB_VERSION, (struct Interface **)&IKeymap);
    if (!KeymapBase)
    {
        return 0;
    }

    // Utility.library
    UtilityBase = openLib("utility.library", MIN_LIB_VERSION, (struct Interface **)&IUtility);
    if (!UtilityBase)
    {
        return 0;
    }

    // Workbench.library
    WorkbenchBase = openLib("workbench.library", MIN_LIB_VERSION, (struct Interface **)&IWorkbench);
    if (!WorkbenchBase)
    {
        return 0;
    }

    // icon.library
    IconBase = openLib("icon.library", MIN_LIB_VERSION, (struct Interface **)&IIcon);
    if (!IconBase)
    {
        return 0;
    }

#ifndef GL4ES
    OGLES2Base = openLib("ogles2.library", MIN_OGLES2_VERSION, (struct Interface **)&IOGLES2);
    if (!OGLES2Base)
    {
        return 0;
    }
#endif

    // AmigaInput
    AIN_Base = openLib("AmigaInput.library", MIN_LIB_VERSION, (struct Interface **)&IAIN);
    if (!AIN_Base)
    {
        return 0;
    }

    // TextClip
    TextClipBase  = openLib("textclip.library", MIN_LIB_VERSION, (struct Interface **)&ITextClip);
    if (!TextClipBase)
    {
        return 0;
    }

    return 1;
}

static void closeLibraries(void)
{
    printf("close libraries\n");
    if (ITextClip) {
        IExec->DropInterface((struct Interface *)ITextClip);
    }
    if (TextClipBase)
    {
        IExec->CloseLibrary(TextClipBase);
    }

    if (IAIN)
    {
        IExec->DropInterface((struct Interface *)IAIN);
    }
    if (AIN_Base)
    {
        IExec->CloseLibrary(AIN_Base);
    }

#ifndef GL4ES
    if (IOGLES2)
    {
        IExec->DropInterface((struct Interface *)IOGLES2);
    }
    if (OGLES2Base)
    {
        IExec->CloseLibrary(OGLES2Base);
    }
#endif

    // Close workbench.library
    if (IWorkbench)
    {
        IExec->DropInterface((struct Interface *)IWorkbench);
        IWorkbench = NULL;
    }
    if (WorkbenchBase)
    {
        IExec->CloseLibrary((struct Library *)WorkbenchBase);
        WorkbenchBase = NULL;
    }

    // Close icon.library
    if (IIcon)
    {
        IExec->DropInterface((struct Interface *)IIcon);
        IIcon = NULL;
    }
    if (IconBase)
    {
        IExec->CloseLibrary((struct Library *)IconBase);
        IconBase = NULL;
    }

    // Close graphics.library
    if (IGraphics)
    {
        IExec->DropInterface((struct Interface *)IGraphics);
        IGraphics = NULL;
    }
    if (GfxBase)
    {
        IExec->CloseLibrary((struct Library *)GfxBase);
        GfxBase = NULL;
    }

    // Close intuition.library
    if (IIntuition)
    {
        IExec->DropInterface((struct Interface *)IIntuition);
        IIntuition = NULL;
    }
    if (IntuitionBase)
    {
        IExec->CloseLibrary((struct Library *)IntuitionBase);
        IntuitionBase = NULL;
    }

    // Close keymap.library
    if (IKeymap)
    {
        IExec->DropInterface((struct Interface *)IKeymap);
        IKeymap = NULL;
    }
    if (KeymapBase)
    {
        IExec->CloseLibrary(KeymapBase);
        KeymapBase = NULL;
    }

    // Close utility.library
    if (IUtility)
    {
        IExec->DropInterface((struct Interface *)IUtility);
        IUtility = NULL;
    }
    if (UtilityBase)
    {
        IExec->CloseLibrary((struct Library *)UtilityBase);
        UtilityBase = NULL;
    }
}

static void createKeyTables(void)
{
    memset(_glfw.os4.keycodes, -1, sizeof(_glfw.os4.keycodes));
    memset(_glfw.os4.scancodes, -1, sizeof(_glfw.os4.scancodes));

    _glfw.os4.keycodes[0xb]      = GLFW_KEY_GRAVE_ACCENT;
    _glfw.os4.keycodes[0x1]          = GLFW_KEY_1;
    _glfw.os4.keycodes[0x2]          = GLFW_KEY_2;
    _glfw.os4.keycodes[0x3]          = GLFW_KEY_3;
    _glfw.os4.keycodes[0x4]          = GLFW_KEY_4;
    _glfw.os4.keycodes[0x5]          = GLFW_KEY_5;
    _glfw.os4.keycodes[0x6]          = GLFW_KEY_6;
    _glfw.os4.keycodes[0x7]          = GLFW_KEY_7;
    _glfw.os4.keycodes[0x8]          = GLFW_KEY_8;
    _glfw.os4.keycodes[0x9]          = GLFW_KEY_9;
    _glfw.os4.keycodes[0xa]          = GLFW_KEY_0;
    _glfw.os4.keycodes[0x40]         = GLFW_KEY_SPACE; //
    _glfw.os4.keycodes[0x3a]         = GLFW_KEY_MINUS;
    _glfw.os4.keycodes[0xc]          = GLFW_KEY_EQUAL;
    _glfw.os4.keycodes[0x10]          = GLFW_KEY_Q;
    _glfw.os4.keycodes[0x11]          = GLFW_KEY_W;
    _glfw.os4.keycodes[0x12]          = GLFW_KEY_E;
    _glfw.os4.keycodes[0x13]          = GLFW_KEY_R;
    _glfw.os4.keycodes[0x14]          = GLFW_KEY_T;
    _glfw.os4.keycodes[0x15]          = GLFW_KEY_Y;
    _glfw.os4.keycodes[0x16]          = GLFW_KEY_U;
    _glfw.os4.keycodes[0x17]          = GLFW_KEY_I;
    _glfw.os4.keycodes[0x18]          = GLFW_KEY_O;
    _glfw.os4.keycodes[0x19]          = GLFW_KEY_P;
    _glfw.os4.keycodes[0x1a]          = GLFW_KEY_LEFT_BRACKET;
    _glfw.os4.keycodes[0x1b]          = GLFW_KEY_RIGHT_BRACKET;
    _glfw.os4.keycodes[0x20]          = GLFW_KEY_A;
    _glfw.os4.keycodes[0x21]          = GLFW_KEY_S;
    _glfw.os4.keycodes[0x22]          = GLFW_KEY_D;
    _glfw.os4.keycodes[0x23]          = GLFW_KEY_F;
    _glfw.os4.keycodes[0x24]          = GLFW_KEY_G;
    _glfw.os4.keycodes[0x25]          = GLFW_KEY_H;
    _glfw.os4.keycodes[0x26]          = GLFW_KEY_J;
    _glfw.os4.keycodes[0x27]          = GLFW_KEY_K;
    _glfw.os4.keycodes[0x28]          = GLFW_KEY_L;
    //_glfw.os4.keycodes[KEY_SEMICOLON]  = GLFW_KEY_SEMICOLON;
    //_glfw.os4.keycodes[KEY_APOSTROPHE] = GLFW_KEY_APOSTROPHE;
    _glfw.os4.keycodes[0x31]          = GLFW_KEY_Z;
    _glfw.os4.keycodes[0x32]          = GLFW_KEY_X;
    _glfw.os4.keycodes[0x33]          = GLFW_KEY_C;
    _glfw.os4.keycodes[0x34]          = GLFW_KEY_V;
    _glfw.os4.keycodes[0x35]          = GLFW_KEY_B;
    _glfw.os4.keycodes[0x36]          = GLFW_KEY_N;
    _glfw.os4.keycodes[0x37]          = GLFW_KEY_M;
    _glfw.os4.keycodes[0x38]      = GLFW_KEY_COMMA;
    _glfw.os4.keycodes[0x39]        = GLFW_KEY_PERIOD;
    _glfw.os4.keycodes[0x3a]      = GLFW_KEY_SLASH;
    _glfw.os4.keycodes[0x0]  = GLFW_KEY_BACKSLASH;
    _glfw.os4.keycodes[RAWKEY_ESC] = GLFW_KEY_ESCAPE;         //
    _glfw.os4.keycodes[RAWKEY_TAB] = GLFW_KEY_TAB;            //
    _glfw.os4.keycodes[RAWKEY_LSHIFT] = GLFW_KEY_LEFT_SHIFT;  //
    _glfw.os4.keycodes[RAWKEY_RSHIFT] = GLFW_KEY_RIGHT_SHIFT; //
    _glfw.os4.keycodes[RAWKEY_LCTRL] = GLFW_KEY_LEFT_CONTROL; //
    _glfw.os4.keycodes[RAWKEY_LCTRL]  = GLFW_KEY_RIGHT_CONTROL;
    _glfw.os4.keycodes[RAWKEY_LALT] = GLFW_KEY_LEFT_ALT;      //
    _glfw.os4.keycodes[RAWKEY_RALT] = GLFW_KEY_RIGHT_ALT;      //
    _glfw.os4.keycodes[RAWKEY_LCOMMAND] = GLFW_KEY_LEFT_SUPER; //
    _glfw.os4.keycodes[RAWKEY_RCOMMAND] = GLFW_KEY_RIGHT_SUPER;
    _glfw.os4.keycodes[RAWKEY_MENU] = GLFW_KEY_MENU; //
    //_glfw.os4.keycodes[KEY_NUMLOCK]       = GLFW_KEY_NUM_LOCK;
    _glfw.os4.keycodes[RAWKEY_CAPSLOCK] = GLFW_KEY_CAPS_LOCK; //
    _glfw.os4.keycodes[RAWKEY_BREAK] = GLFW_KEY_PRINT_SCREEN;
    _glfw.os4.keycodes[0x5f]    = GLFW_KEY_SCROLL_LOCK;
    _glfw.os4.keycodes[RAWKEY_BREAK] = GLFW_KEY_PAUSE;
    _glfw.os4.keycodes[RAWKEY_DEL] = GLFW_KEY_DELETE;          //
    _glfw.os4.keycodes[RAWKEY_BACKSPACE] = GLFW_KEY_BACKSPACE; //
    _glfw.os4.keycodes[RAWKEY_RETURN] = GLFW_KEY_ENTER;        //
    _glfw.os4.keycodes[RAWKEY_HOME] = GLFW_KEY_HOME;           //
    _glfw.os4.keycodes[RAWKEY_END] = GLFW_KEY_END;             //
    _glfw.os4.keycodes[RAWKEY_PAGEUP] = GLFW_KEY_PAGE_UP;      //
    _glfw.os4.keycodes[RAWKEY_PAGEDOWN] = GLFW_KEY_PAGE_DOWN;  //
    _glfw.os4.keycodes[RAWKEY_INSERT] = GLFW_KEY_INSERT;       //
    _glfw.os4.keycodes[RAWKEY_CRSRLEFT] = GLFW_KEY_LEFT;
    _glfw.os4.keycodes[RAWKEY_CRSRRIGHT] = GLFW_KEY_RIGHT; //
    _glfw.os4.keycodes[RAWKEY_CRSRDOWN] = GLFW_KEY_DOWN;   //
    _glfw.os4.keycodes[RAWKEY_CRSRUP] = GLFW_KEY_UP;       //
    _glfw.os4.keycodes[RAWKEY_F1] = GLFW_KEY_F1;           //
    _glfw.os4.keycodes[RAWKEY_F2] = GLFW_KEY_F2;           //
    _glfw.os4.keycodes[RAWKEY_F3] = GLFW_KEY_F3;           //
    _glfw.os4.keycodes[RAWKEY_F4] = GLFW_KEY_F4;           //
    _glfw.os4.keycodes[RAWKEY_F5] = GLFW_KEY_F5;           //
    _glfw.os4.keycodes[RAWKEY_F6] = GLFW_KEY_F6;           //
    _glfw.os4.keycodes[RAWKEY_F7] = GLFW_KEY_F7;           //
    _glfw.os4.keycodes[RAWKEY_F8] = GLFW_KEY_F8;           //
    _glfw.os4.keycodes[RAWKEY_F9] = GLFW_KEY_F9;           //
    _glfw.os4.keycodes[RAWKEY_F10] = GLFW_KEY_F10;         //
    _glfw.os4.keycodes[RAWKEY_F11] = GLFW_KEY_F11;         //
    _glfw.os4.keycodes[RAWKEY_F12] = GLFW_KEY_F12;         //
    _glfw.os4.keycodes[RAWKEY_F13] = GLFW_KEY_F13;         //
    _glfw.os4.keycodes[RAWKEY_F14] = GLFW_KEY_F14;         //
    _glfw.os4.keycodes[RAWKEY_F15] = GLFW_KEY_F15;         //
    _glfw.os4.keycodes[RAWKEY_HELP] = GLFW_KEY_F16;        // Mapped amiga HELP key with F16
    _glfw.os4.keycodes[0x5c]    = GLFW_KEY_KP_DIVIDE;
    _glfw.os4.keycodes[0x5d]      = GLFW_KEY_KP_MULTIPLY;
    _glfw.os4.keycodes[0x4a]    = GLFW_KEY_KP_SUBTRACT;
    _glfw.os4.keycodes[0x5e]     = GLFW_KEY_KP_ADD;
    _glfw.os4.keycodes[0xf]        = GLFW_KEY_KP_0;
    _glfw.os4.keycodes[0x1d]        = GLFW_KEY_KP_1;
    _glfw.os4.keycodes[0x1e]        = GLFW_KEY_KP_2;
    _glfw.os4.keycodes[0x1f]        = GLFW_KEY_KP_3;
    _glfw.os4.keycodes[0x2d]        = GLFW_KEY_KP_4;
    _glfw.os4.keycodes[0x2e]        = GLFW_KEY_KP_5;
    _glfw.os4.keycodes[0x2f]        = GLFW_KEY_KP_6;
    _glfw.os4.keycodes[0x3d]        = GLFW_KEY_KP_7;
    _glfw.os4.keycodes[0x3e]        = GLFW_KEY_KP_8;
    _glfw.os4.keycodes[0x3f]        = GLFW_KEY_KP_9;
    _glfw.os4.keycodes[0x3c]    = GLFW_KEY_KP_DECIMAL;
    /*
    _glfw.os4.keycodes[KEY_KPEQUAL]    = GLFW_KEY_KP_EQUAL;
    */
    _glfw.os4.keycodes[RAWKEY_ENTER] = GLFW_KEY_KP_ENTER; //

    for (int scancode = 0; scancode < 512; scancode++)
    {
        if (_glfw.os4.keycodes[scancode] > 0)
            _glfw.os4.scancodes[_glfw.os4.keycodes[scancode]] = scancode;
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwConnectOS4(int platformID, _GLFWplatform *platform)
{
    const _GLFWplatform os4 =
        {
            GLFW_PLATFORM_OS4,
            _glfwInitOS4,
            _glfwTerminateOS4,
            _glfwGetCursorPosOS4,
            _glfwSetCursorPosOS4,
            _glfwSetCursorModeOS4,
            _glfwSetRawMouseMotionOS4,
            _glfwRawMouseMotionSupportedOS4,
            _glfwCreateCursorOS4,
            _glfwCreateStandardCursorOS4,
            _glfwDestroyCursorOS4,
            _glfwSetCursorOS4,
            _glfwGetScancodeNameOS4,
            _glfwGetKeyScancodeOS4,
            _glfwSetClipboardStringOS4,
            _glfwGetClipboardStringOS4,
            _glfwInitJoysticksOS4,
            _glfwTerminateJoysticksOS4,
            _glfwPollJoystickOS4,
            _glfwGetMappingNameOS4,
            _glfwUpdateGamepadGUIDOS4,
            _glfwFreeMonitorOS4,
            _glfwGetMonitorPosOS4,
            _glfwGetMonitorContentScaleOS4,
            _glfwGetMonitorWorkareaOS4,
            _glfwGetVideoModesOS4,
            _glfwGetVideoModeOS4,
            _glfwGetGammaRampOS4,
            _glfwSetGammaRampOS4,
            _glfwCreateWindowOS4,
            _glfwDestroyWindowOS4,
            _glfwSetWindowTitleOS4,
            _glfwSetWindowIconOS4,
            _glfwGetWindowPosOS4,
            _glfwSetWindowPosOS4,
            _glfwGetWindowSizeOS4,
            _glfwSetWindowSizeOS4,
            _glfwSetWindowSizeLimitsOS4,
            _glfwSetWindowAspectRatioOS4,
            _glfwGetFramebufferSizeOS4,
            _glfwGetWindowFrameSizeOS4,
            _glfwGetWindowContentScaleOS4,
            _glfwIconifyWindowOS4,
            _glfwRestoreWindowOS4,
            _glfwMaximizeWindowOS4,
            _glfwShowWindowOS4,
            _glfwHideWindowOS4,
            _glfwRequestWindowAttentionOS4,
            _glfwFocusWindowOS4,
            _glfwSetWindowMonitorOS4,
            _glfwWindowFocusedOS4,
            _glfwWindowIconifiedOS4,
            _glfwWindowVisibleOS4,
            _glfwWindowMaximizedOS4,
            _glfwWindowHoveredOS4,
            _glfwFramebufferTransparentOS4,
            _glfwGetWindowOpacityOS4,
            _glfwSetWindowResizableOS4,
            _glfwSetWindowDecoratedOS4,
            _glfwSetWindowFloatingOS4,
            _glfwSetWindowOpacityOS4,
            _glfwSetWindowMousePassthroughOS4,
            _glfwPollEventsOS4,
            _glfwWaitEventsOS4,
            _glfwWaitEventsTimeoutOS4,
            _glfwPostEmptyEventOS4,
            _glfwGetRequiredInstanceExtensionsOS4,
            _glfwGetPhysicalDevicePresentationSupportOS4,
            _glfwCreateWindowSurfaceOS4,
        };

    *platform = os4;
    return GLFW_TRUE;
}

int _glfwInitOS4(void)
{
    loadLibraries();
    createKeyTables();
    _glfwPollMonitorsOS4();

    if (!(_glfw.os4.userPort = IExec->AllocSysObjectTags(ASOT_PORT, TAG_DONE))) {
        return GLFW_FALSE;
    }

    if (!(_glfw.os4.appMsgPort = IExec->AllocSysObjectTags(ASOT_PORT, TAG_DONE))) {
        return GLFW_FALSE;
    }

    OS4_LockPubScreen();
    
    OS4_FindApplicationName();

    return GLFW_TRUE;
}

void _glfwTerminateOS4(void)
{
    OS4_UnlockPubScreen();

    if (_glfw.os4.appMsgPort) {
        struct Message *msg;

        while ((msg = IExec->GetMsg(_glfw.os4.appMsgPort))) {
            IExec->ReplyMsg((struct Message *) msg);
        }
        IExec->FreeSysObject(ASOT_PORT, _glfw.os4.appMsgPort);
    }

    if (_glfw.os4.userPort) {
        struct Message *msg;

        while ((msg = IExec->GetMsg(_glfw.os4.userPort))) {
            IExec->ReplyMsg((struct Message *) msg);
        }
        IExec->FreeSysObject(ASOT_PORT, _glfw.os4.userPort);
    }

    if (_glfw.os4.clipboardString) {
        _glfw_free(_glfw.os4.clipboardString);
        _glfw.os4.clipboardString = NULL;
    }
    closeLibraries();
}

/************************************************************************************/
/********************************* AmigaOS4 METHODS *********************************/
/************************************************************************************/

static void
OS4_FindApplicationName()
{
    size_t size;
    char pathBuffer[MAXPATHLEN];

    if (IDOS->GetCliProgramName(pathBuffer, MAXPATHLEN - 1)) {
        printf("GetCliProgramName: '%s'\n", pathBuffer);
    } else {
        printf("Failed to get CLI program name, checking task node\n");

        struct Task* me = IExec->FindTask(NULL);
        snprintf(pathBuffer, MAXPATHLEN, "%s", ((struct Node *)me)->ln_Name);
    }

    size = strlen(pathBuffer) + 1;

    _glfw.os4.appName = malloc(size);

    if (_glfw.os4.appName) {
        snprintf(_glfw.os4.appName, size, pathBuffer);
    }

    printf("Application name: '%s'\n", _glfw.os4.appName);
}