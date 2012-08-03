This directory contains a collection of toolchain definitions for
cross-compiling for Windows using MinGW on various other systems.

To use these files you add a special parameter when configuring the source tree:

  cmake -DCMAKE_TOOLCHAIN_FILE=<toolchain-file> .

The exact file to use depends on the prefix used by the MinGW binaries on your
system.  You can usually see this in the /usr directory, i.e. the Ubuntu
MinGW-w64 packages have /usr/x86_64-w64-mingw32 for the 64-bit compilers, so the
correct invocation would be:

  cmake -DCMAKE_TOOLCHAIN_FILE=CMake/x86_64-w64-mingw32.cmake .

For more details see this article:

  http://www.paraview.org/Wiki/CMake_Cross_Compiling 

