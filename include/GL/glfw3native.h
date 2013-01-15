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

#ifndef _glfw3_native_h_
#define _glfw3_native_h_

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
 * Doxygen documentation
 *************************************************************************/

/*! @defgroup native Native API
 *
 *  By using the native API, you assert that you know what you are doing and how
 *  to fix problems caused by using it.  If you don't, you shouldn't be using
 *  it.
 */


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

/*! @brief Returns the @c HWND of the specified window.
 *  @return The @c HWND of the specified window.
 *  @ingroup native
 */
GLFWAPI HWND glfwGetWin32Window(GLFWwindow window);

/*! @brief Returns the @c HGLRC of the specified window.
 *  @return The @c HGLRC of the specified window.
 *  @ingroup native
 */
GLFWAPI HGLRC glfwGetWGLContext(GLFWwindow window);

#elif defined(GLFW_EXPOSE_NATIVE_COCOA_NSGL)

/*! @brief Returns the @c NSWindow of the specified window.
 *  @return The @c NSWindow of the specified window.
 *  @ingroup native
 */
GLFWAPI id glfwGetCocoaWindow(GLFWwindow window);

/*! @brief Returns the @c NSOpenGLContext of the specified window.
 *  @return The @c NSOpenGLContext of the specified window.
 *  @ingroup native
 */
GLFWAPI id glfwGetNSGLContext(GLFWwindow window);

#elif defined(GLFW_EXPOSE_NATIVE_X11_GLX)

/*! @brief Returns the @c Display used by GLFW.
 *  @return The @c Display used by GLFW.
 *  @ingroup native
 */
GLFWAPI Display* glfwGetX11Display(void);

/*! @brief Returns the @c Window of the specified window.
 *  @return The @c Window of the specified window.
 *  @ingroup native
 */
GLFWAPI Window glfwGetX11Window(GLFWwindow window);

/*! @brief Returns the @c GLXContext of the specified window.
 *  @return The @c GLXContext of the specified window.
 *  @ingroup native
 */
GLFWAPI GLXContext glfwGetGLXContext(GLFWwindow window);

#endif


#ifdef __cplusplus
}
#endif

#endif /* _glfw3_native_h_ */

