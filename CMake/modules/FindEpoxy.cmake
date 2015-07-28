# Find libepoxy
#
# EPOXY_INCLUDE_DIR
# EPOXY_LIBRARY
# EPOXY_FOUND

find_path(EPOXY_INCLUDE_DIR NAMES epoxy/gl.h PATHS /opt/vc/include)

set(EPOXY_NAMES ${EPOXY_NAMES} epoxy)
find_library(EPOXY_LIBRARY NAMES ${EPOXY_NAMES} PATHS /opt/vc/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EPOXY DEFAULT_MSG EPOXY_LIBRARY EPOXY_INCLUDE_DIR)

mark_as_advanced(EPOXY_INCLUDE_DIR EPOXY_LIBRARY)

