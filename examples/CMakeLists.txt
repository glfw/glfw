
link_libraries(glfw)

include_directories("${GLFW_SOURCE_DIR}/deps")

if (MATH_LIBRARY)
    link_libraries("${MATH_LIBRARY}")
endif()

# Workaround for the MS CRT deprecating parts of the standard library
if (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (WIN32)
    set(ICON glfw.rc)
elseif (APPLE)
    set(ICON glfw.icns)
endif()

set(GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h")
set(GLAD_GLES2 "${GLFW_SOURCE_DIR}/deps/glad/gles2.h")
set(GETOPT "${GLFW_SOURCE_DIR}/deps/getopt.h"
           "${GLFW_SOURCE_DIR}/deps/getopt.c")
set(TINYCTHREAD "${GLFW_SOURCE_DIR}/deps/tinycthread.h"
                "${GLFW_SOURCE_DIR}/deps/tinycthread.c")

set(URL_ARCH)
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(URL_ARCH "x86_64")
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(URL_ARCH "i686")
    endif ()
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|armv8|arm)$")
    set(URL_ARCH "aarch64")
else ()
    message(FATAL_ERROR "Unsopported CPU architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif ()

set(URL_OS)
set(WEBGPU_LIBNAME "libwgpu_native.so")
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(URL_OS "windows")
    set(WEBGPU_LIBNAME "wgpu_native.dll")
    if (MSVC)
        set(URL_ARCH "${URL_ARCH}-msvc")
    else ()
        set(URL_ARCH "${URL_ARCH}-gnu")
    endif ()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(WEBGPU_LIBNAME "libwgpu_native.dylib")
    set(URL_OS "macos")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(URL_OS "linux")
else ()
    message(FATAL_ERROR "Unsopported OS: ${CMAKE_SYSTEM_NAME}")
endif ()

set(WEBGPU_URL "https://github.com/gfx-rs/wgpu-native/releases/download/v24.0.3.1/wgpu-${URL_OS}-${URL_ARCH}-release.zip")
include(FetchContent)
FetchContent_Declare(webgpu_bins
    URL ${WEBGPU_URL}
)
FetchContent_MakeAvailable(webgpu_bins)

add_library(webgpu SHARED IMPORTED GLOBAL)
set(WEBGPU_RUNTIME_LIB "${webgpu_bins_SOURCE_DIR}/lib/${WEBGPU_LIBNAME}")
set_target_properties(webgpu PROPERTIES
    IMPORTED_LOCATION "${WEBGPU_RUNTIME_LIB}"
)
target_include_directories(webgpu INTERFACE
    "${webgpu_bins_SOURCE_DIR}/include"
    "${webgpu_bins_SOURCE_DIR}/include/webgpu"
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (MSVC)
        set_target_properties(webgpu PROPERTIES
            IMPORTED_IMPLIB ${webgpu_bins_SOURCE_DIR}/lib/${WEBGPU_LIBNAME}.lib
        )
    else ()
        set_target_properties(webgpu PROPERTIES
            IMPORTED_IMPLIB ${webgpu_bins_SOURCE_DIR}/lib/lib${WEBGPU_LIBNAME}.a
        )
    endif ()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(webgpu PROPERTIES IMPORTED_NO_SONAME True)
else () # macOS
    if (URL_ARCH STREQUAL "aarch64")
        set(URL_ARCH "x86_64")
    else ()
        set(URL_ARCH "aarch64")
    endif ()

    set(WEBGPU_URL "https://github.com/gfx-rs/wgpu-native/releases/download/v24.0.3.1/wgpu-macos-${URL_ARCH}-release.zip")
    FetchContent_Declare(webgpu_other_bins
        URL ${WEBGPU_URL}
    )
    FetchContent_MakeAvailable(webgpu_other_bins)
    set(WEBGPU_RUNTIME_LIB_OTHER "${webgpu_other_bins_SOURCE_DIR}/lib/${WEBGPU_LIBNAME}")
    execute_process(COMMAND lipo -create ${WEBGPU_RUNTIME_LIB} ${WEBGPU_RUNTIME_LIB_OTHER} -output ${WEBGPU_RUNTIME_LIB})
endif ()

add_executable(boing WIN32 MACOSX_BUNDLE boing.c ${ICON} ${GLAD_GL})
add_executable(gears WIN32 MACOSX_BUNDLE gears.c ${ICON} ${GLAD_GL})
add_executable(heightmap WIN32 MACOSX_BUNDLE heightmap.c ${ICON} ${GLAD_GL})
add_executable(offscreen offscreen.c ${ICON} ${GLAD_GL})
add_executable(particles WIN32 MACOSX_BUNDLE particles.c ${ICON} ${TINYCTHREAD} ${GETOPT} ${GLAD_GL})
add_executable(sharing WIN32 MACOSX_BUNDLE sharing.c ${ICON} ${GLAD_GL})
add_executable(splitview WIN32 MACOSX_BUNDLE splitview.c ${ICON} ${GLAD_GL})
add_executable(triangle-opengl WIN32 MACOSX_BUNDLE triangle-opengl.c ${ICON} ${GLAD_GL})
add_executable(triangle-opengles WIN32 MACOSX_BUNDLE triangle-opengles.c ${ICON} ${GLAD_GLES2})
add_executable(triangle-webgpu WIN32 MACOSX_BUNDLE triangle-webgpu.c ${ICON})
add_executable(wave WIN32 MACOSX_BUNDLE wave.c ${ICON} ${GLAD_GL})
add_executable(windows WIN32 MACOSX_BUNDLE windows.c ${ICON} ${GLAD_GL})

target_link_libraries(particles Threads::Threads)
if (RT_LIBRARY)
    target_link_libraries(particles "${RT_LIBRARY}")
endif()

target_link_libraries(triangle-webgpu webgpu)

set(GUI_ONLY_BINARIES boing gears heightmap particles sharing splitview
    triangle-opengl triangle-opengles triangle-webgpu wave windows)
set(CONSOLE_BINARIES offscreen)

set_target_properties(${GUI_ONLY_BINARIES} ${CONSOLE_BINARIES} PROPERTIES
                      C_STANDARD 99
                      FOLDER "GLFW3/Examples")

if (MSVC)
    # Tell MSVC to use main instead of WinMain
    set_target_properties(${GUI_ONLY_BINARIES} PROPERTIES
                          LINK_FLAGS "/ENTRY:mainCRTStartup")
elseif (CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    # Tell Clang using MS CRT to use main instead of WinMain
    set_target_properties(${GUI_ONLY_BINARIES} PROPERTIES
                          LINK_FLAGS "-Wl,/entry:mainCRTStartup")
endif()

if (APPLE)
    set_target_properties(boing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Boing")
    set_target_properties(gears PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Gears")
    set_target_properties(heightmap PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Heightmap")
    set_target_properties(particles PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Particles")
    set_target_properties(sharing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Sharing")
    set_target_properties(triangle-opengl PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "OpenGL Triangle")
    set_target_properties(triangle-opengles PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "OpenGL ES Triangle")
    set_target_properties(triangle-webgpu PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "WebGPU Triangle")
    set_target_properties(splitview PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "SplitView")
    set_target_properties(wave PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Wave")
    set_target_properties(windows PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Windows")

    set_source_files_properties(glfw.icns PROPERTIES
                                MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(${GUI_ONLY_BINARIES} PROPERTIES
                          MACOSX_BUNDLE_SHORT_VERSION_STRING ${GLFW_VERSION}
                          MACOSX_BUNDLE_LONG_VERSION_STRING ${GLFW_VERSION}
                          MACOSX_BUNDLE_ICON_FILE glfw.icns
                          MACOSX_BUNDLE_INFO_PLIST "${GLFW_SOURCE_DIR}/CMake/Info.plist.in")
endif()

