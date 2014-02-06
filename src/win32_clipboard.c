//========================================================================
// GLFW 3.1 Win32 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2010 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string)
{
    WCHAR* wideString;
    HANDLE stringHandle;
    size_t wideSize;

    wideString = _glfwCreateWideStringFromUTF8(string);
    if (!wideString)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to convert clipboard string to "
                        "wide string");
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

    if (!OpenClipboard(window->win32.handle))
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

    if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
    {
        _glfwInputError(GLFW_FORMAT_UNAVAILABLE, NULL);
        return NULL;
    }

    if (!OpenClipboard(window->win32.handle))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Win32: Failed to open clipboard");
        return NULL;
    }

    stringHandle = GetClipboardData(CF_UNICODETEXT);
    if (!stringHandle)
    {
        CloseClipboard();

        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to retrieve clipboard data");
        return NULL;
    }

    free(_glfw.win32.clipboardString);
    _glfw.win32.clipboardString =
        _glfwCreateUTF8FromWideString(GlobalLock(stringHandle));

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

