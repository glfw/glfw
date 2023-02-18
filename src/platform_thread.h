//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2023 Camilla Löwy <elmindreda@glfw.org>
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

#if defined(GLFW_BUILD_WIN32_THREAD) || \
    defined(GLFW_BUILD_POSIX_THREAD)
 #error "You must not define these; define zero or more _GLFW_<platform> macros instead"
#endif

#if defined(_WIN32)
 #define GLFW_BUILD_WIN32_THREAD
#else
 #define GLFW_BUILD_POSIX_THREAD
#endif

#if defined(GLFW_BUILD_WIN32_THREAD)
 #include "win32_thread.h"
 #define GLFW_PLATFORM_TLS_STATE    GLFW_WIN32_TLS_STATE
 #define GLFW_PLATFORM_MUTEX_STATE  GLFW_WIN32_MUTEX_STATE
 #define GLFW_PLATFORM_CONDVAR_STATE  GLFW_WIN32_CONDVAR_STATE
#elif defined(GLFW_BUILD_POSIX_THREAD)
 #include "posix_thread.h"
 #define GLFW_PLATFORM_TLS_STATE    GLFW_POSIX_TLS_STATE
 #define GLFW_PLATFORM_MUTEX_STATE  GLFW_POSIX_MUTEX_STATE
 #define GLFW_PLATFORM_CONDVAR_STATE  GLFW_POSIX_CONDVAR_STATE
#endif
