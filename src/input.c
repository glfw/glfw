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


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Register keyboard activity
//========================================================================

void _glfwInputKey(_GLFWwindow* window, int key, int action)
{
    GLboolean keyrepeat = GL_FALSE;

    if (key < 0 || key > GLFW_KEY_LAST)
        return;

    // Are we trying to release an already released key?
    if (action == GLFW_RELEASE && window->key[key] != GLFW_PRESS)
        return;

    // Register key action
    if(action == GLFW_RELEASE && window->stickyKeys)
        window->key[key] = GLFW_STICK;
    else
    {
        keyrepeat = (window->key[key] == GLFW_PRESS) && (action == GLFW_PRESS);
        window->key[key] = (char) action;
    }

    // Call user callback function
    if (_glfwLibrary.keyCallback && (window->keyRepeat || !keyrepeat))
        _glfwLibrary.keyCallback(window, key, action);
}


//========================================================================
// Register (keyboard) character activity
//========================================================================

void _glfwInputChar(_GLFWwindow* window, int character)
{
    // Valid Unicode (ISO 10646) character?
    if (!((character >= 32 && character <= 126) || character >= 160))
        return;

    if (_glfwLibrary.charCallback)
        _glfwLibrary.charCallback(window, character);
}


//========================================================================
// Register scroll events
//========================================================================

void _glfwInputScroll(_GLFWwindow* window, int xoffset, int yoffset)
{
    window->scrollX += xoffset;
    window->scrollY += yoffset;

    if (_glfwLibrary.scrollCallback)
        _glfwLibrary.scrollCallback(window, xoffset, yoffset);
}


//========================================================================
// Register mouse button clicks
//========================================================================

void _glfwInputMouseClick(_GLFWwindow* window, int button, int action)
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
        return;

    // Register mouse button action
    if (action == GLFW_RELEASE && window->stickyMouseButtons)
        window->mouseButton[button] = GLFW_STICK;
    else
        window->mouseButton[button] = (char) action;

    if (_glfwLibrary.mouseButtonCallback)
        _glfwLibrary.mouseButtonCallback(window, button, action);
}


//========================================================================
// Register cursor moves
//========================================================================

void _glfwInputCursorMotion(_GLFWwindow* window, int x, int y)
{
    if (window->cursorMode == GLFW_CURSOR_CAPTURED)
    {
        if (!x && !y)
            return;

        window->cursorPosX += x;
        window->cursorPosY += y;
    }
    else
    {
        if (window->cursorPosX == x && window->cursorPosY == y)
            return;

        window->cursorPosX = x;
        window->cursorPosY = y;
    }

    if (_glfwLibrary.mousePosCallback)
        _glfwLibrary.mousePosCallback(window,
                                      window->cursorPosX,
                                      window->cursorPosY);
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Returns the state of the specified key for the specified window
//========================================================================

GLFWAPI int glfwGetKey(GLFWwindow handle, int key)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return GLFW_RELEASE;
    }

    // Is it a valid key?
    if (key < 0 || key > GLFW_KEY_LAST)
    {
        // TODO: Decide whether key is a value or enum
        _glfwSetError(GLFW_INVALID_ENUM,
                      "glfwGetKey: The specified key is invalid");
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

GLFWAPI int glfwGetMouseButton(GLFWwindow handle, int button)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return GLFW_RELEASE;
    }

    // Is it a valid mouse button?
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
    {
        _glfwSetError(GLFW_INVALID_ENUM,
                      "glfwGetMouseButton: The specified mouse button is invalid");
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

GLFWAPI void glfwGetMousePos(GLFWwindow handle, int* xpos, int* ypos)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    // Return mouse position
    if (xpos != NULL)
        *xpos = window->cursorPosX;

    if (ypos != NULL)
        *ypos = window->cursorPosY;
}


//========================================================================
// Sets the cursor position relative to the position of the client area of
// the specified window
//========================================================================

GLFWAPI void glfwSetMousePos(GLFWwindow handle, int xpos, int ypos)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (_glfwLibrary.activeWindow != window)
    {
        _glfwSetError(GLFW_WINDOW_NOT_ACTIVE, NULL);
        return;
    }

    // Don't do anything if the mouse position did not change
    if (xpos == window->cursorPosX && ypos == window->cursorPosY)
        return;

    // Set GLFW mouse position
    window->cursorPosX = xpos;
    window->cursorPosY = ypos;

    // Do not move physical cursor in locked cursor mode
    if (window->cursorMode == GLFW_CURSOR_CAPTURED)
        return;

    // Update physical cursor position
    _glfwPlatformSetMouseCursorPos(window, xpos, ypos);
}


//========================================================================
// Returns the scroll offset for the specified window
//========================================================================

GLFWAPI void glfwGetScrollOffset(GLFWwindow handle, int* xoffset, int* yoffset)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (xoffset)
      *xoffset = window->scrollX;

    if (yoffset)
      *yoffset = window->scrollY;
}


//========================================================================
// Sets the cursor mode for the specified window
//========================================================================

GLFWAPI void glfwSetCursorMode(GLFWwindow handle, int mode)
{
    int centerPosX, centerPosY;
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    if (mode != GLFW_CURSOR_NORMAL &&
        mode != GLFW_CURSOR_HIDDEN &&
        mode != GLFW_CURSOR_CAPTURED)
    {
        _glfwSetError(GLFW_INVALID_ENUM, NULL);
        return;
    }

    if (window->cursorMode == mode)
        return;

    centerPosX = window->width / 2;
    centerPosY = window->height / 2;

    if (mode == GLFW_CURSOR_CAPTURED)
        _glfwPlatformSetMouseCursorPos(window, centerPosX, centerPosY);
    else if (window->cursorMode == GLFW_CURSOR_CAPTURED)
    {
        _glfwPlatformSetMouseCursorPos(window, centerPosX, centerPosY);
        _glfwInputCursorMotion(window,
                               centerPosX - window->cursorPosX,
                               centerPosY - window->cursorPosY);
    }

    _glfwPlatformSetCursorMode(window, mode);

    window->cursorMode = mode;
}


//========================================================================
// Set callback function for keyboard input
//========================================================================

GLFWAPI void glfwSetKeyCallback(GLFWkeyfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
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
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
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
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
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
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
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
            cbfun(window, window->cursorPosX, window->cursorPosY);
    }
}


//========================================================================
// Set callback function for scroll events
//========================================================================

GLFWAPI void glfwSetScrollCallback(GLFWscrollfun cbfun)
{
    if (!_glfwInitialized)
    {
        _glfwSetError(GLFW_NOT_INITIALIZED, NULL);
        return;
    }

    // Set callback function
    _glfwLibrary.scrollCallback = cbfun;
}

