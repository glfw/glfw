//========================================================================
// GLFW - An OpenGL library
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


//========================================================================
// Enable and disable sticky keys mode
//========================================================================

static void enableStickyKeys(_GLFWwindow* window)
{
    window->stickyKeys = GL_TRUE;
}

static void disableStickyKeys(_GLFWwindow* window)
{
    int i;

    window->stickyKeys = GL_FALSE;

    // Release all sticky keys
    for (i = 0;  i <= GLFW_KEY_LAST;  i++)
    {
        if (window->key[i] == GLFW_STICK)
            window->key[i] = GLFW_RELEASE;
    }
}


//========================================================================
// Enable and disable sticky mouse buttons mode
//========================================================================

static void enableStickyMouseButtons(_GLFWwindow* window)
{
    window->stickyMouseButtons = GL_TRUE;
}

static void disableStickyMouseButtons(_GLFWwindow* window)
{
    int i;

    window->stickyMouseButtons = GL_FALSE;

    // Release all sticky mouse buttons
    for (i = 0;  i <= GLFW_MOUSE_BUTTON_LAST;  i++)
    {
        if (window->mouseButton[i] == GLFW_STICK)
            window->mouseButton[i] = GLFW_RELEASE;
    }
}


//========================================================================
// Enable and disable system keys
//========================================================================

static void enableSystemKeys(_GLFWwindow* window)
{
    if (!window->sysKeysDisabled)
        return;

    _glfwPlatformEnableSystemKeys(window);

    // Indicate that system keys are no longer disabled
    window->sysKeysDisabled = GL_FALSE;
}

static void disableSystemKeys(_GLFWwindow* window)
{
    if (window->sysKeysDisabled)
        return;

    _glfwPlatformDisableSystemKeys(window);

    // Indicate that system keys are now disabled
    window->sysKeysDisabled = GL_TRUE;
}


//========================================================================
// Enable and disable key repeat
//========================================================================

static void enableKeyRepeat(_GLFWwindow* window)
{
    window->keyRepeat = GL_TRUE;
}

static void disableKeyRepeat(_GLFWwindow* window)
{
    window->keyRepeat = GL_FALSE;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Enable certain GLFW/window/system functions
//========================================================================

GLFWAPI void glfwEnable(GLFWwindow window, int token)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    switch (token)
    {
        case GLFW_STICKY_KEYS:
            enableStickyKeys(window);
            break;
        case GLFW_STICKY_MOUSE_BUTTONS:
            enableStickyMouseButtons(window);
            break;
        case GLFW_SYSTEM_KEYS:
            enableSystemKeys(window);
            break;
        case GLFW_KEY_REPEAT:
            enableKeyRepeat(window);
            break;
        default:
            _glfwSetError(GLFW_INVALID_ENUM, NULL);
            break;
    }
}


//========================================================================
// Disable certain GLFW/window/system functions
//========================================================================

GLFWAPI void glfwDisable(GLFWwindow window, int token)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    switch (token)
    {
        case GLFW_STICKY_KEYS:
            disableStickyKeys(window);
            break;
        case GLFW_STICKY_MOUSE_BUTTONS:
            disableStickyMouseButtons(window);
            break;
        case GLFW_SYSTEM_KEYS:
            disableSystemKeys(window);
            break;
        case GLFW_KEY_REPEAT:
            disableKeyRepeat(window);
            break;
        default:
            _glfwSetError(GLFW_INVALID_ENUM, NULL);
            break;
    }
}

