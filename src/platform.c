//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2018 Camilla Löwy <elmindreda@glfw.org>
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
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#include "internal.h"

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwSelectPlatform(int platformID, _GLFWplatform* platform)
{
    const struct
    {
        int platformID;
        GLFWbool (*detect)(int,_GLFWplatform*);
    } available[] =
    {
#if defined(_GLFW_WIN32)
        { GLFW_PLATFORM_WIN32, _glfwDetectWin32 },
#endif
#if defined(_GLFW_COCOA)
        { GLFW_PLATFORM_COCOA, _glfwDetectCocoa },
#endif
#if defined(_GLFW_X11)
        { GLFW_PLATFORM_X11, _glfwDetectX11 },
#endif
#if defined(_GLFW_WAYLAND)
        { GLFW_PLATFORM_WAYLAND, _glfwDetectWayland },
#endif
    };
    size_t i, count = sizeof(available) / sizeof(available[0]);

    if (platformID != GLFW_ANY_PLATFORM &&
        platformID != GLFW_PLATFORM_WIN32 &&
        platformID != GLFW_PLATFORM_COCOA &&
        platformID != GLFW_PLATFORM_WAYLAND &&
        platformID != GLFW_PLATFORM_X11 &&
        platformID != GLFW_PLATFORM_NULL)
    {
        _glfwInputError(GLFW_INVALID_ENUM, "Invalid platform ID");
        return GLFW_FALSE;
    }

    // Only allow the null platform if specifically requested
    if (platformID == GLFW_PLATFORM_NULL)
        return _glfwDetectNull(platformID, platform);

    // If there is only one available platform, let it emit the error if detection fails
    if (platformID == GLFW_ANY_PLATFORM && count == 1)
        return available[0].detect(available[0].platformID, platform);

    for (i = 0;  i < count;  i++)
    {
        if (platformID == GLFW_ANY_PLATFORM || platformID == available[i].platformID)
        {
            if (available[i].detect(platformID, platform))
                return GLFW_TRUE;
            else if (platformID == available[i].platformID)
                return GLFW_FALSE;
        }
    }

    if (platformID == GLFW_ANY_PLATFORM)
        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "Failed to detect any supported platform");
    else
        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "The requested platform is not supported");

    return GLFW_FALSE;
}

//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI int glfwGetPlatform(void)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(0);
    return _glfw.platform.platformID;
}

GLFWAPI int glfwPlatformSupported(int platformID)
{
    if (platformID != GLFW_PLATFORM_WIN32 &&
        platformID != GLFW_PLATFORM_COCOA &&
        platformID != GLFW_PLATFORM_WAYLAND &&
        platformID != GLFW_PLATFORM_X11 &&
        platformID != GLFW_PLATFORM_NULL)
    {
        _glfwInputError(GLFW_INVALID_ENUM, "Invalid platform ID");
        return GLFW_FALSE;
    }

#if defined(_GLFW_WIN32)
    if (platformID == GLFW_PLATFORM_WIN32)
        return GLFW_TRUE;
#endif
#if defined(_GLFW_COCOA)
    if (platformID == GLFW_PLATFORM_COCOA)
        return GLFW_TRUE;
#endif
#if defined(_GLFW_WAYLAND)
    if (platformID == GLFW_PLATFORM_WAYLAND)
        return GLFW_TRUE;
#endif
#if defined(_GLFW_X11)
    if (platformID == GLFW_PLATFORM_X11)
        return GLFW_TRUE;
#endif
    if (platformID == GLFW_PLATFORM_NULL)
        return GLFW_TRUE;

    return GLFW_FALSE;
}

GLFWAPI const char* glfwGetVersionString(void)
{
    return _GLFW_VERSION_NUMBER
#if defined(_GLFW_WIN32)
        " Win32 WGL"
#endif
#if defined(_GLFW_COCOA)
        " Cocoa NSGL"
#endif
#if defined(_GLFW_WAYLAND)
        " Wayland"
#endif
#if defined(_GLFW_X11)
        " X11 GLX"
#endif
        " Null"
        " EGL"
        " OSMesa"
#if defined(__MINGW64_VERSION_MAJOR)
        " MinGW-w64"
#elif defined(__MINGW32__)
        " MinGW"
#elif defined(_MSC_VER)
        " VisualC"
#endif
#if defined(_GLFW_USE_HYBRID_HPG) || defined(_GLFW_USE_OPTIMUS_HPG)
        " hybrid-GPU"
#endif
#if defined(_POSIX_MONOTONIC_CLOCK)
        " monotonic"
#endif
#if defined(_GLFW_BUILD_DLL)
#if defined(_WIN32)
        " DLL"
#elif defined(__APPLE__)
        " dynamic"
#else
        " shared"
#endif
#endif
        ;
}

