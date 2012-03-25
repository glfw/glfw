
link_libraries(glfw ${OPENGL_glu_LIBRARY})

if (BUILD_SHARED_LIBS)
  add_definitions(-DGLFW_DLL)
else()
  link_libraries(${GLFW_LIBRARIES})
endif()

include_directories(${GLFW_SOURCE_DIR}/include
                    ${GLFW_SOURCE_DIR}/support
                    ${OPENGL_INCLUDE_DIR})

add_executable(defaults defaults.c)
add_executable(events events.c)
add_executable(fsaa fsaa.c getopt.c)
add_executable(fsfocus fsfocus.c)
add_executable(gamma gamma.c getopt.c)
add_executable(glfwinfo glfwinfo.c getopt.c)
add_executable(iconify iconify.c getopt.c)
add_executable(joysticks joysticks.c)
add_executable(listmodes listmodes.c)
add_executable(modes modes.c getopt.c)
add_executable(peter peter.c)
add_executable(reopen reopen.c)

add_executable(accuracy WIN32 MACOSX_BUNDLE accuracy.c)
set_target_properties(accuracy PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Accuracy")

add_executable(sharing WIN32 MACOSX_BUNDLE sharing.c)
set_target_properties(sharing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Sharing")

add_executable(tearing WIN32 MACOSX_BUNDLE tearing.c)
set_target_properties(tearing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Tearing")

add_executable(title WIN32 MACOSX_BUNDLE title.c)
set_target_properties(title PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Title")

add_executable(windows WIN32 MACOSX_BUNDLE windows.c)
set_target_properties(windows PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Windows")

set(WINDOWS_BINARIES accuracy sharing tearing title windows)
set(CONSOLE_BINARIES defaults events fsaa fsfocus gamma glfwinfo iconify
                     joysticks listmodes modes peter reopen)

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

