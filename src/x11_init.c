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

#include <stdlib.h>
#include <string.h>
#include <limits.h>


//========================================================================
// Translate an X11 key code to a GLFW key code.
//========================================================================

static int keyCodeToGLFWKeyCode(int keyCode)
{
    int keySym;

    // Valid key code range is  [8,255], according to the XLib manual
    if (keyCode < 8 || keyCode > 255)
        return -1;

    // Try secondary keysym, for numeric keypad keys
    // Note: This way we always force "NumLock = ON", which is intentional
    // since the returned key code should correspond to a physical
    // location.
#if defined(_GLFW_HAS_XKB)
    keySym = XkbKeycodeToKeysym(_glfwLibrary.X11.display, keyCode, 1, 0);
#else
    keySym = XKeycodeToKeysym(_glfwLibrary.X11.display, keyCode, 1);
#endif
    switch (keySym)
    {
        case XK_KP_0:           return GLFW_KEY_KP_0;
        case XK_KP_1:           return GLFW_KEY_KP_1;
        case XK_KP_2:           return GLFW_KEY_KP_2;
        case XK_KP_3:           return GLFW_KEY_KP_3;
        case XK_KP_4:           return GLFW_KEY_KP_4;
        case XK_KP_5:           return GLFW_KEY_KP_5;
        case XK_KP_6:           return GLFW_KEY_KP_6;
        case XK_KP_7:           return GLFW_KEY_KP_7;
        case XK_KP_8:           return GLFW_KEY_KP_8;
        case XK_KP_9:           return GLFW_KEY_KP_9;
        case XK_KP_Separator:
        case XK_KP_Decimal:     return GLFW_KEY_KP_DECIMAL;
        case XK_KP_Equal:       return GLFW_KEY_KP_EQUAL;
        case XK_KP_Enter:       return GLFW_KEY_KP_ENTER;
        default:                break;
    }

    // Now try pimary keysym for function keys (non-printable keys). These
    // should not be layout dependent (i.e. US layout and international
    // layouts should give the same result).
#if defined(_GLFW_HAS_XKB)
    keySym = XkbKeycodeToKeysym(_glfwLibrary.X11.display, keyCode, 0, 0);
#else
    keySym = XKeycodeToKeysym(_glfwLibrary.X11.display, keyCode, 0);
#endif

    switch (keySym)
    {
        case XK_Escape:         return GLFW_KEY_ESCAPE;
        case XK_Tab:            return GLFW_KEY_TAB;
        case XK_Shift_L:        return GLFW_KEY_LEFT_SHIFT;
        case XK_Shift_R:        return GLFW_KEY_RIGHT_SHIFT;
        case XK_Control_L:      return GLFW_KEY_LEFT_CONTROL;
        case XK_Control_R:      return GLFW_KEY_RIGHT_CONTROL;
        case XK_Meta_L:
        case XK_Alt_L:          return GLFW_KEY_LEFT_ALT;
        case XK_Mode_switch: // Mapped to Alt_R on many keyboards
        case XK_ISO_Level3_Shift: // AltGr on at least some machines
        case XK_Meta_R:
        case XK_Alt_R:          return GLFW_KEY_RIGHT_ALT;
        case XK_Super_L:        return GLFW_KEY_LEFT_SUPER;
        case XK_Super_R:        return GLFW_KEY_RIGHT_SUPER;
        case XK_Menu:           return GLFW_KEY_MENU;
        case XK_Num_Lock:       return GLFW_KEY_NUM_LOCK;
        case XK_Caps_Lock:      return GLFW_KEY_CAPS_LOCK;
        case XK_Scroll_Lock:    return GLFW_KEY_SCROLL_LOCK;
        case XK_Pause:          return GLFW_KEY_PAUSE;
        case XK_Delete:         return GLFW_KEY_DELETE;
        case XK_BackSpace:      return GLFW_KEY_BACKSPACE;
        case XK_Return:         return GLFW_KEY_ENTER;
        case XK_Home:           return GLFW_KEY_HOME;
        case XK_End:            return GLFW_KEY_END;
        case XK_Page_Up:        return GLFW_KEY_PAGE_UP;
        case XK_Page_Down:      return GLFW_KEY_PAGE_DOWN;
        case XK_Insert:         return GLFW_KEY_INSERT;
        case XK_Left:           return GLFW_KEY_LEFT;
        case XK_Right:          return GLFW_KEY_RIGHT;
        case XK_Down:           return GLFW_KEY_DOWN;
        case XK_Up:             return GLFW_KEY_UP;
        case XK_F1:             return GLFW_KEY_F1;
        case XK_F2:             return GLFW_KEY_F2;
        case XK_F3:             return GLFW_KEY_F3;
        case XK_F4:             return GLFW_KEY_F4;
        case XK_F5:             return GLFW_KEY_F5;
        case XK_F6:             return GLFW_KEY_F6;
        case XK_F7:             return GLFW_KEY_F7;
        case XK_F8:             return GLFW_KEY_F8;
        case XK_F9:             return GLFW_KEY_F9;
        case XK_F10:            return GLFW_KEY_F10;
        case XK_F11:            return GLFW_KEY_F11;
        case XK_F12:            return GLFW_KEY_F12;
        case XK_F13:            return GLFW_KEY_F13;
        case XK_F14:            return GLFW_KEY_F14;
        case XK_F15:            return GLFW_KEY_F15;
        case XK_F16:            return GLFW_KEY_F16;
        case XK_F17:            return GLFW_KEY_F17;
        case XK_F18:            return GLFW_KEY_F18;
        case XK_F19:            return GLFW_KEY_F19;
        case XK_F20:            return GLFW_KEY_F20;
        case XK_F21:            return GLFW_KEY_F21;
        case XK_F22:            return GLFW_KEY_F22;
        case XK_F23:            return GLFW_KEY_F23;
        case XK_F24:            return GLFW_KEY_F24;
        case XK_F25:            return GLFW_KEY_F25;

        // Numeric keypad
        case XK_KP_Divide:      return GLFW_KEY_KP_DIVIDE;
        case XK_KP_Multiply:    return GLFW_KEY_KP_MULTIPLY;
        case XK_KP_Subtract:    return GLFW_KEY_KP_SUBTRACT;
        case XK_KP_Add:         return GLFW_KEY_KP_ADD;

        // These should have been detected in secondary keysym test above!
        case XK_KP_Insert:      return GLFW_KEY_KP_0;
        case XK_KP_End:         return GLFW_KEY_KP_1;
        case XK_KP_Down:        return GLFW_KEY_KP_2;
        case XK_KP_Page_Down:   return GLFW_KEY_KP_3;
        case XK_KP_Left:        return GLFW_KEY_KP_4;
        case XK_KP_Right:       return GLFW_KEY_KP_6;
        case XK_KP_Home:        return GLFW_KEY_KP_7;
        case XK_KP_Up:          return GLFW_KEY_KP_8;
        case XK_KP_Page_Up:     return GLFW_KEY_KP_9;
        case XK_KP_Delete:      return GLFW_KEY_KP_DECIMAL;
        case XK_KP_Equal:       return GLFW_KEY_KP_EQUAL;
        case XK_KP_Enter:       return GLFW_KEY_KP_ENTER;

        // Last resort: Check for printable keys (should not happen if the XKB
        // extension is available). This will give a layout dependent mapping
        // (which is wrong, and we may miss some keys, especially on non-US
        // keyboards), but it's better than nothing...
        case XK_a:              return GLFW_KEY_A;
        case XK_b:              return GLFW_KEY_B;
        case XK_c:              return GLFW_KEY_C;
        case XK_d:              return GLFW_KEY_D;
        case XK_e:              return GLFW_KEY_E;
        case XK_f:              return GLFW_KEY_F;
        case XK_g:              return GLFW_KEY_G;
        case XK_h:              return GLFW_KEY_H;
        case XK_i:              return GLFW_KEY_I;
        case XK_j:              return GLFW_KEY_J;
        case XK_k:              return GLFW_KEY_K;
        case XK_l:              return GLFW_KEY_L;
        case XK_m:              return GLFW_KEY_M;
        case XK_n:              return GLFW_KEY_N;
        case XK_o:              return GLFW_KEY_O;
        case XK_p:              return GLFW_KEY_P;
        case XK_q:              return GLFW_KEY_Q;
        case XK_r:              return GLFW_KEY_R;
        case XK_s:              return GLFW_KEY_S;
        case XK_t:              return GLFW_KEY_T;
        case XK_u:              return GLFW_KEY_U;
        case XK_v:              return GLFW_KEY_V;
        case XK_w:              return GLFW_KEY_W;
        case XK_x:              return GLFW_KEY_X;
        case XK_y:              return GLFW_KEY_Y;
        case XK_z:              return GLFW_KEY_Z;
        case XK_1:              return GLFW_KEY_1;
        case XK_2:              return GLFW_KEY_2;
        case XK_3:              return GLFW_KEY_3;
        case XK_4:              return GLFW_KEY_4;
        case XK_5:              return GLFW_KEY_5;
        case XK_6:              return GLFW_KEY_6;
        case XK_7:              return GLFW_KEY_7;
        case XK_8:              return GLFW_KEY_8;
        case XK_9:              return GLFW_KEY_9;
        case XK_0:              return GLFW_KEY_0;
        case XK_space:          return GLFW_KEY_SPACE;
        case XK_minus:          return GLFW_KEY_MINUS;
        case XK_equal:          return GLFW_KEY_EQUAL;
        case XK_bracketleft:    return GLFW_KEY_LEFT_BRACKET;
        case XK_bracketright:   return GLFW_KEY_RIGHT_BRACKET;
        case XK_backslash:      return GLFW_KEY_BACKSLASH;
        case XK_semicolon:      return GLFW_KEY_SEMICOLON;
        case XK_apostrophe:     return GLFW_KEY_APOSTROPHE;
        case XK_grave:          return GLFW_KEY_GRAVE_ACCENT;
        case XK_comma:          return GLFW_KEY_COMMA;
        case XK_period:         return GLFW_KEY_PERIOD;
        case XK_slash:          return GLFW_KEY_SLASH;
        case XK_less:           return GLFW_KEY_WORLD_1; // At least in some layouts...
        default:                break;
    }

    // No matching translation was found, so return -1
    return -1;
}


//========================================================================
// Update the key code LUT
//========================================================================

static void updateKeyCodeLUT(void)
{
    int keyCode;

    // Clear the LUT
    for (keyCode = 0;  keyCode < 256;  keyCode++)
        _glfwLibrary.X11.keyCodeLUT[keyCode] = -1;

#if defined(_GLFW_HAS_XKB)
    // If the Xkb extension is available, use it to determine physical key
    // locations independently of the current keyboard layout
    if (_glfwLibrary.X11.Xkb.available)
    {
        int i, keyCodeGLFW;
        char name[XkbKeyNameLength + 1];
        XkbDescPtr descr;

        // Get keyboard description
        descr = XkbGetKeyboard(_glfwLibrary.X11.display,
                               XkbAllComponentsMask,
                               XkbUseCoreKbd);

        // Find the X11 key code -> GLFW key code mapping
        for (keyCode = descr->min_key_code; keyCode <= descr->max_key_code; ++keyCode)
        {
            // Get the key name
            for (i = 0;  i < XkbKeyNameLength;  i++)
                name[i] = descr->names->keys[keyCode].name[i];

            name[XkbKeyNameLength] = 0;

            // Map the key name to a GLFW key code. Note: We only map printable
            // keys here, and we use the US keyboard layout. The rest of the
            // keys (function keys) are mapped using traditional KeySym
            // translations.
            if (strcmp(name, "TLDE") == 0) keyCodeGLFW = GLFW_KEY_GRAVE_ACCENT;
            else if (strcmp(name, "AE01") == 0) keyCodeGLFW = GLFW_KEY_1;
            else if (strcmp(name, "AE02") == 0) keyCodeGLFW = GLFW_KEY_2;
            else if (strcmp(name, "AE03") == 0) keyCodeGLFW = GLFW_KEY_3;
            else if (strcmp(name, "AE04") == 0) keyCodeGLFW = GLFW_KEY_4;
            else if (strcmp(name, "AE05") == 0) keyCodeGLFW = GLFW_KEY_5;
            else if (strcmp(name, "AE06") == 0) keyCodeGLFW = GLFW_KEY_6;
            else if (strcmp(name, "AE07") == 0) keyCodeGLFW = GLFW_KEY_7;
            else if (strcmp(name, "AE08") == 0) keyCodeGLFW = GLFW_KEY_8;
            else if (strcmp(name, "AE09") == 0) keyCodeGLFW = GLFW_KEY_9;
            else if (strcmp(name, "AE10") == 0) keyCodeGLFW = GLFW_KEY_0;
            else if (strcmp(name, "AE11") == 0) keyCodeGLFW = GLFW_KEY_MINUS;
            else if (strcmp(name, "AE12") == 0) keyCodeGLFW = GLFW_KEY_EQUAL;
            else if (strcmp(name, "AD01") == 0) keyCodeGLFW = GLFW_KEY_Q;
            else if (strcmp(name, "AD02") == 0) keyCodeGLFW = GLFW_KEY_W;
            else if (strcmp(name, "AD03") == 0) keyCodeGLFW = GLFW_KEY_E;
            else if (strcmp(name, "AD04") == 0) keyCodeGLFW = GLFW_KEY_R;
            else if (strcmp(name, "AD05") == 0) keyCodeGLFW = GLFW_KEY_T;
            else if (strcmp(name, "AD06") == 0) keyCodeGLFW = GLFW_KEY_Y;
            else if (strcmp(name, "AD07") == 0) keyCodeGLFW = GLFW_KEY_U;
            else if (strcmp(name, "AD08") == 0) keyCodeGLFW = GLFW_KEY_I;
            else if (strcmp(name, "AD09") == 0) keyCodeGLFW = GLFW_KEY_O;
            else if (strcmp(name, "AD10") == 0) keyCodeGLFW = GLFW_KEY_P;
            else if (strcmp(name, "AD11") == 0) keyCodeGLFW = GLFW_KEY_LEFT_BRACKET;
            else if (strcmp(name, "AD12") == 0) keyCodeGLFW = GLFW_KEY_RIGHT_BRACKET;
            else if (strcmp(name, "AC01") == 0) keyCodeGLFW = GLFW_KEY_A;
            else if (strcmp(name, "AC02") == 0) keyCodeGLFW = GLFW_KEY_S;
            else if (strcmp(name, "AC03") == 0) keyCodeGLFW = GLFW_KEY_D;
            else if (strcmp(name, "AC04") == 0) keyCodeGLFW = GLFW_KEY_F;
            else if (strcmp(name, "AC05") == 0) keyCodeGLFW = GLFW_KEY_G;
            else if (strcmp(name, "AC06") == 0) keyCodeGLFW = GLFW_KEY_H;
            else if (strcmp(name, "AC07") == 0) keyCodeGLFW = GLFW_KEY_J;
            else if (strcmp(name, "AC08") == 0) keyCodeGLFW = GLFW_KEY_K;
            else if (strcmp(name, "AC09") == 0) keyCodeGLFW = GLFW_KEY_L;
            else if (strcmp(name, "AC10") == 0) keyCodeGLFW = GLFW_KEY_SEMICOLON;
            else if (strcmp(name, "AC11") == 0) keyCodeGLFW = GLFW_KEY_APOSTROPHE;
            else if (strcmp(name, "AB01") == 0) keyCodeGLFW = GLFW_KEY_Z;
            else if (strcmp(name, "AB02") == 0) keyCodeGLFW = GLFW_KEY_X;
            else if (strcmp(name, "AB03") == 0) keyCodeGLFW = GLFW_KEY_C;
            else if (strcmp(name, "AB04") == 0) keyCodeGLFW = GLFW_KEY_V;
            else if (strcmp(name, "AB05") == 0) keyCodeGLFW = GLFW_KEY_B;
            else if (strcmp(name, "AB06") == 0) keyCodeGLFW = GLFW_KEY_N;
            else if (strcmp(name, "AB07") == 0) keyCodeGLFW = GLFW_KEY_M;
            else if (strcmp(name, "AB08") == 0) keyCodeGLFW = GLFW_KEY_COMMA;
            else if (strcmp(name, "AB09") == 0) keyCodeGLFW = GLFW_KEY_PERIOD;
            else if (strcmp(name, "AB10") == 0) keyCodeGLFW = GLFW_KEY_SLASH;
            else if (strcmp(name, "BKSL") == 0) keyCodeGLFW = GLFW_KEY_BACKSLASH;
            else if (strcmp(name, "LSGT") == 0) keyCodeGLFW = GLFW_KEY_WORLD_1;
            else keyCodeGLFW = -1;

            // Update the key code LUT
            if ((keyCode >= 0) && (keyCode < 256))
                _glfwLibrary.X11.keyCodeLUT[keyCode] = keyCodeGLFW;
        }

        // Free the keyboard description
        XkbFreeKeyboard(descr, 0, True);
    }
#endif /* _GLFW_HAS_XKB */

    // Translate the un-translated key codes using traditional X11 KeySym
    // lookups
    for (keyCode = 0;  keyCode < 256;  keyCode++)
    {
        if (_glfwLibrary.X11.keyCodeLUT[keyCode] < 0)
        {
            _glfwLibrary.X11.keyCodeLUT[keyCode] =
                keyCodeToGLFWKeyCode(keyCode);
        }
    }
}


//========================================================================
// Retrieve a single window property of the specified type
// Inspired by fghGetWindowProperty from freeglut
//========================================================================

static unsigned long getWindowProperty(Window window,
                                       Atom property,
                                       Atom type,
                                       unsigned char** value)
{
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty(_glfwLibrary.X11.display,
                       window,
                       property,
                       0,
                       LONG_MAX,
                       False,
                       type,
                       &actualType,
                       &actualFormat,
                       &itemCount,
                       &bytesAfter,
                       value);

    if (actualType != type)
        return 0;

    return itemCount;
}


//========================================================================
// Check whether the specified atom is supported
//========================================================================

static Atom getSupportedAtom(Atom* supportedAtoms,
                             unsigned long atomCount,
                             const char* atomName)
{
    Atom atom = XInternAtom(_glfwLibrary.X11.display, atomName, True);
    if (atom != None)
    {
        unsigned long i;

        for (i = 0;  i < atomCount;  i++)
        {
            if (supportedAtoms[i] == atom)
                return atom;
        }
    }

    return None;
}


//========================================================================
// Check whether the running window manager is EWMH-compliant
//========================================================================

static void initEWMH(void)
{
    Window* windowFromRoot = NULL;
    Window* windowFromChild = NULL;

    // First we need a couple of atoms, which should already be there
    Atom supportingWmCheck =
        XInternAtom(_glfwLibrary.X11.display, "_NET_SUPPORTING_WM_CHECK", True);
    Atom wmSupported =
        XInternAtom(_glfwLibrary.X11.display, "_NET_SUPPORTED", True);
    if (supportingWmCheck == None || wmSupported == None)
        return;

    // Then we look for the _NET_SUPPORTING_WM_CHECK property of the root window
    if (getWindowProperty(_glfwLibrary.X11.root,
                          supportingWmCheck,
                          XA_WINDOW,
                          (unsigned char**) &windowFromRoot) != 1)
    {
        XFree(windowFromRoot);
        return;
    }

    // It should be the ID of a child window (of the root)
    // Then we look for the same property on the child window
    if (getWindowProperty(*windowFromRoot,
                          supportingWmCheck,
                          XA_WINDOW,
                          (unsigned char**) &windowFromChild) != 1)
    {
        XFree(windowFromRoot);
        XFree(windowFromChild);
        return;
    }

    // It should be the ID of that same child window
    if (*windowFromRoot != *windowFromChild)
    {
        XFree(windowFromRoot);
        XFree(windowFromChild);
        return;
    }

    XFree(windowFromRoot);
    XFree(windowFromChild);

    // We are now fairly sure that an EWMH-compliant window manager is running

    Atom* supportedAtoms;
    unsigned long atomCount;

    // Now we need to check the _NET_SUPPORTED property of the root window
    // It should be a list of supported WM protocol and state atoms
    atomCount = getWindowProperty(_glfwLibrary.X11.root,
                                  wmSupported,
                                  XA_ATOM,
                                  (unsigned char**) &supportedAtoms);

    // See which of the atoms we support that are supported by the WM

    _glfwLibrary.X11.wmState =
        getSupportedAtom(supportedAtoms, atomCount, "_NET_WM_STATE");

    _glfwLibrary.X11.wmStateFullscreen =
        getSupportedAtom(supportedAtoms, atomCount, "_NET_WM_STATE_FULLSCREEN");

    _glfwLibrary.X11.wmName =
        getSupportedAtom(supportedAtoms, atomCount, "_NET_WM_NAME");

    _glfwLibrary.X11.wmIconName =
        getSupportedAtom(supportedAtoms, atomCount, "_NET_WM_ICON_NAME");

    _glfwLibrary.X11.wmPing =
        getSupportedAtom(supportedAtoms, atomCount, "_NET_WM_PING");

    _glfwLibrary.X11.wmActiveWindow =
        getSupportedAtom(supportedAtoms, atomCount, "_NET_ACTIVE_WINDOW");

    XFree(supportedAtoms);

    _glfwLibrary.X11.hasEWMH = GL_TRUE;
}


//========================================================================
// Initialize X11 display and look for supported X11 extensions
//========================================================================

static GLboolean initDisplay(void)
{
    _glfwLibrary.X11.display = XOpenDisplay(NULL);
    if (!_glfwLibrary.X11.display)
    {
        _glfwSetError(GLFW_OPENGL_UNAVAILABLE, "X11/GLX: Failed to open X display");
        return GL_FALSE;
    }

    // As the API currently doesn't understand multiple display devices, we hard-code
    // this choice and hope for the best
    _glfwLibrary.X11.screen = DefaultScreen(_glfwLibrary.X11.display);
    _glfwLibrary.X11.root = RootWindow(_glfwLibrary.X11.display,
                                       _glfwLibrary.X11.screen);

    // Check for XF86VidMode extension
#ifdef _GLFW_HAS_XF86VIDMODE
    _glfwLibrary.X11.VidMode.available =
        XF86VidModeQueryExtension(_glfwLibrary.X11.display,
                                  &_glfwLibrary.X11.VidMode.eventBase,
                                  &_glfwLibrary.X11.VidMode.errorBase);
#else
    _glfwLibrary.X11.VidMode.available = GL_FALSE;
#endif /*_GLFW_HAS_XF86VIDMODE*/

    // Check for XRandR extension
#ifdef _GLFW_HAS_XRANDR
    _glfwLibrary.X11.RandR.available =
        XRRQueryExtension(_glfwLibrary.X11.display,
                          &_glfwLibrary.X11.RandR.eventBase,
                          &_glfwLibrary.X11.RandR.errorBase);

    if (_glfwLibrary.X11.RandR.available)
    {
        if (!XRRQueryVersion(_glfwLibrary.X11.display,
                             &_glfwLibrary.X11.RandR.majorVersion,
                             &_glfwLibrary.X11.RandR.minorVersion))
        {
            _glfwSetError(GLFW_PLATFORM_ERROR,
                          "X11/GLX: Failed to query RandR version");
            return GL_FALSE;
        }
    }
#else
    _glfwLibrary.X11.RandR.available = GL_FALSE;
#endif /*_GLFW_HAS_XRANDR*/

    // Check if Xkb is supported on this display
#if defined(_GLFW_HAS_XKB)
    _glfwLibrary.X11.Xkb.majorVersion = 1;
    _glfwLibrary.X11.Xkb.minorVersion = 0;
    _glfwLibrary.X11.Xkb.available =
        XkbQueryExtension(_glfwLibrary.X11.display,
                          &_glfwLibrary.X11.Xkb.majorOpcode,
                          &_glfwLibrary.X11.Xkb.eventBase,
                          &_glfwLibrary.X11.Xkb.errorBase,
                          &_glfwLibrary.X11.Xkb.majorVersion,
                          &_glfwLibrary.X11.Xkb.minorVersion);
#else
    _glfwLibrary.X11.Xkb.available = GL_FALSE;
#endif /* _GLFW_HAS_XKB */

    // Update the key code LUT
    // FIXME: We should listen to XkbMapNotify events to track changes to
    // the keyboard mapping.
    updateKeyCodeLUT();

    // Find or create selection property atom
    _glfwLibrary.X11.selection.property =
        XInternAtom(_glfwLibrary.X11.display, "GLFW_SELECTION", False);

    // Find or create clipboard atom
    _glfwLibrary.X11.selection.atom =
        XInternAtom(_glfwLibrary.X11.display, "CLIPBOARD", False);

    // Find or create selection target atoms
    _glfwLibrary.X11.selection.formats[_GLFW_CLIPBOARD_FORMAT_UTF8] =
        XInternAtom(_glfwLibrary.X11.display, "UTF8_STRING", False);
    _glfwLibrary.X11.selection.formats[_GLFW_CLIPBOARD_FORMAT_COMPOUND] =
        XInternAtom(_glfwLibrary.X11.display, "COMPOUND_STRING", False);
    _glfwLibrary.X11.selection.formats[_GLFW_CLIPBOARD_FORMAT_STRING] =
        XA_STRING;

    _glfwLibrary.X11.selection.targets = XInternAtom(_glfwLibrary.X11.display,
                                                     "TARGETS",
                                                     False);

    return GL_TRUE;
}


//========================================================================
// Create a blank cursor (for locked mouse mode)
//========================================================================

static Cursor createNULLCursor(void)
{
    Pixmap cursormask;
    XGCValues xgc;
    GC gc;
    XColor col;
    Cursor cursor;

    // TODO: Add error checks

    cursormask = XCreatePixmap(_glfwLibrary.X11.display,
                               _glfwLibrary.X11.root,
                               1, 1, 1);
    xgc.function = GXclear;
    gc = XCreateGC(_glfwLibrary.X11.display, cursormask, GCFunction, &xgc);
    XFillRectangle(_glfwLibrary.X11.display, cursormask, gc, 0, 0, 1, 1);
    col.pixel = 0;
    col.red = 0;
    col.flags = 4;
    cursor = XCreatePixmapCursor(_glfwLibrary.X11.display,
                                 cursormask, cursormask,
                                 &col, &col, 0, 0);
    XFreePixmap(_glfwLibrary.X11.display, cursormask);
    XFreeGC(_glfwLibrary.X11.display, gc);

    return cursor;
}


//========================================================================
// Terminate X11 display
//========================================================================

static void terminateDisplay(void)
{
    if (_glfwLibrary.X11.display)
    {
        XCloseDisplay(_glfwLibrary.X11.display);
        _glfwLibrary.X11.display = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Initialize various GLFW state
//========================================================================

int _glfwPlatformInit(void)
{
    if (!initDisplay())
        return GL_FALSE;

    _glfwInitGammaRamp();

    if (!_glfwInitOpenGL())
        return GL_FALSE;

    initEWMH();

    _glfwLibrary.X11.cursor = createNULLCursor();

    _glfwInitJoysticks();

    // Start the timer
    _glfwInitTimer();

    return GL_TRUE;
}


//========================================================================
// Close window and shut down library
//========================================================================

int _glfwPlatformTerminate(void)
{
    if (_glfwLibrary.X11.cursor)
    {
        XFreeCursor(_glfwLibrary.X11.display, _glfwLibrary.X11.cursor);
        _glfwLibrary.X11.cursor = (Cursor) 0;
    }

    _glfwTerminateGammaRamp();

    terminateDisplay();

    _glfwTerminateJoysticks();

    _glfwTerminateOpenGL();

    // Free clipboard memory
    if (_glfwLibrary.X11.selection.string)
        free(_glfwLibrary.X11.selection.string);

    return GL_TRUE;
}


//========================================================================
// Get the GLFW version string
//========================================================================

const char* _glfwPlatformGetVersionString(void)
{
    const char* version = _GLFW_VERSION_FULL
#if defined(_GLFW_HAS_XRANDR)
        " XRandR"
#endif
#if defined(_GLFW_HAS_XF86VIDMODE)
        " Xf86VidMode"
#endif
#if !defined(_GLFW_HAS_XRANDR) && !defined(_GLFW_HAS_XF86VIDMODE)
        " no-mode-switching-support"
#endif
#if defined(_GLFW_HAS_XKB)
        " Xkb"
#endif
#if defined(_GLFW_HAS_GLXGETPROCADDRESS)
        " glXGetProcAddress"
#elif defined(_GLFW_HAS_GLXGETPROCADDRESSARB)
        " glXGetProcAddressARB"
#elif defined(_GLFW_HAS_GLXGETPROCADDRESSEXT)
        " glXGetProcAddressEXT"
#elif defined(_GLFW_DLOPEN_LIBGL)
        " dlsym(libGL)"
#else
        " no-extension-support"
#endif
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
        " clock_gettime"
#endif
#if defined(_GLFW_USE_LINUX_JOYSTICKS)
        " Linux-joystick-API"
#else
        " no-joystick-support"
#endif
#if defined(_GLFW_BUILD_DLL)
        " shared"
#endif
        ;

    return version;
}

