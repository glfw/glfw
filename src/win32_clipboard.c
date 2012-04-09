//========================================================================
// GLFW - An OpenGL library
// Platform:    Win32/WGL
// API version: 3.0
// WWW:         http://www.glfw.org/
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


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Set the clipboard contents
//========================================================================

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string)
{
    WCHAR* wideString;
    HANDLE stringHandle;
    size_t wideSize;

    wideString = _glfwCreateWideStringFromUTF8(string);
    if (!wideString)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                        "Win32/WGL: Failed to convert clipboard string to "
                        "wide string");
        return;
    }

    wideSize = (wcslen(wideString) + 1) * sizeof(WCHAR);

    stringHandle = GlobalAlloc(GMEM_MOVEABLE, wideSize);
    if (!stringHandle)
    {
        free(wideString);

        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32/WGL: Failed to allocate global handle for clipboard");
        return;
    }

    memcpy(GlobalLock(stringHandle), wideString, wideSize);
    GlobalUnlock(stringHandle);

    if (!OpenClipboard(window->Win32.handle))
    {
        GlobalFree(stringHandle);
        free(wideString);

        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32/WGL: Failed to open clipboard");
        return;
    }

    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, stringHandle);
    CloseClipboard();

    free(wideString);
}

//========================================================================
// Return the current clipboard contents
//========================================================================

size_t _glfwPlatformGetClipboardString(_GLFWwindow* window, char* string, size_t size)
{
    HANDLE stringHandle;
    char* utf8String;
    size_t utf8Size;

    if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
    {
        _glfwSetError(GLFW_FORMAT_UNAVAILABLE, NULL);
        return 0;
    }

    if (!OpenClipboard(window->Win32.handle))
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32/WGL: Failed to open clipboard");
        return 0;
    }

    stringHandle = GetClipboardData(CF_UNICODETEXT);
    if (!stringHandle)
    {
        CloseClipboard();

        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32/WGL: Failed to retrieve clipboard data");
        return 0;
    }

    utf8String = _glfwCreateUTF8FromWideString(GlobalLock(stringHandle));
    GlobalUnlock(stringHandle);
    CloseClipboard();

    if (!utf8String)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "Win32/WGL: Failed to convert wide string to UTF-8");
        return 0;
    }

    utf8Size = strlen(utf8String) + 1;
    if (utf8Size > size)
    {
        memcpy(string, utf8String, size);
        string[size - 1] = '\0';
    }
    else
        memcpy(string, utf8String, utf8Size);

    free(utf8String);
	return utf8Size;
}

