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

#ifndef _glfw3_h_
#define _glfw3_h_

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
/*! @defgroup context Context handling
 */
/*! @defgroup error Error handling
 */
/*! @defgroup gamma Gamma ramp support
 */
/*! @defgroup init Initialization and version information
 */
/*! @defgroup input Input handling
 */
/*! @defgroup monitor Monitor handling
 */
/*! @defgroup time Time input
 */
/*! @defgroup window Window handling
 *
 *  The primary purpose of GLFW is to provide a simple interface to window
 *  management and OpenGL and OpenGL ES context creation.  GLFW supports
 *  multiple windows, which can be either a normal desktop window or
 *  a fullscreen window.
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

/* Include the chosen client API headers.
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
  #elif defined(GLFW_INCLUDE_ES3)
    #include <GLES3/gl3.h>
  #else
    #include <GL/gl.h>
  #endif
  #if defined(GLFW_INCLUDE_GLU)
    #include <GL/glu.h>
  #endif
#endif


/*************************************************************************
 * GLFW API tokens
 *************************************************************************/

/*! @name GLFW version macros
 *  @{ */
/*! @brief The major version number of the GLFW library.
 *
 *  This is incremented when the API is changed in non-compatible ways.
 *  @ingroup init
 */
#define GLFW_VERSION_MAJOR          3
/*! @brief The minor version number of the GLFW library.
 *
 *  This is incremented when features are added to the API but it remains
 *  backward-compatible.
 *  @ingroup init
 */
#define GLFW_VERSION_MINOR          0
/*! @brief The revision number of the GLFW library.
 *
 *  This is incremented when a bug fix release is made that does not contain any
 *  API changes.
 *  @ingroup init
 */
#define GLFW_VERSION_REVISION       0
/*! @} */

/*! @name Key and button actions
 *  @{ */
/*! @brief The key or button was released.
 *  @ingroup input
 */
#define GLFW_RELEASE                0
/*! @brief The key or button was pressed.
 *  @ingroup input
 */
#define GLFW_PRESS                  1
/*! @brief The key was held down until it repeated.
 *  @ingroup input
 */
#define GLFW_REPEAT                 2
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
#define GLFW_KEY_SPACE              32
#define GLFW_KEY_APOSTROPHE         39  /* ' */
#define GLFW_KEY_COMMA              44  /* , */
#define GLFW_KEY_MINUS              45  /* - */
#define GLFW_KEY_PERIOD             46  /* . */
#define GLFW_KEY_SLASH              47  /* / */
#define GLFW_KEY_0                  48
#define GLFW_KEY_1                  49
#define GLFW_KEY_2                  50
#define GLFW_KEY_3                  51
#define GLFW_KEY_4                  52
#define GLFW_KEY_5                  53
#define GLFW_KEY_6                  54
#define GLFW_KEY_7                  55
#define GLFW_KEY_8                  56
#define GLFW_KEY_9                  57
#define GLFW_KEY_SEMICOLON          59  /* ; */
#define GLFW_KEY_EQUAL              61  /* = */
#define GLFW_KEY_A                  65
#define GLFW_KEY_B                  66
#define GLFW_KEY_C                  67
#define GLFW_KEY_D                  68
#define GLFW_KEY_E                  69
#define GLFW_KEY_F                  70
#define GLFW_KEY_G                  71
#define GLFW_KEY_H                  72
#define GLFW_KEY_I                  73
#define GLFW_KEY_J                  74
#define GLFW_KEY_K                  75
#define GLFW_KEY_L                  76
#define GLFW_KEY_M                  77
#define GLFW_KEY_N                  78
#define GLFW_KEY_O                  79
#define GLFW_KEY_P                  80
#define GLFW_KEY_Q                  81
#define GLFW_KEY_R                  82
#define GLFW_KEY_S                  83
#define GLFW_KEY_T                  84
#define GLFW_KEY_U                  85
#define GLFW_KEY_V                  86
#define GLFW_KEY_W                  87
#define GLFW_KEY_X                  88
#define GLFW_KEY_Y                  89
#define GLFW_KEY_Z                  90
#define GLFW_KEY_LEFT_BRACKET       91  /* [ */
#define GLFW_KEY_BACKSLASH          92  /* \ */
#define GLFW_KEY_RIGHT_BRACKET      93  /* ] */
#define GLFW_KEY_GRAVE_ACCENT       96  /* ` */
#define GLFW_KEY_WORLD_1            161 /* non-US #1 */
#define GLFW_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define GLFW_KEY_ESCAPE             256
#define GLFW_KEY_ENTER              257
#define GLFW_KEY_TAB                258
#define GLFW_KEY_BACKSPACE          259
#define GLFW_KEY_INSERT             260
#define GLFW_KEY_DELETE             261
#define GLFW_KEY_RIGHT              262
#define GLFW_KEY_LEFT               263
#define GLFW_KEY_DOWN               264
#define GLFW_KEY_UP                 265
#define GLFW_KEY_PAGE_UP            266
#define GLFW_KEY_PAGE_DOWN          267
#define GLFW_KEY_HOME               268
#define GLFW_KEY_END                269
#define GLFW_KEY_CAPS_LOCK          280
#define GLFW_KEY_SCROLL_LOCK        281
#define GLFW_KEY_NUM_LOCK           282
#define GLFW_KEY_PRINT_SCREEN       283
#define GLFW_KEY_PAUSE              284
#define GLFW_KEY_F1                 290
#define GLFW_KEY_F2                 291
#define GLFW_KEY_F3                 292
#define GLFW_KEY_F4                 293
#define GLFW_KEY_F5                 294
#define GLFW_KEY_F6                 295
#define GLFW_KEY_F7                 296
#define GLFW_KEY_F8                 297
#define GLFW_KEY_F9                 298
#define GLFW_KEY_F10                299
#define GLFW_KEY_F11                300
#define GLFW_KEY_F12                301
#define GLFW_KEY_F13                302
#define GLFW_KEY_F14                303
#define GLFW_KEY_F15                304
#define GLFW_KEY_F16                305
#define GLFW_KEY_F17                306
#define GLFW_KEY_F18                307
#define GLFW_KEY_F19                308
#define GLFW_KEY_F20                309
#define GLFW_KEY_F21                310
#define GLFW_KEY_F22                311
#define GLFW_KEY_F23                312
#define GLFW_KEY_F24                313
#define GLFW_KEY_F25                314
#define GLFW_KEY_KP_0               320
#define GLFW_KEY_KP_1               321
#define GLFW_KEY_KP_2               322
#define GLFW_KEY_KP_3               323
#define GLFW_KEY_KP_4               324
#define GLFW_KEY_KP_5               325
#define GLFW_KEY_KP_6               326
#define GLFW_KEY_KP_7               327
#define GLFW_KEY_KP_8               328
#define GLFW_KEY_KP_9               329
#define GLFW_KEY_KP_DECIMAL         330
#define GLFW_KEY_KP_DIVIDE          331
#define GLFW_KEY_KP_MULTIPLY        332
#define GLFW_KEY_KP_SUBTRACT        333
#define GLFW_KEY_KP_ADD             334
#define GLFW_KEY_KP_ENTER           335
#define GLFW_KEY_KP_EQUAL           336
#define GLFW_KEY_LEFT_SHIFT         340
#define GLFW_KEY_LEFT_CONTROL       341
#define GLFW_KEY_LEFT_ALT           342
#define GLFW_KEY_LEFT_SUPER         343
#define GLFW_KEY_RIGHT_SHIFT        344
#define GLFW_KEY_RIGHT_CONTROL      345
#define GLFW_KEY_RIGHT_ALT          346
#define GLFW_KEY_RIGHT_SUPER        347
#define GLFW_KEY_MENU               348
#define GLFW_KEY_LAST               GLFW_KEY_MENU

/* GLFW 2.x key name aliases (deprecated) */
#define GLFW_KEY_ESC                GLFW_KEY_ESCAPE
#define GLFW_KEY_DEL                GLFW_KEY_DELETE
#define GLFW_KEY_PAGEUP             GLFW_KEY_PAGE_UP
#define GLFW_KEY_PAGEDOWN           GLFW_KEY_PAGE_DOWN
#define GLFW_KEY_KP_NUM_LOCK        GLFW_KEY_NUM_LOCK
#define GLFW_KEY_LCTRL              GLFW_KEY_LEFT_CONTROL
#define GLFW_KEY_LSHIFT             GLFW_KEY_LEFT_SHIFT
#define GLFW_KEY_LALT               GLFW_KEY_LEFT_ALT
#define GLFW_KEY_LSUPER             GLFW_KEY_LEFT_SUPER
#define GLFW_KEY_RCTRL              GLFW_KEY_RIGHT_CONTROL
#define GLFW_KEY_RSHIFT             GLFW_KEY_RIGHT_SHIFT
#define GLFW_KEY_RALT               GLFW_KEY_RIGHT_ALT
#define GLFW_KEY_RSUPER             GLFW_KEY_RIGHT_SUPER

/*! @} */

/*! @defgroup buttons Mouse buttons
 *  @ingroup input
 *  @{ */
#define GLFW_MOUSE_BUTTON_1         0
#define GLFW_MOUSE_BUTTON_2         1
#define GLFW_MOUSE_BUTTON_3         2
#define GLFW_MOUSE_BUTTON_4         3
#define GLFW_MOUSE_BUTTON_5         4
#define GLFW_MOUSE_BUTTON_6         5
#define GLFW_MOUSE_BUTTON_7         6
#define GLFW_MOUSE_BUTTON_8         7
#define GLFW_MOUSE_BUTTON_LAST      GLFW_MOUSE_BUTTON_8
#define GLFW_MOUSE_BUTTON_LEFT      GLFW_MOUSE_BUTTON_1
#define GLFW_MOUSE_BUTTON_RIGHT     GLFW_MOUSE_BUTTON_2
#define GLFW_MOUSE_BUTTON_MIDDLE    GLFW_MOUSE_BUTTON_3
/*! @} */

/*! @defgroup joysticks Joysticks
 *  @ingroup input
 *  @{ */
#define GLFW_JOYSTICK_1             0
#define GLFW_JOYSTICK_2             1
#define GLFW_JOYSTICK_3             2
#define GLFW_JOYSTICK_4             3
#define GLFW_JOYSTICK_5             4
#define GLFW_JOYSTICK_6             5
#define GLFW_JOYSTICK_7             6
#define GLFW_JOYSTICK_8             7
#define GLFW_JOYSTICK_9             8
#define GLFW_JOYSTICK_10            9
#define GLFW_JOYSTICK_11            10
#define GLFW_JOYSTICK_12            11
#define GLFW_JOYSTICK_13            12
#define GLFW_JOYSTICK_14            13
#define GLFW_JOYSTICK_15            14
#define GLFW_JOYSTICK_16            15
#define GLFW_JOYSTICK_LAST          GLFW_JOYSTICK_16
/*! @} */

/*! @defgroup errors Error codes
 *  @ingroup error
 *  @{ */
/*! @brief No error has occurred.
 */
#define GLFW_NO_ERROR               0
/*! @brief GLFW has not been initialized.
 */
#define GLFW_NOT_INITIALIZED        0x00070001
/*! @brief No context is current for this thread.
 */
#define GLFW_NO_CURRENT_CONTEXT     0x00070002
/*! @brief One of the enum parameters for the function was given an invalid
 *  enum.
 */
#define GLFW_INVALID_ENUM           0x00070003
/*! @brief One of the parameters for the function was given an invalid value.
 */
#define GLFW_INVALID_VALUE          0x00070004
/*! @brief A memory allocation failed.
 */
#define GLFW_OUT_OF_MEMORY          0x00070005
/*! @brief GLFW could not find support for the requested client API on the
 *  system.
 */
#define GLFW_API_UNAVAILABLE        0x00070006
/*! @brief The requested client API version is not available.
 */
#define GLFW_VERSION_UNAVAILABLE    0x00070007
/*! @brief A platform-specific error occurred that does not match any of the
 *  more specific categories.
 */
#define GLFW_PLATFORM_ERROR         0x00070008
/*! @brief The clipboard did not contain data in the requested format.
 */
#define GLFW_FORMAT_UNAVAILABLE     0x00070009
/*! @} */

#define GLFW_FOCUSED                0x00020001
#define GLFW_ICONIFIED              0x00020002
#define GLFW_SHOULD_CLOSE           0x00020003
#define GLFW_RESIZABLE              0x00022007
#define GLFW_VISIBLE                0x00022008

#define GLFW_CONTEXT_REVISION       0x00020004
#define GLFW_RED_BITS               0x00021000
#define GLFW_GREEN_BITS             0x00021001
#define GLFW_BLUE_BITS              0x00021002
#define GLFW_ALPHA_BITS             0x00021003
#define GLFW_DEPTH_BITS             0x00021004
#define GLFW_STENCIL_BITS           0x00021005
#define GLFW_ACCUM_RED_BITS         0x00021006
#define GLFW_ACCUM_GREEN_BITS       0x00021007
#define GLFW_ACCUM_BLUE_BITS        0x00021008
#define GLFW_ACCUM_ALPHA_BITS       0x00021009
#define GLFW_AUX_BUFFERS            0x0002100A
#define GLFW_STEREO                 0x0002100B
#define GLFW_SAMPLES                0x0002100C
#define GLFW_SRGB_CAPABLE           0x0002100D

#define GLFW_CLIENT_API             0x00022000
#define GLFW_CONTEXT_VERSION_MAJOR  0x00022001
#define GLFW_CONTEXT_VERSION_MINOR  0x00022002
#define GLFW_CONTEXT_ROBUSTNESS     0x00022003
#define GLFW_OPENGL_FORWARD_COMPAT  0x00022004
#define GLFW_OPENGL_DEBUG_CONTEXT   0x00022005
#define GLFW_OPENGL_PROFILE         0x00022006

#define GLFW_OPENGL_API             0x00000001
#define GLFW_OPENGL_ES_API          0x00000002

#define GLFW_NO_ROBUSTNESS          0x00000000
#define GLFW_NO_RESET_NOTIFICATION  0x00000001
#define GLFW_LOSE_CONTEXT_ON_RESET  0x00000002

#define GLFW_OPENGL_NO_PROFILE      0x00000000
#define GLFW_OPENGL_CORE_PROFILE    0x00000001
#define GLFW_OPENGL_COMPAT_PROFILE  0x00000002

#define GLFW_CURSOR_MODE            0x00030001
#define GLFW_STICKY_KEYS            0x00030002
#define GLFW_STICKY_MOUSE_BUTTONS   0x00030003

#define GLFW_CURSOR_NORMAL          0x00040001
#define GLFW_CURSOR_HIDDEN          0x00040002
#define GLFW_CURSOR_CAPTURED        0x00040003

#define GLFW_PRESENT                0x00050001
#define GLFW_AXES                   0x00050002
#define GLFW_BUTTONS                0x00050003

#define GLFW_GAMMA_RAMP_SIZE        256

#define GLFW_CONNECTED              0x00061000
#define GLFW_DISCONNECTED           0x00061001


/*************************************************************************
 * GLFW API types
 *************************************************************************/

/*! @brief Client API function pointer type.
 *  @ingroup context
 */
typedef void (*GLFWglproc)(void);

/*! @brief Opaque monitor object.
 *  @ingroup monitor
 */
typedef struct GLFWmonitor GLFWmonitor;

/*! @brief Opaque window object.
 *  @ingroup window
 */
typedef struct GLFWwindow GLFWwindow;

/*! @brief The function signature for error callbacks.
 *  @param[in] error An @link errors error code @endlink.
 *  @param[in] description A UTF-8 encoded string describing the error.
 *  @ingroup error
 *
 *  @sa glfwSetErrorCallback
 */
typedef void (* GLFWerrorfun)(int,const char*);

/*! @brief The function signature for window position callbacks.
 *  @param[in] window The window that the user moved.
 *  @param[in] xpos The new x-coordinate, in pixels, of the upper-left corner of
 *  the client area of the window.
 *  @param[in] ypos The new y-coordinate, in pixels, of the upper-left corner of
 *  the client area of the window.
 *  @ingroup window
 *
 *  @sa glfwSetWindowPosCallback
 */
typedef void (* GLFWwindowposfun)(GLFWwindow*,int,int);

/*! @brief The function signature for window resize callbacks.
 *  @param[in] window The window that the user resized.
 *  @param[in] width The new width, in pixels, of the window.
 *  @param[in] height The new height, in pixels, of the window.
 *  @ingroup window
 *
 *  @sa glfwSetWindowSizeCallback
 */
typedef void (* GLFWwindowsizefun)(GLFWwindow*,int,int);

/*! @brief The function signature for window close callbacks.
 *  @param[in] window The window that the user attempted to close.
 *  @return @c GL_TRUE to allow the window to be closed, or @c GL_FALSE to
 *  ignore the attempt.
 *  @ingroup window
 *
 *  @sa glfwSetWindowCloseCallback
 */
typedef int (* GLFWwindowclosefun)(GLFWwindow*);

/*! @brief The function signature for window content refresh callbacks.
 *  @param[in] window The window whose content needs to be refreshed.
 *  @ingroup window
 *
 *  @sa glfwSetWindowRefreshCallback
 */
typedef void (* GLFWwindowrefreshfun)(GLFWwindow*);

/*! @brief The function signature for window focus/defocus callbacks.
 *  @param[in] window The window that was focused or defocused.
 *  @param[in] focused @c GL_TRUE if the window was focused, or @c GL_FALSE if
 *  it was defocused.
 *  @ingroup window
 *
 *  @sa glfwSetWindowFocusCallback
 */
typedef void (* GLFWwindowfocusfun)(GLFWwindow*,int);

/*! @brief The function signature for window iconify/restore callbacks.
 *  @param[in] window The window that was iconified or restored.
 *  @param[in] iconified @c GL_TRUE if the window was iconified, or @c GL_FALSE
 *  if it was restored.
 *  @ingroup window
 *
 *  @sa glfwSetWindowIconifyCallback
 */
typedef void (* GLFWwindowiconifyfun)(GLFWwindow*,int);

/*! @brief The function signature for mouse button callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] button The @link buttons mouse button @endlink that was pressed
 *  or released.
 *  @param[in] action One of @c GLFW_PRESS or @c GLFW_RELEASE.
 *  @ingroup input
 *
 *  @sa glfwSetMouseButtonCallback
 */
typedef void (* GLFWmousebuttonfun)(GLFWwindow*,int,int);

/*! @brief The function signature for cursor position callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] xpos The new x-coordinate of the cursor.
 *  @param[in] ypos The new y-coordinate of the cursor.
 *  @ingroup input
 *
 *  @sa glfwSetCursorPosCallback
 */
typedef void (* GLFWcursorposfun)(GLFWwindow*,int,int);

/*! @brief The function signature for cursor enter/exit callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] entered @c GL_TRUE if the cursor entered the window's client
 *  area, or @c GL_FALSE if it left it.
 *  @ingroup input
 *
 *  @sa glfwSetCursorEnterCallback
 */
typedef void (* GLFWcursorenterfun)(GLFWwindow*,int);

/*! @brief The function signature for scroll callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] xpos The scroll offset along the x-axis.
 *  @param[in] ypos The scroll offset along the y-axis.
 *  @ingroup input
 *
 *  @sa glfwSetScrollCallback
 */
typedef void (* GLFWscrollfun)(GLFWwindow*,double,double);

/*! @brief The function signature for keyboard key callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] key The @link keys keyboard key @endlink that was pressed or
 *  released.
 *  @param[in] action @ref GLFW_PRESS, @ref GLFW_RELEASE or @ref GLFW_REPEAT.
 *  @ingroup input
 *
 *  @sa glfwSetKeyCallback
 */
typedef void (* GLFWkeyfun)(GLFWwindow*,int,int);

/*! @brief The function signature for Unicode character callbacks.
 *  @param[in] window The window that received the event.
 *  @param[in] character The Unicode code point of the character.
 *  @ingroup input
 *
 *  @sa glfwSetCharCallback
 */
typedef void (* GLFWcharfun)(GLFWwindow*,int);

/*! @brief The function signature for monitor configuration callbacks.
 *  @param[in] monitor The monitor that was connected or disconnected.
 *  @param[in] event One of @c GLFW_CONNECTED or @c GLFW_DISCONNECTED.
 *  @ingroup monitor
 *
 *  @sa glfwSetMonitorCallback
 */
typedef void (* GLFWmonitorfun)(GLFWmonitor*,int);

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
 * GLFW API functions
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
 *  @note <b>Mac OS X:</b> This function will change the current directory of the
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

/*! @brief Returns a string describing the compile-time configuration.
 *
 *  The format of the string is as follows:
 *  @arg The name of the window system API
 *  @arg The name of the context creation API
 *  @arg Any additional options or APIs
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

/*! @brief Sets the error callback.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup error
 *
 *  @remarks This function may be called before @ref glfwInit.
 *
 *  @note The error callback is called by the thread where the error was
 *  generated.  If you are using GLFW from multiple threads, your error callback
 *  needs to be written accordingly.
 *
 *  @note Because the description string provided to the callback may have been
 *  generated specifically for that error, it is not guaranteed to be valid
 *  after the callback has returned.  If you wish to use it after that, you need
 *  to make your own copy of it before returning.
 */
GLFWAPI void glfwSetErrorCallback(GLFWerrorfun cbfun);

/*! @brief Returns the currently connected monitors.
 *  @param[out] count The size of the returned array.
 *  @return An array of monitor handles.
 *  @ingroup monitor
 *
 *  @sa glfwGetPrimaryMonitor
 */
GLFWAPI GLFWmonitor** glfwGetMonitors(int* count);

/*! @brief Returns the primary monitor.
 *  @return The primary monitor.
 *  @ingroup monitor
 *
 *  @sa glfwGetMonitors
 */
GLFWAPI GLFWmonitor* glfwGetPrimaryMonitor(void);

/*! @brief Returns the position of the monitor's viewport on the virtual screen.
 *  @param[in] monitor The monitor to query.
 *  @param[out] xpos The monitor x-coordinate.
 *  @param[out] ypos The monitor y-coordinate.
 *  @ingroup monitor
 */
GLFWAPI void glfwGetMonitorPos(GLFWmonitor* monitor, int* xpos, int* ypos);

/*! @brief Returns the physical size of the monitor.
 *  @param[in] monitor The monitor to query.
 *  @param[out] width The width, in mm, of the monitor's display
 *  @param[out] height The height, in mm, of the monitor's display.
 *  @ingroup monitor
 */
GLFWAPI void glfwGetMonitorPhysicalSize(GLFWmonitor* monitor, int* width, int* height);

/*! @brief Returns the name of the specified monitor.
 *  @param[in] monitor The monitor to query.
 *  @return The UTF-8 encoded name of the monitor.
 *  @ingroup monitor
 */
GLFWAPI const char* glfwGetMonitorName(GLFWmonitor* monitor);

/*! @brief Sets the monitor configuration callback.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup monitor
 */
GLFWAPI void glfwSetMonitorCallback(GLFWmonitorfun cbfun);

/*! @brief Returns the available video modes for the specified monitor.
 *  @param[in] monitor The monitor to query.
 *  @param[out] count The number of video modes in the returned array.
 *  @return An array of video modes.
 *  @ingroup monitor
 *
 *  @sa glfwGetVideoMode
 */
GLFWAPI const GLFWvidmode* glfwGetVideoModes(GLFWmonitor* monitor, int* count);

/*! @brief Returns the current mode of the specified monitor.
 *  @param[in] monitor The monitor to query.
 *  @return The current mode of the monitor.
 *  @ingroup monitor
 *
 *  @sa glfwGetVideoModes
 */
GLFWAPI GLFWvidmode glfwGetVideoMode(GLFWmonitor* monitor);

/*! @brief Generates a gamma ramp and sets it for the specified monitor.
 *  @param[in] monitor The monitor whose gamma ramp to set.
 *  @param[in] gamma The desired exponent.
 *  @ingroup gamma
 *
 *  @remarks This is a helper function on top of @ref glfwSetGammaRamp.
 */
GLFWAPI void glfwSetGamma(GLFWmonitor* monitor, float gamma);

/*! @brief Retrieves the current gamma ramp for the specified monitor.
 *  @param[in] monitor The monitor to query.
 *  @param[out] ramp Where to store the gamma ramp.
 *  @ingroup gamma
 */
GLFWAPI void glfwGetGammaRamp(GLFWmonitor* monitor, GLFWgammaramp* ramp);

/*! @brief Sets the gamma ramp for the specified monitor.
 *  @param[in] monitor The monitor whose gamma ramp to set.
 *  @param[in] ramp The gamma ramp to use.
 *  @ingroup gamma
 */
GLFWAPI void glfwSetGammaRamp(GLFWmonitor* monitor, const GLFWgammaramp* ramp);

/*! @brief Resets all window hints to their default values
 *
 *  The @c GLFW_RED_BITS, @c GLFW_GREEN_BITS, @c GLFW_BLUE_BITS, @c
 *  GLFW_ALPHA_BITS and @c GLFW_STENCIL_BITS hints are set to 8.
 *
 *  The @c GLFW_DEPTH_BITS hint is set to 24.
 *
 *  The @c GLFW_VISIBLE and @c GLFW_RESIZABLE hints are set to 1.
 *
 *  The @c GLFW_CLIENT_API hint is set to @c GLFW_OPENGL_API.
 *
 *  The @c GLFW_CONTEXT_VERSION_MAJOR and @c GLFW_CONTEXT_VERSION_MINOR
 *  hints are set to 1 and 0, respectively.
 *
 *  The @c GLFW_CONTEXT_ROBUSTNESS hint is set to @c GLFW_NO_ROBUSTNESS.
 *
 *  The @c GLFW_OPENGL_PROFILE hint is set to @c GLFW_OPENGL_NO_PROFILE.
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
 *  @param[in] hint The new value of the window hint.
 *  @ingroup window
 *
 *  This function sets hints for the next call to @ref glfwCreateWindow.  The
 *  hints, once set, retain their values until changed by a call to @ref
 *  glfwWindowHint or @ref glfwDefaultWindowHints, or until the library is
 *  terminated with @ref glfwTerminate.
 *
 *  @par Hard and soft constraints
 *
 *  Some window hints are hard constraints.  These must match the available
 *  capabilities @em exactly for window and context creation to succeed.  Hints
 *  that are not hard constraints are matched as closely as possible, but the
 *  resulting window and context may differ from what these hints requested.  To
 *  find out the actual properties of the created window and context, use the
 *  @ref glfwGetWindowParam function.
 *
 *  The following hints are hard constraints:
 *  @arg @c GLFW_STEREO
 *  @arg @c GLFW_CLIENT_API
 *
 *  The following additional hints are hard constraints if requesting an OpenGL
 *  context:
 *  @arg @c GLFW_OPENGL_FORWARD_COMPAT
 *  @arg @c GLFW_OPENGL_PROFILE
 *
 *  Hints that do not apply to a given type of window or context are ignored.
 *
 *  @par Framebuffer hints
 *
 *  The @c GLFW_RED_BITS, @c GLFW_GREEN_BITS, @c GLFW_BLUE_BITS, @c
 *  GLFW_ALPHA_BITS, @c GLFW_DEPTH_BITS and @c GLFW_STENCIL_BITS hints specify
 *  the desired bit depths of the various components of the default framebuffer.
 *
 *  The @c GLFW_ACCUM_RED_BITS, @c GLFW_ACCUM_GREEN_BITS, @c
 *  GLFW_ACCUM_BLUE_BITS and @c GLFW_ACCUM_ALPHA_BITS hints specify the desired
 *  bit depths of the various components of the accumulation buffer.
 *
 *  The @c GLFW_AUX_BUFFERS hint specifies the desired number of auxiliary
 *  buffers.
 *
 *  The @c GLFW_STEREO hint specifies whether to use stereoscopic rendering.
 *
 *  The @c GLFW_SAMPLES hint specifies the desired number of samples to use
 *  for multisampling.
 *
 *  The @c GLFW_SRGB_CAPABLE hint specifies whether the framebuffer should be
 *  sRGB capable.
 *
 *  @par Context hints
 *
 *  The @c GLFW_CLIENT_API hint specifies which client API to create the context
 *  for.  Possible values are @c GLFW_OPENGL_API and @c GLFW_OPENGL_ES_API.
 *
 *  The @c GLFW_CONTEXT_VERSION_MAJOR and @c GLFW_CONTEXT_VERSION_MINOR hints
 *  specify the client API version that the created context must be compatible
 *  with.
 *
 *  For OpenGL, these hints are @em not hard constraints, as they don't have to
 *  match exactly, but @ref glfwCreateWindow will still fail if the resulting
 *  OpenGL version is less than the one requested.  It is therefore perfectly
 *  safe to use the default of version 1.0 for legacy code and you will still
 *  get backwards-compatible contexts of version 3.0 and above when available.
 *
 *  For OpenGL ES, these hints are hard constraints, as there is no backward
 *  compatibility between OpenGL ES versions.
 *
 *  If an OpenGL context is requested, the @c GLFW_OPENGL_FORWARD_COMPAT hint
 *  specifies whether the OpenGL context should be forward-compatible, i.e. one
 *  where all functionality deprecated in the requested version of OpenGL is
 *  removed.  This may only be used if the requested OpenGL version is 3.0 or
 *  above.  If another client API is requested, this hint is ignored.
 *
 *  If an OpenGL context is requested, the @c GLFW_OPENGL_DEBUG_CONTEXT hint
 *  specifies whether to create a debug OpenGL context, which may have
 *  additional error and performance issue reporting functionality.  If another
 *  client API is requested, this hint is ignored.
 *
 *  If an OpenGL context is requested, the @c GLFW_OPENGL_PROFILE hint
 *  specifies which OpenGL profile to create the context for.  Possible values
 *  are one of @c GLFW_OPENGL_CORE_PROFILE or @c GLFW_OPENGL_COMPAT_PROFILE,
 *  or @c GLFW_OPENGL_NO_PROFILE to not request a specific profile.  If
 *  requesting an OpenGL version below 3.2, @c GLFW_OPENGL_NO_PROFILE must be
 *  used.  If another client API is requested, this hint is ignored.
 *
 *  The @c GLFW_CONTEXT_ROBUSTNESS hint specifies the robustness strategy to
 *  be used by the context.  This can be one of @c GLFW_NO_RESET_NOTIFICATION
 *  or @c GLFW_LOSE_CONTEXT_ON_RESET, or @c GLFW_NO_ROBUSTNESS to not
 *  request a robustness strategy.
 *
 *  @par Window hints
 *
 *  The @c GLFW_RESIZABLE hint specifies whether the window will be resizable
 *  by the user.  The window will still be resizable using the @ref
 *  glfwSetWindowSize function.  This hint is ignored for fullscreen windows.
 *
 *  The @c GLFW_VISIBLE hint specifies whether the window will be initially
 *  visible.  This hint is ignored for fullscreen windows.
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
 *  @param[in] title The initial, UTF-8 encoded window title.
 *  @param[in] monitor The monitor to use for fullscreen mode, or @c NULL to use
 *  windowed mode.
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
 *  @remarks To create the window at a specific position, make it initially
 *  invisible using the @c GLFW_VISIBLE window hint, set its position and then
 *  show it.
 *
 *  @remarks For fullscreen windows the initial cursor mode is @c
 *  GLFW_CURSOR_CAPTURED and the screensaver is prohibited from starting.  For
 *  regular windows the initial cursor mode is @c GLFW_CURSOR_NORMAL and the
 *  screensaver is allowed to start.
 *
 *  @remarks In order to determine the actual properties of an opened window,
 *  use @ref glfwGetWindowParam and @ref glfwGetWindowSize.
 *
 *  @remarks <b>Windows:</b> If the executable has an icon resource named @c
 *  GLFW_ICON, it will be set as the icon for the window.  If no such icon is
 *  present, the @c IDI_WINLOGO icon will be used instead.
 *
 *  @remarks <b>Mac OS X:</b> The GLFW window has no icon, as it is not
 *  a document window, but the dock icon will be the same as the application
 *  bundle's icon.  Also, the first time a window is opened the menu bar is
 *  populated with common commands like Hide, Quit and About.  The (minimal)
 *  about dialog uses information from the application's bundle.  For more
 *  information on bundles, see the Bundle Programming Guide provided by Apple.
 *
 *  @note This function may only be called from the main thread.
 *
 *  @sa glfwDestroyWindow
 */
GLFWAPI GLFWwindow* glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);

/*! @brief Destroys the specified window and its context.
 *  @param[in] window The window to destroy.
 *  @ingroup window
 *
 *  @note This function may only be called from the main thread.
 *
 *  @note This function may not be called from a callback.
 *
 *  @note If the window's context is current on the main thread, it is
 *  detached before being destroyed.
 *
 *  @note On calling this function, no further callbacks will be called for
 *  the specified window, even if their associated events occur during window
 *  destruction.
 *
 *  @warning The window's context must not be current on any other thread.
 *
 *  @sa glfwCreateWindow
 */
GLFWAPI void glfwDestroyWindow(GLFWwindow* window);

/*! @brief Sets the title of the specified window.
 *  @param[in] window The window whose title to change.
 *  @param[in] title The UTF-8 encoded window title.
 *  @ingroup window
 *
 *  @note This function may only be called from the main thread.
 */
GLFWAPI void glfwSetWindowTitle(GLFWwindow* window, const char* title);

/*! @brief Retrieves the position of the client area of the specified window.
 *  @param[in] window The window to query.
 *  @param[out] xpos The x-coordinate of the upper-left corner of the client area.
 *  @param[out] ypos The y-coordinate of the upper-left corner of the client area.
 *  @ingroup window
 *
 *  @remarks Either or both coordinate parameters may be @c NULL.
 *
 *  @sa glfwSetWindowPos
 */
GLFWAPI void glfwGetWindowPos(GLFWwindow* window, int* xpos, int* ypos);

/*! @brief Sets the position of the client area of the specified window.
 *  @param[in] window The window to query.
 *  @param[in] xpos The x-coordinate of the upper-left corner of the client area.
 *  @param[in] ypos The y-coordinate of the upper-left corner of the client area.
 *  @ingroup window
 *
 *  @remarks The position is the screen coordinate of the upper-left corner of
 *  the client area of the window.
 *
 *  @remarks  If you wish to set an initial window position you should create
 *  a hidden window (using @ref glfwWindowHint and @c GLFW_VISIBLE), set its
 *  position and then show it.
 *
 *  @note It is very rarely a good idea to move an already visible window, as it
 *  will confuse and annoy the user.
 *
 *  @note This function may only be called from the main thread.
 *
 *  @note The window manager may put limits on what positions are allowed.
 *
 *  @bug <b>X11:</b> Some window managers ignore the set position of hidden
 *  (i.e. unmapped) windows, instead placing them where it thinks is
 *  appropriate once they are shown.
 *
 *  @bug <b>Mac OS X:</b> The screen coordinate system is inverted.
 *
 *  @sa glfwGetWindowPos
 */
GLFWAPI void glfwSetWindowPos(GLFWwindow* window, int xpos, int ypos);

/*! @brief Retrieves the size of the client area of the specified window.
 *  @param[in] window The window whose size to retrieve.
 *  @param[out] width The width of the client area.
 *  @param[out] height The height of the client area.
 *  @ingroup window
 *
 *  @sa glfwSetWindowSize
 */
GLFWAPI void glfwGetWindowSize(GLFWwindow* window, int* width, int* height);

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
GLFWAPI void glfwSetWindowSize(GLFWwindow* window, int width, int height);

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
GLFWAPI void glfwIconifyWindow(GLFWwindow* window);

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
GLFWAPI void glfwRestoreWindow(GLFWwindow* window);

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
GLFWAPI void glfwShowWindow(GLFWwindow* window);

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
GLFWAPI void glfwHideWindow(GLFWwindow* window);

/*! @brief Returns the monitor that the window uses for fullscreen mode
 *  @param[in] window The window to query.
 *  @return The monitor, or @c NULL if the window is in windowed mode.
 *  @ingroup window
 */
GLFWAPI GLFWmonitor* glfwGetWindowMonitor(GLFWwindow* window);

/*! @brief Returns a property of the specified window.
 *  @param[in] window The window to query.
 *  @param[in] param The property whose value to return.
 *  @return The value of the property, or zero if an error occurred.
 *  @ingroup window
 *
 *  @par Window properties
 *
 *  The @c GLFW_FOCUSED property indicates whether the window is focused.
 *
 *  The @c GLFW_ICONIFIED property indicates whether the window is iconified.
 *
 *  The @c GLFW_VISIBLE property indicates whether the window is visible.
 *
 *  The @c GLFW_RESIZABLE property indicates whether the window is resizable
 *  by the user.
 *
 *  The @c GLFW_SHOULD_CLOSE property indicates whether the window has been
 *  requested by the user to close.
 *
 *  @par Context properties
 *
 *  The @c GLFW_CLIENT_API property indicates the client API provided by the
 *  window's context; either @c GLFW_OPENGL_API or @c GLFW_OPENGL_ES_API.
 *
 *  The @c GLFW_CONTEXT_VERSION_MAJOR, @c GLFW_CONTEXT_VERSION_MINOR and @c
 *  GLFW_CONTEXT_REVISION properties indicate the client API version of the
 *  window's context.
 *
 *  The @c GLFW_OPENGL_FORWARD_COMPAT property is @c GL_TRUE if the window's
 *  context is an OpenGL forward-compatible one, or @c GL_FALSE otherwise.
 *
 *  The @c GLFW_OPENGL_DEBUG_CONTEXT property is @c GL_TRUE if the window's
 *  context is an OpenGL debug context, or @c GL_FALSE otherwise.
 *
 *  The @c GLFW_OPENGL_PROFILE property indicates the OpenGL profile used by the
 *  context.  This is @c GLFW_OPENGL_CORE_PROFILE or @c
 *  GLFW_OPENGL_COMPAT_PROFILE if the context uses a known profile, or @c
 *  GLFW_OPENGL_NO_PROFILE if the OpenGL profile is unknown or the context is
 *  for another client API.
 *
 *  The @c GLFW_CONTEXT_ROBUSTNESS property indicates the robustness strategy
 *  used by the context.  This is @c GLFW_LOSE_CONTEXT_ON_RESET or @c
 *  GLFW_NO_RESET_NOTIFICATION if the window's context supports robustness, or
 *  @c GLFW_NO_ROBUSTNESS otherwise.
 */
GLFWAPI int glfwGetWindowParam(GLFWwindow* window, int param);

/*! @brief Sets the user pointer of the specified window.
 *  @param[in] window The window whose pointer to set.
 *  @param[in] pointer The new value.
 *  @ingroup window
 *
 *  @sa glfwGetWindowUserPointer
 */
GLFWAPI void glfwSetWindowUserPointer(GLFWwindow* window, void* pointer);

/*! @brief Returns the user pointer of the specified window.
 *  @param[in] window The window whose pointer to return.
 *  @ingroup window
 *
 *  @sa glfwSetWindowUserPointer
 */
GLFWAPI void* glfwGetWindowUserPointer(GLFWwindow* window);

/*! @brief Sets the position callback for the specified window.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup window
 */
GLFWAPI void glfwSetWindowPosCallback(GLFWwindow* window, GLFWwindowposfun cbfun);

/*! @brief Sets the size callback for the specified window.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup window
 *
 *  This callback is called when the window is resized.
 */
GLFWAPI void glfwSetWindowSizeCallback(GLFWwindow* window, GLFWwindowsizefun cbfun);

/*! @brief Sets the close callback for the specified window.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup window
 *
 *  This callback is called when the user attempts to close the window, i.e.
 *  clicks the window's close widget.  Calling @ref glfwDestroyWindow does not
 *  cause this callback to be called.
 *
 *  The return value of the close callback becomes the new value of the @c
 *  GLFW_SHOULD_CLOSE window parameter.
 *
 *  @remarks <b>Mac OS X:</b> Selecting Quit from the application menu will
 *  trigger the close callback for all windows.
 */
GLFWAPI void glfwSetWindowCloseCallback(GLFWwindow* window, GLFWwindowclosefun cbfun);

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
 *  @note On compositing window systems such as Aero, Compiz or Aqua, where the
 *  window contents are saved off-screen, this callback may be called only very
 *  infrequently or never at all.
 */
GLFWAPI void glfwSetWindowRefreshCallback(GLFWwindow* window, GLFWwindowrefreshfun cbfun);

/*! @brief Sets the focus callback for the specified window.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup window
 *
 *  This callback is called when the window gains or loses focus.
 */
GLFWAPI void glfwSetWindowFocusCallback(GLFWwindow* window, GLFWwindowfocusfun cbfun);

/*! @brief Sets the iconify callback for the specified window.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup window
 *
 *  This callback is called when the window is iconified or restored.
 */
GLFWAPI void glfwSetWindowIconifyCallback(GLFWwindow* window, GLFWwindowiconifyfun cbfun);

/*! @brief Processes all pending events.
 *  @ingroup window
 *
 *  @note This function may only be called from the main thread.
 *
 *  @note This function may not be called from a callback.
 *
 *  @sa glfwWaitEvents
 */
GLFWAPI void glfwPollEvents(void);

/*! @brief Waits until events are pending and processes them.
 *  @ingroup window
 *
 *  @note This function may only be called from the main thread.
 *
 *  @note This function may not be called from a callback.
 *
 *  @sa glfwPollEvents
 */
GLFWAPI void glfwWaitEvents(void);

/*! @brief Returns the value of an input option for the specified window.
 *  @param[in] window The window to query.
 *  @param[in] mode One of the following:
 *  @arg @c GLFW_CURSOR_MODE Sets the cursor mode.
 *  @arg @c GLFW_STICKY_KEYS Sets whether sticky keys are enabled.
 *  @arg @c GLFW_STICKY_MOUSE_BUTTONS Sets whether sticky mouse buttons are enabled.
 *  @ingroup input
 *
 *  @sa glfwSetInputMode
 */
GLFWAPI int glfwGetInputMode(GLFWwindow* window, int mode);

/*! @brief Sets an input option for the specified window.
 *  @param[in] window The window whose input mode to set.
 *  @param[in] mode One of @c GLFW_CURSOR_MODE, @c GLFW_STICKY_KEYS or @c
 *  GLFW_STICKY_MOUSE_BUTTONS.
 *  @param[in] value The new value of the specified input mode.
 *  @ingroup input
 *
 *  @sa glfwGetInputMode
 */
GLFWAPI void glfwSetInputMode(GLFWwindow* window, int mode, int value);

/*! @brief Returns the last reported state of a keyboard key for the specified
 *  window.
 *  @param[in] window The desired window.
 *  @param[in] key The desired @link keys keyboard key @endlink.
 *  @return One of @c GLFW_PRESS or @c GLFW_RELEASE.
 *  @ingroup input
 */
GLFWAPI int glfwGetKey(GLFWwindow* window, int key);

/*! @brief Returns the last reported state of a mouse button for the specified
 *  window.
 *  @param[in] window The desired window.
 *  @param[in] button The desired @link buttons mouse buttons @endlink.
 *  @return One of @c GLFW_PRESS or @c GLFW_RELEASE.
 *  @ingroup input
 */
GLFWAPI int glfwGetMouseButton(GLFWwindow* window, int button);

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
GLFWAPI void glfwGetCursorPos(GLFWwindow* window, int* xpos, int* ypos);

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
GLFWAPI void glfwSetCursorPos(GLFWwindow* window, int xpos, int ypos);

/*! @brief Sets the key callback.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new key callback, or @c NULL to remove the currently
 *  set callback.
 *  @ingroup input
 *
 *  @remarks The key callback deals with physical keys, with @link keys tokens
 *  @endlink named after their use on the standard US keyboard layout.  If you
 *  want to input text, use the Unicode character callback instead.
 */
GLFWAPI void glfwSetKeyCallback(GLFWwindow* window, GLFWkeyfun cbfun);

/*! @brief Sets the Unicode character callback.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup input
 *
 *  @remarks The Unicode character callback is for text input.  If you want to
 *  know whether a specific key was pressed or released, use the key callback.
 */
GLFWAPI void glfwSetCharCallback(GLFWwindow* window, GLFWcharfun cbfun);

/*! @brief Sets the mouse button callback.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup input
 */
GLFWAPI void glfwSetMouseButtonCallback(GLFWwindow* window, GLFWmousebuttonfun cbfun);

/*! @brief Sets the cursor position callback.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup input
 *
 *  @remarks The position is relative to the upper-left corner of the client
 *  area of the window.
 */
GLFWAPI void glfwSetCursorPosCallback(GLFWwindow* window, GLFWcursorposfun cbfun);

/*! @brief Sets the cursor enter/exit callback.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new callback, or @c NULL to remove the currently set
 *  callback.
 *  @ingroup input
 */
GLFWAPI void glfwSetCursorEnterCallback(GLFWwindow* window, GLFWcursorenterfun cbfun);

/*! @brief Sets the scroll callback.
 *  @param[in] window The window whose callback to set.
 *  @param[in] cbfun The new scroll callback, or @c NULL to remove the currently
 *  set callback.
 *  @ingroup input
 *
 *  @note This receives all scrolling input, like that from a mouse wheel or
 *  a touchpad scrolling area.
 */
GLFWAPI void glfwSetScrollCallback(GLFWwindow* window, GLFWscrollfun cbfun);

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
GLFWAPI void glfwSetClipboardString(GLFWwindow* window, const char* string);

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
GLFWAPI const char* glfwGetClipboardString(GLFWwindow* window);

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
 *  @ingroup context
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @note A context may only be current for a single thread at a time.
 *
 *  @sa glfwGetCurrentContext
 */
GLFWAPI void glfwMakeContextCurrent(GLFWwindow* window);

/*! @brief Returns the window whose context is current on this thread.
 *  @return The window whose context is current, or @c NULL if no window's
 *  context is current.
 *  @ingroup context
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @sa glfwMakeContextCurrent
 */
GLFWAPI GLFWwindow* glfwGetCurrentContext(void);

/*! @brief Swaps the front and back buffers of the specified window.
 *  @param[in] window The window whose buffers to swap.
 *  @ingroup context
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @sa glfwSwapInterval
 */
GLFWAPI void glfwSwapBuffers(GLFWwindow* window);

/*! @brief Sets the swap interval for the current context.
 *  @param[in] interval The minimum number of video frame periods to wait for
 *  until the buffers are swapped by @ref glfwSwapBuffers.
 *  @ingroup context
 *
 *  @remarks This function may be called from secondary threads.
 *
 *  @sa glfwSwapBuffers
 */
GLFWAPI void glfwSwapInterval(int interval);

/*! @brief Checks whether the specified extension is available.
 *  @param[in] extension The ASCII encoded name of the extension.
 *  @return @c GL_TRUE if the extension is available, or @c FALSE otherwise.
 *  @ingroup context
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
 *  @ingroup context
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

#endif /* _glfw3_h_ */

