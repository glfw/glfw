project(GLFW C)

cmake_minimum_required(VERSION 2.8)

set(GLFW_VERSION_MAJOR "3")
set(GLFW_VERSION_MINOR "0")
set(GLFW_VERSION_PATCH "0")
set(GLFW_VERSION_EXTRA "")
set(GLFW_VERSION "${GLFW_VERSION_MAJOR}.${GLFW_VERSION_MINOR}")
set(GLFW_VERSION_FULL "${GLFW_VERSION}.${GLFW_VERSION_PATCH}${GLFW_VERSION_EXTRA}")

option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" ON)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

find_package(OpenGL REQUIRED)

#--------------------------------------------------------------------
# Enable all warnings on GCC, regardless of OS
#--------------------------------------------------------------------
if (CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-Wall)
endif()

#--------------------------------------------------------------------
# Detect and select target platform
#--------------------------------------------------------------------
if (WIN32)
    set(_GLFW_WIN32_WGL 1)
    message(STATUS "Building GLFW for WGL on a Win32 system") 
elseif (UNIX AND APPLE)
    set(_GLFW_COCOA_NSGL 1)
    message(STATUS "Building GLFW for Cocoa and NSOpenGL on Mac OS X")
elseif (UNIX AND NOT APPLE)
    set(_GLFW_X11_GLX 1)
    message(STATUS "Building GLFW for X11 and GLX on a Unix-like system")
else()
    message(FATAL_ERROR "No supported platform was detected")
endif()

#--------------------------------------------------------------------
# Set up GLFW for Win32 and WGL on Windows
#--------------------------------------------------------------------
if (_GLFW_WIN32_WGL)

    # Set up library and include paths
    list(APPEND glfw_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
    list(APPEND glfw_LIBRARIES ${OPENGL_gl_LIBRARY})

    if (BUILD_SHARED_LIBS)
        list(APPEND glfw_LIBRARIES winmm)
    endif()
endif()

#--------------------------------------------------------------------
# Set up GLFW for Xlib and GLX on Unix-like systems with X Windows
#--------------------------------------------------------------------
if (_GLFW_X11_GLX)

    find_package(X11 REQUIRED)

    # Set up library and include paths
    list(APPEND glfw_INCLUDE_DIRS ${X11_X11_INCLUDE_PATH} ${OPENGL_INCLUDE_DIR})
    list(APPEND glfw_LIBRARIES ${X11_X11_LIB} ${OPENGL_gl_LIBRARY})

    set(GLFW_PKG_DEPS "gl x11")
    set(GLFW_PKG_LIBS "")

    include(CheckFunctionExists)

    # Check for XRandR (modern resolution switching extension)
    if (X11_Xrandr_FOUND)
        set(_GLFW_HAS_XRANDR 1) 
        list(APPEND glfw_INCLUDE_DIRS ${X11_Xrandr_INCLUDE_PATH})
        list(APPEND glfw_LIBRARIES ${X11_Xrandr_LIB})
        set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} xrandr")
    endif()

    # Check for Xf86VidMode (fallback legacy resolution switching extension)
    if (X11_xf86vmode_FOUND)
        set(_GLFW_HAS_XF86VIDMODE 1)
        list(APPEND glfw_INCLUDE_DIRS ${X11_xf86vmode_INCLUDE_PATH})
    
        # NOTE: This is a workaround for CMake bug 0006976 (missing
        # X11_xf86vmode_LIB variable)
        if (X11_xf86vmode_LIB)
            list(APPEND glfw_LIBRARIES ${X11_xf86vmode_LIB})
        else()
            list(APPEND glfw_LIBRARIES Xxf86vm)
        endif()

        set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} xxf86vm")
    endif() 

    # Check for Xkb (X keyboard extension)
    if (X11_Xkb_FOUND)
        set(_GLFW_HAS_XKB 1)
        list(APPEND glfw_INCLUDE_DIR ${X11_Xkb_INCLUDE_PATH})
    endif() 

    find_library(RT_LIBRARY rt)
    mark_as_advanced(RT_LIBRARY)
    if (RT_LIBRARY)
        list(APPEND glfw_LIBRARIES ${RT_LIBRARY})
        set(GLFW_PKG_LIBS "${GLFW_PKG_LIBS} -lrt")
    endif()

    find_library(MATH_LIBRARY m)
    mark_as_advanced(MATH_LIBRARY)
    if (MATH_LIBRARY)
        list(APPEND glfw_LIBRARIES ${MATH_LIBRARY})
        set(GLFW_PKG_LIBS "${GLFW_PKG_LIBS} -lm")
    endif()

    set(CMAKE_REQUIRED_LIBRARIES ${OPENGL_gl_LIBRARY})

    check_function_exists(glXGetProcAddress _GLFW_HAS_GLXGETPROCADDRESS)

    if (NOT _GLFW_HAS_GLXGETPROCADDRESS)
        check_function_exists(glXGetProcAddressARB _GLFW_HAS_GLXGETPROCADDRESSARB)
    endif()

    if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND NOT _GLFW_HAS_GLXGETPROCADDRESSARB)
        check_function_exists(glXGetProcAddressEXT _GLFW_HAS_GLXGETPROCADDRESSEXT)
    endif()

    if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND
        NOT _GLFW_HAS_GLXGETPROCADDRESSARB AND
        NOT _GLFW_HAS_GLXGETPROCADDRESSEXT)
        message(WARNING "No glXGetProcAddressXXX variant found")

        # Check for dlopen support as a fallback

        find_library(DL_LIBRARY dl)
        mark_as_advanced(DL_LIBRARY)
        if (DL_LIBRARY)
            set(CMAKE_REQUIRED_LIBRARIES ${DL_LIBRARY})
        else()
            set(CMAKE_REQUIRED_LIBRARIES "")
        endif()

        check_function_exists(dlopen _GLFW_HAS_DLOPEN)

        if (NOT _GLFW_HAS_DLOPEN)
            message(FATAL_ERROR "No entry point retrieval mechanism found")
        endif()

        if (DL_LIBRARY)
            list(APPEND glfw_LIBRARIES ${DL_LIBRARY})
            set(GLFW_PKG_LIBS "${GLFW_PKG_LIBS} -ldl")
        endif()
    endif()

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(_GLFW_USE_LINUX_JOYSTICKS 1)
    endif()
endif()

#--------------------------------------------------------------------
# Set up GLFW for Cocoa and NSOpenGL on Mac OS X
#--------------------------------------------------------------------
if (_GLFW_COCOA_NSGL)
        
    option(GLFW_BUILD_UNIVERSAL "Build GLFW as a Universal Binary" OFF)

    # Universal build
    if (GLFW_BUILD_UNIVERSAL)
        message(STATUS "Building GLFW as Universal Binaries")
        set(CMAKE_OSX_ARCHITECTURES ppc;i386;ppc64;x86_64)
        set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.5.sdk)
        set(CMAKE_C_FLAGS "-mmacosx-version-min=10.5") 
    else(GLFW_BUILD_UNIVERSAL)
        message(STATUS "Building GLFW only for the native architecture")
    endif()
    
    # Set up library and include paths
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    list(APPEND glfw_LIBRARIES ${COCOA_FRAMEWORK}
                               ${OPENGL_gl_LIBRARY}
                               ${IOKIT_FRAMEWORK}
                               ${CORE_FOUNDATION_FRAMEWORK})

    set(GLFW_PKG_DEPS "")
    set(GLFW_PKG_LIBS "-framework Cocoa -framework OpenGL -framework IOKit -framework CoreFoundation")
endif()

#--------------------------------------------------------------------
# Export GLFW library dependencies
#--------------------------------------------------------------------
set(GLFW_LIBRARIES ${glfw_LIBRARIES} CACHE STRING "Dependencies of GLFW")

#--------------------------------------------------------------------
# Add subdirectories
#--------------------------------------------------------------------
add_subdirectory(src)

if (GLFW_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (GLFW_BUILD_TESTS)
    add_subdirectory(tests)
endif()

#--------------------------------------------------------------------
# Create generated files
#--------------------------------------------------------------------
configure_file(${GLFW_SOURCE_DIR}/docs/Doxyfile.in
               ${GLFW_BINARY_DIR}/docs/Doxyfile @ONLY)

configure_file(${GLFW_SOURCE_DIR}/src/config.h.in 
               ${GLFW_BINARY_DIR}/src/config.h @ONLY)

#--------------------------------------------------------------------
# Install header and documentation
# The src directory's CMakeLists.txt file installs the library
#--------------------------------------------------------------------
install(DIRECTORY include/GL DESTINATION include 
        FILES_MATCHING PATTERN glfw3.h)

install(FILES COPYING.txt readme.html 
        DESTINATION share/doc/glfw-${GLFW_VERSION_FULL})

#--------------------------------------------------------------------
# Create and install pkg-config file on supported platforms
#--------------------------------------------------------------------
if (_GLFW_X11_GLX OR _GLFW_COCOA_NSGL)
    configure_file(${GLFW_SOURCE_DIR}/src/libglfw.pc.in 
                   ${GLFW_BINARY_DIR}/src/libglfw.pc @ONLY)

    install(FILES ${GLFW_BINARY_DIR}/src/libglfw.pc
            DESTINATION lib/pkgconfig)
endif()

#--------------------------------------------------------------------
# Uninstall operation
# Don't generate this target if a higher-level project already has
#--------------------------------------------------------------------
if (NOT TARGET uninstall)
    configure_file(${GLFW_SOURCE_DIR}/cmake_uninstall.cmake.in
                   ${GLFW_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY)

    add_custom_target(uninstall
                      ${CMAKE_COMMAND} -P
                      ${GLFW_BINARY_DIR}/cmake_uninstall.cmake)
endif()

