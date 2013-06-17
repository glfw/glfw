
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

if (APPLE)
    # Set fancy names for bundles
    add_executable(Boing MACOSX_BUNDLE boing.c)
    add_executable(Gears MACOSX_BUNDLE gears.c)
    add_executable(Simple MACOSX_BUNDLE simple.c)
    add_executable("Split View" MACOSX_BUNDLE splitview.c)
    add_executable(Wave MACOSX_BUNDLE wave.c)

    set_target_properties(Boing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Boing")
    set_target_properties(Gears PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Gears")
    set_target_properties(Simple PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Simple")
    set_target_properties("Split View" PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Split View")
    set_target_properties(Wave PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Wave")
else()
    # Set boring names for executables
    add_executable(boing WIN32 boing.c)
    add_executable(gears WIN32 gears.c)
    add_executable(heightmap WIN32 heightmap.c ${GETOPT})
    add_executable(simple WIN32 simple.c)
    add_executable(splitview WIN32 splitview.c)
    add_executable(wave WIN32 wave.c)
endif()

if (MSVC)
    set(WINDOWS_BINARIES boing gears heightmap simple splitview wave)

    # Tell MSVC to use main instead of WinMain for Windows subsystem executables
    set_target_properties(${WINDOWS_BINARIES} PROPERTIES
                          LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

if (APPLE)
    set(BUNDLE_BINARIES Boing Gears Simple "Split View" Wave)

    set_target_properties(${BUNDLE_BINARIES} PROPERTIES
                          MACOSX_BUNDLE_SHORT_VERSION_STRING ${GLFW_VERSION}
                          MACOSX_BUNDLE_LONG_VERSION_STRING ${GLFW_VERSION_FULL})
endif()

