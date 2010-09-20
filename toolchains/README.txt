This folder contains a collection of toolchains definition in order to 
support cross compilation. The naming scheme is the following:
  host-system-compiler.cmake

to use this at the time you run the initial cmake command use the 
following parameter 
   -DCMAKE_TOOLCHAIN_FILE=./toolchains/XXX-XXX-XXX.cmake
 which maps to file in this folder. 

For more details see: http://www.paraview.org/Wiki/CMake_Cross_Compiling 
