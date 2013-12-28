//========================================================================
// GLFW 3.0 Win32 - www.glfw.org
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
#include <malloc.h>
#include <windowsx.h>

#define _GLFW_KEY_INVALID -2


// Updates the cursor clip rect
//
static void updateClipRect(_GLFWwindow* window)
{
    RECT clipRect;
    GetClientRect(window->win32.handle, &clipRect);
    ClientToScreen(window->win32.handle, (POINT*) &clipRect.left);
    ClientToScreen(window->win32.handle, (POINT*) &clipRect.right);
    ClipCursor(&clipRect);
}

// Hide mouse cursor
//
static void hideCursor(_GLFWwindow* window)
{
    POINT pos;

    ReleaseCapture();
    ClipCursor(NULL);

    if (window->win32.cursorHidden)
    {
        ShowCursor(TRUE);
        window->win32.cursorHidden = GL_FALSE;
    }

    if (GetCursorPos(&pos))
    {
        if (WindowFromPoint(pos) == window->win32.handle)
            SetCursor(NULL);
    }
}

// Capture mouse cursor
//
static void captureCursor(_GLFWwindow* window)
{
    if (!window->win32.cursorHidden)
    {
        ShowCursor(FALSE);
        window->win32.cursorHidden = GL_TRUE;
    }

    updateClipRect(window);
    SetCapture(window->win32.handle);
}

// Show mouse cursor
//
static void showCursor(_GLFWwindow* window)
{
    POINT pos;

    ReleaseCapture();
    ClipCursor(NULL);

    if (window->win32.cursorHidden)
    {
        ShowCursor(TRUE);
        window->win32.cursorHidden = GL_FALSE;
    }

    if (GetCursorPos(&pos))
    {
        if (WindowFromPoint(pos) == window->win32.handle)
            SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
}

// Retrieves and translates modifier keys
//
static int getKeyMods(void)
{
    int mods = 0;

    if (GetKeyState(VK_SHIFT) & (1 << 31))
        mods |= GLFW_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & (1 << 31))
        mods |= GLFW_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & (1 << 31))
        mods |= GLFW_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & (1 << 31))
        mods |= GLFW_MOD_SUPER;

    return mods;
}

// Retrieves and translates modifier keys
//
static int getAsyncKeyMods(void)
{
    int mods = 0;

    if (GetAsyncKeyState(VK_SHIFT) & (1 << 31))
        mods |= GLFW_MOD_SHIFT;
    if (GetAsyncKeyState(VK_CONTROL) & (1 << 31))
        mods |= GLFW_MOD_CONTROL;
    if (GetAsyncKeyState(VK_MENU) & (1 << 31))
        mods |= GLFW_MOD_ALT;
    if ((GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & (1 << 31))
        mods |= GLFW_MOD_SUPER;

    return mods;
}

// Translates a Windows key to the corresponding GLFW key
//
static int translateKey(WPARAM wParam, LPARAM lParam)
{
    // Check for numeric keypad keys
    // NOTE: This way we always force "NumLock = ON", which is intentional since
    //       the returned key code should correspond to a physical location.
    if ((HIWORD(lParam) & 0x100) == 0)
    {
        switch (MapVirtualKey(HIWORD(lParam) & 0xFF, 1))
        {
            case VK_INSERT:   return GLFW_KEY_KP_0;
            case VK_END:      return GLFW_KEY_KP_1;
            case VK_DOWN:     return GLFW_KEY_KP_2;
            case VK_NEXT:     return GLFW_KEY_KP_3;
            case VK_LEFT:     return GLFW_KEY_KP_4;
            case VK_CLEAR:    return GLFW_KEY_KP_5;
            case VK_RIGHT:    return GLFW_KEY_KP_6;
            case VK_HOME:     return GLFW_KEY_KP_7;
            case VK_UP:       return GLFW_KEY_KP_8;
            case VK_PRIOR:    return GLFW_KEY_KP_9;
            case VK_DIVIDE:   return GLFW_KEY_KP_DIVIDE;
            case VK_MULTIPLY: return GLFW_KEY_KP_MULTIPLY;
            case VK_SUBTRACT: return GLFW_KEY_KP_SUBTRACT;
            case VK_ADD:      return GLFW_KEY_KP_ADD;
            case VK_DELETE:   return GLFW_KEY_KP_DECIMAL;
            default:          break;
        }
    }

    // Check which key was pressed or released
    switch (wParam)
    {
        // The SHIFT keys require special handling
        case VK_SHIFT:
        {
            // Compare scan code for this key with that of VK_RSHIFT in
            // order to determine which shift key was pressed (left or
            // right)
            const DWORD scancode = MapVirtualKey(VK_RSHIFT, 0);
            if ((DWORD) ((lParam & 0x01ff0000) >> 16) == scancode)
                return GLFW_KEY_RIGHT_SHIFT;

            return GLFW_KEY_LEFT_SHIFT;
        }

        // The CTRL keys require special handling
        case VK_CONTROL:
        {
            MSG next;
            DWORD time;

            // Is this an extended key (i.e. right key)?
            if (lParam & 0x01000000)
                return GLFW_KEY_RIGHT_CONTROL;

            // Here is a trick: "Alt Gr" sends LCTRL, then RALT. We only
            // want the RALT message, so we try to see if the next message
            // is a RALT message. In that case, this is a false LCTRL!
            time = GetMessageTime();

            if (PeekMessage(&next, NULL, 0, 0, PM_NOREMOVE))
            {
                if (next.message == WM_KEYDOWN ||
                    next.message == WM_SYSKEYDOWN ||
                    next.message == WM_KEYUP ||
                    next.message == WM_SYSKEYUP)
                {
                    if (next.wParam == VK_MENU &&
                        (next.lParam & 0x01000000) &&
                        next.time == time)
                    {
                        // Next message is a RALT down message, which
                        // means that this is not a proper LCTRL message
                        return _GLFW_KEY_INVALID;
                    }
                }
            }

            return GLFW_KEY_LEFT_CONTROL;
        }

        // The ALT keys require special handling
        case VK_MENU:
        {
            // Is this an extended key (i.e. right key)?
            if (lParam & 0x01000000)
                return GLFW_KEY_RIGHT_ALT;

            return GLFW_KEY_LEFT_ALT;
        }

        // The ENTER keys require special handling
        case VK_RETURN:
        {
            // Is this an extended key (i.e. right key)?
            if (lParam & 0x01000000)
                return GLFW_KEY_KP_ENTER;

            return GLFW_KEY_ENTER;
        }

        // Funcion keys (non-printable keys)
        case VK_ESCAPE:        return GLFW_KEY_ESCAPE;
        case VK_TAB:           return GLFW_KEY_TAB;
        case VK_BACK:          return GLFW_KEY_BACKSPACE;
        case VK_HOME:          return GLFW_KEY_HOME;
        case VK_END:           return GLFW_KEY_END;
        case VK_PRIOR:         return GLFW_KEY_PAGE_UP;
        case VK_NEXT:          return GLFW_KEY_PAGE_DOWN;
        case VK_INSERT:        return GLFW_KEY_INSERT;
        case VK_DELETE:        return GLFW_KEY_DELETE;
        case VK_LEFT:          return GLFW_KEY_LEFT;
        case VK_UP:            return GLFW_KEY_UP;
        case VK_RIGHT:         return GLFW_KEY_RIGHT;
        case VK_DOWN:          return GLFW_KEY_DOWN;
        case VK_F1:            return GLFW_KEY_F1;
        case VK_F2:            return GLFW_KEY_F2;
        case VK_F3:            return GLFW_KEY_F3;
        case VK_F4:            return GLFW_KEY_F4;
        case VK_F5:            return GLFW_KEY_F5;
        case VK_F6:            return GLFW_KEY_F6;
        case VK_F7:            return GLFW_KEY_F7;
        case VK_F8:            return GLFW_KEY_F8;
        case VK_F9:            return GLFW_KEY_F9;
        case VK_F10:           return GLFW_KEY_F10;
        case VK_F11:           return GLFW_KEY_F11;
        case VK_F12:           return GLFW_KEY_F12;
        case VK_F13:           return GLFW_KEY_F13;
        case VK_F14:           return GLFW_KEY_F14;
        case VK_F15:           return GLFW_KEY_F15;
        case VK_F16:           return GLFW_KEY_F16;
        case VK_F17:           return GLFW_KEY_F17;
        case VK_F18:           return GLFW_KEY_F18;
        case VK_F19:           return GLFW_KEY_F19;
        case VK_F20:           return GLFW_KEY_F20;
        case VK_F21:           return GLFW_KEY_F21;
        case VK_F22:           return GLFW_KEY_F22;
        case VK_F23:           return GLFW_KEY_F23;
        case VK_F24:           return GLFW_KEY_F24;
        case VK_NUMLOCK:       return GLFW_KEY_NUM_LOCK;
        case VK_CAPITAL:       return GLFW_KEY_CAPS_LOCK;
        case VK_SNAPSHOT:      return GLFW_KEY_PRINT_SCREEN;
        case VK_SCROLL:        return GLFW_KEY_SCROLL_LOCK;
        case VK_PAUSE:         return GLFW_KEY_PAUSE;
        case VK_LWIN:          return GLFW_KEY_LEFT_SUPER;
        case VK_RWIN:          return GLFW_KEY_RIGHT_SUPER;
        case VK_APPS:          return GLFW_KEY_MENU;

        // Numeric keypad
        case VK_NUMPAD0:       return GLFW_KEY_KP_0;
        case VK_NUMPAD1:       return GLFW_KEY_KP_1;
        case VK_NUMPAD2:       return GLFW_KEY_KP_2;
        case VK_NUMPAD3:       return GLFW_KEY_KP_3;
        case VK_NUMPAD4:       return GLFW_KEY_KP_4;
        case VK_NUMPAD5:       return GLFW_KEY_KP_5;
        case VK_NUMPAD6:       return GLFW_KEY_KP_6;
        case VK_NUMPAD7:       return GLFW_KEY_KP_7;
        case VK_NUMPAD8:       return GLFW_KEY_KP_8;
        case VK_NUMPAD9:       return GLFW_KEY_KP_9;
        case VK_DIVIDE:        return GLFW_KEY_KP_DIVIDE;
        case VK_MULTIPLY:      return GLFW_KEY_KP_MULTIPLY;
        case VK_SUBTRACT:      return GLFW_KEY_KP_SUBTRACT;
        case VK_ADD:           return GLFW_KEY_KP_ADD;
        case VK_DECIMAL:       return GLFW_KEY_KP_DECIMAL;

        // Printable keys are mapped according to US layout
        case VK_SPACE:         return GLFW_KEY_SPACE;
        case 0x30:             return GLFW_KEY_0;
        case 0x31:             return GLFW_KEY_1;
        case 0x32:             return GLFW_KEY_2;
        case 0x33:             return GLFW_KEY_3;
        case 0x34:             return GLFW_KEY_4;
        case 0x35:             return GLFW_KEY_5;
        case 0x36:             return GLFW_KEY_6;
        case 0x37:             return GLFW_KEY_7;
        case 0x38:             return GLFW_KEY_8;
        case 0x39:             return GLFW_KEY_9;
        case 0x41:             return GLFW_KEY_A;
        case 0x42:             return GLFW_KEY_B;
        case 0x43:             return GLFW_KEY_C;
        case 0x44:             return GLFW_KEY_D;
        case 0x45:             return GLFW_KEY_E;
        case 0x46:             return GLFW_KEY_F;
        case 0x47:             return GLFW_KEY_G;
        case 0x48:             return GLFW_KEY_H;
        case 0x49:             return GLFW_KEY_I;
        case 0x4A:             return GLFW_KEY_J;
        case 0x4B:             return GLFW_KEY_K;
        case 0x4C:             return GLFW_KEY_L;
        case 0x4D:             return GLFW_KEY_M;
        case 0x4E:             return GLFW_KEY_N;
        case 0x4F:             return GLFW_KEY_O;
        case 0x50:             return GLFW_KEY_P;
        case 0x51:             return GLFW_KEY_Q;
        case 0x52:             return GLFW_KEY_R;
        case 0x53:             return GLFW_KEY_S;
        case 0x54:             return GLFW_KEY_T;
        case 0x55:             return GLFW_KEY_U;
        case 0x56:             return GLFW_KEY_V;
        case 0x57:             return GLFW_KEY_W;
        case 0x58:             return GLFW_KEY_X;
        case 0x59:             return GLFW_KEY_Y;
        case 0x5A:             return GLFW_KEY_Z;
        case 0xBD:             return GLFW_KEY_MINUS;
        case 0xBB:             return GLFW_KEY_EQUAL;
        case 0xDB:             return GLFW_KEY_LEFT_BRACKET;
        case 0xDD:             return GLFW_KEY_RIGHT_BRACKET;
        case 0xDC:             return GLFW_KEY_BACKSLASH;
        case 0xBA:             return GLFW_KEY_SEMICOLON;
        case 0xDE:             return GLFW_KEY_APOSTROPHE;
        case 0xC0:             return GLFW_KEY_GRAVE_ACCENT;
        case 0xBC:             return GLFW_KEY_COMMA;
        case 0xBE:             return GLFW_KEY_PERIOD;
        case 0xBF:             return GLFW_KEY_SLASH;
        case 0xDF:             return GLFW_KEY_WORLD_1;
        case 0xE2:             return GLFW_KEY_WORLD_2;
        default:               break;
    }

    // No matching translation was found
    return GLFW_KEY_UNKNOWN;
}

// Window callback function (handles window events)
//
static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg,
                                   WPARAM wParam, LPARAM lParam)
{
    _GLFWwindow* window = (_GLFWwindow*) GetWindowLongPtr(hWnd, 0);

    switch (uMsg)
    {
        case WM_CREATE:
        {
            CREATESTRUCT* cs = (CREATESTRUCT*) lParam;
            SetWindowLongPtr(hWnd, 0, (LONG_PTR) cs->lpCreateParams);
            break;
        }

        case WM_ACTIVATE:
        {
            // Window was (de)focused and/or (de)iconified

            BOOL focused = LOWORD(wParam) != WA_INACTIVE;
            BOOL iconified = HIWORD(wParam) ? TRUE : FALSE;

            if (focused && iconified)
            {
                if (window->iconified && _glfw.focusedWindow != window)
                {
                    // This is a workaround for window restoration using the
                    // Win+D hot key leading to windows being told they're
                    // focused and iconified and then never told they're
                    // restored
                    iconified = FALSE;
                }
                else
                {
                    // This is a workaround for window iconification using the
                    // taskbar leading to windows being told they're focused and
                    // iconified and then never told they're defocused
                    focused = FALSE;
                }
            }

            if (!focused && _glfw.focusedWindow == window)
            {
                // The window was defocused (or iconified, see above)

                if (window->cursorMode != GLFW_CURSOR_NORMAL)
                    showCursor(window);

                if (window->monitor)
                {
                    if (!iconified)
                    {
                        // Iconify the (on top, borderless, oddly positioned)
                        // window or the user will be annoyed
                        _glfwPlatformIconifyWindow(window);
                    }

                    _glfwRestoreVideoMode(window->monitor);
                }
            }
            else if (focused && _glfw.focusedWindow != window)
            {
                // The window was focused

                if (window->cursorMode == GLFW_CURSOR_DISABLED)
                    captureCursor(window);
                else if (window->cursorMode == GLFW_CURSOR_HIDDEN)
                    hideCursor(window);

                if (window->monitor)
                    _glfwSetVideoMode(window->monitor, &window->videoMode);
            }

            _glfwInputWindowFocus(window, focused);
            _glfwInputWindowIconify(window, iconified);
            return 0;
        }

        case WM_ACTIVATEAPP:
        {
            if (!wParam && IsIconic(hWnd))
            {
                // This is a workaround for full screen windows losing focus
                // through Alt+Tab leading to windows being told they're
                // unfocused and restored and then never told they're iconified
                _glfwInputWindowIconify(window, GL_TRUE);
            }

            return 0;
        }

        case WM_SHOWWINDOW:
        {
            _glfwInputWindowVisibility(window, wParam ? GL_TRUE : GL_FALSE);
            break;
        }

        case WM_SYSCOMMAND:
        {
            switch (wParam & 0xfff0)
            {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                {
                    if (window->monitor)
                    {
                        // We are running in fullscreen mode, so disallow
                        // screen saver and screen blanking
                        return 0;
                    }
                    else
                        break;
                }

                // User trying to access application menu using ALT?
                case SC_KEYMENU:
                    return 0;
            }
            break;
        }

        case WM_CLOSE:
        {
            _glfwInputWindowCloseRequest(window);
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            const int scancode = (lParam >> 16) & 0xff;
            const int key = translateKey(wParam, lParam);
            if (key == _GLFW_KEY_INVALID)
                break;

            _glfwInputKey(window, key, scancode, GLFW_PRESS, getKeyMods());
            break;
        }

        case WM_CHAR:
        case WM_SYSCHAR:
        {
            _glfwInputChar(window, (unsigned int) wParam);
            return 0;
        }

        case WM_UNICHAR:
        {
            // This message is not sent by Windows, but is sent by some
            // third-party input method engines

            if (wParam == UNICODE_NOCHAR)
            {
                // Returning TRUE here announces support for this message
                return TRUE;
            }

            _glfwInputChar(window, (unsigned int) wParam);
            return FALSE;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            const int mods = getKeyMods();
            const int scancode = (lParam >> 16) & 0xff;
            const int key = translateKey(wParam, lParam);
            if (key == _GLFW_KEY_INVALID)
                break;

            if (wParam == VK_SHIFT)
            {
                // Release both Shift keys on Shift up event, as only one event
                // is sent even if both keys are released
                _glfwInputKey(window, GLFW_KEY_LEFT_SHIFT, scancode, GLFW_RELEASE, mods);
                _glfwInputKey(window, GLFW_KEY_RIGHT_SHIFT, scancode, GLFW_RELEASE, mods);
            }
            else if (wParam == VK_SNAPSHOT)
            {
                // Key down is not reported for the print screen key
                _glfwInputKey(window, key, scancode, GLFW_PRESS, mods);
                _glfwInputKey(window, key, scancode, GLFW_RELEASE, mods);
            }
            else
                _glfwInputKey(window, key, scancode, GLFW_RELEASE, mods);

            break;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
        {
            const int mods = getKeyMods();

            SetCapture(hWnd);

            if (uMsg == WM_LBUTTONDOWN)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, mods);
            else if (uMsg == WM_RBUTTONDOWN)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, mods);
            else if (uMsg == WM_MBUTTONDOWN)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS, mods);
            else
            {
                if (HIWORD(wParam) == XBUTTON1)
                    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_4, GLFW_PRESS, mods);
                else if (HIWORD(wParam) == XBUTTON2)
                    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_5, GLFW_PRESS, mods);

                return TRUE;
            }

            return 0;
        }

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
            const int mods = getKeyMods();

            ReleaseCapture();

            if (uMsg == WM_LBUTTONUP)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, mods);
            else if (uMsg == WM_RBUTTONUP)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, mods);
            else if (uMsg == WM_MBUTTONUP)
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_RELEASE, mods);
            else
            {
                if (HIWORD(wParam) == XBUTTON1)
                    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_4, GLFW_RELEASE, mods);
                else if (HIWORD(wParam) == XBUTTON2)
                    _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_5, GLFW_RELEASE, mods);

                return TRUE;
            }

            return 0;
        }

        case WM_MOUSEMOVE:
        {
            const int newCursorX = GET_X_LPARAM(lParam);
            const int newCursorY = GET_Y_LPARAM(lParam);

            if (newCursorX != window->win32.oldCursorX ||
                newCursorY != window->win32.oldCursorY)
            {
                int x, y;

                if (window->cursorMode == GLFW_CURSOR_DISABLED)
                {
                    if (_glfw.focusedWindow != window)
                        return 0;

                    x = newCursorX - window->win32.oldCursorX;
                    y = newCursorY - window->win32.oldCursorY;
                }
                else
                {
                    x = newCursorX;
                    y = newCursorY;
                }

                window->win32.oldCursorX = newCursorX;
                window->win32.oldCursorY = newCursorY;
                window->win32.cursorCentered = GL_FALSE;

                _glfwInputCursorMotion(window, x, y);
            }

            if (!window->win32.cursorInside)
            {
                TRACKMOUSEEVENT tme;
                ZeroMemory(&tme, sizeof(tme));
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = window->win32.handle;
                TrackMouseEvent(&tme);

                window->win32.cursorInside = GL_TRUE;
                _glfwInputCursorEnter(window, GL_TRUE);
            }

            return 0;
        }

        case WM_MOUSELEAVE:
        {
            window->win32.cursorInside = GL_FALSE;
            _glfwInputCursorEnter(window, GL_FALSE);
            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            _glfwInputScroll(window, 0.0, (SHORT) HIWORD(wParam) / (double) WHEEL_DELTA);
            return 0;
        }

        case WM_MOUSEHWHEEL:
        {
            // This message is only sent on Windows Vista and later
            _glfwInputScroll(window, (SHORT) HIWORD(wParam) / (double) WHEEL_DELTA, 0.0);
            return 0;
        }

        case WM_SIZE:
        {
            if (window->cursorMode == GLFW_CURSOR_DISABLED &&
                _glfw.focusedWindow == window)
            {
                updateClipRect(window);
            }

            _glfwInputFramebufferSize(window, LOWORD(lParam), HIWORD(lParam));
            _glfwInputWindowSize(window, LOWORD(lParam), HIWORD(lParam));
            return 0;
        }

        case WM_MOVE:
        {
            if (window->cursorMode == GLFW_CURSOR_DISABLED &&
                _glfw.focusedWindow == window)
            {
                updateClipRect(window);
            }

            // NOTE: This cannot use LOWORD/HIWORD recommended by MSDN, as
            // those macros do not handle negative window positions correctly
            _glfwInputWindowPos(window,
                                GET_X_LPARAM(lParam),
                                GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_PAINT:
        {
            _glfwInputWindowDamage(window);
            break;
        }

        case WM_SETCURSOR:
        {
            if (window->cursorMode != GLFW_CURSOR_NORMAL &&
                _glfw.focusedWindow == window &&
                LOWORD(lParam) == HTCLIENT)
            {
                SetCursor(NULL);
                return TRUE;
            }

            break;
        }

        case WM_DEVICECHANGE:
        {
            if (DBT_DEVNODES_CHANGED == wParam)
            {
                _glfwInputMonitorChange();
                return TRUE;
            }
            break;
        }

        case WM_DWMCOMPOSITIONCHANGED:
        {
            if (_glfwIsCompositionEnabled())
            {
                _GLFWwindow* previous = _glfwPlatformGetCurrentContext();
                _glfwPlatformMakeContextCurrent(window);
                _glfwPlatformSwapInterval(0);
                _glfwPlatformMakeContextCurrent(previous);
            }

            // TODO: Restore vsync if compositing was disabled
            break;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Translate client window size to full window size (including window borders)
//
static void getFullWindowSize(_GLFWwindow* window,
                              int clientWidth, int clientHeight,
                              int* fullWidth, int* fullHeight)
{
    RECT rect = { 0, 0, clientWidth, clientHeight };
    AdjustWindowRectEx(&rect, window->win32.dwStyle,
                       FALSE, window->win32.dwExStyle);
    *fullWidth = rect.right - rect.left;
    *fullHeight = rect.bottom - rect.top;
}

// Registers the GLFW window class
//
static ATOM registerWindowClass(void)
{
    WNDCLASS wc;
    ATOM classAtom;

    // Set window class parameters
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC) windowProc;
    wc.cbClsExtra    = 0;                           // No extra class data
    wc.cbWndExtra    = sizeof(void*) + sizeof(int); // Make room for one pointer
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;                        // No background
    wc.lpszMenuName  = NULL;                        // No menu
    wc.lpszClassName = _GLFW_WNDCLASSNAME;

    // Load user-provided icon if available
    wc.hIcon = LoadIcon(GetModuleHandle(NULL), L"GLFW_ICON");
    if (!wc.hIcon)
    {
        // No user-provided icon found, load default icon
        wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    }

    classAtom = RegisterClass(&wc);
    if (!classAtom)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to register window class");
        return 0;
    }

    return classAtom;
}

// Creates the GLFW window and rendering context
//
static int createWindow(_GLFWwindow* window,
                        const _GLFWwndconfig* wndconfig,
                        const _GLFWfbconfig* fbconfig)
{
    int xpos, ypos, fullWidth, fullHeight;
    WCHAR* wideTitle;

    window->win32.dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    window->win32.dwExStyle = WS_EX_APPWINDOW;

    if (window->monitor)
    {
        window->win32.dwStyle |= WS_POPUP;

        _glfwPlatformGetMonitorPos(wndconfig->monitor, &xpos, &ypos);
        fullWidth  = wndconfig->width;
        fullHeight = wndconfig->height;
    }
    else
    {
        if (wndconfig->decorated)
        {
            window->win32.dwStyle |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

            if (wndconfig->resizable)
            {
                window->win32.dwStyle |= WS_MAXIMIZEBOX | WS_SIZEBOX;
                window->win32.dwExStyle |= WS_EX_WINDOWEDGE;
            }
        }
        else
            window->win32.dwStyle |= WS_POPUP;

        xpos = CW_USEDEFAULT;
        ypos = CW_USEDEFAULT;

        getFullWindowSize(window,
                        wndconfig->width, wndconfig->height,
                        &fullWidth, &fullHeight);
    }

    wideTitle = _glfwCreateWideStringFromUTF8(wndconfig->title);
    if (!wideTitle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to convert title to wide string");
        return GL_FALSE;
    }

    window->win32.handle = CreateWindowEx(window->win32.dwExStyle,
                                          _GLFW_WNDCLASSNAME,
                                          wideTitle,
                                          window->win32.dwStyle,
                                          xpos, ypos,
                                          fullWidth, fullHeight,
                                          NULL, // No parent window
                                          NULL, // No window menu
                                          GetModuleHandle(NULL),
                                          window); // Pass object to WM_CREATE

    free(wideTitle);

    if (!window->win32.handle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to create window");
        return GL_FALSE;
    }

    if (!_glfwCreateContext(window, wndconfig, fbconfig))
        return GL_FALSE;

    return GL_TRUE;
}

// Destroys the GLFW window and rendering context
//
static void destroyWindow(_GLFWwindow* window)
{
    _glfwDestroyContext(window);

    if (window->win32.handle)
    {
        DestroyWindow(window->win32.handle);
        window->win32.handle = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWfbconfig* fbconfig)
{
    int status;

    if (!_glfw.win32.classAtom)
    {
        _glfw.win32.classAtom = registerWindowClass();
        if (!_glfw.win32.classAtom)
            return GL_FALSE;
    }

    if (!createWindow(window, wndconfig, fbconfig))
        return GL_FALSE;

    status = _glfwAnalyzeContext(window, wndconfig, fbconfig);

    if (status == _GLFW_RECREATION_IMPOSSIBLE)
        return GL_FALSE;

    if (status == _GLFW_RECREATION_REQUIRED)
    {
        // Some window hints require us to re-create the context using WGL
        // extensions retrieved through the current context, as we cannot check
        // for WGL extensions or retrieve WGL entry points before we have a
        // current context (actually until we have implicitly loaded the ICD)

        // Yes, this is strange, and yes, this is the proper way on Win32

        // As Windows only allows you to set the pixel format once for a
        // window, we need to destroy the current window and create a new one
        // to be able to use the new pixel format

        // Technically, it may be possible to keep the old window around if
        // we're just creating an OpenGL 3.0+ context with the same pixel
        // format, but it's not worth the added code complexity

        // First we clear the current context (the one we just created)
        // This is usually done by glfwDestroyWindow, but as we're not doing
        // full GLFW window destruction, it's duplicated here
        _glfwPlatformMakeContextCurrent(NULL);

        // Next destroy the Win32 window and WGL context (without resetting or
        // destroying the GLFW window object)
        destroyWindow(window);

        // ...and then create them again, this time with better APIs
        if (!createWindow(window, wndconfig, fbconfig))
            return GL_FALSE;
    }

    if (window->monitor)
    {
        if (!_glfwSetVideoMode(window->monitor, &window->videoMode))
            return GL_FALSE;

        // Place the window above all topmost windows
        _glfwPlatformShowWindow(window);
        SetWindowPos(window->win32.handle, HWND_TOPMOST, 0,0,0,0,
                     SWP_NOMOVE | SWP_NOSIZE);
    }

    return GL_TRUE;
}

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
    destroyWindow(window);

    if (window->monitor)
        _glfwRestoreVideoMode(window->monitor);
}

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
    WCHAR* wideTitle = _glfwCreateWideStringFromUTF8(title);
    if (!wideTitle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to convert title to wide string");
        return;
    }

    SetWindowText(window->win32.handle, wideTitle);
    free(wideTitle);
}

void _glfwPlatformGetWindowPos(_GLFWwindow* window, int* xpos, int* ypos)
{
    POINT pos = { 0, 0 };
    ClientToScreen(window->win32.handle, &pos);

    if (xpos)
        *xpos = pos.x;
    if (ypos)
        *ypos = pos.y;
}

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int xpos, int ypos)
{
    RECT rect = { xpos, ypos, xpos, ypos };
    AdjustWindowRectEx(&rect, window->win32.dwStyle,
                       FALSE, window->win32.dwExStyle);
    SetWindowPos(window->win32.handle, NULL, rect.left, rect.top, 0, 0,
                 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void _glfwPlatformGetWindowSize(_GLFWwindow* window, int* width, int* height)
{
    RECT area;
    GetClientRect(window->win32.handle, &area);

    if (width)
        *width = area.right;
    if (height)
        *height = area.bottom;
}

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    if (window->monitor)
    {
        GLFWvidmode mode;
        _glfwSetVideoMode(window->monitor, &window->videoMode);
        _glfwPlatformGetVideoMode(window->monitor, &mode);

        SetWindowPos(window->win32.handle, HWND_TOP,
                     0, 0, mode.width, mode.height,
                     SWP_NOMOVE);
    }
    else
    {
        int fullWidth, fullHeight;
        getFullWindowSize(window, width, height, &fullWidth, &fullHeight);

        SetWindowPos(window->win32.handle, HWND_TOP,
                     0, 0, fullWidth, fullHeight,
                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
    }
}

void _glfwPlatformGetFramebufferSize(_GLFWwindow* window, int* width, int* height)
{
    _glfwPlatformGetWindowSize(window, width, height);
}

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
    ShowWindow(window->win32.handle, SW_MINIMIZE);
}

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
    ShowWindow(window->win32.handle, SW_RESTORE);
}

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
    ShowWindow(window->win32.handle, SW_SHOWNORMAL);
    BringWindowToTop(window->win32.handle);
    SetForegroundWindow(window->win32.handle);
    SetFocus(window->win32.handle);
}

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
    ShowWindow(window->win32.handle, SW_HIDE);
}

void _glfwPlatformPollEvents(void)
{
    MSG msg;
    _GLFWwindow* window;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            // Treat WM_QUIT as a close on all windows

            window = _glfw.windowListHead;
            while (window)
            {
                _glfwInputWindowCloseRequest(window);
                window = window->next;
            }
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    window = _glfw.focusedWindow;
    if (window)
    {
        // LSHIFT/RSHIFT fixup (keys tend to "stick" without this fix)
        // This is the only async event handling in GLFW, but it solves some
        // nasty problems
        {
            const int mods = getAsyncKeyMods();

            // Get current state of left and right shift keys
            const int lshiftDown = (GetAsyncKeyState(VK_LSHIFT) >> 15) & 1;
            const int rshiftDown = (GetAsyncKeyState(VK_RSHIFT) >> 15) & 1;

            // See if this differs from our belief of what has happened
            // (we only have to check for lost key up events)
            if (!lshiftDown && window->key[GLFW_KEY_LEFT_SHIFT] == 1)
                _glfwInputKey(window, GLFW_KEY_LEFT_SHIFT, 0, GLFW_RELEASE, mods);

            if (!rshiftDown && window->key[GLFW_KEY_RIGHT_SHIFT] == 1)
                _glfwInputKey(window, GLFW_KEY_RIGHT_SHIFT, 0, GLFW_RELEASE, mods);
        }

        // Did the cursor move in an focused window that has captured the cursor
        if (window->cursorMode == GLFW_CURSOR_DISABLED &&
            !window->win32.cursorCentered)
        {
            int width, height;
            _glfwPlatformGetWindowSize(window, &width, &height);
            _glfwPlatformSetCursorPos(window, width / 2.0, height / 2.0);
            window->win32.cursorCentered = GL_TRUE;
        }
    }
}

void _glfwPlatformWaitEvents(void)
{
    WaitMessage();

    _glfwPlatformPollEvents();
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double xpos, double ypos)
{
    POINT pos = { (int) xpos, (int) ypos };
    ClientToScreen(window->win32.handle, &pos);
    SetCursorPos(pos.x, pos.y);

    window->win32.oldCursorX = (int) xpos;
    window->win32.oldCursorY = (int) ypos;
}

void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode)
{
    switch (mode)
    {
        case GLFW_CURSOR_NORMAL:
            showCursor(window);
            break;
        case GLFW_CURSOR_HIDDEN:
            hideCursor(window);
            break;
        case GLFW_CURSOR_DISABLED:
            captureCursor(window);
            break;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI HWND glfwGetWin32Window(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    return window->win32.handle;
}

