//========================================================================
// GLFW - An OpenGL library
// Platform:    Win32/WGL
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
#include <malloc.h>
#include <windowsx.h>

//========================================================================
// Hide mouse cursor
//========================================================================

static void hideCursor(_GLFWwindow* window)
{
}


//========================================================================
// Capture mouse cursor
//========================================================================

static void captureCursor(_GLFWwindow* window)
{
    RECT ClipWindowRect;

    ShowCursor(FALSE);

    // Clip cursor to the window
    if (GetWindowRect(window->Win32.handle, &ClipWindowRect))
        ClipCursor(&ClipWindowRect);

    // Capture cursor to user window
    SetCapture(window->Win32.handle);
}


//========================================================================
// Show mouse cursor
//========================================================================

static void showCursor(_GLFWwindow* window)
{
    // Un-capture cursor
    ReleaseCapture();

    // Release the cursor from the window
    ClipCursor(NULL);

    ShowCursor(TRUE);
}


//========================================================================
// Translates a Windows key to the corresponding GLFW key
//========================================================================

static int translateKey(WPARAM wParam, LPARAM lParam)
{
    MSG next_msg;
    DWORD msg_time;
    DWORD scan_code;

    // Check for numeric keypad keys.
    // Note: This way we always force "NumLock = ON", which is intentional
    // since the returned key code should correspond to a physical
    // location.
    int hiFlags = HIWORD(lParam);
    if (!(hiFlags & 0x100))
    {
        switch (MapVirtualKey(hiFlags & 0xFF, 1))
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
            scan_code = MapVirtualKey(VK_RSHIFT, 0);
            if (((lParam & 0x01ff0000) >> 16) == scan_code)
                return GLFW_KEY_RIGHT_SHIFT;

            return GLFW_KEY_LEFT_SHIFT;
        }

        // The CTRL keys require special handling
        case VK_CONTROL:
        {
            // Is this an extended key (i.e. right key)?
            if (lParam & 0x01000000)
                return GLFW_KEY_RIGHT_CONTROL;

            // Here is a trick: "Alt Gr" sends LCTRL, then RALT. We only
            // want the RALT message, so we try to see if the next message
            // is a RALT message. In that case, this is a false LCTRL!
            msg_time = GetMessageTime();
            if (PeekMessage(&next_msg, NULL, 0, 0, PM_NOREMOVE))
            {
                if (next_msg.message == WM_KEYDOWN ||
                    next_msg.message == WM_SYSKEYDOWN)
                {
                    if (next_msg.wParam == VK_MENU &&
                        (next_msg.lParam & 0x01000000) &&
                        next_msg.time == msg_time)
                    {
                        // Next message is a RALT down message, which
                        // means that this is NOT a proper LCTRL message!
                        return -1;
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

    // No matching translation was found, so return -1
    return -1;
}


//========================================================================
// Translates a Windows key to Unicode
//========================================================================

static void translateChar(_GLFWwindow* window, DWORD wParam, DWORD lParam)
{
    BYTE keyboard_state[256];
    WCHAR unicode_buf[10];
    UINT scan_code;
    int i, num_chars;

    GetKeyboardState(keyboard_state);

    // Derive scan code from lParam and action
    scan_code = (lParam & 0x01ff0000) >> 16;

    num_chars = ToUnicode(
        wParam,          // virtual-key code
        scan_code,       // scan code
        keyboard_state,  // key-state array
        unicode_buf,     // buffer for translated key
        10,              // size of translated key buffer
        0                // active-menu flag
    );

    // Report characters
    for (i = 0;  i < num_chars;  i++)
        _glfwInputChar(window, (int) unicode_buf[i]);
}


//========================================================================
// Window callback function (handles window events)
//========================================================================

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
            // Window was (de)activated and/or (de)iconified

            BOOL active = LOWORD(wParam) != WA_INACTIVE;
            BOOL iconified = HIWORD(wParam) ? TRUE : FALSE;

            if (active && iconified)
            {
                // This is a workaround for window iconification using the
                // taskbar leading to windows being told they're active and
                // iconified and then never told they're deactivated
                active = FALSE;
            }

            if (!active && _glfwLibrary.activeWindow == window)
            {
                // The window was deactivated (or iconified, see above)

                if (window->cursorMode == GLFW_CURSOR_CAPTURED)
                    showCursor(window);

                if (window->mode == GLFW_FULLSCREEN)
                {
                    if (!iconified)
                    {
                        // Iconify the (on top, borderless, oddly positioned)
                        // window or the user will be annoyed
                        _glfwPlatformIconifyWindow(window);
                    }

                    if (_glfwLibrary.Win32.monitor.modeChanged)
                    {
                        _glfwRestoreVideoMode();
                        _glfwLibrary.Win32.monitor.modeChanged = GL_FALSE;
                    }
                }
            }
            else if (active && _glfwLibrary.activeWindow != window)
            {
                // The window was activated

                if (window->cursorMode == GLFW_CURSOR_CAPTURED)
                    captureCursor(window);

                if (window->mode == GLFW_FULLSCREEN)
                {
                    if (!_glfwLibrary.Win32.monitor.modeChanged)
                    {
                        _glfwSetVideoMode(&_glfwLibrary.Win32.monitor.width,
                                          &_glfwLibrary.Win32.monitor.height,
                                          &_glfwLibrary.Win32.monitor.bitsPerPixel,
                                          &_glfwLibrary.Win32.monitor.refreshRate,
                                          GL_TRUE);

                        _glfwLibrary.Win32.monitor.modeChanged = GL_TRUE;
                    }
                }
            }

            _glfwInputWindowFocus(window, active);
            _glfwInputWindowIconify(window, iconified);
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
                    if (window->mode == GLFW_FULLSCREEN)
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
            _glfwInputKey(window, translateKey(wParam, lParam), GLFW_PRESS);

            if (_glfwLibrary.charCallback)
                translateChar(window, (DWORD) wParam, (DWORD) lParam);

            break;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            // Special trick: release both shift keys on SHIFT up event
            if (wParam == VK_SHIFT)
            {
                _glfwInputKey(window, GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE);
                _glfwInputKey(window, GLFW_KEY_RIGHT_SHIFT, GLFW_RELEASE);
            }
            else
                _glfwInputKey(window, translateKey(wParam, lParam), GLFW_RELEASE);

            break;
        }

        case WM_LBUTTONDOWN:
        {
            SetCapture(hWnd);
            _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
            return 0;
        }

        case WM_RBUTTONDOWN:
        {
            SetCapture(hWnd);
            _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);
            return 0;
        }

        case WM_MBUTTONDOWN:
        {
            SetCapture(hWnd);
            _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS);
            return 0;
        }

        case WM_XBUTTONDOWN:
        {
            if (HIWORD(wParam) == XBUTTON1)
            {
                SetCapture(hWnd);
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_4, GLFW_PRESS);
            }
            else if (HIWORD(wParam) == XBUTTON2)
            {
                SetCapture(hWnd);
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_5, GLFW_PRESS);
            }

            return 1;
        }

        case WM_LBUTTONUP:
        {
            ReleaseCapture();
            _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
            return 0;
        }

        case WM_RBUTTONUP:
        {
            ReleaseCapture();
            _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE);
            return 0;
        }

        case WM_MBUTTONUP:
        {
            ReleaseCapture();
            _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_RELEASE);
            return 0;
        }

        case WM_XBUTTONUP:
        {
            if (HIWORD(wParam) == XBUTTON1)
            {
                ReleaseCapture();
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_4, GLFW_RELEASE);
            }
            else if (HIWORD(wParam) == XBUTTON2)
            {
                ReleaseCapture();
                _glfwInputMouseClick(window, GLFW_MOUSE_BUTTON_5, GLFW_RELEASE);
            }

            return 1;
        }

        case WM_MOUSEMOVE:
        {
            int newCursorX, newCursorY;

            // Get signed (!) cursor position
            newCursorX = GET_X_LPARAM(lParam);
            newCursorY = GET_Y_LPARAM(lParam);

            if (newCursorX != window->Win32.oldCursorX ||
                newCursorY != window->Win32.oldCursorY)
            {
                int x, y;

                if (window->cursorMode == GLFW_CURSOR_CAPTURED)
                {
                    if (_glfwLibrary.activeWindow != window)
                        return 0;

                    x = newCursorX - window->Win32.oldCursorX;
                    y = newCursorY - window->Win32.oldCursorY;
                }
                else
                {
                    x = newCursorX;
                    y = newCursorY;
                }

                window->Win32.oldCursorX = newCursorX;
                window->Win32.oldCursorY = newCursorY;
                window->Win32.cursorCentered = GL_FALSE;

                _glfwInputCursorMotion(window, x, y);
            }

            if (!window->Win32.cursorInside)
            {
                TRACKMOUSEEVENT tme;
                ZeroMemory(&tme, sizeof(tme));
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = window->Win32.handle;
                TrackMouseEvent(&tme);

                window->Win32.cursorInside = GL_TRUE;
                _glfwInputCursorEnter(window, GL_TRUE);
            }

            return 0;
        }

        case WM_MOUSELEAVE:
        {
            window->Win32.cursorInside = GL_FALSE;
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
            // If window is in cursor capture mode, update clipping rect
            if (window->cursorMode == GLFW_CURSOR_CAPTURED)
            {
                RECT ClipWindowRect;
                if (GetWindowRect(window->Win32.handle, &ClipWindowRect))
                    ClipCursor(&ClipWindowRect);
            }

            _glfwInputWindowSize(window, LOWORD(lParam), HIWORD(lParam));
            return 0;
        }

        case WM_MOVE:
        {
            // If window is in cursor capture mode, update clipping rect
            if (window->cursorMode == GLFW_CURSOR_CAPTURED)
            {
                RECT ClipWindowRect;
                if (GetWindowRect(window->Win32.handle, &ClipWindowRect))
                    ClipCursor(&ClipWindowRect);
            }

            _glfwInputWindowPos(window, LOWORD(lParam), HIWORD(lParam));
            return 0;
        }

        // Was the window contents damaged?
        case WM_PAINT:
        {
            _glfwInputWindowDamage(window);
            break;
        }

        case WM_DISPLAYCHANGE:
        {
            // TODO: Do stuff here.

            break;
        }
    }

    // Pass all unhandled messages to DefWindowProc
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


//========================================================================
// Translate client window size to full window size (including window borders)
//========================================================================

static void getFullWindowSize(_GLFWwindow* window,
                              int clientWidth, int clientHeight,
                              int* fullWidth, int* fullHeight)
{
    RECT rect;

    // Create a window rectangle
    rect.left   = (long) 0;
    rect.right  = (long) clientWidth - 1;
    rect.top    = (long) 0;
    rect.bottom = (long) clientHeight - 1;

    // Adjust according to window styles
    AdjustWindowRectEx(&rect, window->Win32.dwStyle, FALSE, window->Win32.dwExStyle);

    // Calculate width and height of full window
    *fullWidth = rect.right - rect.left + 1;
    *fullHeight = rect.bottom - rect.top + 1;
}


//========================================================================
// Registers the GLFW window class
//========================================================================

static ATOM registerWindowClass(void)
{
    WNDCLASS wc;
    ATOM classAtom;

    // Set window class parameters
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Redraw on...
    wc.lpfnWndProc   = (WNDPROC) windowProc;          // Message handler
    wc.cbClsExtra    = 0;                             // No extra class data
    wc.cbWndExtra    = sizeof(void*) + sizeof(int);   // Make room for one pointer
    wc.hInstance     = _glfwLibrary.Win32.instance;   // Set instance
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);   // Load arrow pointer
    wc.hbrBackground = NULL;                          // No background
    wc.lpszMenuName  = NULL;                          // No menu
    wc.lpszClassName = _GLFW_WNDCLASSNAME;            // Set class name

    // Load user-provided icon if available
    wc.hIcon = LoadIcon(_glfwLibrary.Win32.instance, L"GLFW_ICON");
    if (!wc.hIcon)
    {
        // Load default icon
        wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    }

    classAtom = RegisterClass(&wc);
    if (!classAtom)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32: Failed to register window class");
        return 0;
    }

    return classAtom;
}


//========================================================================
// Creates the GLFW window and rendering context
//========================================================================

static int createWindow(_GLFWwindow* window,
                        const _GLFWwndconfig* wndconfig,
                        const _GLFWfbconfig* fbconfig)
{
    DWORD dwStyle, dwExStyle;
    int fullWidth, fullHeight;
    RECT wa;
    POINT pos;
    WCHAR* wideTitle;

    // Set common window styles
    dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    dwExStyle = WS_EX_APPWINDOW;

    // Set window style, depending on fullscreen mode
    if (window->mode == GLFW_FULLSCREEN)
    {
        dwStyle |= WS_POPUP;

        // Here's a trick for helping us getting window focus
        // (SetForegroundWindow doesn't work properly under
        // Win98/ME/2K/.NET/+)
        /*
        if (_glfwLibrary.Sys.WinVer != _GLFW_WIN_95 &&
            _glfwLibrary.Sys.WinVer != _GLFW_WIN_NT4 &&
            _glfwLibrary.Sys.WinVer != _GLFW_WIN_XP)
        {
            dwStyle |= WS_MINIMIZE;
        }
        */
    }
    else
    {
        dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

        if (wndconfig->resizable)
        {
            dwStyle |= (WS_MAXIMIZEBOX | WS_SIZEBOX);
            dwExStyle |= WS_EX_WINDOWEDGE;
        }
    }

    // Remember window styles (used by getFullWindowSize)
    window->Win32.dwStyle   = dwStyle;
    window->Win32.dwExStyle = dwExStyle;

    // Adjust window size for frame and title bar
    getFullWindowSize(window, window->width, window->height, &fullWidth, &fullHeight);

    // Adjust window position to working area (e.g. if the task bar is at
    // the top of the display). Fullscreen windows are always opened in
    // the upper left corner regardless of the desktop working area.
    if (window->mode == GLFW_FULLSCREEN)
        wa.left = wa.top = 0;
    else
        SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);

    wideTitle = _glfwCreateWideStringFromUTF8(wndconfig->title);
    if (!wideTitle)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32: Failed to convert title to wide string");
        return GL_FALSE;
    }

    window->Win32.handle = CreateWindowEx(window->Win32.dwExStyle,
                                          _GLFW_WNDCLASSNAME,
                                          wideTitle,
                                          window->Win32.dwStyle,
                                          wa.left, wa.top,       // Window position
                                          fullWidth,             // Decorated window width
                                          fullHeight,            // Decorated window height
                                          NULL,                  // No parent window
                                          NULL,                  // No menu
                                          _glfwLibrary.Win32.instance,
                                          window);  // Pass GLFW window to WM_CREATE

    if (!window->Win32.handle)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR, "Win32: Failed to create window");
        return GL_FALSE;
    }

    free(wideTitle);

    // Initialize cursor position data
    GetCursorPos(&pos);
    ScreenToClient(window->Win32.handle, &pos);
    window->Win32.oldCursorX = window->cursorPosX = pos.x;
    window->Win32.oldCursorY = window->cursorPosY = pos.y;

    if (!_glfwCreateContext(window, wndconfig, fbconfig))
        return GL_FALSE;

    return GL_TRUE;
}


//========================================================================
// Destroys the GLFW window and rendering context
//========================================================================

static void destroyWindow(_GLFWwindow* window)
{
    _glfwDestroyContext(window);

    // This is duplicated from glfwDestroyWindow
    // TODO: Stop duplicating code
    if (window == _glfwLibrary.activeWindow)
        _glfwLibrary.activeWindow = NULL;

    if (window->Win32.handle)
    {
        DestroyWindow(window->Win32.handle);
        window->Win32.handle = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Here is where the window is created, and the OpenGL rendering context is
// created
//========================================================================

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWfbconfig* fbconfig)
{
    GLboolean recreateContext = GL_FALSE;

    window->Win32.desiredRefreshRate = wndconfig->refreshRate;

    if (!_glfwLibrary.Win32.classAtom)
    {
        _glfwLibrary.Win32.classAtom = registerWindowClass();
        if (!_glfwLibrary.Win32.classAtom)
            return GL_FALSE;
    }

    if (window->mode == GLFW_FULLSCREEN)
    {
        int bpp = fbconfig->redBits + fbconfig->greenBits + fbconfig->blueBits;
        if (bpp < 15 || bpp >= 24)
            bpp = 32;

        _glfwLibrary.Win32.monitor.width = window->width;
        _glfwLibrary.Win32.monitor.height = window->height;
        _glfwLibrary.Win32.monitor.refreshRate = wndconfig->refreshRate;
        _glfwLibrary.Win32.monitor.bitsPerPixel = bpp;

        _glfwSetVideoMode(&_glfwLibrary.Win32.monitor.width,
                          &_glfwLibrary.Win32.monitor.height,
                          &_glfwLibrary.Win32.monitor.bitsPerPixel,
                          &_glfwLibrary.Win32.monitor.refreshRate,
                          GL_FALSE);

        _glfwLibrary.Win32.monitor.modeChanged = GL_TRUE;
    }

    if (!createWindow(window, wndconfig, fbconfig))
        return GL_FALSE;

    if (wndconfig->glMajor != 1 || wndconfig->glMinor != 0)
    {
        if (window->WGL.ARB_create_context)
            recreateContext = GL_TRUE;
    }

    if (wndconfig->glDebug)
    {
        if (window->WGL.ARB_create_context)
            recreateContext = GL_TRUE;
    }

    if (wndconfig->glForward)
    {
        if (!window->WGL.ARB_create_context)
        {
            _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                          "WGL: A forward compatible OpenGL context requested "
                          "but WGL_ARB_create_context is unavailable");
            return GL_FALSE;
        }

        recreateContext = GL_TRUE;
    }

    if (wndconfig->glProfile)
    {
        if (!window->WGL.ARB_create_context_profile)
        {
            _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                          "WGL: OpenGL profile requested but "
                          "WGL_ARB_create_context_profile is unavailable");
            return GL_FALSE;
        }

        recreateContext = GL_TRUE;
    }

    if (fbconfig->samples > 0)
    {
        // We want FSAA, but can we get it?
        // FSAA is not a hard constraint, so otherwise we just don't care

        if (window->WGL.ARB_multisample && window->WGL.ARB_pixel_format)
        {
            // We appear to have both the FSAA extension and the means to ask for it
            recreateContext = GL_TRUE;
        }
    }

    if (recreateContext)
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

        destroyWindow(window);

        if (!createWindow(window, wndconfig, fbconfig))
            return GL_FALSE;
    }

    if (window->mode == GLFW_FULLSCREEN)
    {
        // Place the window above all topmost windows
        SetWindowPos(window->Win32.handle, HWND_TOPMOST, 0,0,0,0,
                     SWP_NOMOVE | SWP_NOSIZE);
    }

    return GL_TRUE;
}


//========================================================================
// Properly kill the window / video display
//========================================================================

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
    destroyWindow(window);

    if (window->mode == GLFW_FULLSCREEN)
    {
        if (_glfwLibrary.Win32.monitor.modeChanged)
        {
            _glfwRestoreVideoMode();
            _glfwLibrary.Win32.monitor.modeChanged = GL_FALSE;
        }
    }
}


//========================================================================
// Set the window title
//========================================================================

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
    WCHAR* wideTitle = _glfwCreateWideStringFromUTF8(title);
    if (!wideTitle)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32: Failed to convert title to wide string");
        return;
    }

    SetWindowText(window->Win32.handle, wideTitle);

    free(wideTitle);
}


//========================================================================
// Set the window size.
//========================================================================

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    GLboolean sizeChanged = GL_FALSE;

    if (window->mode == GLFW_FULLSCREEN)
    {
        if (width > window->width || height > window->height)
        {
            // The new video mode is larger than the current one, so we resize
            // the window before switch modes to avoid exposing whatever is
            // underneath

            SetWindowPos(window->Win32.handle, HWND_TOP, 0, 0, width, height,
                         SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
            sizeChanged = GL_TRUE;
        }

        // TODO: Change video mode
    }
    else
    {
        // If we are in windowed mode, adjust the window size to
        // compensate for window decorations
        getFullWindowSize(window, width, height, &width, &height);
    }

    // Set window size (if we haven't already)
    if (!sizeChanged)
    {
        SetWindowPos(window->Win32.handle, HWND_TOP, 0, 0, width, height,
                     SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
    }
}


//========================================================================
// Set the window position
//========================================================================

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int x, int y)
{
    RECT rect;

    GetClientRect(window->Win32.handle, &rect);
    AdjustWindowRectEx(&rect, window->Win32.dwStyle, FALSE, window->Win32.dwExStyle);

    SetWindowPos(window->Win32.handle, HWND_TOP,
                 x + rect.left, y + rect.top, 0, 0,
                 SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
}


//========================================================================
// Window iconification
//========================================================================

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
    ShowWindow(window->Win32.handle, SW_MINIMIZE);
}


//========================================================================
// Window un-iconification
//========================================================================

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
    ShowWindow(window->Win32.handle, SW_RESTORE);
}


//========================================================================
// Show or hide window
//========================================================================

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
    ShowWindow(window->Win32.handle, SW_SHOWNORMAL);
    BringWindowToTop(window->Win32.handle);
    SetForegroundWindow(window->Win32.handle);
    SetFocus(window->Win32.handle);
}


//========================================================================
// Show or hide window
//========================================================================

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
    ShowWindow(window->Win32.handle, SW_HIDE);
}

//========================================================================
// Write back window parameters into GLFW window structure
//========================================================================

void _glfwPlatformRefreshWindowParams(_GLFWwindow* window)
{
    DEVMODE dm;

    ZeroMemory(&dm, sizeof(DEVMODE));
    dm.dmSize = sizeof(DEVMODE);

    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
    {
        window->refreshRate = dm.dmDisplayFrequency;
        if (window->refreshRate <= 1)
            window->refreshRate = 0;
    }
    else
        window->refreshRate = 0;
}


//========================================================================
// Poll for new window and input events
//========================================================================

void _glfwPlatformPollEvents(void)
{
    MSG msg;
    _GLFWwindow* window;

    window = _glfwLibrary.activeWindow;
    if (window)
    {
        window->Win32.cursorCentered = GL_FALSE;
        window->Win32.oldCursorX = window->width / 2;
        window->Win32.oldCursorY = window->height / 2;
    }
    else
    {
        //window->Win32.oldCursorX = window->cursorPosX;
        //window->Win32.oldCursorY = window->cursorPosY;
    }

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        switch (msg.message)
        {
            case WM_QUIT:
            {
                // Treat WM_QUIT as a close on all windows

                window = _glfwLibrary.windowListHead;
                while (window)
                {
                    _glfwInputWindowCloseRequest(window);
                    window = window->next;
                }

                break;
            }

            default:
            {
                DispatchMessage(&msg);
                break;
            }
        }
    }

    // LSHIFT/RSHIFT fixup (keys tend to "stick" without this fix)
    // This is the only async event handling in GLFW, but it solves some
    // nasty problems.
    window = _glfwLibrary.activeWindow;
    if (window)
    {
        int lshift_down, rshift_down;

        // Get current state of left and right shift keys
        lshift_down = (GetAsyncKeyState(VK_LSHIFT) >> 15) & 1;
        rshift_down = (GetAsyncKeyState(VK_RSHIFT) >> 15) & 1;

        // See if this differs from our belief of what has happened
        // (we only have to check for lost key up events)
        if (!lshift_down && window->key[GLFW_KEY_LEFT_SHIFT] == 1)
            _glfwInputKey(window, GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE);

        if (!rshift_down && window->key[GLFW_KEY_RIGHT_SHIFT] == 1)
            _glfwInputKey(window, GLFW_KEY_RIGHT_SHIFT, GLFW_RELEASE);
    }

    // Did the cursor move in an active window that has captured the cursor
    window = _glfwLibrary.activeWindow;
    if (window)
    {
        if (window->cursorMode == GLFW_CURSOR_CAPTURED &&
            !window->Win32.cursorCentered)
        {
            _glfwPlatformSetCursorPos(window,
                                      window->width / 2,
                                      window->height / 2);
            window->Win32.cursorCentered = GL_TRUE;
        }
    }
}


//========================================================================
// Wait for new window and input events
//========================================================================

void _glfwPlatformWaitEvents(void)
{
    WaitMessage();

    _glfwPlatformPollEvents();
}


//========================================================================
// Set physical cursor position
//========================================================================

void _glfwPlatformSetCursorPos(_GLFWwindow* window, int x, int y)
{
    POINT pos;

    // Convert client coordinates to screen coordinates
    pos.x = x;
    pos.y = y;
    ClientToScreen(window->Win32.handle, &pos);

    SetCursorPos(pos.x, pos.y);
}


//========================================================================
// Set physical mouse cursor mode
//========================================================================

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
        case GLFW_CURSOR_CAPTURED:
            captureCursor(window);
            break;
    }
}

