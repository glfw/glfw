project('glfw', 'c',
    version: '3.3.0',
    license: 'zlib')

cc = meson.get_compiler('c')
compiler_id = cc.get_id()

math_dep = cc.find_library('m', required: false)
rt_dep = cc.find_library('rt', required: false)
threads_dep = dependency('threads')


#--------------------------------------------------------------------
# Set compiler specific flags
#--------------------------------------------------------------------

if compiler_id == 'msvc'
    if get_option('glfw_use_msvc_runtime_library_dll')
        #foreach (flag CMAKE_C_FLAGS
        #              CMAKE_C_FLAGS_DEBUG
        #              CMAKE_C_FLAGS_RELEASE
        #              CMAKE_C_FLAGS_MINSIZEREL
        #              CMAKE_C_FLAGS_RELWITHDEBINFO)
        #    if (${flag} MATCHES "/MD")
        #        string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
        #    endif()
        #    if (${flag} MATCHES "/MDd")
        #        string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
        #    endif()
        #endforeach()
    endif
endif


#--------------------------------------------------------------------
# Detect and select backend APIs
#--------------------------------------------------------------------

host_system = host_machine.system()

backend = get_option('glfw_backend')
if backend == 'auto'
    if host_system == 'windows' or host_system == 'cygwin'
        backend = 'win32'
    elif host_system == 'darwin'
        backend = 'cocoa'
    elif host_system == 'linux' or host_system == 'bsd'
        backend = 'x11'
    else
        error('Unable to determine the backend automatically.')
    endif
endif


#--------------------------------------------------------------------
# Add subdirectories
#--------------------------------------------------------------------

subdir('deps')
subdir('src')

if get_option('glfw_build_tests')
    subdir('tests')
endif

if get_option('glfw_build_examples')
    subdir('examples')
endif

if get_option('glfw_build_docs')
    subdir('docs')
endif


#--------------------------------------------------------------------
# Install files other than the library
# The library is installed by src/CMakeLists.txt
#--------------------------------------------------------------------

#if (GLFW_INSTALL)
#    install(DIRECTORY include/GLFW DESTINATION include
#            FILES_MATCHING PATTERN glfw3.h PATTERN glfw3native.h)
#
#    install(FILES "${GLFW_BINARY_DIR}/src/glfw3Config.cmake"
#                  "${GLFW_BINARY_DIR}/src/glfw3ConfigVersion.cmake"
#            DESTINATION "${GLFW_CONFIG_PATH}")
#
#    install(EXPORT glfwTargets FILE glfw3Targets.cmake
#            EXPORT_LINK_INTERFACE_LIBRARIES
#            DESTINATION "${GLFW_CONFIG_PATH}")
#    install(FILES "${GLFW_BINARY_DIR}/src/glfw3.pc"
#            DESTINATION "lib${LIB_SUFFIX}/pkgconfig")
#
#    # Only generate this target if no higher-level project already has
#    if (NOT TARGET uninstall)
#        configure_file(cmake_uninstall.cmake.in
#                       cmake_uninstall.cmake IMMEDIATE @ONLY)
#
#        add_custom_target(uninstall
#                          "${CMAKE_COMMAND}" -P
#                          "${GLFW_BINARY_DIR}/cmake_uninstall.cmake")
#    endif()
#endif()


# vim: set softtabstop=4 shiftwidth=4:
