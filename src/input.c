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


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Returns the state of the specified key for the specified window
//========================================================================

GLFWAPI int glfwGetKey(GLFWwindow window, int key)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return GLFW_RELEASE;
    }

    // Is it a valid key?
    if (key < 0 || key > GLFW_KEY_LAST)
    {
        // TODO: Decide whether key is a value or enum
        _glfwSetError(GLFW_INVALID_VALUE);
        return GLFW_RELEASE;
    }

    if (window->key[key] == GLFW_STICK)
    {
        // Sticky mode: release key now
        window->key[key] = GLFW_RELEASE;
        return GLFW_PRESS;
    }

    return (int) window->key[key];
}


//========================================================================
// Returns the state of the specified mouse button for the specified window
//========================================================================

GLFWAPI int glfwGetMouseButton(GLFWwindow window, int button)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return GLFW_RELEASE;
    }

    // Is it a valid mouse button?
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
    {
        _glfwSetError(GLFW_INVALID_ENUM);
        return GLFW_RELEASE;
    }

    if (window->mouseButton[button] == GLFW_STICK)
    {
        // Sticky mode: release mouse button now
        window->mouseButton[button] = GLFW_RELEASE;
        return GLFW_PRESS;
    }

    return (int) window->mouseButton[button];
}


//========================================================================
// Returns the last reported cursor position for the specified window
//========================================================================

GLFWAPI void glfwGetMousePos(GLFWwindow window, int* xpos, int* ypos)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return;
    }

    // Return mouse position
    if (xpos != NULL)
        *xpos = window->mousePosX;

    if (ypos != NULL)
        *ypos = window->mousePosY;
}


//========================================================================
// Sets the cursor position relative to the position of the client area of
// the specified window
//========================================================================

GLFWAPI void glfwSetMousePos(GLFWwindow window, int xpos, int ypos)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return;
    }

    // Don't do anything if the mouse position did not change
    if (xpos == window->mousePosX && ypos == window->mousePosY)
        return;

    // Set GLFW mouse position
    window->mousePosX = xpos;
    window->mousePosY = ypos;

    // If we have a locked mouse, do not change cursor position
    if (_glfwLibrary.cursorLockWindow == window)
        return;

    // Update physical cursor position
    _glfwPlatformSetMouseCursorPos(window, xpos, ypos);
}


//========================================================================
// Returns the scroll offset for the specified window
//========================================================================

GLFWAPI void glfwGetScrollOffset(GLFWwindow window, int* x, int* y)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return;
    }

    if (x)
      *x = window->scrollX;

    if (y)
      *y = window->scrollY;
}


//========================================================================
// Set callback function for keyboard input
//========================================================================

GLFWAPI void glfwSetKeyCallback(GLFWkeyfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return;
    }

    _glfwLibrary.keyCallback = cbfun;
}


//========================================================================
// Set callback function for character input
//========================================================================

GLFWAPI void glfwSetCharCallback(GLFWcharfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return;
    }

    _glfwLibrary.charCallback = cbfun;
}


//========================================================================
// Set callback function for mouse clicks
//========================================================================

GLFWAPI void glfwSetMouseButtonCallback(GLFWmousebuttonfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return;
    }

    _glfwLibrary.mouseButtonCallback = cbfun;
}


//========================================================================
// Set callback function for mouse moves
//========================================================================

GLFWAPI void glfwSetMousePosCallback(GLFWmouseposfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return;
    }

    // Set callback function
    _glfwLibrary.mousePosCallback = cbfun;

    // Call the callback function to let the application know the current
    // mouse position
    if (cbfun)
    {
        _GLFWwindow* window;

        for (window = _glfwLibrary.windowListHead;  window;  window = window->next)
            cbfun(window, window->mousePosX, window->mousePosY);
    }
}


//========================================================================
// Set callback function for scroll events
//========================================================================

GLFWAPI void glfwSetScrollCallback(GLFWscrollfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED);
        return;
    }

    // Set callback function
    _glfwLibrary.scrollCallback = cbfun;
}

