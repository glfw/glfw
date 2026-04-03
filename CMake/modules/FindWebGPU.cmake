set(WebGPU_FOUND True) # since we fetch the depencency it will always be found

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

add_library(WebGPU::WebGPU SHARED IMPORTED GLOBAL)
set(WEBGPU_RUNTIME_LIB "${webgpu_bins_SOURCE_DIR}/lib/${WEBGPU_LIBNAME}")
set_target_properties(WebGPU::WebGPU PROPERTIES
    IMPORTED_LOCATION "${WEBGPU_RUNTIME_LIB}"
)
target_include_directories(WebGPU::WebGPU INTERFACE
    "${webgpu_bins_SOURCE_DIR}/include"
    "${webgpu_bins_SOURCE_DIR}/include/webgpu"
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (MSVC)
        set_target_properties(WebGPU::WebGPU PROPERTIES
            IMPORTED_IMPLIB ${webgpu_bins_SOURCE_DIR}/lib/${WEBGPU_LIBNAME}.lib
        )
    else ()
        set_target_properties(WebGPU::WebGPU PROPERTIES
            IMPORTED_IMPLIB ${webgpu_bins_SOURCE_DIR}/lib/lib${WEBGPU_LIBNAME}.a
        )
    endif ()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(WebGPU::WebGPU PROPERTIES IMPORTED_NO_SONAME True)
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

unset(URL_ARCH)
unset(URL_OS)
unset(WEBGPU_URL)
unset(WEBGPU_RUNTIME_LIB)
unset(WEBGPU_RUNTIME_LIB_OTHER)
