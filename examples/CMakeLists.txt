# This line is used to link with static libraries 
# Note that the library list should be updated to be obtained from 
# the main CMakeLists.txt 
link_libraries(libglfwStatic ${GLFW_LIBRARIES} ${OPENGL_glu_LIBRARY})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include ${OPENGL_INCLUDE_DIR})

add_executable(listmodes listmodes.c)

if(APPLE)
  # Set fancy names for bundles
  add_executable(Boing MACOSX_BUNDLE boing.c)
  add_executable(Gears MACOSX_BUNDLE gears.c)
  add_executable("Split View" MACOSX_BUNDLE splitview.c)
  add_executable(Triangle MACOSX_BUNDLE triangle.c)
  add_executable(Wave MACOSX_BUNDLE wave.c)
else(APPLE)
  # Set boring names for executables
  add_executable(boing WIN32 boing.c)
  add_executable(gears WIN32 gears.c)
  add_executable(heightmap WIN32 heightmap.c getopt.c)
  add_executable(splitview WIN32 splitview.c)
  add_executable(triangle WIN32 triangle.c)
  add_executable(wave WIN32 wave.c)
endif(APPLE)

set(WINDOWS_BINARIES boing gears heightmap splitview triangle wave)
set(CONSOLE_BINARIES listmodes)

if(MSVC)
  # Tell MSVC to use main instead of WinMain for Windows subsystem executables
  set_target_properties(${WINDOWS_BINARIES} ${CONSOLE_BINARIES}
                        PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)

if(CYGWIN)
  # Set cross-compile and subsystem compile and link flags
  set_target_properties(${WINDOWS_BINARIES} ${CONSOLE_BINARIES}
                        PROPERTIES COMPILE_FLAGS "-mno-cygwin")
  set_target_properties(${WINDOWS_BINARIES}
                        PROPERTIES LINK_FLAGS "-mno-cygwin -mwindows")
  set_target_properties(${CONSOLE_BINARIES}
                        PROPERTIES LINK_FLAGS "-mno-cygwin -mconsole")
endif(CYGWIN)

