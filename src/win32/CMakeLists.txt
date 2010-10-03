
if(CYGWIN)
    configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/libglfw.pc.cmake 
	${CMAKE_CURRENT_BINARY_DIR}/libglfw.pc @ONLY)

# These lines are intended to remove the --export-all-symbols
# flag added in the Modules/Platform/CYGWIN.cmake file of the
# CMake distribution.
# This is a HACK.  If you have trouble _linking_ the GLFW
# _shared_ library on Cygwin, try disabling this.
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared")
    set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS})

endif(CYGWIN)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} 
                    ${GLFW_SOURCE_DIR}/src
                    ${GLFW_BINARY_DIR}/src
                    ${GLFW_INCLUDE_DIR})

set(libglfw_SOURCES
    ${common_SOURCES}
    win32_enable.c 
    win32_fullscreen.c
    win32_glext.c
    win32_init.c 
    win32_joystick.c
    win32_time.c 
    win32_window.c
    win32_dllmain.c)

add_library(libglfwStatic STATIC ${libglfw_SOURCES})
add_library(libglfwShared SHARED ${libglfw_SOURCES}) 

target_link_libraries(libglfwShared ${OPENGL_gl_LIBRARY})
set_target_properties(libglfwShared PROPERTIES 
    DEFINE_SYMBOL GLFW_BUILD_DLL
    PREFIX ""
    IMPORT_PREFIX ""
    IMPORT_SUFFIX "dll.lib")

set_target_properties(libglfwStatic libglfwShared PROPERTIES 
    CLEAN_DIRECT_OUTPUT 1
    OUTPUT_NAME glfw)

if(CYGWIN)
    # Build for the regular Win32 environment (not Cygwin)
    set_target_properties(libglfwStatic libglfwShared PROPERTIES COMPILE_FLAGS "-mwin32 -mno-cygwin")
    set_target_properties(libglfwStatic libglfwShared PROPERTIES LINK_FLAGS "-mwin32 -mno-cygwin")
endif(CYGWIN)

install(TARGETS libglfwStatic libglfwShared DESTINATION lib)

if(CYGWIN)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libglfw.pc DESTINATION lib/pkgconfig)
endif(CYGWIN)

