This directory contains a collection of toolchain definitions for cross
compilation, currently limited to compiling Win32 binaries on Linux.

The toolchain file naming scheme is as follows:

  host-system-compiler.cmake

To use these files you add a special parameter when configuring the source tree:

  cmake -DCMAKE_TOOLCHAIN_FILE=<toolchain-file> .

For example, to use the Debian GNU/Linux MinGW package, run CMake like this:

  cmake -DCMAKE_TOOLCHAIN_FILE=CMake/linux-i586-mingw32msvc.cmake .

For more details see this article:

  http://www.paraview.org/Wiki/CMake_Cross_Compiling 

