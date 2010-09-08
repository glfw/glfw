//========================================================================
// GLFW - An OpenGL framework
// Platform:    Any
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


//************************************************************************
//****                  GLFW internal functions                       ****
//************************************************************************

//========================================================================
// Enable (show) mouse cursor
//========================================================================

static void enableMouseCursor(void)
{
    int centerPosX, centerPosY;

    if (!_glfwWin.opened || !_glfwWin.mouseLock)
        return;

    // Show mouse cursor
    _glfwPlatformShowMouseCursor();

    centerPosX = _glfwWin.width / 2;
    centerPosY = _glfwWin.height / 2;

    if (centerPosX != _glfwInput.MousePosX || centerPosY != _glfwInput.MousePosY)
    {
        _glfwPlatformSetMouseCursorPos(centerPosX, centerPosY);

        _glfwInput.MousePosX = centerPosX;
        _glfwInput.MousePosY = centerPosY;

        if (_glfwWin.mousePosCallback)
        {
            _glfwWin.mousePosCallback(_glfwInput.MousePosX,
                                      _glfwInput.MousePosY);
        }
    }

    // From now on the mouse is unlocked
    _glfwWin.mouseLock = GL_FALSE;
}

//========================================================================
// Disable (hide) mouse cursor
//========================================================================

static void disableMouseCursor(void)
{
    if (!_glfwWin.opened || _glfwWin.mouseLock)
        return;

    // Hide mouse cursor
    _glfwPlatformHideMouseCursor();

    // Move cursor to the middle of the window
    _glfwPlatformSetMouseCursorPos(_glfwWin.width >> 1,
                                   _glfwWin.height >> 1);

    // From now on the mouse is locked
    _glfwWin.mouseLock = GL_TRUE;
}


//========================================================================
// Enable sticky keys
//========================================================================

static void enableStickyKeys(void)
{
    _glfwInput.StickyKeys = 1;
}

//========================================================================
// Disable sticky keys
//========================================================================

static void disableStickyKeys(void)
{
    int i;

    _glfwInput.StickyKeys = 0;

    // Release all sticky keys
    for (i = 0;  i <= GLFW_KEY_LAST;  i++)
    {
        if (_glfwInput.Key[i] == 2)
            _glfwInput.Key[i] = 0;
    }
}


//========================================================================
// Enable sticky mouse buttons
//========================================================================

static void enableStickyMouseButtons(void)
{
    _glfwInput.StickyMouseButtons = 1;
}

//========================================================================
// Disable sticky mouse buttons
//========================================================================

static void disableStickyMouseButtons(void)
{
    int i;

    _glfwInput.StickyMouseButtons = 0;

    // Release all sticky mouse buttons
    for (i = 0;  i <= GLFW_MOUSE_BUTTON_LAST;  i++)
    {
        if (_glfwInput.MouseButton[i] == 2)
            _glfwInput.MouseButton[i] = 0;
    }
}


//========================================================================
// Enable system keys
//========================================================================

static void enableSystemKeys(void)
{
    if (!_glfwWin.sysKeysDisabled)
        return;

    _glfwPlatformEnableSystemKeys();

    // Indicate that system keys are no longer disabled
    _glfwWin.sysKeysDisabled = GL_FALSE;
}

//========================================================================
// Disable system keys
//========================================================================

static void disableSystemKeys(void)
{
    if (_glfwWin.sysKeysDisabled)
        return;

    _glfwPlatformDisableSystemKeys();

    // Indicate that system keys are now disabled
    _glfwWin.sysKeysDisabled = GL_TRUE;
}


//========================================================================
// Enable key repeat
//========================================================================

static void enableKeyRepeat(void)
{
    _glfwInput.KeyRepeat = 1;
}

//========================================================================
// Disable key repeat
//========================================================================

static void disableKeyRepeat(void)
{
    _glfwInput.KeyRepeat = 0;
}


//************************************************************************
//****                    GLFW user functions                         ****
//************************************************************************

//========================================================================
// Enable certain GLFW/window/system functions.
//========================================================================

GLFWAPI void glfwEnable(int token)
{
    if (!_glfwInitialized)
        return;

    switch (token)
    {
        case GLFW_MOUSE_CURSOR:
            enableMouseCursor();
            break;
        case GLFW_STICKY_KEYS:
            enableStickyKeys();
            break;
        case GLFW_STICKY_MOUSE_BUTTONS:
            enableStickyMouseButtons();
            break;
        case GLFW_SYSTEM_KEYS:
            enableSystemKeys();
            break;
        case GLFW_KEY_REPEAT:
            enableKeyRepeat();
            break;
        default:
            break;
    }
}


//========================================================================
// Disable certain GLFW/window/system functions.
//========================================================================

GLFWAPI void glfwDisable(int token)
{
    if (!_glfwInitialized)
        return;

    switch (token)
    {
        case GLFW_MOUSE_CURSOR:
            disableMouseCursor();
            break;
        case GLFW_STICKY_KEYS:
            disableStickyKeys();
            break;
        case GLFW_STICKY_MOUSE_BUTTONS:
            disableStickyMouseButtons();
            break;
        case GLFW_SYSTEM_KEYS:
            disableSystemKeys();
            break;
        case GLFW_KEY_REPEAT:
            disableKeyRepeat();
            break;
        default:
            break;
    }
}

