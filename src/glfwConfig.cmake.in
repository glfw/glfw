# - Config file for the glfw package
# It defines the following variables
#   GLFW_INCLUDE_DIR, the path where GLFW headers are located
#   GLFW_LIBRARY_DIR, folder in which the GLFW library is located
#   GLFW_LIBRARY, library to link against to use GLFW

set(GLFW_INCLUDE_DIR "@CMAKE_INSTALL_PREFIX@/include")
set(GLFW_LIBRARY_DIR "@CMAKE_INSTALL_PREFIX@/lib@LIB_SUFFIX@")

find_library(GLFW_LIBRARY "@GLFW_LIB_NAME@" HINTS ${GLFW_LIBRARY_DIR})
