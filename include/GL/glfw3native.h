/*************************************************************************
 * GLFW - An OpenGL library
 * API version: 3.0
 * WWW:         http://www.glfw.org/
 *------------------------------------------------------------------------
 * Copyright (c) 2002-2006 Marcus Geelnard
 * Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 *************************************************************************/

#ifndef __glfw3_platform_h__
#define __glfw3_platform_h__

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
 * System headers and types
 *************************************************************************/

#if defined(GLFW_EXPOSE_NATIVE_WIN32_WGL)

 /* We are building for Win32 and WGL */
 #include <windows.h>

#elif defined(GLFW_EXPOSE_NATIVE_COCOA_NSGL)

 /* We are building for Cocoa and NSOpenGL */
 #if defined(__OBJC__)
  #import <Cocoa/Cocoa.h>
 #else
  typedef void* id;
 #endif

#elif defined(GLFW_EXPOSE_NATIVE_X11_GLX)

 /* We are building for X11 and GLX */
 #include <X11/Xlib.h>

#else

 #error "No platform specified"

#endif


/*************************************************************************
 * Functions
 *************************************************************************/

#if defined(GLFW_EXPOSE_NATIVE_WIN32_WGL)

GLFWAPI HWND glfwGetWin32Window(GLFWwindow window);
GLFWAPI HGLRC glfwGetWGLContext(GLFWwindow window);

#elif defined(GLFW_EXPOSE_NATIVE_COCOA_NSGL)

GLFWAPI id glfwGetCocoaWindow(GLFWwindow window);
GLFWAPI id glfwGetNSGLContext(GLFWwindow window);

#elif defined(GLFW_EXPOSE_NATIVE_X11_GLX)

GLFWAPI Display* glfwGetX11Display(void);

GLFWAPI Window glfwGetX11Window(GLFWwindow window);
GLFWAPI GLXContext glfwGetGLXContext(GLFWwindow window);

#endif


#ifdef __cplusplus
}
#endif

#endif /* __glfw3_platform_h__ */

