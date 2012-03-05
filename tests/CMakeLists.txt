
set(STATIC_DEPS libglfwStatic ${GLFW_LIBRARIES} ${OPENGL_glu_LIBRARY})
set(SHARED_DEPS libglfwShared ${GLFW_LIBRARIES} ${OPENGL_glu_LIBRARY})

include_directories(${GLFW_SOURCE_DIR}/include
                    ${GLFW_SOURCE_DIR}/support
                    ${OPENGL_INCLUDE_DIR})

add_executable(defaults defaults.c)
target_link_libraries(defaults ${STATIC_DEPS})

add_executable(dynamic dynamic.c)
target_link_libraries(dynamic ${SHARED_DEPS})

add_executable(events events.c)
target_link_libraries(events ${STATIC_DEPS})

add_executable(fsaa fsaa.c getopt.c)
target_link_libraries(fsaa ${STATIC_DEPS})

add_executable(fsfocus fsfocus.c)
target_link_libraries(fsfocus ${STATIC_DEPS})

add_executable(gamma gamma.c getopt.c)
target_link_libraries(gamma ${STATIC_DEPS})

add_executable(glfwinfo glfwinfo.c getopt.c)
target_link_libraries(glfwinfo ${STATIC_DEPS})

add_executable(iconify iconify.c getopt.c)
target_link_libraries(iconify ${STATIC_DEPS})

add_executable(joysticks joysticks.c)
target_link_libraries(joysticks ${STATIC_DEPS})

add_executable(listmodes listmodes.c)
target_link_libraries(listmodes ${STATIC_DEPS})

add_executable(peter peter.c)
target_link_libraries(peter ${STATIC_DEPS})

add_executable(reopen reopen.c)
target_link_libraries(reopen ${STATIC_DEPS})

add_executable(accuracy WIN32 MACOSX_BUNDLE accuracy.c)
target_link_libraries(accuracy ${STATIC_DEPS})
set_target_properties(accuracy PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Accuracy")

add_executable(sharing WIN32 MACOSX_BUNDLE sharing.c)
target_link_libraries(sharing ${STATIC_DEPS})
set_target_properties(sharing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Sharing")

add_executable(tearing WIN32 MACOSX_BUNDLE tearing.c)
target_link_libraries(tearing ${STATIC_DEPS})
set_target_properties(tearing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Tearing")

add_executable(title WIN32 MACOSX_BUNDLE title.c)
target_link_libraries(title ${STATIC_DEPS})
set_target_properties(title PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Title")

add_executable(windows WIN32 MACOSX_BUNDLE windows.c)
target_link_libraries(windows ${STATIC_DEPS})
set_target_properties(windows PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Windows")

set(WINDOWS_BINARIES accuracy sharing tearing title windows)
set(CONSOLE_BINARIES defaults events fsaa fsfocus gamma glfwinfo iconify
                     joysticks listmodes peter reopen)

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

