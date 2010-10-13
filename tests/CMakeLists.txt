
link_libraries(libglfwStatic ${GLFW_LIBRARIES} ${OPENGL_glu_LIBRARY})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include ${OPENGL_INCLUDE_DIR})

add_executable(defaults defaults.c)
add_executable(events events.c)
add_executable(fsfocus fsfocus.c)
add_executable(gamma gamma.c getopt.c)
add_executable(iconify iconify.c getopt.c)
add_executable(joysticks joysticks.c)
add_executable(listmodes listmodes.c)
add_executable(peter peter.c)
add_executable(reopen reopen.c)
add_executable(version version.c getopt.c)

if(APPLE)
  # Set fancy names for bundles
  add_executable(Accuracy MACOSX_BUNDLE accuracy.c)
  add_executable(FSAA MACOSX_BUNDLE fsaa.c)
  add_executable(Sharing MACOSX_BUNDLE sharing.c)
  add_executable(Tearing MACOSX_BUNDLE tearing.c)
  add_executable(Windows MACOSX_BUNDLE windows.c)
else()
  # Set boring names for executables
  add_executable(accuracy WIN32 accuracy.c)
  add_executable(sharing WIN32 sharing.c)
  add_executable(tearing WIN32 tearing.c)
  add_executable(fsaa WIN32 fsaa.c)
  add_executable(windows WIN32 windows.c)
endif(APPLE)

set(WINDOWS_BINARIES accuracy sharing tearing windows)
set(CONSOLE_BINARIES defaults events fsaa fsfocus gamma iconify joysticks
                     listmodes peter reopen version)

if(MSVC)
  # Tell MSVC to use main instead of WinMain for Windows subsystem executables
  set_target_properties(${WINDOWS_BINARIES} ${CONSOLE_BINARIES} PROPERTIES
                        LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)

if(CYGWIN)
  # Set cross-compile and subsystem compile and link flags
  set_target_properties(${WINDOWS_BINARIES} ${CONSOLE_BINARIES} PROPERTIES
                        COMPILE_FLAGS "-mno-cygwin")
  set_target_properties(${WINDOWS_BINARIES} PROPERTIES
                        LINK_FLAGS "-mno-cygwin -mwindows")
  set_target_properties(${CONSOLE_BINARIES} PROPERTIES
                        LINK_FLAGS "-mno-cygwin -mconsole")
endif(CYGWIN)

