
link_libraries(glfw ${OPENGL_glu_LIBRARY})

if (BUILD_SHARED_LIBS)
    add_definitions(-DGLFW_DLL)
    link_libraries(${OPENGL_gl_LIBRARY} ${MATH_LIBRARY})
else()
    link_libraries(${glfw_LIBRARIES})
endif()

include_directories(${GLFW_SOURCE_DIR}/include
                    ${GLFW_SOURCE_DIR}/deps)

if (NOT APPLE)
    # HACK: This is NOTFOUND on OS X 10.8
    include_directories(${OPENGL_INCLUDE_DIR})
endif()

set(GETOPT ${GLFW_SOURCE_DIR}/deps/getopt.h
           ${GLFW_SOURCE_DIR}/deps/getopt.c)
set(TINYCTHREAD ${GLFW_SOURCE_DIR}/deps/tinycthread.h
                ${GLFW_SOURCE_DIR}/deps/tinycthread.c)

add_executable(clipboard clipboard.c ${GETOPT})
add_executable(defaults defaults.c)
add_executable(events events.c)
add_executable(fsaa fsaa.c ${GETOPT})
add_executable(gamma gamma.c ${GETOPT})
add_executable(glfwinfo glfwinfo.c ${GETOPT})
add_executable(iconify iconify.c ${GETOPT})
add_executable(joysticks joysticks.c)
add_executable(modes modes.c ${GETOPT})
add_executable(peter peter.c)
add_executable(reopen reopen.c)

add_executable(accuracy WIN32 MACOSX_BUNDLE accuracy.c)
set_target_properties(accuracy PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Accuracy")

add_executable(sharing WIN32 MACOSX_BUNDLE sharing.c)
set_target_properties(sharing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Sharing")

add_executable(tearing WIN32 MACOSX_BUNDLE tearing.c)
set_target_properties(tearing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Tearing")

add_executable(threads WIN32 MACOSX_BUNDLE threads.c ${TINYCTHREAD})
set_target_properties(threads PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Threads")

add_executable(title WIN32 MACOSX_BUNDLE title.c)
set_target_properties(title PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Title")

add_executable(windows WIN32 MACOSX_BUNDLE windows.c)
set_target_properties(windows PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Windows")

target_link_libraries(threads ${CMAKE_THREAD_LIBS_INIT} ${RT_LIBRARY})

set(WINDOWS_BINARIES accuracy sharing tearing threads title windows)
set(CONSOLE_BINARIES clipboard defaults events fsaa gamma glfwinfo
                     iconify joysticks modes peter reopen)

if (MSVC)
    # Tell MSVC to use main instead of WinMain for Windows subsystem executables
    set_target_properties(${WINDOWS_BINARIES} ${CONSOLE_BINARIES} PROPERTIES
                          LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

if (APPLE)
    set_target_properties(${WINDOWS_BINARIES} ${CONSOLE_BINARIES} PROPERTIES
                          MACOSX_BUNDLE_SHORT_VERSION_STRING ${GLFW_VERSION}
                          MACOSX_BUNDLE_LONG_VERSION_STRING ${GLFW_VERSION_FULL})
endif()

