if (DEFINED glfw_msvc_runtime_library_include_guard)
    return()
endif()
set(glfw_msvc_runtime_library_include_guard ON)

include(CMakeDependentOption)

macro(glfw_msvc_runtime_library GLFW_STANDALONE)
    # Prior to cmake 3.15 there wasn't a very good way to handle building against the static/dyanmic
    # MSVC runtime libraries. As of cmake 3.15 this is no longer the case and clients can use the
    # official cmake variable CMAKE_MSVC_RUNTIME_LIBRARY:
    # https://cmake.org/cmake/help/latest/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html#variable:CMAKE_MSVC_RUNTIME_LIBRARY
    #
    # The new way has better support for multi-configuration generators and clang-cl. So if clients are interested in
    # these features/functionality they should be using a newer version of cmake.
    #
    # However for backwards compatibility keep the old way.
    if (${CMAKE_VERSION} VERSION_LESS "3.15")
        cmake_dependent_option(USE_MSVC_RUNTIME_LIBRARY_DLL "Use MSVC runtime library DLL" ON "MSVC" OFF)

        #--------------------------------------------------------------------
        # Set compiler specific flags
        #--------------------------------------------------------------------
        if (MSVC)
            if (NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
                foreach (flag CMAKE_C_FLAGS
                            CMAKE_C_FLAGS_DEBUG
                            CMAKE_C_FLAGS_RELEASE
                            CMAKE_C_FLAGS_MINSIZEREL
                            CMAKE_C_FLAGS_RELWITHDEBINFO)

                    if (${flag} MATCHES "/MD")
                        string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
                    endif()
                    if (${flag} MATCHES "/MDd")
                        string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
                    endif()

                endforeach()
            endif()
        endif()
    else()
        # If building GLFW standalone just use the static libraries.
        # It simplifies working/deploying on test machines.
        if (${GLFW_STANDALONE})
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            # Otherwise the client should specify this critical build variable.
            # When compiling for the WIN32 platform gives clients a helpful warning about what they should do.
            if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY AND WIN32)
                message(AUTHOR_WARNING "GLFW: User has not defined CMAKE_MSVC_RUNTIME_LIBRARY.")
            endif()
        endif()
    endif()
endmacro()
