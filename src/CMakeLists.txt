
if(CYGWIN)

    # These lines are intended to remove the --export-all-symbols
    # flag added in the Modules/Platform/CYGWIN.cmake file of the
    # CMake distribution.
    # This is a HACK.  If you have trouble _linking_ the GLFW
    # _shared_ library on Cygwin, try disabling this.
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared")
    set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS})

endif(CYGWIN)

if(UNIX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libglfw.pc.cmake 
                   ${CMAKE_CURRENT_BINARY_DIR}/libglfw.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libglfw.pc DESTINATION lib/pkgconfig)
endif(UNIX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} 
                    ${GLFW_SOURCE_DIR}/src
                    ${GLFW_BINARY_DIR}/src
                    ${GLFW_INCLUDE_DIR}
                    ${GLFW_SOURCE_DIR}/support)

set(common_SOURCES enable.c error.c fullscreen.c gamma.c init.c input.c
                   joystick.c opengl.c time.c window.c)

if(_GLFW_COCOA_NSGL)
    set(libglfw_SOURCES ${common_SOURCES} cocoa_enable.m cocoa_fullscreen.m
                        cocoa_gamma.m cocoa_init.m cocoa_joystick.m
                        cocoa_opengl.m cocoa_time.m cocoa_window.m)

    # For some reason, CMake doesn't know about .m
    set_source_files_properties(${libglfw_SOURCES} PROPERTIES LANGUAGE C)
elseif(_GLFW_WIN32_WGL)
    set(libglfw_SOURCES ${common_SOURCES} win32_enable.c win32_fullscreen.c
                        win32_gamma.c win32_init.c win32_joystick.c
                        win32_opengl.c win32_time.c win32_window.c
                        win32_dllmain.c)
elseif(_GLFW_X11_GLX)
    set(libglfw_SOURCES ${common_SOURCES} x11_enable.c x11_fullscreen.c
                        x11_gamma.c x11_init.c x11_joystick.c
                        x11_keysym2unicode.c x11_opengl.c x11_time.c
                        x11_window.c)
else()
    message(FATAL_ERROR "No supported platform was selected")
endif(_GLFW_COCOA_NSGL)

add_library(libglfwStatic STATIC ${libglfw_SOURCES})
add_library(libglfwShared SHARED ${libglfw_SOURCES}) 
target_link_libraries(libglfwShared  ${GLFW_LIBRARIES})
set_target_properties(libglfwStatic libglfwShared PROPERTIES 
                      CLEAN_DIRECT_OUTPUT 1
                      OUTPUT_NAME glfw)

if(WIN32)
    # The GLFW DLL needs a special compile-time macro and import library name
    set_target_properties(libglfwShared PROPERTIES 
                          DEFINE_SYMBOL GLFW_BUILD_DLL
                          PREFIX ""
                          IMPORT_PREFIX ""
                          IMPORT_SUFFIX "dll.lib")
endif(WIN32)

if(CYGWIN)
    # Build for the regular Win32 environment (not Cygwin)
    set_target_properties(libglfwStatic libglfwShared PROPERTIES
                          COMPILE_FLAGS "-mwin32 -mno-cygwin"
                          LINK_FLAGS "-mwin32 -mno-cygwin")
endif(CYGWIN)

if(APPLE)
    # Append -fno-common to the compile flags to work around a bug in the Apple GCC
    get_target_property(CFLAGS libglfwShared COMPILE_FLAGS)
    if(NOT CFLAGS)
        set(CFLAGS "")
    endif(NOT CFLAGS)
    set_target_properties(libglfwShared PROPERTIES COMPILE_FLAGS "${CFLAGS} -fno-common")
endif(APPLE)

install(TARGETS libglfwStatic libglfwShared DESTINATION lib)

