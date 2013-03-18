
add_custom_target(docs ALL ${DOXYGEN_EXECUTABLE}
                  WORKING_DIRECTORY ${GLFW_BINARY_DIR}/docs
                  COMMENT "Generating HTML documentation" VERBATIM)

