//========================================================================
// GLFW 3.2 Win32 - www.glfw.org
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
#include <string.h>
#include <windowsx.h>
#include <shellapi.h>

#define _GLFW_KEY_INVALID -2

// Returns the window style for the specified window
//
static DWORD getWindowStyle(const _GLFWwindow* window)
{
    DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (window->decorated && !window->monitor)
    {
        style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

        if (window->resizable)
            style |= WS_MAXIMIZEBOX | WS_SIZEBOX;
    }
    else
        style |= WS_POPUP;

    return style;
}

// Returns the extended window style for the specified window
//
static DWORD getWindowExStyle(const _GLFWwindow* window)
{
    DWORD style = WS_EX_APPWINDOW;

    if (window->decorated && !window->monitor)
        style |= WS_EX_WINDOWEDGE;

    return style;
}

// Translate client window size to full window size according to styles
//
static void getFullWindowSize(DWORD style, DWORD exStyle,
                              int clientWidth, int clientHeight,
                              int* fullWidth, int* fullHeight)
{
    RECT rect = { 0, 0, clientWidth, clientHeight };
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    *fullWidth = rect.right - rect.left;
    *fullHeight = rect.bottom - rect.top;
}

// Enforce the client rect aspect ratio based on which edge is being dragged
//
static void applyAspectRatio(_GLFWwindow* window, int edge, RECT* area)
{
    int xoff, yoff;
    const float ratio = (float) window->win32.numer /
                        (float) window->win32.denom;

    getFullWindowSize(getWindowStyle(window), getWindowExStyle(window),
                      0, 0, &xoff, &yoff);

    if (edge == WMSZ_LEFT  || edge == WMSZ_BOTTOMLEFT ||
        edge == WMSZ_RIGHT || edge == WMSZ_BOTTOMRIGHT)
    {
        area->bottom = area->top + yoff +
            (int) ((area->right - area->left - xoff) / ratio);
    }
    else if (edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT)
    {
        area->top = area->bottom - yoff -
            (int) ((area->right - area->left - xoff) / ratio);
    }
    else if (edge == WMSZ_TOP || edge == WMSZ_BOTTOM)
    {
        area->right = area->left + xoff +
            (int) ((area->bottom - area->top - yoff) * ratio);
    }
}

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

// Translates a GLFW standard cursor to a resource ID
//
static LPWSTR translateCursorShape(int shape)
{
    switch (shape)
    {
        case GLFW_ARROW_CURSOR:
            return IDC_ARROW;
        case GLFW_IBEAM_CURSOR:
            return IDC_IBEAM;
        case GLFW_CROSSHAIR_CURSOR:
            return IDC_CROSS;
        case GLFW_HAND_CURSOR:
            return IDC_HAND;
        case GLFW_HRESIZE_CURSOR:
            return IDC_SIZEWE;
        case GLFW_VRESIZE_CURSOR:
            return IDC_SIZENS;
    }

    return NULL;
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
    if (wParam == VK_CONTROL)
    {
        // The CTRL keys require special handling

        MSG next;
        DWORD time;

        // Is this an extended key (i.e. right key)?
        if (lParam & 0x01000000)
            return GLFW_KEY_RIGHT_CONTROL;

        // Here is a trick: "Alt Gr" sends LCTRL, then RALT. We only
        // want the RALT message, so we try to see if the next message
        // is a RALT message. In that case, this is a false LCTRL!
        time = GetMessageTime();

        if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
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

    return _glfw.win32.publicKeys[HIWORD(lParam) & 0x1FF];
}

// Enter full screen mode
//
static GLFWbool enterFullscreenMode(_GLFWwindow* window)
{
    GLFWvidmode mode;
    GLFWbool status;
    int xpos, ypos;

    status = _glfwSetVideoModeWin32(window->monitor, &window->videoMode);

    _glfwPlatformGetVideoMode(window->monitor, &mode);
    _glfwPlatformGetMonitorPos(window->monitor, &xpos, &ypos);

    SetWindowPos(window->win32.handle, HWND_TOPMOST,
                 xpos, ypos, mode.width, mode.height, SWP_NOCOPYBITS);

    return status;
}

// Leave full screen mode
//
static void leaveFullscreenMode(_GLFWwindow* window)
{
    _glfwRestoreVideoModeWin32(window->monitor);
}

// Window callback function (handles window messages)
//
static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg,
                                   WPARAM wParam, LPARAM lParam)
{
    _GLFWwindow* window = (_GLFWwindow*) GetWindowLongPtrW(hWnd, 0);
    if (!window)
    {
        switch (uMsg)
        {
            case WM_NCCREATE:
            {
                CREATESTRUCTW* cs = (CREATESTRUCTW*) lParam;
                SetWindowLongPtrW(hWnd, 0, (LONG_PTR) cs->lpCreateParams);
                break;
            }

            case WM_DEVICECHANGE:
            {
                if (wParam == DBT_DEVNODES_CHANGED)
                {
                    _glfwInputMonitorChange();
                    return TRUE;
                }

                break;
            }
        }

        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    switch (uMsg)
    {
        case WM_SETFOCUS:
        {
            if (window->cursorMode == GLFW_CURSOR_DISABLED)
                _glfwPlatformSetCursorMode(window, GLFW_CURSOR_DISABLED);

            _glfwInputWindowFocus(window, GLFW_TRUE);
            return 0;
        }

        case WM_KILLFOCUS:
        {
            if (window->cursorMode == GLFW_CURSOR_DISABLED)
                _glfwPlatformSetCursorMode(window, GLFW_CURSOR_NORMAL);

            if (window->monitor && window->autoIconify)
                _glfwPlatformIconifyWindow(window);

            _glfwInputWindowFocus(window, GLFW_FALSE);
            return 0;
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
                        // We are running in full screen mode, so disallow
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

        case WM_CHAR:
        case WM_SYSCHAR:
        case WM_UNICHAR:
        {
            const GLFWbool plain = (uMsg != WM_SYSCHAR);

            if (uMsg == WM_UNICHAR && wParam == UNICODE_NOCHAR)
            {
                // WM_UNICHAR is not sent by Windows, but is sent by some
                // third-party input method engine
                // Returning TRUE here announces support for this message
                return TRUE;
            }

            _glfwInputChar(window, (unsigned int) wParam, getKeyMods(), plain);
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            const int key = translateKey(wParam, lParam);
            const int scancode = (lParam >> 16) & 0x1ff;
            const int action = ((lParam >> 31) & 1) ? GLFW_RELEASE : GLFW_PRESS;
            const int mods = getKeyMods();

            if (key == _GLFW_KEY_INVALID)
                break;

            if (action == GLFW_RELEASE && wParam == VK_SHIFT)
            {
                // Release both Shift keys on Shift up event, as only one event
                // is sent even if both keys are released
                _glfwInputKey(window, GLFW_KEY_LEFT_SHIFT, scancode, action, mods);
                _glfwInputKey(window, GLFW_KEY_RIGHT_SHIFT, scancode, action, mods);
            }
            else if (wParam == VK_SNAPSHOT)
            {
                // Key down is not reported for the Print Screen key
                _glfwInputKey(window, key, scancode, GLFW_PRESS, mods);
                _glfwInputKey(window, key, scancode, GLFW_RELEASE, mods);
            }
            else
                _glfwInputKey(window, key, scancode, action, mods);

            break;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
            int button, action;

            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP)
                button = GLFW_MOUSE_BUTTON_LEFT;
            else if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP)
                button = GLFW_MOUSE_BUTTON_RIGHT;
            else if (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP)
                button = GLFW_MOUSE_BUTTON_MIDDLE;
            else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
                button = GLFW_MOUSE_BUTTON_4;
            else
                button = GLFW_MOUSE_BUTTON_5;

            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN ||
                uMsg == WM_MBUTTONDOWN || uMsg == WM_XBUTTONDOWN)
            {
                action = GLFW_PRESS;
                SetCapture(hWnd);
            }
            else
            {
                action = GLFW_RELEASE;
                ReleaseCapture();
            }

            _glfwInputMouseClick(window, button, action, getKeyMods());

            if (uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONUP)
                return TRUE;

            return 0;
        }

        case WM_MOUSEMOVE:
        {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);

            if (window->cursorMode == GLFW_CURSOR_DISABLED)
            {
                if (_glfw.cursorWindow != window)
                    break;

                _glfwInputCursorMotion(window,
                                       x - window->win32.cursorPosX,
                                       y - window->win32.cursorPosY);
            }
            else
                _glfwInputCursorMotion(window, x, y);

            window->win32.cursorPosX = x;
            window->win32.cursorPosY = y;

            if (!window->win32.cursorTracked)
            {
                TRACKMOUSEEVENT tme;
                ZeroMemory(&tme, sizeof(tme));
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = window->win32.handle;
                TrackMouseEvent(&tme);

                window->win32.cursorTracked = GLFW_TRUE;
                _glfwInputCursorEnter(window, GLFW_TRUE);
            }

            return 0;
        }

        case WM_MOUSELEAVE:
        {
            window->win32.cursorTracked = GLFW_FALSE;
            _glfwInputCursorEnter(window, GLFW_FALSE);
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
            // NOTE: The X-axis is inverted for consistency with OS X and X11.
            _glfwInputScroll(window, -((SHORT) HIWORD(wParam) / (double) WHEEL_DELTA), 0.0);
            return 0;
        }

        case WM_SIZE:
        {
            if (_glfw.cursorWindow == window)
            {
                if (window->cursorMode == GLFW_CURSOR_DISABLED)
                    updateClipRect(window);
            }

            if (!window->win32.iconified && wParam == SIZE_MINIMIZED)
            {
                window->win32.iconified = GLFW_TRUE;
                if (window->monitor)
                    leaveFullscreenMode(window);

                _glfwInputWindowIconify(window, GLFW_TRUE);
            }
            else if (window->win32.iconified &&
                     (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED))
            {
                window->win32.iconified = GLFW_FALSE;
                if (window->monitor)
                    enterFullscreenMode(window);

                _glfwInputWindowIconify(window, GLFW_FALSE);
            }

            _glfwInputFramebufferSize(window, LOWORD(lParam), HIWORD(lParam));
            _glfwInputWindowSize(window, LOWORD(lParam), HIWORD(lParam));
            return 0;
        }

        case WM_MOVE:
        {
            if (_glfw.cursorWindow == window)
            {
                if (window->cursorMode == GLFW_CURSOR_DISABLED)
                    updateClipRect(window);
            }

            // NOTE: This cannot use LOWORD/HIWORD recommended by MSDN, as
            // those macros do not handle negative window positions correctly
            _glfwInputWindowPos(window,
                                GET_X_LPARAM(lParam),
                                GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_SIZING:
        {
            if (window->win32.numer == GLFW_DONT_CARE ||
                window->win32.denom == GLFW_DONT_CARE)
            {
                break;
            }

            applyAspectRatio(window, (int) wParam, (RECT*) lParam);
            return TRUE;
        }

        case WM_GETMINMAXINFO:
        {
            int xoff, yoff;
            MINMAXINFO* mmi = (MINMAXINFO*) lParam;

            getFullWindowSize(getWindowStyle(window), getWindowExStyle(window),
                              0, 0, &xoff, &yoff);

            if (window->win32.minwidth != GLFW_DONT_CARE &&
                window->win32.minheight != GLFW_DONT_CARE)
            {
                mmi->ptMinTrackSize.x = window->win32.minwidth + xoff;
                mmi->ptMinTrackSize.y = window->win32.minheight + yoff;
            }

            if (window->win32.maxwidth != GLFW_DONT_CARE &&
                window->win32.maxheight != GLFW_DONT_CARE)
            {
                mmi->ptMaxTrackSize.x = window->win32.maxwidth + xoff;
                mmi->ptMaxTrackSize.y = window->win32.maxheight + yoff;
            }

            return 0;
        }

        case WM_PAINT:
        {
            _glfwInputWindowDamage(window);
            break;
        }

        case WM_ERASEBKGND:
        {
            return TRUE;
        }

        case WM_SETCURSOR:
        {
            if (_glfw.cursorWindow == window && LOWORD(lParam) == HTCLIENT)
            {
                if (window->cursorMode == GLFW_CURSOR_HIDDEN ||
                    window->cursorMode == GLFW_CURSOR_DISABLED)
                {
                    SetCursor(NULL);
                    return TRUE;
                }
                else if (window->cursor)
                {
                    SetCursor(window->cursor->win32.handle);
                    return TRUE;
                }
            }

            break;
        }

        case WM_DPICHANGED:
        {
            RECT* rect = (RECT*) lParam;
            SetWindowPos(window->win32.handle,
                         HWND_TOP,
                         rect->left,
                         rect->top,
                         rect->right - rect->left,
                         rect->bottom - rect->top,
                         SWP_NOACTIVATE | SWP_NOZORDER);
            break;
        }

        case WM_DROPFILES:
        {
            HDROP drop = (HDROP) wParam;
            POINT pt;
            int i;

            const int count = DragQueryFileW(drop, 0xffffffff, NULL, 0);
            char** paths = calloc(count, sizeof(char*));

            // Move the mouse to the position of the drop
            DragQueryPoint(drop, &pt);
            _glfwInputCursorMotion(window, pt.x, pt.y);

            for (i = 0;  i < count;  i++)
            {
                const UINT length = DragQueryFileW(drop, i, NULL, 0);
                WCHAR* buffer = calloc(length + 1, sizeof(WCHAR));

                DragQueryFileW(drop, i, buffer, length + 1);
                paths[i] = _glfwCreateUTF8FromWideStringWin32(buffer);

                free(buffer);
            }

            _glfwInputDrop(window, count, (const char**) paths);

            for (i = 0;  i < count;  i++)
                free(paths[i]);
            free(paths);

            DragFinish(drop);
            return 0;
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

// Creates the GLFW window and rendering context
//
static int createWindow(_GLFWwindow* window, const _GLFWwndconfig* wndconfig)
{
    int xpos, ypos, fullWidth, fullHeight;
    WCHAR* wideTitle;

    if (wndconfig->monitor)
    {
        GLFWvidmode mode;

        // NOTE: This window placement is temporary and approximate, as the
        //       correct position and size cannot be known until the monitor
        //       video mode has been set
        _glfwPlatformGetMonitorPos(wndconfig->monitor, &xpos, &ypos);
        _glfwPlatformGetVideoMode(wndconfig->monitor, &mode);
        fullWidth  = mode.width;
        fullHeight = mode.height;
    }
    else
    {
        xpos = CW_USEDEFAULT;
        ypos = CW_USEDEFAULT;

        getFullWindowSize(getWindowStyle(window), getWindowExStyle(window),
                          wndconfig->width, wndconfig->height,
                          &fullWidth, &fullHeight);
    }

    wideTitle = _glfwCreateWideStringFromUTF8Win32(wndconfig->title);
    if (!wideTitle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to convert window title to UTF-16");
        return GLFW_FALSE;
    }

    window->win32.handle = CreateWindowExW(getWindowExStyle(window),
                                           _GLFW_WNDCLASSNAME,
                                           wideTitle,
                                           getWindowStyle(window),
                                           xpos, ypos,
                                           fullWidth, fullHeight,
                                           NULL, // No parent window
                                           NULL, // No window menu
                                           GetModuleHandleW(NULL),
                                           window); // Pass object to WM_CREATE

    free(wideTitle);

    if (!window->win32.handle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to create window");
        return GLFW_FALSE;
    }

    if (_glfw_ChangeWindowMessageFilterEx)
    {
        _glfw_ChangeWindowMessageFilterEx(window->win32.handle,
                                          WM_DROPFILES, MSGFLT_ALLOW, NULL);
        _glfw_ChangeWindowMessageFilterEx(window->win32.handle,
                                          WM_COPYDATA, MSGFLT_ALLOW, NULL);
        _glfw_ChangeWindowMessageFilterEx(window->win32.handle,
                                          WM_COPYGLOBALDATA, MSGFLT_ALLOW, NULL);
    }

    if (wndconfig->floating && !wndconfig->monitor)
    {
        SetWindowPos(window->win32.handle,
                     HWND_TOPMOST,
                     0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    }

    DragAcceptFiles(window->win32.handle, TRUE);

    window->win32.minwidth  = GLFW_DONT_CARE;
    window->win32.minheight = GLFW_DONT_CARE;
    window->win32.maxwidth  = GLFW_DONT_CARE;
    window->win32.maxheight = GLFW_DONT_CARE;
    window->win32.numer     = GLFW_DONT_CARE;
    window->win32.denom     = GLFW_DONT_CARE;

    return GLFW_TRUE;
}

// Destroys the GLFW window and rendering context
//
static void destroyWindow(_GLFWwindow* window)
{
    if (window->win32.handle)
    {
        DestroyWindow(window->win32.handle);
        window->win32.handle = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Registers the GLFW window class
//
GLFWbool _glfwRegisterWindowClassWin32(void)
{
    WNDCLASSEXW wc;

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC) windowProc;
    wc.cbWndExtra    = sizeof(void*) + sizeof(int); // Make room for one pointer
    wc.hInstance     = GetModuleHandleW(NULL);
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.lpszClassName = _GLFW_WNDCLASSNAME;

    // Load user-provided icon if available
    wc.hIcon = LoadImageW(GetModuleHandleW(NULL),
                          L"GLFW_ICON", IMAGE_ICON,
                          0, 0, LR_DEFAULTSIZE | LR_SHARED);
    if (!wc.hIcon)
    {
        // No user-provided icon found, load default icon
        wc.hIcon = LoadImageW(NULL,
                              IDI_APPLICATION, IMAGE_ICON,
                              0, 0, LR_DEFAULTSIZE | LR_SHARED);
    }

    if (!RegisterClassExW(&wc))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to register window class");
        return GLFW_FALSE;
    }

    return GLFW_TRUE;
}

// Unregisters the GLFW window class
//
void _glfwUnregisterWindowClassWin32(void)
{
    UnregisterClassW(_GLFW_WNDCLASSNAME, GetModuleHandleW(NULL));
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWctxconfig* ctxconfig,
                              const _GLFWfbconfig* fbconfig)
{
    int status;

    if (!createWindow(window, wndconfig))
        return GLFW_FALSE;

    if (ctxconfig->api != GLFW_NO_API)
    {
#if defined(_GLFW_WGL)
        if (!_glfwCreateContextWGL(window, ctxconfig, fbconfig))
            return GLFW_FALSE;

        status = _glfwAnalyzeContextWGL(window, ctxconfig, fbconfig);

        if (status == _GLFW_RECREATION_IMPOSSIBLE)
            return GLFW_FALSE;

        if (status == _GLFW_RECREATION_REQUIRED)
        {
            // Some window hints require us to re-create the context using WGL
            // extensions retrieved through the current context, as we cannot
            // check for WGL extensions or retrieve WGL entry points before we
            // have a current context (actually until we have implicitly loaded
            // the vendor ICD)

            // Yes, this is strange, and yes, this is the proper way on WGL

            // As Windows only allows you to set the pixel format once for
            // a window, we need to destroy the current window and create a new
            // one to be able to use the new pixel format

            // Technically, it may be possible to keep the old window around if
            // we're just creating an OpenGL 3.0+ context with the same pixel
            // format, but it's not worth the added code complexity

            // First we clear the current context (the one we just created)
            // This is usually done by glfwDestroyWindow, but as we're not doing
            // full GLFW window destruction, it's duplicated here
            _glfwPlatformMakeContextCurrent(NULL);

            // Next destroy the Win32 window and WGL context (without resetting
            // or destroying the GLFW window object)
            _glfwDestroyContextWGL(window);
            destroyWindow(window);

            // ...and then create them again, this time with better APIs
            if (!createWindow(window, wndconfig))
                return GLFW_FALSE;
            if (!_glfwCreateContextWGL(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }
#elif defined(_GLFW_EGL)
        if (!_glfwCreateContextEGL(window, ctxconfig, fbconfig))
            return GLFW_FALSE;
#endif
    }

    if (window->monitor)
    {
        _glfwPlatformShowWindow(window);
        if (!enterFullscreenMode(window))
            return GLFW_FALSE;
    }

    return GLFW_TRUE;
}

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
    if (window->monitor)
        leaveFullscreenMode(window);

    if (window->context.api != GLFW_NO_API)
    {
#if defined(_GLFW_WGL)
        _glfwDestroyContextWGL(window);
#elif defined(_GLFW_EGL)
        _glfwDestroyContextEGL(window);
#endif
    }

    destroyWindow(window);
}

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
    WCHAR* wideTitle = _glfwCreateWideStringFromUTF8Win32(title);
    if (!wideTitle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to convert window title to UTF-16");
        return;
    }

    SetWindowTextW(window->win32.handle, wideTitle);
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
    AdjustWindowRectEx(&rect, getWindowStyle(window),
                       FALSE, getWindowExStyle(window));
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
        enterFullscreenMode(window);
    else
    {
        int fullWidth, fullHeight;
        getFullWindowSize(getWindowStyle(window), getWindowExStyle(window),
                          width, height, &fullWidth, &fullHeight);

        SetWindowPos(window->win32.handle, HWND_TOP,
                     0, 0, fullWidth, fullHeight,
                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
    }
}

void _glfwPlatformSetWindowSizeLimits(_GLFWwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{
    RECT area;

    window->win32.minwidth  = minwidth;
    window->win32.minheight = minheight;
    window->win32.maxwidth  = maxwidth;
    window->win32.maxheight = maxheight;

    if ((minwidth == GLFW_DONT_CARE || minheight == GLFW_DONT_CARE) &&
        (maxwidth == GLFW_DONT_CARE || maxheight == GLFW_DONT_CARE))
    {
        return;
    }

    GetWindowRect(window->win32.handle, &area);
    MoveWindow(window->win32.handle,
               area.left, area.top,
               area.right - area.left,
               area.bottom - area.top, TRUE);
}

void _glfwPlatformSetWindowAspectRatio(_GLFWwindow* window, int numer, int denom)
{
    RECT area;

    window->win32.numer = numer;
    window->win32.denom = denom;

    if (numer == GLFW_DONT_CARE || denom == GLFW_DONT_CARE)
        return;

    GetWindowRect(window->win32.handle, &area);
    applyAspectRatio(window, WMSZ_BOTTOMRIGHT, &area);
    MoveWindow(window->win32.handle,
               area.left, area.top,
               area.right - area.left,
               area.bottom - area.top, TRUE);
}

void _glfwPlatformGetFramebufferSize(_GLFWwindow* window, int* width, int* height)
{
    _glfwPlatformGetWindowSize(window, width, height);
}

void _glfwPlatformGetWindowFrameSize(_GLFWwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
    RECT rect;
    int width, height;

    _glfwPlatformGetWindowSize(window, &width, &height);
    SetRect(&rect, 0, 0, width, height);
    AdjustWindowRectEx(&rect, getWindowStyle(window),
                       FALSE, getWindowExStyle(window));

    if (left)
        *left = -rect.left;
    if (top)
        *top = -rect.top;
    if (right)
        *right = rect.right - width;
    if (bottom)
        *bottom = rect.bottom - height;
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
    ShowWindow(window->win32.handle, SW_SHOW);
    BringWindowToTop(window->win32.handle);
    SetForegroundWindow(window->win32.handle);
    SetFocus(window->win32.handle);
}

void _glfwPlatformUnhideWindow(_GLFWwindow* window)
{
    ShowWindow(window->win32.handle, SW_SHOW);
}

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
    ShowWindow(window->win32.handle, SW_HIDE);
}

int _glfwPlatformWindowFocused(_GLFWwindow* window)
{
    return window->win32.handle == GetActiveWindow();
}

int _glfwPlatformWindowIconified(_GLFWwindow* window)
{
    return IsIconic(window->win32.handle);
}

int _glfwPlatformWindowVisible(_GLFWwindow* window)
{
    return IsWindowVisible(window->win32.handle);
}

void _glfwPlatformPollEvents(void)
{
    MSG msg;

    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            // Treat WM_QUIT as a close on all windows
            // While GLFW does not itself post WM_QUIT, other processes may post
            // it to this one, for example Task Manager

            _GLFWwindow* window = _glfw.windowListHead;
            while (window)
            {
                _glfwInputWindowCloseRequest(window);
                window = window->next;
            }
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    if (_glfw.cursorWindow)
    {
        _GLFWwindow* window = _glfw.cursorWindow;

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
            if (!lshiftDown && window->keys[GLFW_KEY_LEFT_SHIFT] == 1)
                _glfwInputKey(window, GLFW_KEY_LEFT_SHIFT, 0, GLFW_RELEASE, mods);

            if (!rshiftDown && window->keys[GLFW_KEY_RIGHT_SHIFT] == 1)
                _glfwInputKey(window, GLFW_KEY_RIGHT_SHIFT, 0, GLFW_RELEASE, mods);
        }

        if (window->cursorMode == GLFW_CURSOR_DISABLED)
        {
            int width, height;
            _glfwPlatformGetWindowSize(window, &width, &height);

            // NOTE: Re-center the cursor only if it has moved since the last
            //       call, to avoid breaking glfwWaitEvents with WM_MOUSEMOVE
            if (window->win32.cursorPosX != width / 2 ||
                window->win32.cursorPosY != height / 2)
            {
                _glfwPlatformSetCursorPos(window, width / 2, height / 2);
            }
        }
    }
}

void _glfwPlatformWaitEvents(void)
{
    WaitMessage();

    _glfwPlatformPollEvents();
}

void _glfwPlatformPostEmptyEvent(void)
{
    _GLFWwindow* window = _glfw.windowListHead;
    PostMessage(window->win32.handle, WM_NULL, 0, 0);
}

void _glfwPlatformGetCursorPos(_GLFWwindow* window, double* xpos, double* ypos)
{
    POINT pos;

    if (GetCursorPos(&pos))
    {
        ScreenToClient(window->win32.handle, &pos);

        if (xpos)
            *xpos = pos.x;
        if (ypos)
            *ypos = pos.y;
    }
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double xpos, double ypos)
{
    POINT pos = { (int) xpos, (int) ypos };

    // Store the new position so it can be recognized later
    window->win32.cursorPosX = pos.x;
    window->win32.cursorPosY = pos.y;

    ClientToScreen(window->win32.handle, &pos);
    SetCursorPos(pos.x, pos.y);
}

void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode)
{
    POINT pos;

    if (mode == GLFW_CURSOR_DISABLED)
        updateClipRect(window);
    else
        ClipCursor(NULL);

    if (!GetCursorPos(&pos))
        return;

    if (WindowFromPoint(pos) != window->win32.handle)
        return;

    if (mode == GLFW_CURSOR_NORMAL)
    {
        if (window->cursor)
            SetCursor(window->cursor->win32.handle);
        else
            SetCursor(LoadCursorW(NULL, IDC_ARROW));
    }
    else
        SetCursor(NULL);
}

const char* _glfwPlatformGetKeyName(int key, int scancode)
{
    WCHAR name[16];

    if (key != GLFW_KEY_UNKNOWN)
        scancode = _glfw.win32.nativeKeys[key];

    if (!_glfwIsPrintable(_glfw.win32.publicKeys[scancode]))
        return NULL;

    if (!GetKeyNameTextW(scancode << 16, name, sizeof(name) / sizeof(WCHAR)))
        return NULL;

    if (!WideCharToMultiByte(CP_UTF8, 0, name, -1,
                             _glfw.win32.keyName,
                             sizeof(_glfw.win32.keyName),
                             NULL, NULL))
    {
        return NULL;
    }

    return _glfw.win32.keyName;
}

int _glfwPlatformCreateCursor(_GLFWcursor* cursor,
                              const GLFWimage* image,
                              int xhot, int yhot)
{
    HDC dc;
    HBITMAP bitmap, mask;
    BITMAPV5HEADER bi;
    ICONINFO ii;
    DWORD* target = 0;
    BYTE* source = (BYTE*) image->pixels;
    int i;

    ZeroMemory(&bi, sizeof(bi));
    bi.bV5Size        = sizeof(BITMAPV5HEADER);
    bi.bV5Width       = image->width;
    bi.bV5Height      = -image->height;
    bi.bV5Planes      = 1;
    bi.bV5BitCount    = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask     = 0x00ff0000;
    bi.bV5GreenMask   = 0x0000ff00;
    bi.bV5BlueMask    = 0x000000ff;
    bi.bV5AlphaMask   = 0xff000000;

    dc = GetDC(NULL);
    bitmap = CreateDIBSection(dc, (BITMAPINFO*) &bi, DIB_RGB_COLORS,
                              (void**) &target, NULL, (DWORD) 0);
    ReleaseDC(NULL, dc);

    if (!bitmap)
        return GLFW_FALSE;

    mask = CreateBitmap(image->width, image->height, 1, 1, NULL);
    if (!mask)
    {
        DeleteObject(bitmap);
        return GLFW_FALSE;
    }

    for (i = 0;  i < image->width * image->height;  i++, target++, source += 4)
    {
        *target = (source[3] << 24) |
                  (source[0] << 16) |
                  (source[1] <<  8) |
                   source[2];
    }

    ZeroMemory(&ii, sizeof(ii));
    ii.fIcon    = FALSE;
    ii.xHotspot = xhot;
    ii.yHotspot = yhot;
    ii.hbmMask  = mask;
    ii.hbmColor = bitmap;

    cursor->win32.handle = (HCURSOR) CreateIconIndirect(&ii);

    DeleteObject(bitmap);
    DeleteObject(mask);

    if (!cursor->win32.handle)
        return GLFW_FALSE;

    return GLFW_TRUE;
}

int _glfwPlatformCreateStandardCursor(_GLFWcursor* cursor, int shape)
{
    cursor->win32.handle =
        CopyCursor(LoadCursorW(NULL, translateCursorShape(shape)));
    if (!cursor->win32.handle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to create standard cursor");
        return GLFW_FALSE;
    }

    return GLFW_TRUE;
}

void _glfwPlatformDestroyCursor(_GLFWcursor* cursor)
{
    if (cursor->win32.handle)
        DestroyIcon((HICON) cursor->win32.handle);
}

void _glfwPlatformSetCursor(_GLFWwindow* window, _GLFWcursor* cursor)
{
    POINT pos;

    if (_glfw.cursorWindow != window)
        return;

    if (window->cursorMode != GLFW_CURSOR_NORMAL)
        return;

    if (!GetCursorPos(&pos))
        return;

    if (WindowFromPoint(pos) != window->win32.handle)
        return;

    if (cursor)
        SetCursor(cursor->win32.handle);
    else
        SetCursor(LoadCursorW(NULL, IDC_ARROW));
}

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string)
{
    WCHAR* wideString;
    HANDLE stringHandle;
    size_t wideSize;

    wideString = _glfwCreateWideStringFromUTF8Win32(string);
    if (!wideString)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to convert string to UTF-16");
        return;
    }

    wideSize = (wcslen(wideString) + 1) * sizeof(WCHAR);

    stringHandle = GlobalAlloc(GMEM_MOVEABLE, wideSize);
    if (!stringHandle)
    {
        free(wideString);

        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to allocate global handle for clipboard");
        return;
    }

    memcpy(GlobalLock(stringHandle), wideString, wideSize);
    GlobalUnlock(stringHandle);

    if (!OpenClipboard(_glfw.win32.helperWindow))
    {
        GlobalFree(stringHandle);
        free(wideString);

        _glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to open clipboard");
        return;
    }

    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, stringHandle);
    CloseClipboard();

    free(wideString);
}

const char* _glfwPlatformGetClipboardString(_GLFWwindow* window)
{
    HANDLE stringHandle;

    if (!OpenClipboard(_glfw.win32.helperWindow))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to open clipboard");
        return NULL;
    }

    stringHandle = GetClipboardData(CF_UNICODETEXT);
    if (!stringHandle)
    {
        CloseClipboard();

        _glfwInputError(GLFW_FORMAT_UNAVAILABLE,
                        "Win32: Failed to convert clipboard to string");
        return NULL;
    }

    free(_glfw.win32.clipboardString);
    _glfw.win32.clipboardString =
        _glfwCreateUTF8FromWideStringWin32(GlobalLock(stringHandle));

    GlobalUnlock(stringHandle);
    CloseClipboard();

    if (!_glfw.win32.clipboardString)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to convert wide string to UTF-8");
        return NULL;
    }

    return _glfw.win32.clipboardString;
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

