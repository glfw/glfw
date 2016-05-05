
link_libraries(glfw)

if (BUILD_SHARED_LIBS)
    link_libraries("${MATH_LIBRARY}")
endif()

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories("${GLFW_SOURCE_DIR}/deps")

set(GLAD "${GLFW_SOURCE_DIR}/deps/glad/glad.h"
         "${GLFW_SOURCE_DIR}/deps/glad.c")
set(GETOPT "${GLFW_SOURCE_DIR}/deps/getopt.h"
           "${GLFW_SOURCE_DIR}/deps/getopt.c")
set(TINYCTHREAD "${GLFW_SOURCE_DIR}/deps/tinycthread.h"
                "${GLFW_SOURCE_DIR}/deps/tinycthread.c")

add_executable(clipboard clipboard.c ${GETOPT} ${GLAD})
add_executable(events events.c ${GETOPT} ${GLAD})
add_executable(msaa msaa.c ${GETOPT} ${GLAD})
add_executable(gamma gamma.c ${GETOPT} ${GLAD})
add_executable(glfwinfo glfwinfo.c ${GETOPT} ${GLAD})
add_executable(iconify iconify.c ${GETOPT} ${GLAD})
add_executable(joysticks joysticks.c ${GLAD})
add_executable(monitors monitors.c ${GETOPT} ${GLAD})
add_executable(reopen reopen.c ${GLAD})
add_executable(cursor cursor.c ${GLAD})

add_executable(empty WIN32 MACOSX_BUNDLE empty.c ${TINYCTHREAD} ${GLAD})
add_executable(icon WIN32 MACOSX_BUNDLE icon.c ${GLAD})
add_executable(sharing WIN32 MACOSX_BUNDLE sharing.c ${GLAD})
add_executable(tearing WIN32 MACOSX_BUNDLE tearing.c ${GETOPT} ${GLAD})
add_executable(threads WIN32 MACOSX_BUNDLE threads.c ${TINYCTHREAD} ${GLAD})
add_executable(timeout WIN32 MACOSX_BUNDLE timeout.c ${GLAD})
add_executable(title WIN32 MACOSX_BUNDLE title.c ${GLAD})
add_executable(windows WIN32 MACOSX_BUNDLE windows.c ${GETOPT} ${GLAD})

target_link_libraries(empty "${CMAKE_THREAD_LIBS_INIT}" "${RT_LIBRARY}")
target_link_libraries(threads "${CMAKE_THREAD_LIBS_INIT}" "${RT_LIBRARY}")

set(WINDOWS_BINARIES empty icon sharing tearing threads timeout title windows)
set(CONSOLE_BINARIES clipboard events msaa gamma glfwinfo
                     iconify joysticks monitors reopen cursor)

if (VULKAN_FOUND)
    add_executable(vulkan WIN32 vulkan.c ${ICON})
    target_include_directories(vulkan PRIVATE "${VULKAN_INCLUDE_DIR}")
    target_link_libraries(vulkan "${VULKAN_LIBRARY}")
    list(APPEND WINDOWS_BINARIES vulkan)
endif()

set_target_properties(${WINDOWS_BINARIES} ${CONSOLE_BINARIES} PROPERTIES
                      FOLDER "GLFW3/Tests")

if (MSVC)
    # Tell MSVC to use main instead of WinMain for Windows subsystem executables
    set_target_properties(${WINDOWS_BINARIES} ${CONSOLE_BINARIES} PROPERTIES
                          LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

if (APPLE)
    set_target_properties(empty PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Empty Event")
    set_target_properties(sharing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Sharing")
    set_target_properties(tearing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Tearing")
    set_target_properties(threads PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Threads")
    set_target_properties(timeout PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Timeout")
    set_target_properties(title PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Title")
    set_target_properties(windows PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Windows")

    set_target_properties(${WINDOWS_BINARIES} ${CONSOLE_BINARIES} PROPERTIES
                          MACOSX_BUNDLE_SHORT_VERSION_STRING ${GLFW_VERSION}
                          MACOSX_BUNDLE_LONG_VERSION_STRING ${GLFW_VERSION_FULL}
                          MACOSX_BUNDLE_INFO_PLIST "${GLFW_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in")
endif()

