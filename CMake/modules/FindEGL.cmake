# Find EGL
#
# EGL_INCLUDE_DIR
# EGL_LIBRARY
# EGL_FOUND

find_path(EGL_INCLUDE_DIR NAMES EGL/egl.h)

set(EGL_NAMES ${EGL_NAMES} egl EGL)
find_library(EGL_LIBRARY NAMES ${EGL_NAMES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EGL DEFAULT_MSG EGL_LIBRARY EGL_INCLUDE_DIR)

mark_as_advanced(EGL_INCLUDE_DIR EGL_LIBRARY)

