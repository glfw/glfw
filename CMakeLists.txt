project(GLFW C)

cmake_minimum_required(VERSION 2.8)

set(GLFW_VERSION_MAJOR "3")
set(GLFW_VERSION_MINOR "0")
set(GLFW_VERSION_PATCH "3")
set(GLFW_VERSION_EXTRA "")
set(GLFW_VERSION "${GLFW_VERSION_MAJOR}.${GLFW_VERSION_MINOR}")
set(GLFW_VERSION_FULL "${GLFW_VERSION}.${GLFW_VERSION_PATCH}${GLFW_VERSION_EXTRA}")
set(LIB_SUFFIX "" CACHE STRING "Takes an empty string or 64. Directory where lib will be installed: lib or lib64")

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" ON)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" ON)
option(GLFW_INSTALL "Generate installation target" ON)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)

if (WIN32)
    option(GLFW_USE_DWM_SWAP_INTERVAL "Set swap interval even when DWM compositing is enabled" OFF)
    option(GLFW_USE_OPTIMUS_HPG "Force use of high-performance GPU on Optimus systems" OFF)
endif()

if (APPLE)
    option(GLFW_BUILD_UNIVERSAL "Build GLFW as a Universal Binary" OFF)
    option(GLFW_USE_CHDIR "Make glfwInit chdir to Contents/Resources" ON)
    option(GLFW_USE_MENUBAR "Populate the menu bar on first window creation" ON)
else()
    option(GLFW_USE_EGL "Use EGL for context creation" OFF)
endif()

if (MSVC)
    option(USE_MSVC_RUNTIME_LIBRARY_DLL "Use MSVC runtime library DLL" ON)
endif()

if (BUILD_SHARED_LIBS)
    set(_GLFW_BUILD_DLL 1)
endif()

if (GLFW_USE_EGL)
    set(GLFW_CLIENT_LIBRARY "opengl" CACHE STRING
        "The client library to use; one of opengl, glesv1 or glesv2")

    if (${GLFW_CLIENT_LIBRARY} STREQUAL "opengl")
        set(_GLFW_USE_OPENGL 1)
    elseif (${GLFW_CLIENT_LIBRARY} STREQUAL "glesv1")
        set(_GLFW_USE_GLESV1 1)
    elseif (${GLFW_CLIENT_LIBRARY} STREQUAL "glesv2")
        set(_GLFW_USE_GLESV2 1)
    else()
        message(FATAL_ERROR "Unsupported client library")
    endif()

    set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake/modules)
    find_package(EGL REQUIRED)

    if (NOT _GLFW_USE_OPENGL)
        set(GLFW_BUILD_EXAMPLES OFF)
        set(GLFW_BUILD_TESTS OFF)
        message(STATUS "NOTE: Examples and tests require OpenGL")
    endif()
else()
    set(_GLFW_USE_OPENGL 1)
endif()

if (_GLFW_USE_OPENGL)
    find_package(OpenGL REQUIRED)
elseif (_GLFW_USE_GLESV1)
    find_package(GLESv1 REQUIRED)
elseif (_GLFW_USE_GLESV2)
    find_package(GLESv2 REQUIRED)
endif()

find_package(Threads REQUIRED)

set(DOXYGEN_SKIP_DOT TRUE)
find_package(Doxygen)

if (GLFW_DOCUMENT_INTERNALS)
    set(GLFW_INTERNAL_DOCS "${GLFW_SOURCE_DIR}/src/internal.h ${GLFW_SOURCE_DIR}/docs/internal.dox")
endif()

#--------------------------------------------------------------------
# Set compiler specific flags
#--------------------------------------------------------------------
if (UNIX)
    add_definitions(-Wall)

    if (BUILD_SHARED_LIBS)
        add_definitions(-fvisibility=hidden)
    endif()
endif()

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    if (NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
        foreach (flag CMAKE_C_FLAGS
                        CMAKE_C_FLAGS_DEBUG
                        CMAKE_C_FLAGS_RELEASE
                        CMAKE_C_FLAGS_MINSIZEREL
                        CMAKE_C_FLAGS_RELWITHDEBINFO)

            if (${flag} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
            endif()
            if (${flag} MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
            endif()

        endforeach()
    endif()
endif()

#--------------------------------------------------------------------
# Detect and select backend APIs
#--------------------------------------------------------------------
if (WIN32)
    set(_GLFW_WIN32 1)
    message(STATUS "Using Win32 for window creation") 

    if (GLFW_USE_EGL)
        set(_GLFW_EGL 1)
        message(STATUS "Using EGL for context creation")
    else()
        set(_GLFW_WGL 1)
        message(STATUS "Using WGL for context creation")
    endif()
elseif (APPLE)
    set(_GLFW_COCOA 1)
    message(STATUS "Using Cocoa for window creation")
    set(_GLFW_NSGL 1)
    message(STATUS "Using NSGL for context creation")
elseif (UNIX)
    set(_GLFW_X11 1)
    message(STATUS "Using X11 for window creation") 

    if (GLFW_USE_EGL)
        set(_GLFW_EGL 1)
        message(STATUS "Using EGL for context creation")
    else()
        set(_GLFW_GLX 1)
        message(STATUS "Using GLX for context creation")
    endif()
else()
    message(FATAL_ERROR "No supported platform was detected")
endif()

#--------------------------------------------------------------------
# Use Win32 for window creation
#--------------------------------------------------------------------
if (_GLFW_WIN32)
    # The DLL links against winmm; the static library loads it
    # That way, both code paths receive testing
    if (BUILD_SHARED_LIBS)
        set(_GLFW_NO_DLOAD_WINMM 1)
        list(APPEND glfw_LIBRARIES winmm)
    endif()

    if (GLFW_USE_DWM_SWAP_INTERVAL)
        set(_GLFW_USE_DWM_SWAP_INTERVAL 1)
    endif()
    if (GLFW_USE_OPTIMUS_HPG)
        set(_GLFW_USE_OPTIMUS_HPG 1)
    endif()

    # HACK: When building on MinGW, WINVER and UNICODE need to be defined before
    # the inclusion of stddef.h (by glfw3.h), which is itself included before
    # win32_platform.h.  We define them here until a saner solution can be found
    # NOTE: MinGW-w64 and Visual C++ do /not/ need this hack.
    add_definitions(-DUNICODE)
    add_definitions(-DWINVER=0x0501)
endif()

#--------------------------------------------------------------------
# Use WGL for context creation
#--------------------------------------------------------------------
if (_GLFW_WGL)
    list(APPEND glfw_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
    list(APPEND glfw_LIBRARIES ${OPENGL_gl_LIBRARY})
endif()

#--------------------------------------------------------------------
# Use X11 for window creation
#--------------------------------------------------------------------
if (_GLFW_X11)

    find_package(X11 REQUIRED)

    set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} x11")

    # Set up library and include paths
    list(APPEND glfw_INCLUDE_DIRS ${X11_X11_INCLUDE_PATH})
    list(APPEND glfw_LIBRARIES ${X11_X11_LIB} ${CMAKE_THREAD_LIBS_INIT})
    if (UNIX AND NOT APPLE)
        list(APPEND glfw_LIBRARIES ${RT_LIBRARY})
    endif()

    # Check for XRandR (modern resolution switching and gamma control)
    if (NOT X11_Xrandr_FOUND)
        message(FATAL_ERROR "The RandR library and headers were not found")
    endif()

    list(APPEND glfw_INCLUDE_DIRS ${X11_Xrandr_INCLUDE_PATH})
    list(APPEND glfw_LIBRARIES ${X11_Xrandr_LIB})
    set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} xrandr")

    # Check for XInput (high-resolution cursor motion)
    if (NOT X11_Xinput_FOUND)
        message(FATAL_ERROR "The XInput library and headers were not found")
    endif()

    list(APPEND glfw_INCLUDE_DIRS ${X11_Xinput_INCLUDE_PATH})

    if (X11_Xinput_LIB)
        list(APPEND glfw_LIBRARIES ${X11_Xinput_LIB})
    else()
        # Backwards compatibility (bug in CMake 2.8.7)
        list(APPEND glfw_LIBRARIES Xi)
    endif()
    set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} xi")

    # Check for Xf86VidMode (fallback gamma control)
    if (NOT X11_xf86vmode_FOUND)
        message(FATAL_ERROR "The Xf86VidMode library and headers were not found")
    endif()

    list(APPEND glfw_INCLUDE_DIRS ${X11_xf86vmode_INCLUDE_PATH})
    set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} xxf86vm")

    if (X11_Xxf86vm_LIB)
        list(APPEND glfw_LIBRARIES ${X11_Xxf86vm_LIB})
    else()
        # Backwards compatibility (see CMake bug 0006976)
        list(APPEND glfw_LIBRARIES Xxf86vm)
    endif()

    # Check for Xkb (X keyboard extension)
    if (NOT X11_Xkb_FOUND)
        message(FATAL_ERROR "The X keyboard extension headers were not found")
    endif() 

    list(APPEND glfw_INCLUDE_DIR ${X11_Xkb_INCLUDE_PATH})

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

endif()

#--------------------------------------------------------------------
# Use GLX for context creation
#--------------------------------------------------------------------
if (_GLFW_GLX)

    list(APPEND glfw_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
    list(APPEND glfw_LIBRARIES ${OPENGL_gl_LIBRARY})

    set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} gl")

    include(CheckFunctionExists)

    set(CMAKE_REQUIRED_LIBRARIES ${OPENGL_gl_LIBRARY})
    check_function_exists(glXGetProcAddress _GLFW_HAS_GLXGETPROCADDRESS)
    check_function_exists(glXGetProcAddressARB _GLFW_HAS_GLXGETPROCADDRESSARB)
    check_function_exists(glXGetProcAddressEXT _GLFW_HAS_GLXGETPROCADDRESSEXT)

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

endif()

#--------------------------------------------------------------------
# Use EGL for context creation
#--------------------------------------------------------------------
if (_GLFW_EGL)

    list(APPEND glfw_INCLUDE_DIRS ${EGL_INCLUDE_DIR})
    list(APPEND glfw_LIBRARIES ${EGL_LIBRARY})

    if (UNIX)
        set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} egl")
    endif()

    if (_GLFW_USE_OPENGL)
        list(APPEND glfw_LIBRARIES ${OPENGL_gl_LIBRARY})
        list(APPEND glfw_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
        set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} gl")
    elseif (_GLFW_USE_GLESV1)
        list(APPEND glfw_LIBRARIES ${GLESv1_LIBRARY})
        list(APPEND glfw_INCLUDE_DIRS ${GLESv1_INCLUDE_DIR})
        set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} glesv1_cm")
    elseif (_GLFW_USE_GLESV2)
        list(APPEND glfw_LIBRARIES ${GLESv2_LIBRARY})
        list(APPEND glfw_INCLUDE_DIRS ${GLESv2_INCLUDE_DIR})
        set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} glesv2")
    endif()

endif()

#--------------------------------------------------------------------
# Use Cocoa for window creation and NSOpenGL for context creation
#--------------------------------------------------------------------
if (_GLFW_COCOA AND _GLFW_NSGL)
        
    if (GLFW_USE_MENUBAR)
        set(_GLFW_USE_MENUBAR 1)
    endif()

    if (GLFW_USE_CHDIR)
        set(_GLFW_USE_CHDIR 1)
    endif()

    if (GLFW_BUILD_UNIVERSAL)
        message(STATUS "Building GLFW as Universal Binaries")
        set(CMAKE_OSX_ARCHITECTURES i386;x86_64)
    else()
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
# Choose library output name
#--------------------------------------------------------------------
if (BUILD_SHARED_LIBS AND UNIX)
    # On Unix-like systems, shared libraries can use the soname system.
    set(GLFW_LIB_NAME glfw)
else()
    set(GLFW_LIB_NAME glfw3)
endif()

#--------------------------------------------------------------------
# Create generated files
#--------------------------------------------------------------------
configure_file(${GLFW_SOURCE_DIR}/docs/Doxyfile.in
               ${GLFW_BINARY_DIR}/docs/Doxyfile @ONLY)

configure_file(${GLFW_SOURCE_DIR}/src/config.h.in 
               ${GLFW_BINARY_DIR}/src/config.h @ONLY)

configure_file(${GLFW_SOURCE_DIR}/src/glfwConfig.cmake.in
               ${GLFW_BINARY_DIR}/src/glfwConfig.cmake @ONLY)

configure_file(${GLFW_SOURCE_DIR}/src/glfwConfigVersion.cmake.in
               ${GLFW_BINARY_DIR}/src/glfwConfigVersion.cmake @ONLY)

if (UNIX)
    configure_file(${GLFW_SOURCE_DIR}/src/glfw3.pc.in
                   ${GLFW_BINARY_DIR}/src/glfw3.pc @ONLY)
endif()

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

if (DOXYGEN_FOUND)
    add_subdirectory(docs)
endif()

#--------------------------------------------------------------------
# Install files other than the library
# The library is installed by src/CMakeLists.txt
#--------------------------------------------------------------------
if (GLFW_INSTALL)
    install(DIRECTORY include/GLFW DESTINATION include 
            FILES_MATCHING PATTERN glfw3.h PATTERN glfw3native.h)

    install(FILES ${GLFW_BINARY_DIR}/src/glfwConfig.cmake
                  ${GLFW_BINARY_DIR}/src/glfwConfigVersion.cmake
            DESTINATION lib${LIB_SUFFIX}/cmake/glfw)

    if (UNIX)
        install(EXPORT glfwTargets DESTINATION lib${LIB_SUFFIX}/cmake/glfw)
        install(FILES ${GLFW_BINARY_DIR}/src/glfw3.pc
                DESTINATION lib${LIB_SUFFIX}/pkgconfig)
    endif()

    # Only generate this target if no higher-level project already has
    if (NOT TARGET uninstall)
        configure_file(${GLFW_SOURCE_DIR}/cmake_uninstall.cmake.in
                       ${GLFW_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY)

        add_custom_target(uninstall
                          ${CMAKE_COMMAND} -P
                          ${GLFW_BINARY_DIR}/cmake_uninstall.cmake)
    endif()
endif()

