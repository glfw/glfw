INIT_BIICODE_BLOCK()

#Biicode defines its own targets. Include src CMakeLists to load variables
#with the adequate source files depending on the system

FOREACH(var ${BII_LIB_SRC})
    STRING(REGEX MATCH "deps/.*\\.c" item ${var})
    IF(item)
        LIST(APPEND glfw_deps_SOURCES ${var})
    ENDIF()
ENDFOREACH()
SET(BII_LIB_SRC)
SET(BII_LIB_DEPS glfw ${glfw_LIBRARIES})

if(glfw_deps_SOURCES)
    SET(BII_BLOCK_TARGET "${BII_BLOCK_USER}_${BII_BLOCK_NAME}_interface")
    ADD_LIBRARY(glfw_deps ${glfw_deps_SOURCES})
    target_include_directories(glfw_deps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps)
    SET(BII_LIB_DEPS ${BII_LIB_DEPS} glfw_deps)
endif()

# This defines the library (static), tests and examples executables
ADD_BIICODE_TARGETS()

# to make transitive the location of includes
target_include_directories(${BII_LIB_TARGET} INTERFACE ${GLFW_BINARY_DIR}/src)
