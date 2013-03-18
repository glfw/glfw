
add_custom_target(docs ${DOXYGEN_EXECUTABLE}
                  WORKING_DIRECTORY ${GLFW_BINARY_DIR}/docs
                  COMMENT "Generating HTML documentation" VERBATIM)

