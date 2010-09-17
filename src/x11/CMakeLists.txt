
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/libglfw.pc.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/libglfw.pc @ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} 
                    ${GLFW_SOURCE_DIR}/src
                    ${GLFW_BINARY_DIR}/src
                    ${GLFW_INCLUDE_DIR})

set(libglfw_SOURCES
    ${common_SOURCES}
    x11_enable.c 
    x11_fullscreen.c
    x11_glext.c
    x11_init.c 
    x11_joystick.c
    x11_keysym2unicode.c
    x11_time.c 
    x11_window.c)

add_library(libglfwStatic STATIC ${libglfw_SOURCES})
add_library(libglfwShared SHARED ${libglfw_SOURCES}) 
target_link_libraries(libglfwShared  ${GLFW_LIBRARIES})
set_target_properties(libglfwStatic libglfwShared PROPERTIES 
    CLEAN_DIRECT_OUTPUT 1
    OUTPUT_NAME glfw
)

install(TARGETS libglfwStatic libglfwShared DESTINATION lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libglfw.pc DESTINATION lib/pkgconfig)

