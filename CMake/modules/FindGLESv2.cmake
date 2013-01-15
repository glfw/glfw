# Find GLESv2
#
# GLESv2_INCLUDE_DIR
# GLESv2_LIBRARY
# GLESv2_FOUND

find_path(GLESv2_INCLUDE_DIR NAMES GLES2/gl2.h)

set(GLESv2_NAMES ${GLESv2_NAMES} GLESv2)
find_library(GLESv2_LIBRARY NAMES ${GLESv2_NAMES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLESv2 DEFAULT_MSG GLESv2_LIBRARY GLESv2_INCLUDE_DIR)

mark_as_advanced(GLESv2_INCLUDE_DIR GLESv2_LIBRARY)

