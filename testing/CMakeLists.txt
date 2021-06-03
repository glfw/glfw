cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(test
    VERSION 1.0
    LANGUAGES CXX)

add_executable(test 
    test.cpp
)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
add_subdirectory(D:/src/glfw ${CMAKE_BINARY_DIR}/glfw-build)

target_include_directories(test PUBLIC D:/src/glfw/include ${OPENGL_INCLUDE_DIRS})
target_link_libraries(test glfw ${OPENGL_LIBRARIES})