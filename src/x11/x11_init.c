//========================================================================
// GLFW - An OpenGL framework
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

#include <stdio.h>
#include <stdlib.h>


//========================================================================
// Dynamically load libraries
//========================================================================

static void initLibraries(void)
{
#ifdef _GLFW_DLOPEN_LIBGL
    int i;
    char* libGL_names[ ] =
    {
        "libGL.so",
        "libGL.so.1",
        "/usr/lib/libGL.so",
        "/usr/lib/libGL.so.1",
        NULL
    };

    _glfwLibrary.X11.libGL = NULL;
    for (i = 0;  libGL_names[i] != NULL;  i++)
    {
        _glfwLibrary.X11.libGL = dlopen(libGL_names[i], RTLD_LAZY | RTLD_GLOBAL);
        if (_glfwLibrary.X11.libGL)
            break;
    }
#endif
}


//========================================================================
// Update the key code LUT
//========================================================================

static void updateKeyCodeLUT(void)
{
    int i, keyCode, keyCodeGLFW;
    char name[XkbKeyNameLength+1];
    XkbDescPtr descr;

    // Clear the LUT
    for (i = 0; i < 256; ++i)
    {
        _glfwLibrary.X11.Xkb.keyCodeLUT[i] = -1;
    }

    // This functionality requires the Xkb extension
    if (!_glfwLibrary.X11.Xkb.available)
    {
        return;
    }

    // Get keyboard description
    descr = XkbGetKeyboard(_glfwLibrary.X11.display,
                           XkbAllComponentsMask,
                           XkbUseCoreKbd);

    // Find the X11 key code -> GLFW key code mapping
    for (keyCode = descr->min_key_code; keyCode <= descr->max_key_code; ++keyCode)
    {
        // Get the key name
        for (i = 0; i < XkbKeyNameLength; ++i)
        {
            name[i] = descr->names->keys[keyCode].name[i];
        }
        name[XkbKeyNameLength] = 0;

        // Map the key name to a GLFW key code. Note: We only map printable
        // keys here, and we use the US keyboard layout.
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
        else if (strcmp(name, "AD11") == 0) keyCodeGLFW = GLFW_KEY_LEFT_SQUARE_BRACKET;
        else if (strcmp(name, "AD12") == 0) keyCodeGLFW = GLFW_KEY_RIGHT_SQUARE_BRACKET;
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
        {
            _glfwLibrary.X11.Xkb.keyCodeLUT[keyCode] = keyCodeGLFW;
        }
    }

    // Free the keyboard description
    XkbFreeKeyboard(descr, 0, True);
}


//========================================================================
// Initialize X11 display and look for supported X11 extensions
//========================================================================

static GLboolean initDisplay(void)
{
    _glfwLibrary.X11.display = XOpenDisplay(0);
    if (!_glfwLibrary.X11.display)
    {
        fprintf(stderr, "Failed to open X display\n");
        _glfwSetError(GLFW_OPENGL_UNAVAILABLE);
        return GL_FALSE;
    }

    // As the API currently doesn't understand multiple display devices, we hardcode
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

    if (!XRRQueryVersion(_glfwLibrary.X11.display,
                        &_glfwLibrary.X11.RandR.majorVersion,
                        &_glfwLibrary.X11.RandR.minorVersion))
    {
        fprintf(stderr, "Unable to query RandR version number\n");
    }
#else
    _glfwLibrary.X11.RandR.available = GL_FALSE;
#endif /*_GLFW_HAS_XRANDR*/

    // Check if GLX is supported on this display
    if (!glXQueryExtension(_glfwLibrary.X11.display, NULL, NULL))
    {
        fprintf(stderr, "GLX not supported\n");
        _glfwSetError(GLFW_OPENGL_UNAVAILABLE);
        return GL_FALSE;
    }

    if (!glXQueryVersion(_glfwLibrary.X11.display,
                         &_glfwLibrary.X11.glxMajor,
                         &_glfwLibrary.X11.glxMinor))
    {
        fprintf(stderr, "Unable to query GLX version\n");
        _glfwSetError(GLFW_OPENGL_UNAVAILABLE);
        return GL_FALSE;
    }

    // Check if Xkb is supported on this display
    _glfwLibrary.X11.Xkb.majorVersion = 1;
    _glfwLibrary.X11.Xkb.minorVersion = 0;
    _glfwLibrary.X11.Xkb.available =
        XkbQueryExtension(_glfwLibrary.X11.display,
                          &_glfwLibrary.X11.Xkb.majorOpcode,
                          &_glfwLibrary.X11.Xkb.eventBase,
                          &_glfwLibrary.X11.Xkb.errorBase,
                          &_glfwLibrary.X11.Xkb.majorVersion,
                          &_glfwLibrary.X11.Xkb.minorVersion);

    // Update the key code LUT
    // FIXME: We should listen to XkbMapNotify events to track changes to
    // the keyboard mapping.
    updateKeyCodeLUT();

    return GL_TRUE;
}


//========================================================================
// Detect gamma ramp support and save original gamma ramp, if available
//========================================================================

static void initGammaRamp(void)
{
#ifdef _GLFW_HAS_XRANDR
    // RandR gamma support is only available with version 1.2 and above
    if (_glfwLibrary.X11.RandR.available &&
        (_glfwLibrary.X11.RandR.majorVersion > 1 ||
         _glfwLibrary.X11.RandR.majorVersion == 1 &&
         _glfwLibrary.X11.RandR.minorVersion >= 2))
    {
        // FIXME: Assumes that all monitors have the same size gamma tables
        // This is reasonable as I suspect the that if they did differ, it
        // would imply that setting the gamma size to an arbitary size is
        // possible as well.
        XRRScreenResources* rr = XRRGetScreenResources(_glfwLibrary.X11.display,
                                                       _glfwLibrary.X11.root);

        _glfwLibrary.originalRampSize = XRRGetCrtcGammaSize(_glfwLibrary.X11.display,
                                                            rr->crtcs[0]);
        if (!_glfwLibrary.originalRampSize)
        {
            // This is probably Nvidia RandR with broken gamma support
            // Flag it as useless and try Xf86VidMode below, if available
            _glfwLibrary.X11.RandR.gammaBroken = GL_TRUE;
            fprintf(stderr, "Ignoring broken nVidia implementation of RandR 1.2+ gamma\n");
        }

        XRRFreeScreenResources(rr);
    }
#endif /*_GLFW_HAS_XRANDR*/

#if defined(_GLFW_HAS_XF86VIDMODE)
    if (_glfwLibrary.X11.VidMode.available &&
        !_glfwLibrary.originalRampSize)
    {
        // Get the gamma size using XF86VidMode
        XF86VidModeGetGammaRampSize(_glfwLibrary.X11.display,
                                    _glfwLibrary.X11.screen,
                                    &_glfwLibrary.originalRampSize);
    }
#endif /*_GLFW_HAS_XF86VIDMODE*/

    if (!_glfwLibrary.originalRampSize)
        fprintf(stderr, "No supported gamma ramp API found\n");

    // Save the original gamma ramp
    _glfwPlatformGetGammaRamp(&_glfwLibrary.originalRamp);
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

    cursormask = XCreatePixmap(_glfwLibrary.X11.display, _glfwLibrary.X11.root, 1, 1, 1);
    xgc.function = GXclear;
    gc = XCreateGC(_glfwLibrary.X11.display, cursormask, GCFunction, &xgc);
    XFillRectangle(_glfwLibrary.X11.display, cursormask, gc, 0, 0, 1, 1);
    col.pixel = 0;
    col.red = 0;
    col.flags = 4;
    cursor = XCreatePixmapCursor(_glfwLibrary.X11.display, cursormask, cursormask,
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
    if (_glfwLibrary.originalRampSize)
        _glfwPlatformSetGammaRamp(&_glfwLibrary.originalRamp);

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

    initGammaRamp();

    _glfwLibrary.X11.cursor = createNULLCursor();

    // Try to load libGL.so if necessary
    initLibraries();

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

    terminateDisplay();

    _glfwTerminateJoysticks();

    // Unload libGL.so if necessary
#ifdef _GLFW_DLOPEN_LIBGL
    if (_glfwLibrary.X11.libGL != NULL)
    {
        dlclose(_glfwLibrary.X11.libGL);
        _glfwLibrary.X11.libGL = NULL;
    }
#endif

    return GL_TRUE;
}


//========================================================================
// Get the GLFW version string
//========================================================================

const char* _glfwPlatformGetVersionString(void)
{
    const char* version = "GLFW " _GLFW_VERSION_FULL
#if defined(_GLFW_HAS_XRANDR)
        " XRandR"
#endif
#if defined(_GLFW_HAS_XF86VIDMODE)
        " Xf86VidMode"
#endif
#if !defined(_GLFW_HAS_XRANDR) && !defined(_GLFW_HAS_XF86VIDMODE)
        " no-mode-switching-support"
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
#if defined(_GLFW_USE_LINUX_JOYSTICKS)
        " Linux-joystick-API"
#else
        " no-joystick-support"
#endif
        ;

    return version;
}

