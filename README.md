# GLFW

## Introduction

GLFW is a free, Open Source, portable library for OpenGL and OpenGL ES
application development.  It provides a simple, platform-independent API for
creating windows and contexts, reading input, handling events, etc.

Version 3.0.2 is *not yet described*.  As this is a patch release, there are no
API changes.

If you are new to GLFW, you may find the
[introductory tutorial](http://www.glfw.org/docs/3.0/quick.html) for GLFW
3 useful.  If you have used GLFW 2 in the past, there is a
[transition guide](http://www.glfw.org/docs/3.0/moving.html) for moving to the
GLFW 3 API.


## Compiling GLFW

To compile GLFW and the accompanying example programs, you will need the
[CMake](http://www.cmake.org/) build system.


### Dependencies

#### X11 dependencies

To compile GLFW for X11 and GLX, you need to have the X and OpenGL header
packages installed.  For example, on Ubuntu and other distributions based on
Debian GNU/Linux, you need to install the `xorg-dev` and `libglu1-mesa-dev`
packages.  Note that using header files from Mesa *will not* tie your binary to
the Mesa implementation of OpenGL.


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

`USE_MSVC_RUNTIME_LIBRARY_DLL` determines whether to use the DLL version or the
static library version of the Visual C++ runtime library.


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

See the [GLFW 3.0 documentation](http://www.glfw.org/docs/3.0/).


## Changelog

 - Bugfix: The `-Wall` flag was not used with Clang and other GCC compatibles
 - Bugfix: The default for `GLFW_ALPHA_BITS` was set to zero
 - [Win32] Bugfix: The clipboard string was not freed on terminate
 - [Win32] Bugfix: Entry points for OpenGL 1.0 and 1.1 functions were not
                   returned by `glfwGetProcAddress`
 - [Win32] Bugfix: The user32 and dwmapi module handles were not freed on
                   library termination
 - [Cocoa] Bugfix: The clipboard string was not freed on terminate
 - [Cocoa] Bugfix: Selectors were used that are not declared by the 10.6 SDK
 - [X11] Bugfix: Override-redirect windows were resized to the desired instead
                 of the actual resolution of the selected video mode
 - [X11] Bugfix: Screensaver override for full screen windows had a possible
                 race condition


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
 - Matt Arsenault
 - Keith Bauer
 - John Bartholomew
 - Niklas Behrens
 - Niklas Bergström
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

