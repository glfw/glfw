# GLFW 3.0

## Introduction

GLFW is a free, Open Source, portable library for OpenGL and OpenGL ES
application development.  It provides a simple, platform-independent API for
creating windows and contexts, reading input, handling events, etc.

Version 3.0 brings a new API with many new features such as multiple windows
and contexts, multi-monitor support, EGL and OpenGL ES support, clipboard text
support, an error description callback, gamma ramp control, layout-independent
keyboard input and UTF-8 for all strings.

Certain features like the threading and image loading functions from GLFW 2.x
have been [removed](http://wiki.glfw.org/wiki/Rationale_for_removing).


## Compiling GLFW

To compile GLFW and the accompanying example programs, you will need the
[CMake](http://www.cmake.org/) build system.

### CMake options

There are a number of CMake build options for GLFW, although not all are
available on all supported platforms.  Some of these are de facto standards
among CMake users and so have no `GLFW_` prefix.

#### Shared options

`BUILD_SHARED_LIBS` determines whether GLFW is built as a static
library or as a DLL / shared library / dynamic library.

`LIB_SUFFIX` affects where the GLFW shared /dynamic library is
installed.  If it is empty, it is installed to `$PREFIX/lib`.  If it is set to
`64`, it is installed to `$PREFIX/lib64`.

`GLFW_BUILD_EXAMPLES` determines whether the GLFW examples are built
along with the library.

`GLFW_BUILD_TESTS` determines whether the GLFW test programs are
built along with the library.

#### Mac OS X specific options

`GLFW_USE_CHDIR` determines whether `glfwInit` changes the current
directory of bundled applications to the `Contents/Resources` directory.

`GLFW_USE_MENUBAR` determines whether the first call to
`glfwCreateWindow` sets up a minimal menu bar.

`GLFW_BUILD_UNIVERSAL` determines whether to build Universal Binaries.

#### Windows specific options

`USE_MSVC_RUNTIME_LIBRARY_DLL` determines whether to use the DLL version of the
Visual C++ runtime library.

#### EGL specific options

`GLFW_USE_EGL` determines whether to use EGL instead of the platform-specific
context creation API.  Note that EGL is not yet provided on all supported
platforms.

`GLFW_CLIENT_LIBRARY` determines which client API library to use.  If set to
`opengl` the OpenGL library is used, if set to `glesv1` for the OpenGL ES 1.x
library is used, or if set to `glesv2` the OpenGL ES 2.0 library is used.  The
selected library and its header files must be present on the system for this to
work.


## Installing GLFW

A rudimentary installation target is provided for all supported platforms via
CMake.


## Using GLFW

There are two aspects to using GLFW:

 * Using the GLFW API
 * Compiling and linking programs using the GLFW library

The first point is covered in the WIP
[reference manual](http://www.glfw.org/TEMP/3.0/).


### Include the GLFW header file

In the files of your program where you use OpenGL or GLFW, you should include
the GLFW 3 header file, i.e.:

    #include <GL/glfw3.h>

This defines all the constants, types and function prototypes of the GLFW API.
It also includes the chosen client API header files (by default OpenGL), and
defines all the constants and types necessary for those headers to work on that
platform.

For example, under Windows you are normally required to include `windows.h`
before including `GL/gl.h`.  This would make your source file tied to Windows
and pollute your code's namespace with the whole Win32 API.

Instead, the GLFW header takes care of this for you, not by including
`windows.h`, but rather by itself duplicating only the necessary parts of it.
It does this only where needed, so if `windows.h` *is* included, the GLFW header
does not try to redefine those symbols.

In other words:

 * Do *not* include the OpenGL headers yourself, as GLFW does this for you
 * Do *not* include `windows.h` or other platform-specific headers unless you
   plan on using those APIs directly
 * If you *do* need to include such headers, do it *before* including
   the GLFW and it will detect this

If you are using an OpenGL extension loading library such as
[GLEW](http://glew.sourceforge.net/), the GLEW header should also be included
*before* the GLFW one.  The GLEW header defines macros that disable any OpenGL
header that the GLFW header includes and GLEW will work as expected.

#### GLFW header option macros

These macros may be defined before the inclusion of the GLFW header.

`GLFW_INCLUDE_GLCOREARB` makes the header include the modern `GL/glcorearb.h`
header (`OpenGL/gl3.h` on Mac OS X) instead of the regular OpenGL header.

`GLFW_INCLUDE_ES1` makes the header include the OpenGL ES 1.x `GLES/gl.h` header
instead of the regular OpenGL header.

`GLFW_INCLUDE_ES2` makes the header include the OpenGL ES 2.0 `GLES2/gl2.h`
header instead of the regular OpenGL header.

`GLFW_INCLUDE_ES3` makes the header include the OpenGL ES 3.0 `GLES3/gl3.h`
header instead of the regular OpenGL header.

`GLFW_INCLUDE_GLU` makes the header include the GLU header.  This only makes
sense if you are using OpenGL.

`GLFW_DLL` is necessary when using the GLFW DLL on Windows.


### Link with the right libraries


#### Using GLFW from CMake

The `GLFW_LIBRARIES` cache variable contains all link-time dependencies of GLFW
as it is currently configured, so to link against GLFW simply do:

    target_link_libraries(myapp glfw ${GLFW_LIBRARIES})

Note that this does not include GLU, as GLFW does not use it.  If your
application needs GLU, you can add it to the list of dependencies with the
`OPENGL_glu_LIBRARY` cache variable.


#### Windows static library

The static version of the GLFW library is named `glfw3`.  When using this
version, it is also necessary to link with some libraries that GLFW uses.

When linking a program under Windows that uses the static version of GLFW, you
must link with `opengl32`.  If you are using GLU, you must also link with
`glu32`.


#### Windows DLL

The link library for the GLFW DLL is named `glfw3dll`.  When compiling a program
that uses the DLL version of GLFW, you need to define the `GLFW_DLL` macro
*before* any inclusion of the GLFW header.  This can be done either with
a compiler switch or by defining it in your source code.

A program using the GLFW DLL does not need to link against any of its
dependencies, but you still have to link against `opengl32` if your program uses
OpenGL and `glu32` if it uses GLU.



#### Unix library

GLFW supports [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/),
and `glfw3.pc` file is generated when the library is built and installed along
with it.  You can use it without installation using the `PKG_CONFIG_PATH`
environment variable.  See the documentation for pkg-config for more details.

A typical compile and link command-line when using the static may look like this:

    cc `pkg-config --cflags glfw3` -o myprog myprog.c `pkg-config --static --libs glfw3`

If you are using the shared library, simply omit the `--static` flag.

If you are using GLU, you should also add `-lGLU` to your link flags.


#### Mac OS X static library

GLFW on Mac OS X uses the Cocoa, OpenGL and IOKit frameworks.

If you are using Xcode, you can simply add the GLFW library and these frameworks
as dependencies.

If you are building from the
command-line, it is recommended that you use pkg-config

GLFW supports [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/),
and `glfw3.pc` file is generated when the library is built and installed along
with it.  You can use it without installation using the `PKG_CONFIG_PATH`
environment variable.  See the documentation for pkg-config for more details.

You can find pkg-config in most package systems such as
[Fink](http://www.finkproject.org/) and [MacPorts](http://www.macports.org/), so
if you have one of them installed, simply install pkg-config.  Once you have
pkg-config available, the command-line for compiling and linking your
program is:

    cc `pkg-config --cflags glfw3` -o myprog myprog.c `pkg-config --static --libs glfw3`

If you do not wish to use pkg-config, you need to add the required frameworks
and libraries to your command-line using the `-l` and `-framework` switches,
i.e.:

    cc -o myprog myprog.c -lglfw -framework Cocoa -framework OpenGL -framework IOKit

Note that you do not add the `.framework` extension to a framework when adding
it from the command-line.

The OpenGL framework contains both the OpenGL and GLU APIs, so there is no need
to add additional libraries or frameworks when using GLU.  Also note that even
though your machine may have `libGL`-style OpenGL libraries, they are for use
with the X Window System and will *not* work with the Mac OS X native version of
GLFW.


## Changes for version 3.0

 * Added `GLFWmonitor` and updated monitor-related functions to take a monitor
   handle
 * Added `glfwGetMonitors` and `glfwGetPrimaryMonitor` for enumerating available
   monitors
 * Added `glfwGetMonitorPos`, `glfwGetMonitorPhysicalSize` and
   `glfwGetMonitorName` for retrieving monitor properties
 * Added `glfwSetMonitorCallback` and `GLFWmonitorfun` for notification of
   changes in the set of available monitors
 * Added `GLFWwindow` and updated window-related functions and callbacks to take
   a window handle
 * Added `glfwSetWindowShouldClose` and `glfwWindowShouldClose` for setting and
   retrieving the window close flag
 * Added `glfwGetWindowPos` for retrieving the position of a window
 * Added `glfwDefaultWindowHints` for resetting all window hints to their
   default values
 * Added `glfwMakeContextCurrent` for making the context of the specified window
   current
 * Added `glfwSetErrorCallback`, `GLFWerrorfun` and error type tokens for
   receiving error notifications
 * Added `glfwSetWindowUserPointer` and `glfwGetWindowUserPointer` for
   per-window user pointers
 * Added `glfwGetVersionString` for determining which code paths were enabled at
   compile time
 * Added `glfwGetWindowMonitor` for querying the monitor, if any, of the
   specified window
 * Added `glfwSetWindowPosCallback` and `GLFWwindowposfun` for receiving window
   position events
 * Added `glfwSetWindowFocusCallback` and `GLFWwindowfocusfun` for receiving
   window focus events
 * Added `glfwSetWindowIconifyCallback` and `GLFWwindowiconifyfun` for receiving
   window iconification events
 * Added `glfwGetClipboardString` and `glfwSetClipboardString` for interacting
   with the system clipboard
 * Added `glfwGetJoystickName` for retrieving the name of a joystick
 * Added `glfwGetCurrentContext` for retrieving the window whose OpenGL context
   is current
 * Added `GLFW_SRGB_CAPABLE` for requesting sRGB capable framebuffers
 * Added `GLFW_CLIENT_API` and its values `GLFW_OPENGL_API` and
   `GLFW_OPENGL_ES_API` for selecting client API
 * Added `GLFW_CONTEXT_ROBUSTNESS` and values `GLFW_NO_ROBUSTNESS`,
   `GLFW_NO_RESET_NOTIFICATION` and `GLFW_LOSE_CONTEXT_ON_RESET` for
   `GL_ARB_robustness` support
 * Added `GLFW_OPENGL_REVISION` to make up for removal of `glfwGetGLVersion`
 * Added `GLFW_INCLUDE_GLCOREARB` macro for including `GL/glcorearb.h` instead of
   `GL/gl.h`
 * Added `GLFW_INCLUDE_ES1` macro for telling the GLFW header to use `GLES/gl.h`
   instead of `GL/gl.h`
 * Added `GLFW_INCLUDE_ES2` macro for telling the GLFW header to use
   `GLES2/gl2.h` instead of `GL/gl.h`
 * Added `GLFW_VISIBLE` window hint and parameter for controlling and polling
   window visibility
 * Added `GLFW_REPEAT` key action for repeated keys
 * Added `windows` simple multi-window test program
 * Added `sharing` simple OpenGL object sharing test program
 * Added `modes` video mode enumeration and setting test program
 * Added `threads` simple multi-threaded rendering test program
 * Added `glfw3native.h` header and platform-specific functions for explicit
   access to native display, window and context handles
 * Added `glfwSetGamma`, `glfwSetGammaRamp` and `glfwGetGammaRamp` functions and
   `GLFWgammaramp` type for monitor gamma ramp control
 * Added window parameter to `glfwSwapBuffers`
 * Changed buffer bit depth parameters of `glfwOpenWindow` to window hints
 * Changed `glfwOpenWindow` and `glfwSetWindowTitle` to use UTF-8 encoded
   strings
 * Changed `glfwGetProcAddress` to return a (generic) function pointer
 * Changed `glfwGetVideoModes` to return a dynamic, unlimited number of video
   modes for the specified monitor
 * Renamed `glfw.h` to `glfw3.h` to avoid conflicts with 2.x series
 * Renamed `glfwOpenWindowHint` to `glfwWindowHint`
 * Renamed `GLFW_ACTIVE` to `GLFW_FOCUSED`
 * Renamed `GLFW_FSAA_SAMPLES` to `GLFW_SAMPLES`
 * Renamed `GLFW_WINDOW_NO_RESIZE` to `GLFW_RESIZABLE`
 * Renamed `GLFW_BUILD_DLL` to `_GLFW_BUILD_DLL`
 * Renamed `version` test to `glfwinfo`
 * Renamed `GLFW_NO_GLU` to `GLFW_INCLUDE_GLU` and made it disabled by default
 * Renamed `glfwGetJoystickPos` to `glfwGetJoystickAxes` to match
   `glfwGetJoystickButtons`
 * Renamed mouse position functions to cursor position equivalents
 * Replaced `glfwOpenWindow` and `glfwCloseWindow` with `glfwCreateWindow` and
   `glfwDestroyWindow`
 * Replaced `glfwGetDesktopMode` width `glfwGetVideoMode`
 * Replaced ad hoc build system with CMake
 * Replaced layout-dependent key codes with single, platform-independent set
   based on US layout
 * Replaced mouse wheel interface with two-dimensional, floating point scrolling
   interface
 * Replaced `glfwEnable` and `glfwDisable` with `glfwGetInputMode` and
   `glfwSetInputMode`
 * Replaced `joystick` test with graphical version
 * Replaced automatic closing of windows with the window close flag
 * Removed the `GLFW_KEY_REPEAT` input option
 * Removed event auto-polling and the `GLFW_AUTO_POLL_EVENTS` window enable
 * Removed the Win32 port .def files
 * Removed the entire threading API
 * Removed the entire image loading API
 * Removed deprecated Carbon port
 * Removed registering `glfwTerminate` with `atexit`
 * Removed `glfwSleep` function
 * Removed `glfwGetNumberOfProcessors` function
 * Removed `glfwGetGLVersion` function
 * Removed `GLFW_OPENED` window parameter
 * Removed `GLFW_WINDOW` and `GLFW_FULLSCREEN`
 * Removed nonsensical key actions for Unicode character input
 * Removed `GLFWCALL` and `GLFWAPIENTRY` macros for stdcall calling convention
 * Removed `GLFW_ACCELERATED` window parameter
 * Removed default framebuffer attributes from `glfwGetWindowParam`
 * Bugfix: The default OpenGL version in the `glfwinfo` test was set to 1.1
 * Bugfix: The OpenGL profile and forward-compatibility window parameters were
           not saved after context creation
 * Bugfix: The FSAA test did not check for the availability of
           `GL_ARB_multisample`
 * Bugfix: Cursor centering upon leaving captured cursor mode was reported
           before the mode was changed to non-captured
 * [Cocoa] Added support for OpenGL 3.2 core profile in 10.7 Lion and above
 * [Cocoa] Added support for joysticks
 * [Cocoa] Postponed menu creation to first window creation
 * [Cocoa] Replaced `NSDate` time source with `mach_absolute_time`
 * [Cocoa] Replaced all deprecated CoreGraphics calls with non-deprecated
           counterparts
 * [Cocoa] Bugfix: The `NSOpenGLPFAFullScreen` pixel format attribute caused
                   creation to fail on some machines
 * [Cocoa] Bugfix: `glfwOpenWindow` did not properly enforce the
                   forward-compatible and context profile hints
 * [Cocoa] Bugfix: The loop condition for saving video modes used the wrong
                   index variable
 * [Cocoa] Bugfix: The OpenGL framework was not retrieved, making
                   `glfwGetProcAddress` crash
 * [Cocoa] Bugfix: `glfwInit` changed the current directory for unbundled
                   executables
 * [Cocoa] Bugfix: The `GLFW_WINDOW_NO_RESIZE` window parameter was always zero
 * [Cocoa] Bugfix: The cursor position incorrectly rounded during conversion
 * [Cocoa] Bugfix: Cursor positioning led to nonsensical results for fullscreen
                   windows
 * [Cocoa] Bugfix: The GLFW window was flagged as restorable
 * [X11] Added support for the `GLX_EXT_swap_control` and
         `GLX_MESA_swap_control` extensions as alternatives to
         `GLX_SGI_swap_control`
 * [X11] Added the POSIX `CLOCK_MONOTONIC` time source as the preferred method
 * [X11] Added dependency on libm, where present
 * [X11] Added support for the `_NET_WM_NAME` and `_NET_WM_ICON_NAME` EWMH
         window properties
 * [X11] Made client-side RandR and Xf86VidMode extensions required
 * [X11] Bugfix: Some window properties required by the ICCCM were not set
 * [X11] Bugfix: Calling `glXCreateContextAttribsARB` with an unavailable OpenGL
                 version caused the application to terminate with a `BadMatch`
                 Xlib error
 * [X11] Bugfix: A synchronization point necessary for jitter-free locked cursor
                 mode was incorrectly removed
 * [X11] Bugfix: The window size hints were not updated when calling
                 `glfwSetWindowSize` on a non-resizable window
 * [Win32] Changed port to use Unicode mode only
 * [Win32] Removed explicit support for versions of Windows older than Windows
           XP
 * [Win32] Bugfix: Window activation and iconification did not work as expected
 * [Win32] Bugfix: Software rasterizer pixel formats were not discarded by the
                   `WGL_ARB_pixel_format` code path
 * [Win32] Bugfix: The array for WGL context attributes was too small and could
                   overflow
 * [Win32] Bugfix: Alt+F4 hot key was not translated into `WM_CLOSE`
 * [Win32] Bugfix: The `GLFW_WINDOW_NO_RESIZE` window parameter was always zero
 * [Win32] Bugfix: A test was missing for whether all available pixel formats
                   had been disqualified


## Contact

The official website for GLFW is [glfw.org](http://www.glfw.org/).  There you
can find the latest version of GLFW, as well as news, documentation and other
information about the project.

If you have questions related to the use of GLFW, we have a
[support forum](https://sourceforge.net/p/glfw/discussion/247562/), and the IRC
channel `#glfw` on [Freenode](http://freenode.net/).

If you have a bug to report, a patch to submit or a feature you'd like to
request, please file it in one of the
[issue trackers](https://sourceforge.net/p/glfw/_list/tickets) on SF.net.

Finally, if you're interested in helping out with the development of GLFW or
porting it to your favorite platform, we have a
[developer's mailing list](https://lists.stacken.kth.se/mailman/listinfo/glfw-dev),
or you could join us on `#glfw`.


## Acknowledgements

GLFW exists because people around the world donated their time and lent their
skills.

 * Bobyshev Alexander
 * artblanc
 * Matt Arsenault
 * Keith Bauer
 * John Bartholomew
 * blanco
 * Lambert Clara
 * Noel Cower
 * Jarrod Davis
 * Olivier Delannoy
 * Paul R. Deppe
 * Jonathan Dummer
 * Ralph Eastwood
 * Gerald Franz
 * GeO4d
 * Marcus Geelnard
 * Stefan Gustavson
 * Sylvain Hellegouarch
 * heromyth
 * Toni Jovanoski
 * Osman Keskin
 * Cameron King
 * Peter Knut
 * Robin Leffmann
 * Glenn Lewis
 * Shane Liesegang
 * Дмитри Малышев
 * Martins Mozeiko
 * Tristam MacDonald
 * Hans Mackowiak
 * David Medlock
 * Jonathan Mercier
 * Marcel Metz
 * Kenneth Miller
 * Jeff Molofee
 * Jon Morton
 * Julian Møller
 * Ozzy at Orkysquad
 * Peoro
 * Braden Pellett
 * Arturo J. Pérez
 * Jorge Rodriguez
 * Ed Ropple
 * Riku Salminen
 * Sebastian Schuberth
 * Matt Sealey
 * SephiRok
 * Steve Sexton
 * Dmitri Shuralyov
 * Daniel Skorupski
 * Bradley Smith
 * Julian Squires
 * Johannes Stein
 * TTK-Bandit
 * Sergey Tikhomirov
 * Samuli Tuomola
 * Jari Vetoniemi
 * Simon Voordouw
 * Torsten Walluhn
 * Frank Wille
 * yuriks
 * Santi Zupancic
 * Lasse Öörni
 * All the unmentioned and anonymous contributors in the GLFW community, for bug
   reports, patches, feedback, testing and encouragement

