
link_libraries(glfw)

if (BUILD_SHARED_LIBS)
    add_definitions(-DGLFW_DLL)
    link_libraries("${OPENGL_gl_LIBRARY}" "${MATH_LIBRARY}")
else()
    link_libraries(${glfw_LIBRARIES})
endif()

include_directories("${GLFW_SOURCE_DIR}/include"
                    "${GLFW_SOURCE_DIR}/deps")

if ("${OPENGL_INCLUDE_DIR}")
    include_directories("${OPENGL_INCLUDE_DIR}")
endif()

set(GLAD "${GLFW_SOURCE_DIR}/deps/glad/glad.h"
         "${GLFW_SOURCE_DIR}/deps/glad.c")
set(GETOPT "${GLFW_SOURCE_DIR}/deps/getopt.h"
           "${GLFW_SOURCE_DIR}/deps/getopt.c")
set(TINYCTHREAD "${GLFW_SOURCE_DIR}/deps/tinycthread.h"
                "${GLFW_SOURCE_DIR}/deps/tinycthread.c")

add_executable(boing WIN32 MACOSX_BUNDLE boing.c)
add_executable(gears WIN32 MACOSX_BUNDLE gears.c)
add_executable(heightmap WIN32 MACOSX_BUNDLE heightmap.c ${GLAD})
add_executable(particles WIN32 MACOSX_BUNDLE particles.c ${TINYCTHREAD} ${GETOPT})
add_executable(simple WIN32 MACOSX_BUNDLE simple.c)
add_executable(splitview WIN32 MACOSX_BUNDLE splitview.c)
add_executable(wave WIN32 MACOSX_BUNDLE wave.c)

target_link_libraries(particles "${CMAKE_THREAD_LIBS_INIT}" "${RT_LIBRARY}")

set(WINDOWS_BINARIES boing gears heightmap particles simple splitview wave)

set_target_properties(${WINDOWS_BINARIES} PROPERTIES FOLDER "GLFW3/Examples")

if (MSVC)
    # Tell MSVC to use main instead of WinMain for Windows subsystem executables
    set_target_properties(${WINDOWS_BINARIES} PROPERTIES
                          LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

if (APPLE)
    set_target_properties(boing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Boing")
    set_target_properties(gears PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Gears")
    set_target_properties(heightmap PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Heightmap")
    set_target_properties(particles PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Particles")
    set_target_properties(simple PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Simple")
    set_target_properties(splitview PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Split View")
    set_target_properties(wave PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Wave")

    set_target_properties(${WINDOWS_BINARIES} PROPERTIES
                          MACOSX_BUNDLE_SHORT_VERSION_STRING ${GLFW_VERSION}
                          MACOSX_BUNDLE_LONG_VERSION_STRING ${GLFW_VERSION_FULL}
                          MACOSX_BUNDLE_INFO_PLIST "${GLFW_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in")
endif()

