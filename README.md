# GLFW

[![Build status](https://github.com/glfw/glfw/actions/workflows/build.yml/badge.svg)](https://github.com/glfw/glfw/actions)
[![Build status](https://ci.appveyor.com/api/projects/status/0kf0ct9831i5l6sp/branch/master?svg=true)](https://ci.appveyor.com/project/elmindreda/glfw)
[![Coverity Scan](https://scan.coverity.com/projects/4884/badge.svg)](https://scan.coverity.com/projects/glfw-glfw)

## Introduction

GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan
application development.  It provides a simple, platform-independent API for
creating windows, contexts and surfaces, reading input, handling events, etc.

GLFW natively supports Windows, macOS and Linux and other Unix-like systems.  On
Linux both X11 and Wayland are supported.

GLFW is licensed under the [zlib/libpng
license](https://www.glfw.org/license.html).

You can [download](https://www.glfw.org/download.html) the latest stable release
as source or Windows binaries, or fetch the `latest` branch from GitHub.  Each
release starting with 3.0 also has a corresponding [annotated
tag](https://github.com/glfw/glfw/releases) with source and binary archives.

The [documentation](https://www.glfw.org/docs/latest/) is available online and is
included in all source and binary archives.  See the [release
notes](https://www.glfw.org/docs/latest/news.html) for new features, caveats and
deprecations in the latest release.  For more details see the [version
history](https://www.glfw.org/changelog.html).

The `master` branch is the stable integration branch and _should_ always compile
and run on all supported platforms, although details of newly added features may
change until they have been included in a release.  New features and many bug
fixes live in [other branches](https://github.com/glfw/glfw/branches/all) until
they are stable enough to merge.

If you are new to GLFW, you may find the
[tutorial](https://www.glfw.org/docs/latest/quick.html) for GLFW 3 useful.  If
you have used GLFW 2 in the past, there is a [transition
guide](https://www.glfw.org/docs/latest/moving.html) for moving to the GLFW
3 API.

GLFW exists because of the contributions of [many people](CONTRIBUTORS.md)
around the world, whether by reporting bugs, providing community support, adding
features, reviewing or testing code, debugging, proofreading docs, suggesting
features or fixing bugs.


## Compiling GLFW

GLFW itself requires only the headers and libraries for your OS and window
system.  It does not need the headers for any context creation API (WGL, GLX,
EGL, NSGL, OSMesa) or rendering API (OpenGL, OpenGL ES, Vulkan) to enable
support for them.

GLFW supports compilation on Windows with Visual C++ 2010 and later, MinGW and
MinGW-w64, on macOS with Clang and on Linux and other Unix-like systems with GCC
and Clang.  It will likely compile in other environments as well, but this is
not regularly tested.

There are [pre-compiled Windows binaries](https://www.glfw.org/download.html)
available for all supported compilers.

See the [compilation guide](https://www.glfw.org/docs/latest/compile.html) for
more information about how to compile GLFW yourself.


## Using GLFW

See the [documentation](https://www.glfw.org/docs/latest/) for tutorials, guides
and the API reference.


## Contributing to GLFW

See the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
more information.


## System requirements

GLFW supports Windows XP and later and macOS 10.8 and later.  Linux and other
Unix-like systems running the X Window System are supported even without
a desktop environment or modern extensions, although some features require
a running window or clipboard manager.  The OSMesa backend requires Mesa 6.3.

See the [compatibility guide](https://www.glfw.org/docs/latest/compat.html)
in the documentation for more information.


## Dependencies

GLFW itself depends only on the headers and libraries for your window system.

The (experimental) Wayland backend also depends on the `extra-cmake-modules`
package, which is used to generate Wayland protocol headers.

The examples and test programs depend on a number of tiny libraries.  These are
located in the `deps/` directory.

 - [getopt\_port](https://github.com/kimgr/getopt_port/) for examples
   with command-line options
 - [TinyCThread](https://github.com/tinycthread/tinycthread) for threaded
   examples
 - [glad2](https://github.com/Dav1dde/glad) for loading OpenGL and Vulkan
   functions
 - [linmath.h](https://github.com/datenwolf/linmath.h) for linear algebra in
   examples
 - [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) for test and example UI
 - [stb\_image\_write](https://github.com/nothings/stb) for writing images to disk

The documentation is generated with [Doxygen](https://doxygen.org/) if CMake can
find that tool.


## Reporting bugs

Bugs are reported to our [issue tracker](https://github.com/glfw/glfw/issues).
Please check the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
information on what to include when reporting a bug.


## Changelog

 - Added `GLFW_NATIVE_INCLUDE_NONE` for disabling inclusion of native headers (#1348)
 - Bugfix: `glfwMakeContextCurrent` would access TLS slot before initialization
 - Bugfix: `glfwSetGammaRamp` could emit `GLFW_INVALID_VALUE` before initialization
 - Bugfix: `glfwGetJoystickUserPointer` returned `NULL` during disconnection (#2092)
 - [Win32] Bugfix: `Alt+PrtSc` would emit `GLFW_KEY_UNKNOWN` and a different
   scancode than `PrtSc` (#1993)
 - [Win32] Bugfix: `GLFW_KEY_PAUSE` scancode from `glfwGetKeyScancode` did not
   match event scancode (#1993)
 - [Win32] Bugfix: Instance-local operations used executable instance (#469,#1296,#1395)
 - [Win32] Bugfix: The OSMesa library was not unloaded on termination
 - [Win32] Bugfix: Right shift emitted `GLFW_KEY_UNKNOWN` when using a CJK IME (#2050)
 - [Cocoa] Disabled macOS fullscreen when `GLFW_RESIZABLE` is false
 - [Cocoa] Bugfix: A connected Apple AirPlay would emit a useless error (#1791)
 - [Cocoa] Bugfix: The EGL and OSMesa libraries were not unloaded on termination
 - [Cocoa] Bugfix: `GLFW_MAXIMIZED` was always true when `GLFW_RESIZABLE` was false
 - [Cocoa] Bugfix: Changing `GLFW_DECORATED` in macOS fullscreen would abort
   application (#1886)
 - [Cocoa] Bugfix: Setting a monitor from macOS fullscreen would abort
   application (#2110)
 - [Cocoa] Bugfix: The Vulkan loader was not loaded from the `Frameworks` bundle
   subdirectory (#2113,#2120)
 - [X11] Bugfix: The OSMesa libray was not unloaded on termination
 - [X11] Bugfix: A malformed response during selection transfer could cause a segfault
 - [X11] Bugfix: Some calls would reset Xlib to the default error handler (#2108)
 - [Wayland] Added support for file path drop events (#2040)
 - [Wayland] Added support for more human-readable monitor names where available
 - [Wayland] Removed support for the deprecated wl\_shell protocol
 - [Wayland] Bugfix: `glfwSetClipboardString` would fail if set to result of
   `glfwGetClipboardString`
 - [Wayland] Bugfix: Data source creation error would cause double free at termination
 - [Wayland] Bugfix: Partial writes of clipboard string would cause beginning to repeat
 - [Wayland] Bugfix: Some errors would cause clipboard string transfer to hang
 - [Wayland] Bugfix: Drag and drop data was misinterpreted as clipboard string
 - [Wayland] Bugfix: MIME type matching was not performed for clipboard string
 - [Wayland] Bugfix: The OSMesa library was not unloaded on termination
 - [Wayland] Bugfix: `glfwCreateWindow` could emit `GLFW_PLATFORM_ERROR`
 - [Wayland] Bugfix: Lock key modifier bits were only set when lock keys were pressed
 - [Wayland] Bugfix: A window leaving full screen mode would be iconified (#1995)
 - [Wayland] Bugfix: A window leaving full screen mode ignored its desired size
 - [Wayland] Bugfix: `glfwSetWindowMonitor` did not update windowed mode size
 - [Wayland] Bugfix: `glfwRestoreWindow` would make a full screen window windowed
 - [Wayland] Bugfix: A window maximized or restored by the user would enter an
   inconsistent state
 - [Wayland] Bugfix: Window maximization events were not emitted
 - [Wayland] Bugfix: `glfwRestoreWindow` assumed it was always in windowed mode
 - [Wayland] Bugfix: `glfwSetWindowSize` would resize a full screen window
 - [Wayland] Bugfix: A window content scale event would be emitted every time
   the window resized
 - [Wayland] Bugfix: If `glfwInit` failed it would close stdin
 - [Wayland] Bugfix: Manual resizing with fallback decorations behaved erratically
   (#1991,#2115,#2127)
 - [Wayland] Bugfix: Size limits included frame size for fallback decorations
 - [Wayland] Bugfix: Updating `GLFW_DECORATED` had no effect on server-side
   decorations
 - [Wayland] Bugfix: A monitor would be reported as connected again if its scale
   changed
 - [Wayland] Bugfix: `glfwTerminate` would segfault if any monitor had changed
   scale
 - [Wayland] Bugfix: Window content scale events were not emitted when monitor
   scale changed
 - [Wayland] Bugfix: `glfwSetWindowAspectRatio` reported an error instead of
   applying the specified ratio
 - [Wayland] Bugfix: `GLFW_MAXIMIZED` window hint had no effect
 - [Wayland] Bugfix: `glfwRestoreWindow` had no effect before first show
 - [Wayland] Bugfix: Hiding and then showing a window caused program abort on
   wlroots compositors (#1268)
 - [Wayland] Bugfix: `GLFW_DECORATED` was ignored when showing a window with XDG
   decorations


## Contact

On [glfw.org](https://www.glfw.org/) you can find the latest version of GLFW, as
well as news, documentation and other information about the project.

If you have questions related to the use of GLFW, we have a
[forum](https://discourse.glfw.org/), and the `#glfw` IRC channel on
[Libera.Chat](https://libera.chat/).

If you have a bug to report, a patch to submit or a feature you'd like to
request, please file it in the
[issue tracker](https://github.com/glfw/glfw/issues) on GitHub.

Finally, if you're interested in helping out with the development of GLFW or
porting it to your favorite platform, join us on the forum, GitHub or IRC.

