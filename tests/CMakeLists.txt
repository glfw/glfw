
link_libraries(libglfwStatic ${GLFW_LIBRARIES} ${OPENGL_glu_LIBRARY})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include ${OPENGL_INCLUDE_DIR})

add_executable(defaults defaults.c)
add_executable(events events.c)
add_executable(joysticks joysticks.c)
add_executable(peter peter.c)
add_executable(reopen reopen.c)
add_executable(version version.c)

if(APPLE)
  # Set fancy names for bundles
  add_executable(Accuracy MACOSX_BUNDLE accuracy.c)
  add_executable(FSAA MACOSX_BUNDLE fsaa.c)
  add_executable(Tearing MACOSX_BUNDLE tearing.c)
else()
  # Set boring names for executables
  add_executable(accuracy WIN32 accuracy.c)
  add_executable(tearing WIN32 tearing.c)
  add_executable(fsaa WIN32 fsaa.c)
endif(APPLE)

if(MSVC)
  # Tell MSVC to use main instead of WinMain for Windows subsystem executables
  set_target_properties(accuracy defaults events fsaa peter reopen tearing version PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)

if(CYGWIN)
  # Set cross-compile and subsystem compile and link flags
  set_target_properties(accuracy defaults events fsaa joysticks peter reopen tearing version PROPERTIES COMPILE_FLAGS "-mno-cygwin")
  set_target_properties(accuracy fsaa tearing PROPERTIES LINK_FLAGS "-mno-cygwin -mwindows")
  set_target_properties(events defaults joysticks peter reopen version PROPERTIES LINK_FLAGS "-mno-cygwin -mconsole")
endif(CYGWIN)

