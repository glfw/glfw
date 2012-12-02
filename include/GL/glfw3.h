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

#ifndef __glfw3_h__
#define __glfw3_h__

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
 * Doxygen documentation
 *************************************************************************/

/*! @mainpage notitle
 *
 *  @section intro Introduction
 *
 *  This is the reference documentation for the GLFW library.
 */

/*! @defgroup clipboard Clipboard support
 */
/*! @defgroup error Error handling
 */
/*! @defgroup gamma Gamma ramp support
 */
/*! @defgroup init Initialization and version information
 */
/*! @defgroup input Input handling
 */
/*! @defgroup opengl OpenGL support
 */
/*! @defgroup time Time input
 */
/*! @defgroup window Window handling
 *
 *  The primary purpose of GLFW is to provide a simple interface to OpenGL
 *  context creation and window management.  GLFW supports multiple windows,
 *  which can be either a normal desktop window or a fullscreen window.
 */
/*! @defgroup monitor Monitor handling
 */


/*************************************************************************
 * Global definitions
 *************************************************************************/

/* ------------------- BEGIN SYSTEM/COMPILER SPECIFIC -------------------- */

/* Please report any problems that you find with your compiler, which may
 * be solved in this section! There are several compilers that I have not
 * been able to test this file with yet.
 *
 * First: If we are we on Windows, we want a single define for it (_WIN32)
 * (Note: For Cygwin the compiler flag -mwin32 should be used, but to
 * make sure that things run smoothly for Cygwin users, we add __CYGWIN__
 * to the list of "valid Win32 identifiers", which removes the need for
 * -mwin32)
 */
#if !defined(_WIN32) && (defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__))
 #define _WIN32
#endif /* _WIN32 */

/* In order for extension support to be portable, we need to define an
 * OpenGL function call method. We use the keyword APIENTRY, which is
 * defined for Win32. (Note: Windows also needs this for <GL/gl.h>)
 */
#ifndef APIENTRY
 #ifdef _WIN32
  #define APIENTRY __stdcall
 #else
  #define APIENTRY
 #endif
#endif /* APIENTRY */

/* The following three defines are here solely to make some Windows-based
 * <GL/gl.h> files happy. Theoretically we could include <windows.h>, but
 * it has the major drawback of severely polluting our namespace.
 */

/* Under Windows, we need WINGDIAPI defined */
#if !defined(WINGDIAPI) && defined(_WIN32)
 #if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__POCC__)
  /* Microsoft Visual C++, Borland C++ Builder and Pelles C */
  #define WINGDIAPI __declspec(dllimport)
 #elif defined(__LCC__)
  /* LCC-Win32 */
  #define WINGDIAPI __stdcall
 #else
  /* Others (e.g. MinGW, Cygwin) */
  #define WINGDIAPI extern
 #endif
 #define GLFW_WINGDIAPI_DEFINED
#endif /* WINGDIAPI */

/* Some <GL/glu.h> files also need CALLBACK defined */
#if !defined(CALLBACK) && defined(_WIN32)
 #if defined(_MSC_VER)
  /* Microsoft Visual C++ */
  #if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
   #define CALLBACK __stdcall
  #else
   #define CALLBACK
  #endif
 #else
  /* Other Windows compilers */
  #define CALLBACK __stdcall
 #endif
 #define GLFW_CALLBACK_DEFINED
#endif /* CALLBACK */

/* Most <GL/glu.h> variants on Windows need wchar_t */
#if defined(_WIN32)
 #include <stddef.h>
#endif


/* ---------------- GLFW related system specific defines ----------------- */

#if defined(GLFW_DLL) && defined(_GLFW_BUILD_DLL)
 #error "You must not have both GLFW_DLL and _GLFW_BUILD_DLL defined"
#endif

#if defined(_WIN32) && defined(_GLFW_BUILD_DLL)

 /* We are building a Win32 DLL */
 #define GLFWAPI __declspec(dllexport)

#elif defined(_WIN32) && defined(GLFW_DLL)

 /* We are calling a Win32 DLL */
 #if defined(__LCC__)
  #define GLFWAPI extern
 #else
  #define GLFWAPI __declspec(dllimport)
 #endif

#else

 /* We are either building/calling a static lib or we are non-win32 */
 #define GLFWAPI

#endif

/* -------------------- END SYSTEM/COMPILER SPECIFIC --------------------- */

/* Include the chosen OpenGL header and, optionally, the GLU header.
 */
#if defined(__APPLE_CC__)
  #if defined(GLFW_INCLUDE_GLCOREARB)
    #include <OpenGL/gl3.h>
  #else
    #define GL_GLEXT_LEGACY
    #include <OpenGL/gl.h>
  #endif
  #if defined(GLFW_INCLUDE_GLU)
    #include <OpenGL/glu.h>
  #endif
#else
  #if defined(GLFW_INCLUDE_GLCOREARB)
    #include <GL/glcorearb.h>
  #elif defined(GLFW_INCLUDE_ES1)
    #include <GLES/gl.h>
  #elif defined(GLFW_INCLUDE_ES2)
    #include <GLES2/gl2.h>
  #else
    #include <GL/gl.h>
  #endif
  #if defined(GLFW_INCLUDE_GLU)
    #include <GL/glu.h>
  #endif
#endif


/*************************************************************************
 * GLFW version
 *************************************************************************/

/*! @name GLFW version macros
 *  @{ */
/*! @brief The major version number of the GLFW library.
 *
 *  This is incremented when the API is changed in non-compatible ways.
 *  @ingroup init
 */
#define GLFW_VERSION_MAJOR    3
/*! @brief The minor version number of the GLFW library.
 *
 *  This is incremented when features are added to the API but it remains
 *  backward-compatible.
 *  @ingroup init
 */
#define GLFW_VERSION_MINOR    0
/*! @brief The revision number of the GLFW library.
 *
 *  This is incremented when a bug fix release is made that does not contain any
 *  API changes.
 *  @ingroup init
 */
#define GLFW_VERSION_REVISION 0
/*! @} */


/*************************************************************************
 * Input handling definitions
 *************************************************************************/

/*! @name Key and button actions
 *  @{ */
/*! @brief The key or button was released.
 *  @ingroup input
 */
#define GLFW_RELEASE            0
/*! @brief The key or button was pressed.
 *  @ingroup input
 */
#define GLFW_PRESS              1
/*! @} */

/* Keyboard raw key codes.
 * These key codes are inspired by the USB HID Usage Tables v1.12 (p. 53-60),
 * but re-arranged to map to 7-bit ASCII for printable keys (function keys are
 * put in the 256+ range).
 * The naming of the key codes follow these rules:
 *  - The US keyboard layout is used.
 *  - Names of printable alpha-numeric characters are used (e.g. "A", "R",
 *    "3", etc).
 *  - For non-alphanumeric characters, Unicode:ish names are used (e.g.
 *    "COMMA", "LEFT_SQUARE_BRACKET", etc). Note that some names do not
 *    correspond to the Unicode standard (usually for brevity).
 *  - Keys that lack a clear US mapping are named "WORLD_x".
 *  - For non-printable keys, custom names are used (e.g. "F4",
 *    "BACKSPACE", etc).
 */

/*! @defgroup keys Keyboard keys
 *  @ingroup input
 *  @{
 */

/* Printable keys */
#define GLFW_KEY_SPACE                  32
#define GLFW_KEY_APOSTROPHE             39  /* ' */
#define GLFW_KEY_COMMA                  44  /* , */
#define GLFW_KEY_MINUS                  45  /* - */
#define GLFW_KEY_PERIOD                 46  /* . */
#define GLFW_KEY_SLASH                  47  /* / */
#define GLFW_KEY_0                      48
#define GLFW_KEY_1                      49
#define GLFW_KEY_2                      50
#define GLFW_KEY_3                      51
#define GLFW_KEY_4                      52
#define GLFW_KEY_5                      53
#define GLFW_KEY_6                      54
#define GLFW_KEY_7                      55
#define GLFW_KEY_8                      56
#define GLFW_KEY_9                      57
#define GLFW_KEY_SEMICOLON              59  /* ; */
#define GLFW_KEY_EQUAL                  61  /* = */
#define GLFW_KEY_A                      65
#define GLFW_KEY_B                      66
#define GLFW_KEY_C                      67
#define GLFW_KEY_D                      68
#define GLFW_KEY_E                      69
#define GLFW_KEY_F                      70
#define GLFW_KEY_G                      71
#define GLFW_KEY_H                      72
#define GLFW_KEY_I                      73
#define GLFW_KEY_J                      74
#define GLFW_KEY_K                      75
#define GLFW_KEY_L                      76
#define GLFW_KEY_M                      77
#define GLFW_KEY_N                      78
#define GLFW_KEY_O                      79
#define GLFW_KEY_P                      80
#define GLFW_KEY_Q                      81
#define GLFW_KEY_R                      82
#define GLFW_KEY_S                      83
#define GLFW_KEY_T                      84
#define GLFW_KEY_U                      85
#define GLFW_KEY_V                      86
#define GLFW_KEY_W                      87
#define GLFW_KEY_X                      88
#define GLFW_KEY_Y                      89
#define GLFW_KEY_Z                      90
#define GLFW_KEY_LEFT_BRACKET           91  /* [ */
#define GLFW_KEY_BACKSLASH              92  /* \ */
#define GLFW_KEY_RIGHT_BRACKET          93  /* ] */
#define GLFW_KEY_GRAVE_ACCENT           96  /* ` */
#define GLFW_KEY_WORLD_1                161 /* non-US #1 */
#define GLFW_KEY_WORLD_2                162 /* non-US #2 */

/* Function keys */
#define GLFW_KEY_ESCAPE                 256
#define GLFW_KEY_ENTER                  257
#define GLFW_KEY_TAB                    258
#define GLFW_KEY_BACKSPACE              259
#define GLFW_KEY_INSERT                 260
#define GLFW_KEY_DELETE                 261
#define GLFW_KEY_RIGHT                  262
#define GLFW_KEY_LEFT                   263
#define GLFW_KEY_DOWN                   264
#define GLFW_KEY_UP                     265
#define GLFW_KEY_PAGE_UP                266
#define GLFW_KEY_PAGE_DOWN              267
#define GLFW_KEY_HOME                   268
#define GLFW_KEY_END                    269
#define GLFW_KEY_CAPS_LOCK              280
#define GLFW_KEY_SCROLL_LOCK            281
#define GLFW_KEY_NUM_LOCK               282
#define GLFW_KEY_PRINT_SCREEN           283
#define GLFW_KEY_PAUSE                  284
#define GLFW_KEY_F1                     290
#define GLFW_KEY_F2                     291
#define GLFW_KEY_F3                     292
#define GLFW_KEY_F4                     293
#define GLFW_KEY_F5                     294
#define GLFW_KEY_F6                     295
#define GLFW_KEY_F7                     296
#define GLFW_KEY_F8                     297
#define GLFW_KEY_F9                     298
#define GLFW_KEY_F10                    299
#define GLFW_KEY_F11                    300
#define GLFW_KEY_F12                    301
#define GLFW_KEY_F13                    302
#define GLFW_KEY_F14                    303
#define GLFW_KEY_F15                    304
#define GLFW_KEY_F16                    305
#define GLFW_KEY_F17                    306
#define GLFW_KEY_F18                    307
#define GLFW_KEY_F19                    308
#define GLFW_KEY_F20                    309
#define GLFW_KEY_F21                    310
#define GLFW_KEY_F22                    311
#define GLFW_KEY_F23                    312
#define GLFW_KEY_F24                    313
#define GLFW_KEY_F25                    314
#define GLFW_KEY_KP_0                   320
#define GLFW_KEY_KP_1                   321
#define GLFW_KEY_KP_2                   322
#define GLFW_KEY_KP_3                   323
#define GLFW_KEY_KP_4                   324
#define GLFW_KEY_KP_5                   325
#define GLFW_KEY_KP_6                   326
#define GLFW_KEY_KP_7                   327
#define GLFW_KEY_KP_8                   328
#define GLFW_KEY_KP_9                   329
#define GLFW_KEY_KP_DECIMAL             330
#define GLFW_KEY_KP_DIVIDE              331
#define GLFW_KEY_KP_MULTIPLY            332
#define GLFW_KEY_KP_SUBTRACT            333
#define GLFW_KEY_KP_ADD                 334
#define GLFW_KEY_KP_ENTER               335
#define GLFW_KEY_KP_EQUAL               336
#define GLFW_KEY_LEFT_SHIFT             340
#define GLFW_KEY_LEFT_CONTROL           341
#define GLFW_KEY_LEFT_ALT               342
#define GLFW_KEY_LEFT_SUPER             343
#define GLFW_KEY_RIGHT_SHIFT            344
#define GLFW_KEY_RIGHT_CONTROL          345
#define GLFW_KEY_RIGHT_ALT              346
#define GLFW_KEY_RIGHT_SUPER            347
#define GLFW_KEY_MENU                   348
#define GLFW_KEY_LAST                   GLFW_KEY_MENU

/* GLFW 2.x key name aliases (deprecated) */
#define GLFW_KEY_ESC            GLFW_KEY_ESCAPE
#define GLFW_KEY_DEL            GLFW_KEY_DELETE
#define GLFW_KEY_PAGEUP         GLFW_KEY_PAGE_UP
#define GLFW_KEY_PAGEDOWN       GLFW_KEY_PAGE_DOWN
#define GLFW_KEY_KP_NUM_LOCK    GLFW_KEY_NUM_LOCK
#define GLFW_KEY_LCTRL          GLFW_KEY_LEFT_CONTROL
#define GLFW_KEY_LSHIFT         GLFW_KEY_LEFT_SHIFT
#define GLFW_KEY_LALT           GLFW_KEY_LEFT_ALT
#define GLFW_KEY_LSUPER         GLFW_KEY_LEFT_SUPER
#define GLFW_KEY_RCTRL          GLFW_KEY_RIGHT_CONTROL
#define GLFW_KEY_RSHIFT         GLFW_KEY_RIGHT_SHIFT
#define GLFW_KEY_RALT           GLFW_KEY_RIGHT_ALT
#define GLFW_KEY_RSUPER         GLFW_KEY_RIGHT_SUPER

/*! @} */

/*! @defgroup buttons Mouse buttons
 *  @ingroup input
 *  @{ */
#define GLFW_MOUSE_BUTTON_1      0
#define GLFW_MOUSE_BUTTON_2      1
#define GLFW_MOUSE_BUTTON_3      2
#define GLFW_MOUSE_BUTTON_4      3
#define GLFW_MOUSE_BUTTON_5      4
#define GLFW_MOUSE_BUTTON_6      5
#define GLFW_MOUSE_BUTTON_7      6
#define GLFW_MOUSE_BUTTON_8      7
#define GLFW_MOUSE_BUTTON_LAST   GLFW_MOUSE_BUTTON_8
#define GLFW_MOUSE_BUTTON_LEFT   GLFW_MOUSE_BUTTON_1
#define GLFW_MOUSE_BUTTON_RIGHT  GLFW_MOUSE_BUTTON_2
#define GLFW_MOUSE_BUTTON_MIDDLE GLFW_MOUSE_BUTTON_3
/*! @} */

/*! @defgroup joysticks Joysticks
 *  @ingroup input
 *  @{ */
#define GLFW_JOYSTICK_1          0
#define GLFW_JOYSTICK_2          1
#define GLFW_JOYSTICK_3          2
#define GLFW_JOYSTICK_4          3
#define GLFW_JOYSTICK_5          4
#define GLFW_JOYSTICK_6          5
#define GLFW_JOYSTICK_7          6
#define GLFW_JOYSTICK_8          7
#define GLFW_JOYSTICK_9          8
#define GLFW_JOYSTICK_10         9
#define GLFW_JOYSTICK_11         10
#define GLFW_JOYSTICK_12         11
#define GLFW_JOYSTICK_13         12
#define GLFW_JOYSTICK_14         13
#define GLFW_JOYSTICK_15         14
#define GLFW_JOYSTICK_16         15
#define GLFW_JOYSTICK_LAST       GLFW_JOYSTICK_16
/*! @} */


/*************************************************************************
 * Other definitions
 *************************************************************************/

/*! @brief A regular, overlapped window.
 *  @ingroup window
 */
#define GLFW_WINDOWED             0x00010001
/*! @brief A fullscreen window that may changed the monitor's resolution.
 *  @ingroup window
 */
#define GLFW_FULLSCREEN           0x00010002

/*! @defgroup paramhints Window parameters and hints
 *  @ingroup window
 *  @{ */

/*! @brief @c GL_TRUE if the window has focus, or @c GL_FALSE otherwise.
 */
#define GLFW_FOCUSED              0x00020001
/*! @brief @c GL_TRUE if the window is iconified, or @c GL_FALSE otherwise.
 */
#define GLFW_ICONIFIED            0x00020002
/*! @brief @c GL_TRUE if the window has been requested to close, or @c GL_FALSE
 *  otherwise.
 */
#define GLFW_CLOSE_REQUESTED      0x00020003
/*! @brief The OpenGL API version revision.
 */
#define GLFW_OPENGL_REVISION      0x00020004

/*! @brief The bit depth of the red component of the color buffer.
 */
#define GLFW_RED_BITS             0x00021000
/*! @brief The bit depth of the green component of the color buffer.
 */
#define GLFW_GREEN_BITS           0x00021001
/*! @brief The bit depth of the blue component of the color buffer.
 */
#define GLFW_BLUE_BITS            0x00021002
/*! @brief The bit depth of the alpha component of the color buffer.
 */
#define GLFW_ALPHA_BITS           0x00021003
/*! @brief The bit depth of the depth buffer of the default framebuffer.
 */
#define GLFW_DEPTH_BITS           0x00021004
/*! @brief The bit depth of the stencil buffer of the default framebuffer.
 */
#define GLFW_STENCIL_BITS         0x00021005
/*! @brief The monitor refresh rate.
 */
#define GLFW_REFRESH_RATE         0x00021006
/*! @brief The bit depth of the red component of the accumulation buffer.
 */
#define GLFW_ACCUM_RED_BITS       0x00021007
/*! @brief The bit depth of the red component of the accumulation buffer.
 */
#define GLFW_ACCUM_GREEN_BITS     0x00021008
/*! @brief The bit depth of the red component of the accumulation buffer.
 */
#define GLFW_ACCUM_BLUE_BITS      0x00021009
/*! @brief The bit depth of the red component of the accumulation buffer.
 */
#define GLFW_ACCUM_ALPHA_BITS     0x0002100A
/*! @brief The number of auxiliary buffers.
 */
#define GLFW_AUX_BUFFERS          0x0002100B
/*! @brief @c GL_TRUE for stereo rendering, or @c GL_FALSE otherwise.
 */
#define GLFW_STEREO               0x0002100C
/*! @brief The number of samples used for default framebuffer multisampling.
 */
#define GLFW_FSAA_SAMPLES         0x0002100E
/*! @brief @c GL_TRUE if the framebuffer should be sRGB capable, or @c GL_FALSE
 *  otherwise.
 */
#define GLFW_SRGB_CAPABLE         0x0002100F

/*! @brief The @link clients client API @endlink to create a context for.
 */
#define GLFW_CLIENT_API           0x00022000
#define GLFW_OPENGL_VERSION_MAJOR 0x00022001
#define GLFW_OPENGL_VERSION_MINOR 0x00022002
#define GLFW_OPENGL_FORWARD_COMPAT 0x00022003
#define GLFW_OPENGL_DEBUG_CONTEXT 0x00022004
#define GLFW_OPENGL_PROFILE       0x00022005
#define GLFW_OPENGL_ROBUSTNESS    0x00022006
/*! @brief @c GL_TRUE if the window is resizable, or @c GL_FALSE otherwise.
 */
#define GLFW_RESIZABLE            0x00022007
/*! @brief @c GL_TRUE if the window is visible, or @c GL_FALSE otherwise.
 */
#define GLFW_VISIBLE              0x00022008
/*! @brief The x-coordinate, in pixels, of the upper-left corner of the
 *  client area of the window.
 */
#define GLFW_POSITION_X           0x00022009
/*! @brief The y-coordinate, in pixels, of the upper-left corner of the
 *  client area of the window.
 */
#define GLFW_POSITION_Y           0x0002200A

/*! @} */

/*! @name Client APIs
 *  @{ */
/*! @brief The OpenGL API.
 *  @ingroup opengl
 */
#define GLFW_OPENGL_API           0x00000001
/*! @brief The OpenGL ES API.
 *  @ingroup opengl
 */
#define GLFW_OPENGL_ES_API        0x00000002
/*! @} */

/*! @name OpenGL robustness strategies
 *  @{ */
/*! @brief No robustness strategy is used.
 *
 *  This is the default.
 *  @ingroup opengl
 */
#define GLFW_OPENGL_NO_ROBUSTNESS         0x00000000
/*! @brief 
 *  @ingroup opengl
 */
#define GLFW_OPENGL_NO_RESET_NOTIFICATION 0x00000001
/*! @brief 
 *  @ingroup opengl
 */
#define GLFW_OPENGL_LOSE_CONTEXT_ON_RESET 0x00000002
/*! @} */

/*! @name OpenGL profiles
 *  @{ */
/*! @brief No OpenGL profile.
 *  @ingroup opengl
 */
#define GLFW_OPENGL_NO_PROFILE    0x00000000
/*! @brief The OpenGL core profile.
 *  @ingroup opengl
 */
#define GLFW_OPENGL_CORE_PROFILE  0x00000001
/*! @brief The OpenGL compatibility profile.
 *  @ingroup opengl
 */
#define GLFW_OPENGL_COMPAT_PROFILE 0x00000002
/*! @} */

/*! @name Input modes
 *  @{ */
/*! @brief The behaviour of the cursor.
 *  @ingroup input
 */
#define GLFW_CURSOR_MODE          0x00030001
/*! @brief Whether the @ref glfwGetKey function uses sticky state.
 *  @ingroup input
 */
#define GLFW_STICKY_KEYS          0x00030002
/*! @brief Whether the @ref glfwGetMouseButton function uses sticky state.
 *  @ingroup input
 */
#define GLFW_STICKY_MOUSE_BUTTONS 0x00030003
/*! @} */

/*! @name Cursor modes
 *  @{ */
/*! @brief The cursor is visible and behaves normally.
 *  @ingroup input
 */
#define GLFW_CURSOR_NORMAL       0x00040001
/*! @brief The cursor is hidden when over the client area of the window.
 *  @ingroup input
 */
#define GLFW_CURSOR_HIDDEN       0x00040002
/*! @brief The cursor is disabled and cursor movement is unbounded.
 *  @ingroup input
 */
#define GLFW_CURSOR_CAPTURED     0x00040003
/*! @} */

/*! @name Joystick parameters
 *  @{ */
/*! @brief @c GL_TRUE if the joystick is present, or @c GL_FALSE otherwise.
 *  @ingroup input
 */
#define GLFW_PRESENT              0x00050001
/*! @brief The number of axes on the specified joystick, or zero if the joystick
 *  is not present.
 *  @ingroup input
 */
#define GLFW_AXES                 0x00050002
/*! @brief The number of buttons on the specified joystick, or zero if the
 *  joystick is not present.
 *  @ingroup input
 */
#define GLFW_BUTTONS              0x00050003
/*! @} */

/*! @defgroup errors Error codes
 *  @ingroup error
 *  @{ */
/*! @brief No error has occurred.
 */
#define GLFW_NO_ERROR             0
/*! @brief GLFW has not been initialized.
 */
#define GLFW_NOT_INITIALIZED      0x00070001
/*! @brief No context is current for this thread.
 */
#define GLFW_NO_CURRENT_CONTEXT   0x00070002
/*! @brief One of the enum parameters for the function was given an invalid
 *  enum.
 */
#define GLFW_INVALID_ENUM         0x00070003
/*! @brief One of the parameters for the function was given an invalid value.
 */
#define GLFW_INVALID_VALUE        0x00070004
/*! @brief A memory allocation failed.
 */
#define GLFW_OUT_OF_MEMORY        0x00070005
/*! @brief GLFW could not find support for the requested client API on the
 *  system.
 */
#define GLFW_API_UNAVAILABLE      0x00070006
/*! @brief The requested OpenGL or GLES version is not available.
 */
#define GLFW_VERSION_UNAVAILABLE  0x00070007
/*! @brief A platform-specific error occurred that does not match any of the
 *  more specific categories.
 */
#define GLFW_PLATFORM_ERROR       0x00070008
/*! @brief The clipboard did not contain data in the requested format.
 */
#define GLFW_FORMAT_UNAVAILABLE   0x00070009
/*! @} */

/*! @brief The number of entries in the gamma ramp.
 *  @ingroup gamma
 */
#define GLFW_GAMMA_RAMP_SIZE      256


/*************************************************************************
 * Typedefs
 *************************************************************************/

/*! @brief OpenGL function pointer type.
 *  @ingroup opengl
 */
typedef void (*GLFWglproc)(void);

/*! @brief Window handle type.
 *  @ingroup window
 */
typedef void* GLFWwindow;

/*! @brief The function signature for error callbacks.
 *  @param[in] error An @link errors error code @endlink.
 *  @param[in] description A UTF-8 encoded string describing the error.
 *  @ingroup error
 */
typedef void (* GLFWerrorfun)(int,const char*);

/*! @brief The function signature for window position callbacks.
 *  @param[in] window The window that the user moved.
 *  @param[in] x The new x-coordinate, in pixels, of the upper-left corner of
 *  the client area of the window.
 *  @param[in] y The new y-coordinate, in pixels, of the upper-left corner of
 *  the client area of the window.
 *  @ingroup window
 */
typedef void (* GLFWwindowposfun)(GLFWwindow,int,int);

/*! @brief The function signature for window resize callbacks.
 *  @param[in] window The window that the user resized.
 *  @param[in] width The new width, in pixels, of the window.
 *  @param[in] height The new height, in pixels, of the window.
 *  @ingroup window
 */
typedef void (* GLFWwindowsizefun)(GLFWwindow,int,int);

/*! @brief The function signature for window close callbacks.
 *  @param[in] window The window that the user attempted to close.
 *  @return @c GL_TRUE to allow the window to be closed, or @c GL_FALSE to
 *  ignore the attempt.
 *  @ingroup window
 */
typedef int (* GLFWwindowclosefun)(GLFWwindow);

/*! @brief The function signature for window content refresh callbacks.
 *  @param[in] window The window whose content needs to be refreshed.
 *  @ingroup window
 */
typedef void (* GLFWwindowrefreshfun)(GLFWwindow);

/*! @brief The function signature for window focus/defocus callbacks.
 *  @param[in] window The window that was focused or defocused.
 *  @param[in] focused @c GL_TRUE if the window was focused, or @c GL_FALSE if
 *  it was defocused.
 *  @ingroup window
 */
typedef void (* GLFWwindowfocusfun)(GLFWwindow,int);

/*! @brief The function signature for window iconify/restore callbacks.
 *  @param[in] window The window that was iconified or restored.
 *  @param[in] iconified @c GL_TRUE if the window was iconified, or @c GL_FALSE
 *  if it was restored.
 *  @ingroup window
 */
typedef void (* GLFWwindowiconifyfun)(GLFWwindow,int);

/*! @brief The function signature for mouse button callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] button The @link buttons mouse button @endlink that was pressed
 *  or released.
 *  @param[in] action @ref GLFW_PRESS or @ref GLFW_RELEASE.
 *  @ingroup input
 */
typedef void (* GLFWmousebuttonfun)(GLFWwindow,int,int);

/*! @brief The function signature for cursor position callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] x The new x-coordinate of the cursor.
 *  @param[in] y The new y-coordinate of the cursor.
 *  @ingroup input
 */
typedef void (* GLFWcursorposfun)(GLFWwindow,int,int);

/*! @brief The function signature for cursor enter/exit callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] entered @c GL_TRUE if the cursor entered the window's client
 *  area, or @c GL_FALSE if it left it.
 *  @ingroup input
 */
typedef void (* GLFWcursorenterfun)(GLFWwindow,int);

/*! @brief The function signature for scroll callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] x The scroll offset along the x-axis.
 *  @param[in] y The scroll offset along the y-axis.
 *  @ingroup input
 */
typedef void (* GLFWscrollfun)(GLFWwindow,double,double);

/*! @brief The function signature for keyboard key callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] key The @link keys keyboard key @endlink that was pressed or
 *  released.
 *  @param[in] action @ref GLFW_PRESS or @ref GLFW_RELEASE.
 *  @ingroup input
 */
typedef void (* GLFWkeyfun)(GLFWwindow,int,int);

/*! @brief The function signature for Unicode character callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] character The Unicode code point of the character.
 *  @ingroup input
 */
typedef void (* GLFWcharfun)(GLFWwindow,int);

/* @brief Video mode type.
 * @ingroup monitor
 */
typedef struct
{
    int width;
    int height;
    int redBits;
    int blueBits;
    int greenBits;
} GLFWvidmode;

/*! @brief Gamma ramp.
 *  @ingroup gamma
 */
typedef struct
{
    unsigned short red[GLFW_GAMMA_RAMP_SIZE];
    unsigned short green[GLFW_GAMMA_RAMP_SIZE];
    unsigned short blue[GLFW_GAMMA_RAMP_SIZE];
} GLFWgammaramp;


/*************************************************************************
 * Prototypes
 *************************************************************************/

/*! @brief Initializes the GLFW library.
 *
 *  Before most GLFW functions can be used, GLFW must be initialized, and before
 *  a program terminates GLFW should be terminated in order to free allocated
 *  resources, memory, etc.
 *
 *  @return @c GL_TRUE if successful, or @c GL_FALSE if an error occurred.
 *  @ingroup init
 *
 *  @remarks Additional calls to this function after successful initialization
 *  but before termination will succeed but will do nothing.
 *
 *  @note This function may only be called from the main thread.
 *
 *  @note This function may take several seconds to complete on some systems,
 *  while on other systems it may take only a fraction of a second to complete.
 *
 *  @note On Mac OS X, this function will change the current directory of the
 *  application to the @c Contents/Resources subdirectory of the application's
 *  bundle, if present.
 *
 *  @sa glfwTerminate
 */
GLFWAPI int glfwInit(void);

/*! @brief Terminates the GLFW library.
 *  @ingroup init
 *
 *  @remarks This function may be called before @ref glfwInit.
 *
 *  @note This function may only be called from the main thread.
 *
 *  @note This function closes all GLFW windows.
 *
 *  @note This function should be called before the program exits.
 *
 *  @warning No window's context may be current on another thread when this
 *  function is called.
 *
 *  @sa glfwInit
 */
GLFWAPI void glfwTerminate(void);

/*! @brief Retrieves the version of the GLFW library.
 *  @param[out] major Where to store the major version number, or @c NULL.
 *  @param[out] minor Where to store the minor version number, or @c NULL.
 *  @param[out] rev Where to store the revision number, or @c NULL.
 *  @ingroup init
 *
 *  @remarks This function may be called before @ref glfwInit.
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @sa glfwGetVersionString
 */
GLFWAPI void glfwGetVersion(int* major, int* minor, int* rev);

/*! @brief Returns the version string of the GLFW library.
 *
 *  The version string contains information about what compile-time options were
 *  enabled when the library was built.
 *
 *  @return The GLFW version string.
 *  @ingroup init
 *
 *  @remarks This function may be called before @ref glfwInit.
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @sa glfwGetVersion
 */
GLFWAPI const char* glfwGetVersionString(void);

/*! @brief Retrieves the latest error.
 *  @return The latest @link errors error code @endlink.
 *  @ingroup error
 *
 *  @remarks This function may be called before @ref glfwInit.
 */
GLFWAPI int glfwGetError(void);

/*! @brief Retrieves a generic, human readable description of the specified error.
 *  @param[in] error The @link errors error code @endlink to be described.
 *  @return A UTF-8 encoded string describing the error.
 *  @ingroup error
 *
 *  @remarks This function may be called before @ref glfwInit.
 *
 *  @remarks This function may be called from secondary threads.
 */
GLFWAPI const char* glfwErrorString(int error);

/*! @brief Sets the error callback.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup error
 *
 *  @remarks This function may be called before @ref glfwInit.
 *
 *  @remarks The error callback is the preferred error retrieval mechanism, as
 *  it may be provided with a more specific error description than the generic
 *  one returned by @ref glfwErrorString.
 *
 *  @note Because the description string provided to the callback may have been
 *  generated specifically for that error, it is not guaranteed to be valid
 *  after the callback has returned.  If you wish to use it after that, you need
 *  to make your own copy of it before returning.
 */
GLFWAPI void glfwSetErrorCallback(GLFWerrorfun cbfun);

/*! @brief This function will be replaced when the @c multi-monitor branch is
 *  merged.
 *  @ingroup monitor
 */
GLFWAPI GLFWvidmode* glfwGetVideoModes(int* count);

/*! @brief This function will be replaced when the @c multi-monitor branch is
 *  merged.
 *  @ingroup monitor
 */
GLFWAPI void glfwGetDesktopMode(GLFWvidmode* mode);

/*! @brief Sets the system gamma ramp to one generated from the specified
 *  exponent.
 *  @param[in] gamma The desired exponent.
 *  @ingroup gamma
 */
GLFWAPI void glfwSetGamma(float gamma);

/*! @brief Retrieves the current system gamma ramp.
 *  @param[out] ramp Where to store the gamma ramp.
 *  @ingroup gamma
 */
GLFWAPI void glfwGetGammaRamp(GLFWgammaramp* ramp);

/*! @brief Sets the system gamma ramp to the one specified.
 *  @param[in] ramp The gamma ramp to use.
 *  @ingroup gamma
 */
GLFWAPI void glfwSetGammaRamp(const GLFWgammaramp* ramp);

/*! @brief Resets all window hints to their default values
 *
 *  The @ref GLFW_RED_BITS, @ref GLFW_GREEN_BITS, @ref GLFW_BLUE_BITS, @ref
 *  GLFW_ALPHA_BITS and @ref GLFW_STENCIL_BITS hints are set to 8.
 *
 *  The @ref GLFW_DEPTH_BITS hint is set to 24.
 *
 *  The @ref GLFW_VISIBLE and @ref GLFW_RESIZABLE hints are set to 1.
 *
 *  The @ref GLFW_CLIENT_API hint is set to @ref GLFW_OPENGL_API.
 *
 *  The @ref GLFW_OPENGL_VERSION_MAJOR and @ref GLFW_OPENGL_VERSION_MINOR hints
 *  are set to 1 and 0, respectively.
 *
 *  All other hints are set to 0.
 *
 *  @ingroup window
 *
 *  @note This function may only be called from the main thread.
 *
 *  @sa glfwWindowHint
 */
GLFWAPI void glfwDefaultWindowHints(void);

/*! @brief Sets the specified window hint to the desired value.
 *  @param[in] target The window hint to set.
 *  @param[in] target The new value of the window hint.
 *  @ingroup window
 *
 *  The @ref GLFW_RED_BITS, @ref GLFW_GREEN_BITS, @ref GLFW_BLUE_BITS, @ref
 *  GLFW_ALPHA_BITS, @ref GLFW_DEPTH_BITS and @ref GLFW_STENCIL_BITS hints
 *  specify the desired bit depths of the various components of the default
 *  framebuffer.
 *
 *  The @ref GLFW_REFRESH_RATE hint specifies the desired monitor refresh rate
 *  for fullscreen windows.
 *
 *  The @ref GLFW_ACCUM_RED_BITS, @ref GLFW_ACCUM_GREEN_BITS, @ref
 *  GLFW_ACCUM_BLUE_BITS and @ref GLFW_ACCUM_ALPHA_BITS hints specify the
 *  desired bit depths of the various components of the accumulation buffer.
 *
 *  The @ref GLFW_AUX_BUFFERS hint specifies the desired number of auxiliary
 *  buffers.
 *
 *  The @ref GLFW_STEREO hint specifies whether to use stereoscopic rendering.
 *
 *  The @ref GLFW_FSAA_SAMPLES hint specifies the desired number of samples to
 *  use for multisampling.
 *
 *  The @ref GLFW_SRGB_CAPABLE hint specifies whether the framebuffer should be
 *  sRGB capable.
 *
 *  The @ref GLFW_CLIENT_API hint specifies which client API to create the
 *  context for.  Possible values are @ref GLFW_OPENGL_API and @ref
 *  GLFW_OPENGL_ES_API.
 *
 *  The @ref GLFW_OPENGL_VERSION_MAJOR and @ref GLFW_OPENGL_VERSION_MINOR hints
 *  specify the OpenGL version that the created context must be compatible with.
 *
 *  These hints are @em not hard constraints, as they don't have to match
 *  exactly, but @ref glfwCreateWindow will still fail if the resulting OpenGL
 *  version is less than the one requested with hints.  It is therefore
 *  perfectly safe to use the default of version 1.0 for legacy code and you
 *  will still get backwards-compatible contexts of version 3.0 and above when
 *  available.
 *
 *  The @ref GLFW_OPENGL_FORWARD_COMPAT hint specifies whether the OpenGL
 *  context should be forward-compatible.
 *
 *  The @ref GLFW_OPENGL_DEBUG_CONTEXT hint specifies whether to create a debug
 *  OpenGL context.
 *
 *  The @ref GLFW_OPENGL_PROFILE hint specifies which OpenGL profile to create
 *  the context for.  Possible values are @ref GLFW_OPENGL_NO_PROFILE, @ref
 *  GLFW_OPENGL_CORE_PROFILE and @ref GLFW_OPENGL_COMPAT_PROFILE.
 *
 *  The @ref GLFW_OPENGL_ROBUSTNESS hint specifies the robustness strategy to be
 *  used by the OpenGL context.
 *
 *  The @ref GLFW_RESIZABLE hint specifies whether the window will be resizable
 *  by the user.  The window will still be resizable using the @ref
 *  glfwSetWindowSize function.  This hint is ignored for fullscreen windows.
 *
 *  The @ref GLFW_VISIBLE hint specifies whether the window will be initially
 *  visible.  This hint is ignored for fullscreen windows.
 *
 *  The @ref GLFW_POSITION_X and @ref GLFW_POSITION_Y hints specify the initial
 *  position of the window.  These hints are ignored for fullscreen windows.
 *
 *  Some window hints are hard constraints.  These must match the available
 *  capabilities @em exactly for window and context creation to succeed.  Hints
 *  that are not hard constraints are matched as closely as possible, but the
 *  resulting window and context may differ from what these hints requested.  To
 *  find out the actual properties of the created window and context, use the
 *  @ref glfwGetWindowParam function.
 *
 *  The following window hints are hard constraints:
 *  @arg @ref GLFW_STEREO
 *  @arg @ref GLFW_CLIENT_API
 *  @arg @ref GLFW_OPENGL_FORWARD_COMPAT
 *  @arg @ref GLFW_OPENGL_PROFILE
 *
 *  @note This function may only be called from the main thread.
 *
 *  @sa glfwDefaultWindowHints
 */
GLFWAPI void glfwWindowHint(int target, int hint);

/*! @brief Creates a window and its associated context.
 *
 *  @param[in] width The desired width, in pixels, of the window.  This must be
 *  greater than zero.
 *  @param[in] height The desired height, in pixels, of the window.  This must
 *  be greater than zero.
 *  @param[in] mode One of @ref GLFW_WINDOWED or @ref GLFW_FULLSCREEN.
 *  @param[in] title The initial, UTF-8 encoded window title.
 *  @param[in] share The window whose context to share resources with, or @c
 *  NULL to not share resources.
 *  @return The handle of the created window, or @c NULL if an error occurred.
 *  @ingroup window
 *
 *  @remarks Most of the options for how the window and its context should be
 *  created are specified via the @ref glfwWindowHint function.
 *
 *  @remarks This function does not change which context is current.  Before you
 *  can use the newly created context, you need to make it current using @ref
 *  glfwMakeContextCurrent.
 *
 *  @remarks For fullscreen windows the initial cursor mode is @ref
 *  GLFW_CURSOR_CAPTURED and the screensaver is prohibited from starting.  For
 *  regular windows the initial cursor mode is @ref GLFW_CURSOR_NORMAL and the
 *  screensaver is allowed to start.
 *
 *  @remarks In order to determine the actual properties of an opened window,
 *  use @ref glfwGetWindowParam and @ref glfwGetWindowSize.
 *
 *  @remarks On Microsoft Windows, if the executable has an icon resource named
 *  @c GLFW_ICON, it will be set as the icon for the window.  If no such icon is
 *  present, the @c IDI_WINLOGO icon will be used instead.
 *
 *  @remarks On Mac OS X the GLFW window has no icon, but programs using GLFW
 *  will use the application bundle's icon.  Also, the first time a window is
 *  opened the menu bar is populated with common commands like Hide, Quit and
 *  About.  The (minimal) about dialog uses information from the application's
 *  bundle.  For more information on bundles, see the Bundle Programming Guide
 *  provided by Apple.
 *
 *  @note This function may only be called from the main thread.
 *
 *  @sa glfwDestroyWindow
 */
GLFWAPI GLFWwindow glfwCreateWindow(int width, int height, int mode, const char* title, GLFWwindow share);

/*! @brief Destroys the specified window and its context.
 *  @param[in] window The window to destroy.
 *  @ingroup window
 *
 *  @note This function may only be called from the main thread.
 *
 *  @note If the window's context is current on the main thread, it is
 *  detached before being destroyed.
 *
 *  @warning The window's context must not be current on any other thread.
 *
 *  @sa glfwCreateWindow
 */
GLFWAPI void glfwDestroyWindow(GLFWwindow window);

/*! @brief Sets the title of the specified window.
 *  @param[in] window The window whose title to change.
 *  @param[in] title The UTF-8 encoded window title.
 *  @ingroup window
 *
 *  @note This function may only be called from the main thread.
 */
GLFWAPI void glfwSetWindowTitle(GLFWwindow window, const char* title);

/*! @brief Retrieves the size of the client area of the specified window.
 *  @param[in] window The window whose size to retrieve.
 *  @param[out] width The width of the client area.
 *  @param[out] height The height of the client area.
 *  @ingroup window
 *
 *  @sa glfwSetWindowSize
 */
GLFWAPI void glfwGetWindowSize(GLFWwindow window, int* width, int* height);

/*! @brief Sets the size of the client area of the specified window.
 *  @param[in] window The window to resize.
 *  @param[in] width The desired width of the specified window.
 *  @param[in] height The desired height of the specified window.
 *  @ingroup window
 *
 *  @note This function may only be called from the main thread.
 *
 *  @note The window manager may put limits on what window sizes are allowed.
 *
 *  @note For fullscreen windows, this function selects and switches to the
 *  resolution closest to the specified size, without destroying the window's
 *  context.
 *
 *  @sa glfwGetWindowSize
 */
GLFWAPI void glfwSetWindowSize(GLFWwindow window, int width, int height);

/*! @brief Iconifies the specified window.
 *  @param[in] window The window to iconify.
 *  @ingroup window
 *
 *  @remarks If the window is already iconified, this function does nothing.
 *
 *  @note This function may only be called from the main thread.
 *
 *  @sa glfwRestoreWindow
 */
GLFWAPI void glfwIconifyWindow(GLFWwindow window);

/*! @brief Restores the specified window.
 *  @param[in] window The window to restore.
 *  @ingroup window
 *
 *  @remarks If the window is already restored, this function does nothing.
 *
 *  @note This function may only be called from the main thread.
 *
 *  @sa glfwIconifyWindow
 */
GLFWAPI void glfwRestoreWindow(GLFWwindow window);

/*! @brief Makes the specified window visible.
 *  @param[in] window The window to make visible.
 *  @ingroup window
 *
 *  @remarks If the window is already visible or is in fullscreen mode, this
 *  function does nothing.
 *
 *  @note This function may only be called from the main thread.
 *
 *  @sa glfwHideWindow
 */
GLFWAPI void glfwShowWindow(GLFWwindow window);

/*! @brief Hides the specified window.
 *  @param[in] window The window to hide.
 *  @ingroup window
 *
 *  @remarks If the window is already hidden or is in fullscreen mode, this
 *  function does nothing.
 *
 *  @note This function may only be called from the main thread.
 *
 *  @sa glfwShowWindow
 */
GLFWAPI void glfwHideWindow(GLFWwindow window);

/*! @brief Returns a property of the specified window.
 *  @param[in] window The window to query.
 *  @param[in] param The property whose value to return.
 *  @ingroup window
 *
 *  The @ref GLFW_FOCUSED property indicates whether the window is focused.
 *
 *  The @ref GLFW_ICONIFIED property indicates whether the window is iconified.
 *
 *  The @ref GLFW_VISIBLE property indicates whether the window is visible.
 *
 *  The @ref GLFW_RESIZABLE property indicates whether the window is resizable
 *  by the user.
 *
 *  The @ref GLFW_CLOSE_REQUESTED property indicates whether the window has been
 *  requested by the user to close.
 *
 *  The @ref GLFW_REFRESH_RATE property will be replaced when the @c
 *  multi-monitor branch is merged.
 *
 *  The @ref GLFW_POSITION_X and @ref GLFW_POSITION_Y properties indicate the
 *  screen position, in pixels, of the upper-left corner of the window's client
 *  area.
 *
 *  The @ref GLFW_CLIENT_API property indicates the client API provided by the
 *  window's context.
 *
 *  The @ref GLFW_OPENGL_VERSION_MAJOR, @ref GLFW_OPENGL_VERSION_MINOR and @ref
 *  GLFW_OPENGL_REVISION properties indicate the API version of the window's
 *  context.
 *
 *  The @ref GLFW_OPENGL_FORWARD_COMPAT property indicates whether an OpenGL
 *  context is forward-compatible.
 *
 *  The @ref GLFW_OPENGL_DEBUG_CONTEXT property indicates whether the
 *  corresponding window hint was used when the window was created.
 *
 *  The @ref GLFW_OPENGL_PROFILE property indicates the profile used by the
 *  OpenGL context, or @ref GLFW_OPENGL_NO_PROFILE if the context is for another
 *  client API than OpenGL.
 *
 *  The @ref GLFW_OPENGL_ROBUSTNESS property indicates the robustness strategy
 *  used by the OpenGL context, or @ref GLFW_OPENGL_NO_ROBUSTNESS if robustness
 *  is not used.
 */
GLFWAPI int glfwGetWindowParam(GLFWwindow window, int param);

/*! @brief Sets the user pointer of the specified window.
 *  @param[in] window The window whose pointer to set.
 *  @param[in] pointer The new value.
 *  @ingroup window
 *
 *  @sa glfwGetWindowUserPointer
 */
GLFWAPI void glfwSetWindowUserPointer(GLFWwindow window, void* pointer);

/*! @brief Returns the user pointer of the specified window.
 *  @param[in] window The window whose pointer to return.
 *  @ingroup window
 *
 *  @sa glfwSetWindowUserPointer
 */
GLFWAPI void* glfwGetWindowUserPointer(GLFWwindow window);

/*! @brief Sets the position callback for the specified window.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup window
 */
GLFWAPI void glfwSetWindowPosCallback(GLFWwindow window, GLFWwindowposfun cbfun);

/*! @brief Sets the size callback for the specified window.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup window
 *
 *  This callback is called when the window is resized.
 */
GLFWAPI void glfwSetWindowSizeCallback(GLFWwindow window, GLFWwindowsizefun cbfun);

/*! @brief Sets the close callback for the specified window.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup window
 *
 *  This callback is called when the user attempts to close the window, i.e.
 *  clicks the window's close widget or, on Mac OS X, selects @b Quit from the
 *  application menu.  Calling @ref glfwDestroyWindow does not cause this
 *  callback to be called.
 *
 *  The return value of the close callback becomes the new value of the @ref
 *  GLFW_CLOSE_REQUESTED window parameter.
 */
GLFWAPI void glfwSetWindowCloseCallback(GLFWwindow window, GLFWwindowclosefun cbfun);

/*! @brief Sets the refresh callback for the specified window.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup window
 *
 *  This callback is called when the client area of the window needs to be
 *  redrawn, for example if the window has been exposed after having been
 *  covered by another window.
 *
 *  @note On compositing window systems such as Mac OS X, where the window
 *  contents are saved off-screen, this callback may never be called.
 */
GLFWAPI void glfwSetWindowRefreshCallback(GLFWwindow window, GLFWwindowrefreshfun cbfun);

/*! @brief Sets the focus callback for the specified window.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup window
 *
 *  This callback is called when the window gains or loses focus.
 */
GLFWAPI void glfwSetWindowFocusCallback(GLFWwindow window, GLFWwindowfocusfun cbfun);

/*! @brief Sets the iconify callback for the specified window.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup window
 *
 *  This callback is called when the window is iconified or restored.
 */
GLFWAPI void glfwSetWindowIconifyCallback(GLFWwindow window, GLFWwindowiconifyfun cbfun);

/*! @brief Processes all pending events.
 *  @ingroup window
 *
 *  @note This function may only be called from the main thread.
 *
 *  @sa glfwWaitEvents
 */
GLFWAPI void glfwPollEvents(void);

/*! @brief Waits until events are pending and processes them.
 *  @ingroup window
 *
 *  @note This function may only be called from the main thread.
 *
 *  @sa glfwPollEvents
 */
GLFWAPI void glfwWaitEvents(void);

/*! @brief Returns the value of an input option for the specified window.
 *  @param[in] window The window to query.
 *  @param[in] mode One of the following:
 *  @arg @ref GLFW_CURSOR_MODE Sets the cursor mode.
 *  @arg @ref GLFW_STICKY_KEYS Sets whether sticky keys are enabled.
 *  @arg @ref GLFW_STICKY_MOUSE_BUTTONS Sets whether sticky mouse buttons are enabled.
 *  @ingroup input
 *
 *  @sa glfwSetInputMode
 */
GLFWAPI int glfwGetInputMode(GLFWwindow window, int mode);

/*! @brief Sets an input option for the specified window.
 *  @param[in] mode One of the following:
 *  @arg @ref GLFW_CURSOR_MODE Sets the cursor mode.
 *  @arg @ref GLFW_STICKY_KEYS Sets whether sticky keys are enabled.
 *  @arg @ref GLFW_STICKY_MOUSE_BUTTONS Sets whether sticky mouse buttons are enabled.
 *  @ingroup input
 *
 *  @sa glfwGetInputMode
 */
GLFWAPI void glfwSetInputMode(GLFWwindow window, int mode, int value);

/*! @brief Returns the last reported state of a keyboard key for the specified
 *  window.
 *  @param[in] window The desired window.
 *  @param[in] key The desired @link keys keyboard key @endlink.
 *  @return @ref GLFW_PRESS or @ref GLFW_RELEASE.
 *  @ingroup input
 */
GLFWAPI int glfwGetKey(GLFWwindow window, int key);

/*! @brief Returns the last reported state of a mouse button for the specified
 *  window.
 *  @param[in] window The desired window.
 *  @param[in] key The desired @link buttons mouse buttons @endlink.
 *  @return @ref GLFW_PRESS or @ref GLFW_RELEASE.
 *  @ingroup input
 */
GLFWAPI int glfwGetMouseButton(GLFWwindow window, int button);

/*! @brief Retrieves the last reported cursor position, relative to the client
 *  area of the window.
 *  @param[in] window The desired window.
 *  @param[out] xpos The cursor x-coordinate, relative to the left edge of the
 *  client area, or @c NULL.
 *  @param[out] ypos The cursor y-coordinate, relative to the to top edge of the
 *  client area, or @c NULL.
 *  @ingroup input
 *
 *  @sa glfwSetCursorPos
 */
GLFWAPI void glfwGetCursorPos(GLFWwindow window, int* xpos, int* ypos);

/*! @brief Sets the position of the cursor, relative to the client area of the window.
 *  @param[in] window The desired window.
 *  @param[in] xpos The desired x-coordinate, relative to the left edge of the
 *  client area, or @c NULL.
 *  @param[in] ypos The desired y-coordinate, relative to the top edge of the
 *  client area, or @c NULL.
 *  @ingroup input
 *
 *  @note The specified window must be focused.
 *
 *  @sa glfwGetCursorPos
 */
GLFWAPI void glfwSetCursorPos(GLFWwindow window, int xpos, int ypos);

/*! @ingroup input
 */
GLFWAPI void glfwGetScrollOffset(GLFWwindow window, double* xoffset, double* yoffset);

/*! @brief Sets the key callback.
 *  @param[in] cbfun The new key callback, or @c NULL to remove the currently
 *  set callback.
 *  @ingroup input
 *
 *  @remarks The key callback deals with physical keys, with @link keys tokens
 *  @endlink named after their use on the standard US keyboard layout.  If you
 *  want to input text, use the Unicode character callback instead.
 */
GLFWAPI void glfwSetKeyCallback(GLFWwindow window, GLFWkeyfun cbfun);

/*! @brief Sets the Unicode character callback.
 *  @param[in] cbfun The new Unicode character callback, or @c NULL to remove
 *  the currently set callback.
 *  @ingroup input
 *
 *  @remarks The Unicode character callback is for text input.  If you want to
 *  know whether a specific key was pressed or released, use the key callback.
 */
GLFWAPI void glfwSetCharCallback(GLFWwindow window, GLFWcharfun cbfun);

/*! @brief Sets the mouse button callback.
 *  @param[in] cbfun The new mouse button callback, or @c NULL to remove the
 *  currently set callback.
 *  @ingroup input
 */
GLFWAPI void glfwSetMouseButtonCallback(GLFWwindow window, GLFWmousebuttonfun cbfun);

/*! @brief Sets the cursor position callback.
 *  @param[in] cbfun The new cursor position callback, or @c NULL to remove the
 *  currently set callback.
 *  @ingroup input
 *
 *  @remarks The position is relative to the upper-left corner of the client
 *  area of the window.
 */
GLFWAPI void glfwSetCursorPosCallback(GLFWwindow window, GLFWcursorposfun cbfun);

/*! @brief Sets the cursor enter/exit callback.
 *  @param[in] cbfun The new cursor enter/exit callback, or @c NULL to remove
 *  the currently set callback.
 *  @ingroup input
 */
GLFWAPI void glfwSetCursorEnterCallback(GLFWwindow window, GLFWcursorenterfun cbfun);

/*! @brief Sets the scroll callback.
 *  @param[in] cbfun The new scroll callback, or @c NULL to remove the currently
 *  set callback.
 *  @ingroup input
 *
 *  @note This receives all scrolling input, like that from a mouse wheel or
 *  a touchpad scrolling area.
 */
GLFWAPI void glfwSetScrollCallback(GLFWwindow window, GLFWscrollfun cbfun);

/*! @brief Returns a property of the specified joystick.
 *  @param[in] joy The joystick to query.
 *  @param[in] param The property whose value to return.
 *  @return The specified joystick's current value, or zero if the joystick is
 *  not present.
 *  @ingroup input
 */
GLFWAPI int glfwGetJoystickParam(int joy, int param);

/*! @brief Returns the values of axes of the specified joystick.
 *  @param[in] joy The joystick to query.
 *  @param[out] axes The array to hold the values.
 *  @param[in] numaxes The size of the provided array.
 *  @return The number of values written to @p axes.
 *  @ingroup input
 */
GLFWAPI int glfwGetJoystickAxes(int joy, float* axes, int numaxes);

/*! @brief Returns the values of buttons of the specified joystick.
 *  @param[in] joy The joystick to query.
 *  @param[out] buttons The array to hold the values.
 *  @param[in] numbuttons The size of the provided array.
 *  @return The number of values written to @p buttons.
 *  @ingroup input
 */
GLFWAPI int glfwGetJoystickButtons(int joy, unsigned char* buttons, int numbuttons);

/*! @brief Returns the name of the specified joystick.
 *  @param[in] joy The joystick to query.
 *  @return The UTF-8 encoded name of the joystick, or @c NULL if the joystick
 *  is not present.
 *  @ingroup input
 *
 *  @note The returned string is valid only until the next call to @ref
 *  glfwGetJoystickName.
 */
GLFWAPI const char* glfwGetJoystickName(int joy);

/*! @brief Sets the clipboard to the specified string.
 *  @param[in] window The window that will own the clipboard contents.
 *  @param[in] string A UTF-8 encoded string.
 *  @ingroup clipboard
 *
 *  @note This function may only be called from the main thread.
 *
 *  @sa glfwGetClipboardString
 */
GLFWAPI void glfwSetClipboardString(GLFWwindow window, const char* string);

/*! @brief Retrieves the contents of the clipboard as a string.
 *  @param[in] window The window that will request the clipboard contents.
 *  @return The contents of the clipboard as a UTF-8 encoded string, or @c NULL
 *  if that format was unavailable.
 *  @ingroup clipboard
 *
 *  @note This function may only be called from the main thread.
 *
 *  @note The returned string is valid only until the next call to @ref
 *  glfwGetClipboardString or @ref glfwSetClipboardString.
 *
 *  @sa glfwSetClipboardString
 */
GLFWAPI const char* glfwGetClipboardString(GLFWwindow window);

/*! @brief Retrieves the current value of the GLFW timer.
 *  @return The current value, in seconds.
 *  @ingroup time
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @remarks Unless the timer has been set using @ref glfwSetTime, the timer
 *  measures time elapsed since GLFW was initialized.
 *
 *  @note The resolution of the timer is system dependent.
 */
GLFWAPI double glfwGetTime(void);

/*! @brief Sets the GLFW timer.
 *  @param[in] time The new value, in seconds.
 *  @ingroup time
 *
 *  @note The resolution of the timer is system dependent.
 */
GLFWAPI void glfwSetTime(double time);

/*! @brief Makes the context of the specified window current for this thread.
 *  @param[in] window The window whose context to make current, or @c NULL to
 *  detach the current context.
 *  @ingroup opengl
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @note A context may only be current for a single thread at a time.
 *
 *  @sa glfwGetCurrentContext
 */
GLFWAPI void glfwMakeContextCurrent(GLFWwindow window);

/*! @brief Returns the window whose context is current on this thread.
 *  @return The window whose context is current, or @c NULL if no window's
 *  context is current.
 *  @ingroup opengl
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @sa glfwMakeContextCurrent
 */
GLFWAPI GLFWwindow glfwGetCurrentContext(void);

/*! @brief Swaps the front and back buffers of the specified window.
 *  @param[in] The window whose buffers to swap.
 *  @ingroup opengl
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @sa glfwSwapInterval
 */
GLFWAPI void glfwSwapBuffers(GLFWwindow window);

/*! @brief Sets the swap interval for the current context.
 *  @param[in] interval The minimum number of video frame periods to wait for
 *  until the buffers are swapped by @ref glfwSwapBuffers.
 *  @ingroup opengl
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @sa glfwSwapBuffers
 */
GLFWAPI void glfwSwapInterval(int interval);

/*! @brief Checks whether the specified extension is available.
 *  @param[in] extension The ASCII encoded name of the extension.
 *  @return @c GL_TRUE if the extension is available, or @c FALSE otherwise.
 *  @ingroup opengl
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @note This function checks not only the client API extension string, but
 *  also any platform-specific context creation API extension strings.
 */
GLFWAPI int glfwExtensionSupported(const char* extension);

/*! @brief Returns the address of the specified client API function for the
 *  current context.
 *  @param[in] procname The ASCII encoded name of the function.
 *  @return The address of the function, or @c NULL if the function is
 *  unavailable.
 *  @ingroup opengl
 *
 *  @remarks This function may be called from secondary threads.
 */
GLFWAPI GLFWglproc glfwGetProcAddress(const char* procname);


/*************************************************************************
 * Global definition cleanup
 *************************************************************************/

/* ------------------- BEGIN SYSTEM/COMPILER SPECIFIC -------------------- */

#ifdef GLFW_WINGDIAPI_DEFINED
 #undef WINGDIAPI
 #undef GLFW_WINGDIAPI_DEFINED
#endif

#ifdef GLFW_CALLBACK_DEFINED
 #undef CALLBACK
 #undef GLFW_CALLBACK_DEFINED
#endif

/* -------------------- END SYSTEM/COMPILER SPECIFIC --------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __glfw3_h__ */

