
set(glfw_DOCS_SOURCES
    "${GLFW_SOURCE_DIR}/include/GLFW/glfw3.h"
    "${GLFW_SOURCE_DIR}/include/GLFW/glfw3native.h"
    "${GLFW_SOURCE_DIR}/docs/main.dox"
    "${GLFW_SOURCE_DIR}/docs/news.dox"
    "${GLFW_SOURCE_DIR}/docs/quick.dox"
    "${GLFW_SOURCE_DIR}/docs/moving.dox"
    "${GLFW_SOURCE_DIR}/docs/compile.dox"
    "${GLFW_SOURCE_DIR}/docs/build.dox"
    "${GLFW_SOURCE_DIR}/docs/intro.dox"
    "${GLFW_SOURCE_DIR}/docs/context.dox"
    "${GLFW_SOURCE_DIR}/docs/monitor.dox"
    "${GLFW_SOURCE_DIR}/docs/window.dox"
    "${GLFW_SOURCE_DIR}/docs/input.dox"
    "${GLFW_SOURCE_DIR}/docs/vulkan.dox"
    "${GLFW_SOURCE_DIR}/docs/compat.dox"
    "${GLFW_SOURCE_DIR}/docs/internal.dox")

foreach(arg ${glfw_DOCS_SOURCES})
    set(GLFW_DOCS_SOURCES "${GLFW_DOCS_SOURCES} \\\n\"${arg}\"")
endforeach()

configure_file(Doxyfile.in Doxyfile @ONLY)

add_custom_target(docs ALL "${DOXYGEN_EXECUTABLE}"
                  WORKING_DIRECTORY "${GLFW_BINARY_DIR}/docs"
                  COMMENT "Generating HTML documentation" VERBATIM)

