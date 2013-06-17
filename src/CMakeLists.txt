
include_directories(${GLFW_SOURCE_DIR}/src
                    ${GLFW_BINARY_DIR}/src
                    ${glfw_INCLUDE_DIRS})

set(common_HEADERS ${GLFW_BINARY_DIR}/src/config.h internal.h
                   ${GLFW_SOURCE_DIR}/include/GLFW/glfw3.h
                   ${GLFW_SOURCE_DIR}/include/GLFW/glfw3native.h)
set(common_SOURCES clipboard.c context.c gamma.c init.c input.c joystick.c
                   monitor.c time.c window.c)

if (_GLFW_COCOA)
    set(glfw_HEADERS ${common_HEADERS} cocoa_platform.h)
    set(glfw_SOURCES ${common_SOURCES} cocoa_clipboard.m cocoa_gamma.c
                     cocoa_init.m cocoa_joystick.m cocoa_monitor.m cocoa_time.c
                     cocoa_window.m)
elseif (_GLFW_WIN32)
    set(glfw_HEADERS ${common_HEADERS} win32_platform.h)
    set(glfw_SOURCES ${common_SOURCES} win32_clipboard.c win32_gamma.c
                     win32_init.c win32_joystick.c win32_monitor.c win32_time.c
                     win32_window.c)
elseif (_GLFW_X11)
    set(glfw_HEADERS ${common_HEADERS} x11_platform.h)
    set(glfw_SOURCES ${common_SOURCES} x11_clipboard.c x11_gamma.c x11_init.c
                     x11_joystick.c x11_monitor.c x11_time.c x11_window.c
                     x11_unicode.c)
endif()

if (_GLFW_EGL)
    list(APPEND glfw_HEADERS ${common_HEADERS} egl_platform.h)
    list(APPEND glfw_SOURCES ${common_SOURCES} egl_context.c)
elseif (_GLFW_NSGL)
    list(APPEND glfw_HEADERS ${common_HEADERS} nsgl_platform.h)
    list(APPEND glfw_SOURCES ${common_SOURCES} nsgl_context.m)
elseif (_GLFW_WGL)
    list(APPEND glfw_HEADERS ${common_HEADERS} wgl_platform.h)
    list(APPEND glfw_SOURCES ${common_SOURCES} wgl_context.c)
elseif (_GLFW_X11)
    list(APPEND glfw_HEADERS ${common_HEADERS} glx_platform.h)
    list(APPEND glfw_SOURCES ${common_SOURCES} glx_context.c)
endif()

if (APPLE)
    # For some reason, CMake doesn't know about .m
    set_source_files_properties(${glfw_SOURCES} PROPERTIES LANGUAGE C)
endif()

add_library(glfw ${glfw_SOURCES} ${glfw_HEADERS})
set_target_properties(glfw PROPERTIES OUTPUT_NAME "${GLFW_LIB_NAME}")

if (BUILD_SHARED_LIBS)
    # Include version information in the output
    set_target_properties(glfw PROPERTIES VERSION ${GLFW_VERSION})
    if (UNIX)
        set_target_properties(glfw PROPERTIES SOVERSION ${GLFW_VERSION_MAJOR})
    endif()

    if (WIN32)
        # The GLFW DLL needs a special compile-time macro and import library name
        set_target_properties(glfw PROPERTIES PREFIX "" IMPORT_PREFIX "")

        if (MINGW)
            set_target_properties(glfw PROPERTIES IMPORT_SUFFIX "dll.a")
        else()
            set_target_properties(glfw PROPERTIES IMPORT_SUFFIX "dll.lib")
        endif()
    elseif (APPLE)
        # Append -fno-common to the compile flags to work around a bug in
        # Apple's GCC
        get_target_property(glfw_CFLAGS glfw COMPILE_FLAGS)
        if (NOT glfw_CFLAGS)
            set(glfw_CFLAGS "")
        endif()
        set_target_properties(glfw PROPERTIES
                              COMPILE_FLAGS "${glfw_CFLAGS} -fno-common")
    endif()

    target_link_libraries(glfw ${glfw_LIBRARIES})
    target_link_libraries(glfw LINK_INTERFACE_LIBRARIES)
endif()

if (GLFW_INSTALL)
    install(TARGETS glfw EXPORT glfwTargets DESTINATION lib${LIB_SUFFIX})
endif()

