ExternalProject_Add(libdecoration
    GIT_REPOSITORY https://gitlab.gnome.org/jadahl/libdecoration.git
    GIT_TAG master
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/extern/libdecoration"
    CONFIGURE_COMMAND meson --prefix "${CMAKE_CURRENT_BINARY_DIR}/install" --libdir "lib" ../libdecoration
    BUILD_COMMAND ninja
    INSTALL_COMMAND ninja install
)

target_include_directories(glfw PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/install/include")
target_link_libraries(glfw PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/install/lib/libdecoration${CMAKE_SHARED_LIBRARY_SUFFIX})

add_dependencies(glfw libdecoration)
