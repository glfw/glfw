project(GLFW C)

cmake_minimum_required(VERSION 2.8.9)

if (CMAKE_VERSION VERSION_EQUAL "3.0" OR CMAKE_VERSION VERSION_GREATER "3.0")
    # Until all major package systems have moved to CMake 3,
    # we stick with the older INSTALL_NAME_DIR mechanism
    cmake_policy(SET CMP0042 OLD)
endif()

set(GLFW_VERSION_MAJOR "3")
set(GLFW_VERSION_MINOR "1")
set(GLFW_VERSION_PATCH "1")
set(GLFW_VERSION_EXTRA "")
set(GLFW_VERSION "${GLFW_VERSION_MAJOR}.${GLFW_VERSION_MINOR}")
set(GLFW_VERSION_FULL "${GLFW_VERSION}.${GLFW_VERSION_PATCH}${GLFW_VERSION_EXTRA}")
set(LIB_SUFFIX "" CACHE STRING "Takes an empty string or 64. Directory where lib will be installed: lib or lib64")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" ON)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" ON)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" ON)
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
    option(GLFW_USE_RETINA "Use the full resolution of Retina displays" ON)
else()
    option(GLFW_USE_EGL "Use EGL for context creation" OFF)
endif()

if (UNIX AND NOT APPLE)
    option(GLFW_USE_WAYLAND "Use Wayland for context creation (implies EGL as well)" OFF)
    option(GLFW_USE_MIR     "Use Mir for context creation (implies EGL as well)" OFF)
endif()

if (MSVC)
    option(USE_MSVC_RUNTIME_LIBRARY_DLL "Use MSVC runtime library DLL" ON)
endif()

if (BUILD_SHARED_LIBS)
    set(_GLFW_BUILD_DLL 1)
endif()

if (GLFW_USE_WAYLAND)
    set(GLFW_USE_EGL ON)
elseif (GLFW_USE_MIR)
    set(GLFW_USE_EGL ON)
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

    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake/modules")
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

if (GLFW_BUILD_DOCS)
    set(DOXYGEN_SKIP_DOT TRUE)
    find_package(Doxygen)

    if (GLFW_DOCUMENT_INTERNALS)
        set(GLFW_INTERNAL_DOCS "${GLFW_SOURCE_DIR}/src/internal.h ${GLFW_SOURCE_DIR}/docs/internal.dox")
    endif()
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

if (MINGW)
    # Enable link-time exploit mitigation features enabled by default on MSVC

    include(CheckCCompilerFlag)

    # Compatibility with data execution prevention (DEP)
    set(CMAKE_REQUIRED_FLAGS "-Wl,--nxcompat")
    check_c_compiler_flag("" _GLFW_HAS_DEP)
    if (_GLFW_HAS_DEP)
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--nxcompat ${CMAKE_SHARED_LINKER_FLAGS}")
    endif()

    # Compatibility with address space layout randomization (ASLR)
    set(CMAKE_REQUIRED_FLAGS "-Wl,--dynamicbase")
    check_c_compiler_flag("" _GLFW_HAS_ASLR)
    if (_GLFW_HAS_ASLR)
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--dynamicbase ${CMAKE_SHARED_LINKER_FLAGS}")
    endif()

    # Compatibility with 64-bit address space layout randomization (ASLR)
    set(CMAKE_REQUIRED_FLAGS "-Wl,--high-entropy-va")
    check_c_compiler_flag("" _GLFW_HAS_64ASLR)
    if (_GLFW_HAS_64ASLR)
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--high-entropy-va ${CMAKE_SHARED_LINKER_FLAGS}")
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
    if (GLFW_USE_WAYLAND)
        set(_GLFW_WAYLAND 1)
        message(STATUS "Using Wayland for window creation")
    elseif (GLFW_USE_MIR)
        set(_GLFW_MIR 1)
        message(STATUS "Using Mir for window creation")
    else()
        set(_GLFW_X11 1)
        message(STATUS "Using X11 for window creation")
    endif()

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

    list(APPEND glfw_PKG_LIBS "-lgdi32")

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
    if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
        add_definitions(-DUNICODE -DWINVER=0x0501)
    endif()
endif()

#--------------------------------------------------------------------
# Use WGL for context creation
#--------------------------------------------------------------------
if (_GLFW_WGL)

    list(APPEND glfw_PKG_LIBS "-lopengl32")

    list(APPEND glfw_INCLUDE_DIRS "${OPENGL_INCLUDE_DIR}")
    list(APPEND glfw_LIBRARIES "${OPENGL_gl_LIBRARY}")
endif()

#--------------------------------------------------------------------
# Use X11 for window creation
#--------------------------------------------------------------------
if (_GLFW_X11)

    find_package(X11 REQUIRED)

    list(APPEND glfw_PKG_DEPS "x11")

    # Set up library and include paths
    list(APPEND glfw_INCLUDE_DIRS "${X11_X11_INCLUDE_PATH}")
    list(APPEND glfw_LIBRARIES "${X11_X11_LIB}" "${CMAKE_THREAD_LIBS_INIT}")
    if (UNIX AND NOT APPLE)
        list(APPEND glfw_LIBRARIES "${RT_LIBRARY}")
    endif()

    # Check for XRandR (modern resolution switching and gamma control)
    if (NOT X11_Xrandr_FOUND)
        message(FATAL_ERROR "The RandR library and headers were not found")
    endif()

    list(APPEND glfw_INCLUDE_DIRS "${X11_Xrandr_INCLUDE_PATH}")
    list(APPEND glfw_LIBRARIES "${X11_Xrandr_LIB}")
    list(APPEND glfw_PKG_DEPS "xrandr")

    # Check for Xinerama (legacy multi-monitor support)
    if (NOT X11_Xinerama_FOUND)
        message(FATAL_ERROR "The Xinerama library and headers were not found")
    endif()

    list(APPEND glfw_INCLUDE_DIRS "${X11_Xinerama_INCLUDE_PATH}")
    list(APPEND glfw_LIBRARIES "${X11_Xinerama_LIB}")
    list(APPEND glfw_PKG_DEPS "xinerama")

    # Check for XInput (high-resolution cursor motion)
    if (X11_Xinput_FOUND)
        list(APPEND glfw_INCLUDE_DIRS "${X11_Xinput_INCLUDE_PATH}")
        list(APPEND glfw_PKG_DEPS "xi")

        if (X11_Xinput_LIB)
            list(APPEND glfw_LIBRARIES "${X11_Xinput_LIB}")
        else()
            # Backwards compatibility (bug in CMake 2.8.7)
            list(APPEND glfw_LIBRARIES Xi)
        endif()

        set(_GLFW_HAS_XINPUT TRUE)
    endif()

    # Check for Xf86VidMode (fallback gamma control)
    if (X11_xf86vmode_FOUND)
        list(APPEND glfw_INCLUDE_DIRS "${X11_xf86vmode_INCLUDE_PATH}")
        list(APPEND glfw_PKG_DEPS "xxf86vm")

        if (X11_Xxf86vm_LIB)
            list(APPEND glfw_LIBRARIES "${X11_Xxf86vm_LIB}")
        else()
            # Backwards compatibility (see CMake bug 0006976)
            list(APPEND glfw_LIBRARIES Xxf86vm)
        endif()

        set(_GLFW_HAS_XF86VM TRUE)
    endif()

    # Check for Xkb (X keyboard extension)
    if (NOT X11_Xkb_FOUND)
        message(FATAL_ERROR "The X keyboard extension headers were not found")
    endif()

    list(APPEND glfw_INCLUDE_DIR "${X11_Xkb_INCLUDE_PATH}")

    find_library(RT_LIBRARY rt)
    mark_as_advanced(RT_LIBRARY)
    if (RT_LIBRARY)
        list(APPEND glfw_LIBRARIES "${RT_LIBRARY}")
        list(APPEND glfw_PKG_LIBS "-lrt")
    endif()

    find_library(MATH_LIBRARY m)
    mark_as_advanced(MATH_LIBRARY)
    if (MATH_LIBRARY)
        list(APPEND glfw_LIBRARIES "${MATH_LIBRARY}")
        list(APPEND glfw_PKG_LIBS "-lm")
    endif()

    # Check for Xcursor
    if (NOT X11_Xcursor_FOUND)
        message(FATAL_ERROR "The Xcursor libraries and headers were not found")
    endif()

    list(APPEND glfw_INCLUDE_DIR "${X11_Xcursor_INCLUDE_PATH}")
    list(APPEND glfw_LIBRARIES "${X11_Xcursor_LIB}")
    list(APPEND glfw_PKG_DEPS "xcursor")

endif()

#--------------------------------------------------------------------
# Use Wayland for window creation
#--------------------------------------------------------------------
if (_GLFW_WAYLAND)
    find_package(Wayland REQUIRED)
    list(APPEND glfw_PKG_DEPS "wayland-egl")

    list(APPEND glfw_INCLUDE_DIRS "${WAYLAND_INCLUDE_DIR}")
    list(APPEND glfw_LIBRARIES "${WAYLAND_LIBRARIES}" "${CMAKE_THREAD_LIBS_INIT}")

    find_package(XKBCommon REQUIRED)
    list(APPEND glfw_PKG_DEPS "xkbcommon")
    list(APPEND glfw_INCLUDE_DIRS "${XKBCOMMON_INCLUDE_DIRS}")
    list(APPEND glfw_LIBRARIES "${XKBCOMMON_LIBRARY}")

    find_library(MATH_LIBRARY m)
    mark_as_advanced(MATH_LIBRARY)
    if (MATH_LIBRARY)
        list(APPEND glfw_LIBRARIES "${MATH_LIBRARY}")
        list(APPEND glfw_PKG_LIBS "-lm")
    endif()
endif()

#--------------------------------------------------------------------
# Use Mir for window creation
#--------------------------------------------------------------------
if (_GLFW_MIR)
    find_package(Mir REQUIRED)
    list(APPEND glfw_PKG_DEPS "mirclient")

    list(APPEND glfw_INCLUDE_DIRS "${MIR_INCLUDE_DIR}")
    list(APPEND glfw_LIBRARIES "${MIR_LIBRARIES}" "${CMAKE_THREAD_LIBS_INIT}")

    find_package(XKBCommon REQUIRED)
    list(APPEND glfw_PKG_DEPS "xkbcommon")
    list(APPEND glfw_INCLUDE_DIRS "${XKBCOMMON_INCLUDE_DIRS}")
    list(APPEND glfw_LIBRARIES "${XKBCOMMON_LIBRARY}")

    find_library(MATH_LIBRARY m)
    mark_as_advanced(MATH_LIBRARY)
    if (MATH_LIBRARY)
        list(APPEND glfw_LIBRARIES "${MATH_LIBRARY}")
        list(APPEND glfw_PKG_LIBS "-lm")
    endif()
endif()

#--------------------------------------------------------------------
# Use GLX for context creation
#--------------------------------------------------------------------
if (_GLFW_GLX)

    list(APPEND glfw_INCLUDE_DIRS "${OPENGL_INCLUDE_DIR}")
    list(APPEND glfw_LIBRARIES "${OPENGL_gl_LIBRARY}")

    list(APPEND glfw_PKG_DEPS "gl")

    include(CheckFunctionExists)

    set(CMAKE_REQUIRED_LIBRARIES "${OPENGL_gl_LIBRARY}")
    check_function_exists(glXGetProcAddress _GLFW_HAS_GLXGETPROCADDRESS)
    check_function_exists(glXGetProcAddressARB _GLFW_HAS_GLXGETPROCADDRESSARB)
    check_function_exists(glXGetProcAddressEXT _GLFW_HAS_GLXGETPROCADDRESSEXT)

    if (NOT _GLFW_HAS_GLXGETPROCADDRESS AND
        NOT _GLFW_HAS_GLXGETPROCADDRESSARB AND
        NOT _GLFW_HAS_GLXGETPROCADDRESSEXT)
        message(WARNING "No glXGetProcAddressXXX variant found")

        # Check for dlopen support as a fallback

        set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_DL_LIBS}")
        check_function_exists(dlopen _GLFW_HAS_DLOPEN)
        if (NOT _GLFW_HAS_DLOPEN)
            message(FATAL_ERROR "No entry point retrieval mechanism found")
        endif()

        if (CMAKE_DL_LIBS)
            list(APPEND glfw_LIBRARIES "${CMAKE_DL_LIBS}")
            list(APPEND glfw_PKG_LIBS "-l${CMAKE_DL_LIBS}")
        endif()
    endif()

endif()

#--------------------------------------------------------------------
# Use EGL for context creation
#--------------------------------------------------------------------
if (_GLFW_EGL)

    list(APPEND glfw_INCLUDE_DIRS "${EGL_INCLUDE_DIR}")
    list(APPEND glfw_LIBRARIES "${EGL_LIBRARY}")

    list(APPEND glfw_PKG_DEPS "egl")

    if (_GLFW_USE_OPENGL)
        list(APPEND glfw_LIBRARIES "${OPENGL_gl_LIBRARY}")
        list(APPEND glfw_INCLUDE_DIRS "${OPENGL_INCLUDE_DIR}")
        list(APPEND glfw_PKG_DEPS "gl")
    elseif (_GLFW_USE_GLESV1)
        list(APPEND glfw_LIBRARIES "${GLESv1_LIBRARY}")
        list(APPEND glfw_INCLUDE_DIRS "${GLESv1_INCLUDE_DIR}")
        list(APPEND glfw_PKG_DEPS "glesv1_cm")
    elseif (_GLFW_USE_GLESV2)
        list(APPEND glfw_LIBRARIES "${GLESv2_LIBRARY}")
        list(APPEND glfw_INCLUDE_DIRS "${GLESv2_INCLUDE_DIR}")
        list(APPEND glfw_PKG_DEPS "glesv2")
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

    if (GLFW_USE_RETINA)
        set(_GLFW_USE_RETINA 1)
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
    find_library(CORE_VIDEO_FRAMEWORK CoreVideo)
    mark_as_advanced(COCOA_FRAMEWORK
                     IOKIT_FRAMEWORK
                     CORE_FOUNDATION_FRAMEWORK
                     CORE_VIDEO_FRAMEWORK)
    list(APPEND glfw_LIBRARIES "${COCOA_FRAMEWORK}"
                               "${OPENGL_gl_LIBRARY}"
                               "${IOKIT_FRAMEWORK}"
                               "${CORE_FOUNDATION_FRAMEWORK}"
                               "${CORE_VIDEO_FRAMEWORK}")

    set(glfw_PKG_DEPS "")
    set(glfw_PKG_LIBS "-framework Cocoa -framework OpenGL -framework IOKit -framework CoreFoundation -framework CoreVideo")
endif()

#--------------------------------------------------------------------
# Export GLFW library dependencies
#--------------------------------------------------------------------
set(GLFW_LIBRARIES ${glfw_LIBRARIES} CACHE STRING "Dependencies of GLFW")
foreach(arg ${glfw_PKG_DEPS})
    set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} ${arg}")
endforeach()
foreach(arg ${glfw_PKG_LIBS})
    set(GLFW_PKG_LIBS "${GLFW_PKG_LIBS} ${arg}")
endforeach()

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
include(CMakePackageConfigHelpers)

if (UNIX)
    set(GLFW_CONFIG_PATH "${CMAKE_INSTALL_PREFIX}/lib/cmake/glfw3/")
else()
    set(GLFW_CONFIG_PATH "${CMAKE_INSTALL_PREFIX}/")
endif()

configure_package_config_file("${GLFW_SOURCE_DIR}/src/glfw3Config.cmake.in"
                              "${GLFW_BINARY_DIR}/src/glfw3Config.cmake"
                              INSTALL_DESTINATION "${GLFW_CONFIG_PATH}"
                              PATH_VARS CMAKE_INSTALL_PREFIX
                              NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file("${GLFW_BINARY_DIR}/src/glfw3ConfigVersion.cmake" 
                                 VERSION ${GLFW_VERSION_FULL}
                                 COMPATIBILITY SameMajorVersion)

if (GLFW_BUILD_DOCS)
    configure_file("${GLFW_SOURCE_DIR}/docs/Doxyfile.in"
                   "${GLFW_BINARY_DIR}/docs/Doxyfile" @ONLY)
endif()

configure_file("${GLFW_SOURCE_DIR}/src/glfw_config.h.in"
               "${GLFW_BINARY_DIR}/src/glfw_config.h" @ONLY)

configure_file("${GLFW_SOURCE_DIR}/src/glfw3.pc.in"
               "${GLFW_BINARY_DIR}/src/glfw3.pc" @ONLY)

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

if (DOXYGEN_FOUND AND GLFW_BUILD_DOCS)
    add_subdirectory(docs)
endif()

#--------------------------------------------------------------------
# Install files other than the library
# The library is installed by src/CMakeLists.txt
#--------------------------------------------------------------------
if (GLFW_INSTALL)
    install(DIRECTORY include/GLFW DESTINATION include
            FILES_MATCHING PATTERN glfw3.h PATTERN glfw3native.h)

    install(FILES "${GLFW_BINARY_DIR}/src/glfw3Config.cmake"
                  "${GLFW_BINARY_DIR}/src/glfw3ConfigVersion.cmake"
            DESTINATION lib${LIB_SUFFIX}/cmake/glfw)

    install(EXPORT glfwTargets DESTINATION lib${LIB_SUFFIX}/cmake/glfw)
    install(FILES "${GLFW_BINARY_DIR}/src/glfw3.pc"
            DESTINATION lib${LIB_SUFFIX}/pkgconfig)

    # Only generate this target if no higher-level project already has
    if (NOT TARGET uninstall)
        configure_file("${GLFW_SOURCE_DIR}/cmake_uninstall.cmake.in"
                       "${GLFW_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

        add_custom_target(uninstall
                          "${CMAKE_COMMAND}" -P
                          "${GLFW_BINARY_DIR}/cmake_uninstall.cmake")
    endif()
endif()

