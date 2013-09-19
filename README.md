# GLFW

## Introduction

GLFW is a free, Open Source, portable library for OpenGL and OpenGL ES
application development.  It provides a simple, platform-independent API for
creating windows and contexts, reading input, handling events, etc.

Version 3.0.3 adds fixes for a number of bugs that together affect all supported
platforms, most notably MinGW compilation issues and cursor mode issues on OS X.
As this is a patch release, there are no API changes.

If you are new to GLFW, you may find the
[introductory tutorial](http://www.glfw.org/docs/latest/quick.html) for GLFW
3 useful.  If you have used GLFW 2 in the past, there is a
[transition guide](http://www.glfw.org/docs/latest/moving.html) for moving to
the GLFW 3 API.


## Compiling GLFW

### Dependencies

To compile GLFW and the accompanying example programs, you will need **CMake**,
which will generate the project files or makefiles for your particular
development environment.  If you are on a Unix-like system such as Linux or
FreeBSD or have a package system like Fink, MacPorts, Cygwin or Homebrew, you
can simply install its CMake package.  If not, you can get installers for
Windows and OS X from the [CMake website](http://www.cmake.org/).

Additional dependencies are listed below.


#### Visual C++ on Windows

The Microsoft Platform SDK that is installed along with Visual C++ contains all
the necessary headers, link libraries and tools except for CMake.


#### MinGW or MinGW-w64 on Windows

These packages contain all the necessary headers, link libraries and tools
except for CMake.


#### MinGW or MinGW-w64 cross-compilation

Both Cygwin and many Linux distributions have MinGW or MinGW-w64 packages.  For
example, Cygwin has the `mingw64-i686-gcc` and `mingw64-x86_64-gcc` packages
for 32- and 64-bit version of MinGW-w64, while Debian GNU/Linux and derivatives
like Ubuntu have the `mingw-w64` package for both.

GLFW has CMake toolchain files in the `CMake/` directory that allow for easy
cross-compilation of Windows binaries.  To use these files you need to add a
special parameter when generating the project files or makefiles:

    cmake -DCMAKE_TOOLCHAIN_FILE=<toolchain-file> .

The exact toolchain file to use depends on the prefix used by the MinGW or
MinGW-w64 binaries on your system.  You can usually see this in the /usr
directory.  For example, both the Debian/Ubuntu and Cygwin MinGW-w64 packages
have `/usr/x86_64-w64-mingw32` for the 64-bit compilers, so the correct
invocation would be:

    cmake -DCMAKE_TOOLCHAIN_FILE=CMake/x86_64-w64-mingw32.cmake .

For more details see the article
[CMake Cross Compiling](http://www.paraview.org/Wiki/CMake_Cross_Compiling) on
the CMake wiki.


#### Xcode on OS X

Xcode contains all necessary tools except for CMake.  The necessary headers and
libraries are included in the core OS frameworks.  Xcode can be downloaded from
the Mac App Store.


#### Unix-like systems with X11

To compile GLFW for X11, you need to have the X11 and OpenGL header packages
installed, as well as the basic development tools like GCC and make.  For
example, on Ubuntu and other distributions based on Debian GNU/Linux, you need
to install the `xorg-dev` and `libglu1-mesa-dev` packages.  The former pulls in
all X.org header packages and the latter pulls in the Mesa OpenGL and GLU
packages.  Note that using header files and libraries from Mesa during
compilation *will not* tie your binaries to the Mesa implementation of OpenGL.


### Generating with CMake

Once you have all necessary dependencies, it is time to generate the project
files or makefiles for your development environment.  CMake needs to know two
paths for this: the path to the source directory and the target path for the
generated files and compiled binaries.  If these are the same, it is called an
in-tree build, otherwise it is called an out-of-tree build.

One of several advantages of out-of-tree builds is that you can generate files
and compile for different development environments using a single source tree.


#### Using CMake from the command-line

To make an in-tree build, enter the root directory of the GLFW source tree and
run CMake.  The current directory is used as target path, while the path
provided as an argument is used to find the source tree.

    cd <glfw-root-dir>
    cmake .

To make an out-of-tree build, make another directory, enter it and run CMake
with the (relative or absolute) path to the root of the source tree as an
argument.

    cd <glfw-root-dir>
    mkdir build
    cd build
    cmake ..


#### Using the CMake GUI

If you are using the GUI version, choose the root of the GLFW source tree as
source location and the same directory or another, empty directory as the
destination for binaries.  Choose *Configure*, change any options you wish to,
*Configure* again to let the changes take effect and then *Generate*.


### CMake options

The CMake files for GLFW provide a number of options, although not all are
available on all supported platforms.  Some of these are de facto standards
among CMake users and so have no `GLFW_` prefix.

If you are using the GUI version of CMake, these are listed and can be changed
from there.  If you are using the command-line version, use the `ccmake` tool.
Some package systems like Ubuntu and other distributions based on Debian
GNU/Linux have this tool in a separate `cmake-curses-gui` package.


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


#### OS X specific options

`GLFW_USE_CHDIR` determines whether `glfwInit` changes the current
directory of bundled applications to the `Contents/Resources` directory.

`GLFW_USE_MENUBAR` determines whether the first call to
`glfwCreateWindow` sets up a minimal menu bar.

`GLFW_BUILD_UNIVERSAL` determines whether to build Universal Binaries.


#### Windows specific options

`USE_MSVC_RUNTIME_LIBRARY_DLL` determines whether to use the DLL version or the
static library version of the Visual C++ runtime library.

`GLFW_USE_DWM_SWAP_INTERVAL` determines whether the swap interval is set even
when DWM compositing is enabled.  This can lead to severe jitter and is not
usually recommended.

`GLFW_USE_OPTIMUS_HPG` determines whether to export the `NvOptimusEnablement`
symbol, which forces the use of the high-performance GPU on nVidia Optimus
systems.


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

See the [GLFW documentation](http://www.glfw.org/docs/latest/).


## Changelog

 - [Win32] Bugfix: `_WIN32_WINNT` was not set to Windows XP or later
 - [Win32] Bugfix: Legacy MinGW needs `WINVER` and `UNICODE` before `stddef.h`
 - [Cocoa] Bugfix: Cursor was not visible in normal mode in full screen
 - [Cocoa] Bugfix: Cursor was not actually hidden in hidden mode
 - [Cocoa] Bugfix: Cursor modes were not applied to inactive windows
 - [X11] Bugfix: Events for mouse buttons 4 and above were not reported
 - [X11] Bugfix: CMake 2.8.7 does not set `X11_Xinput_LIB` even when found


## Contact

The official website for GLFW is [glfw.org](http://www.glfw.org/).  There you
can find the latest version of GLFW, as well as news, documentation and other
information about the project.

If you have questions related to the use of GLFW, we have a
[support forum](https://sourceforge.net/p/glfw/discussion/247562/), and the IRC
channel `#glfw` on [Freenode](http://freenode.net/).

If you have a bug to report, a patch to submit or a feature you'd like to
request, please file it in the
[issue tracker](https://github.com/glfw/glfw/issues) on GitHub.

Finally, if you're interested in helping out with the development of GLFW or
porting it to your favorite platform, we have an occasionally active
[developer's mailing list](https://lists.stacken.kth.se/mailman/listinfo/glfw-dev),
or you could join us on `#glfw`.


## Acknowledgements

GLFW exists because people around the world donated their time and lent their
skills.

 - Bobyshev Alexander
 - artblanc
 - arturo
 - Matt Arsenault
 - Keith Bauer
 - John Bartholomew
 - Niklas Behrens
 - Niklas Bergström
 - Doug Binks
 - blanco
 - Lambert Clara
 - Noel Cower
 - Jarrod Davis
 - Olivier Delannoy
 - Paul R. Deppe
 - Jonathan Dummer
 - Ralph Eastwood
 - Gerald Franz
 - GeO4d
 - Marcus Geelnard
 - Stefan Gustavson
 - Sylvain Hellegouarch
 - heromyth
 - Paul Holden
 - Toni Jovanoski
 - Osman Keskin
 - Cameron King
 - Peter Knut
 - Robin Leffmann
 - Glenn Lewis
 - Shane Liesegang
 - Дмитри Малышев
 - Martins Mozeiko
 - Tristam MacDonald
 - Hans Mackowiak
 - Kyle McDonald
 - David Medlock
 - Jonathan Mercier
 - Marcel Metz
 - Kenneth Miller
 - Bruce Mitchener
 - Jeff Molofee
 - Jon Morton
 - Pierre Moulon
 - Julian Møller
 - Ozzy
 - Peoro
 - Braden Pellett
 - Arturo J. Pérez
 - Jorge Rodriguez
 - Ed Ropple
 - Riku Salminen
 - Sebastian Schuberth
 - Matt Sealey
 - SephiRok
 - Steve Sexton
 - Dmitri Shuralyov
 - Daniel Skorupski
 - Bradley Smith
 - Julian Squires
 - Johannes Stein
 - Justin Stoecker
 - Nathan Sweet
 - TTK-Bandit
 - Sergey Tikhomirov
 - Samuli Tuomola
 - Jari Vetoniemi
 - Simon Voordouw
 - Torsten Walluhn
 - Jay Weisskopf
 - Frank Wille
 - yuriks
 - Santi Zupancic
 - Lasse Öörni
 - All the unmentioned and anonymous contributors in the GLFW community, for bug
   reports, patches, feedback, testing and encouragement

