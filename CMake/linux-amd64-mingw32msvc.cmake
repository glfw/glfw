# Define the cross compilation environment for cross compiling from linux 
# to Win64 it is to be used when Debian cross compilation toolchain is 
# available.
SET(CMAKE_SYSTEM_NAME    Windows) # Target system name
SET(CMAKE_SYSTEM_VERSION 1)       # Not really used.
SET(CMAKE_C_COMPILER     "amd64-mingw32msvc-gcc")
SET(CMAKE_CXX_COMPILER   "amd64-mingw32msvc-g++")
SET(CMAKE_RANLIB         "amd64-mingw32msvc-ranlib")


#Configure the behaviour of the find commands 
SET(CMAKE_FIND_ROOT_PATH "/usr/amd64-mingw32msvc")
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
