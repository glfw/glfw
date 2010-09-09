project(GLFW C)

set(GLFW_VERSION_MAJOR "2")
set(GLFW_VERSION_MINOR "7")
set(GLFW_VERSION_PATCH "0")
set(GLFW_VERSION_EXTRA "")
set(GLFW_VERSION "${GLFW_VERSION_MAJOR}.${GLFW_VERSION_MINOR}")
set(GLFW_VERSION_FULL  
    "${GLFW_VERSION}.${GLFW_VERSION_PATCH}${GLFW_VERSION_EXTRA}")

cmake_minimum_required(VERSION 2.4)

include(CheckFunctionExists)
include(CheckSymbolExists)

# Stuff common to all platform 
find_package(OpenGL REQUIRED)

set(common_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/enable.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fullscreen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glext.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/init.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/joystick.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/time.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/window.c
)

# Stuff specific to WGL on Win32
if (WIN32)
    message(STATUS "Building GLFW for WGL on a Win32 system") 

    # Set up library and include paths
    set(CMAKE_REQUIRED_LIBRARIES ${OPENGL_gl_LIBRARY})
    list(APPEND GLFW_INCLUDE_DIR ${OPENGL_INCLUDE_DIR})
    list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})

    # Select platform specific code
    add_subdirectory(src/win32)
endif (WIN32)

# Stuff specific to GLX on Unix-like platforms
if (UNIX AND NOT APPLE AND NOT CYGWIN)
    message(STATUS "Building GLFW for GLX on a Unix-like system")
    
    # Set up library and include paths
    set(CMAKE_REQUIRED_LIBRARIES ${X11_X11_LIB} ${OPENGL_gl_LIBRARY})
    list(APPEND GLFW_INCLUDE_DIR ${X11_X11_INCLUDE_PATH})
    list(APPEND GLFW_LIBRARIES ${X11_X11_LIB})
    list(APPEND GLFW_INCLUDE_DIR ${OPENGL_INCLUDE_DIR})
    list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})

    # Detect X11 extension 
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/CheckX11Extensions.cmake)
    
    # Check for XRandR (modern resolution switching extension)
    CHECK_X11_XRANDR()
    if (X11_XRANDR_FOUND)
        set(_GLFW_HAS_XRANDR 1) 
        list(APPEND GLFW_INCLUDE_DIR ${X11_XRANDR_INCLUDE_DIR})
        list(APPEND GLFW_LIBRARIES ${X11_XRANDR_LIBRARIES})
    endif(X11_XRANDR_FOUND)

    # Check for xf86vidmode (fallback legacy resolution switching extension)
    if (NOT X11_XRANDR_FOUND)
        CHECK_X11_XF86VIDMODE()
        if (X11_XF86VIDMODE_FOUND)
            set(_GLFW_HAS_XF86VIDMODE 1)
            list(APPEND GLFW_INCLUDE_DIR ${X11_XF86VIDMODE_INCLUDE_DIR})
            list(APPEND GLFW_LIBRARIES ${X11_XF86VIDMODE_LIBRARIES})
        endif(X11_XF86VIDMODE_FOUND) 
    endif (NOT X11_XRANDR_FOUND)

    CHECK_FUNCTION_EXISTS(glXGetProcAddress _GLFW_HAS_GLXGETPROCADDRESS)

    if (NOT _GLFW_HAS_GLXGETPROCADDRESS)
        CHECK_FUNCTION_EXISTS(glXGetProcAddressARB _GLFW_HAS_GLXGETPROCADDRESSARB)
    endif (NOT _GLFW_HAS_GLXGETPROCADDRESS)

    if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND NOT _GLFW_HAS_GLXGETPROCADDRESSARB)
        CHECK_FUNCTION_EXISTS(glXGetProcAddressEXT _GLFW_HAS_GLXGETPROCADDRESSEXT)
    endif (NOT _GLFW_HAS_GLXGETPROCADDRESS AND NOT _GLFW_HAS_GLXGETPROCADDRESSARB)

    if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND
        NOT _GLFW_HAS_GLXGETPROCADDRESSARB AND
        NOT _GLFW_HAS_GLXGETPROCADDRESSEXT)
        message(WARNING "No glXGetProcAddressXXX variant found")
    endif (NOT _GLFW_HAS_GLXGETPROCADDRESS AND
           NOT _GLFW_HAS_GLXGETPROCADDRESSARB AND
           NOT _GLFW_HAS_GLXGETPROCADDRESSEXT)

    # Select platform specific code
    add_subdirectory(src/x11)
endif(UNIX AND NOT APPLE AND NOT CYGWIN)

# Stuff specific to AGL and CGL on Mac OS X
if (UNIX AND APPLE)
    message(STATUS "Building GLFW for NSOpenGL on Mac OS X")
        
    # Universal build, decent set of warning flags...
    set(CMAKE_OSX_ARCHITECTURES ppc;i386;ppc64;x86_64)
    set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.5.sdk)
    set(CMAKE_C_FLAGS "-mmacosx-version-min=10.5 -Wall -Wextra -Wno-unused-parameter -Werror") 
    
    # Set up library and include paths
    find_library(COCOA_FRAMEWORK Cocoa)
    list(APPEND GLFW_LIBRARIES ${COCOA_FRAMEWORK})
    list(APPEND GLFW_LIBRARIES ${OPENGL_gl_LIBRARY})

    # Select platform specific code
    add_subdirectory(src/cocoa)
endif(UNIX AND APPLE)

add_subdirectory(examples)
add_subdirectory(tests)
#add_subdirectory(docs/doxygen)
#add_subdirectory(docs/manuals)


#--------------------------------------------------------------------
# -- Install standard files 
#--------------------------------------------------------------------

# Install the GLFW header file
install(DIRECTORY include/ DESTINATION include 
    PATTERN ".svn" EXCLUDE 
    PATTERN "include/*"
)

# Install documentation
install(
    FILES 
        COPYING.txt 
        readme.html 
    DESTINATION 
        share/doc/glfw-${GLFW_VERSION_FULL}/
)

#--------------------------------------------------------------------
# -- Additional stuff 
#--------------------------------------------------------------------

#--------------------------------------------------------------------
# -- Documentation generation 
#--------------------------------------------------------------------
#include("${CMAKE_CURRENT_SOURCE_DIR}/documentation.cmake")
#configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
#               "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
#               IMMEDIATE @ONLY)
#add_doxygen_target("${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
#add_subdirectory(docs)

#--------------------------------------------------------------------
# -- Uninstall operation
# -------------------------------------------------------------------
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in" 
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

