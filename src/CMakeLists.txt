
if (_GLFW_X11_GLX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libglfw.pc.cmake 
                   ${CMAKE_CURRENT_BINARY_DIR}/libglfw.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libglfw.pc
            DESTINATION lib/pkgconfig)
endif()

include_directories(${GLFW_SOURCE_DIR}/src
                    ${GLFW_BINARY_DIR}/src
                    ${glfw_INCLUDE_DIRS})

set(common_SOURCES error.c fullscreen.c gamma.c init.c input.c
                   joystick.c opengl.c time.c window.c)

if (_GLFW_COCOA_NSGL)
    set(glfw_SOURCES ${common_SOURCES} cocoa_fullscreen.m cocoa_gamma.c
                     cocoa_init.m cocoa_input.m cocoa_joystick.m
                     cocoa_opengl.m cocoa_time.c cocoa_window.m)

    # For some reason, CMake doesn't know about .m
    set_source_files_properties(${glfw_SOURCES} PROPERTIES LANGUAGE C)
elseif (_GLFW_WIN32_WGL)
    set(glfw_SOURCES ${common_SOURCES} win32_fullscreen.c win32_gamma.c
                     win32_init.c win32_input.c win32_joystick.c
                     win32_opengl.c win32_time.c win32_window.c
                     win32_dllmain.c)
elseif (_GLFW_X11_GLX)
    set(glfw_SOURCES ${common_SOURCES} x11_fullscreen.c x11_gamma.c
                     x11_init.c x11_input.c x11_joystick.c
                     x11_keysym2unicode.c x11_opengl.c x11_time.c
                     x11_window.c)
endif()

add_library(glfw ${glfw_SOURCES})

if (BUILD_SHARED_LIBS)

    if (_GLFW_WIN32_WGL)
        # The GLFW DLL needs a special compile-time macro and import library name
        set_target_properties(glfw PROPERTIES 
                              COMPILE_DEFINITIONS "GLFW_BUILD_DLL;_GLFW_NO_DLOAD_GDI32;_GLFW_NO_DLOAD_WINMM"
                              PREFIX ""
                              IMPORT_PREFIX ""
                              IMPORT_SUFFIX "dll.lib")
    elseif (_GLFW_COCOA_NSGL)
        # Append -fno-common to the compile flags to work around a bug in the Apple GCC
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

install(TARGETS glfw DESTINATION lib)

