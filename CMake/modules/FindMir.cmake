# Try to find Mir on a Unix system
#
# This will define:
#
#   MIR_LIBRARIES   - Link these to use Wayland
#   MIR_INCLUDE_DIR - Include directory for Wayland
#
# Copyright (c) 2014 Brandon Schaefer <brandon.schaefer@canonical.com>

if (NOT WIN32)

  find_package (PkgConfig)
  pkg_check_modules (PKG_MIR QUIET mirclient)

  set (MIR_INCLUDE_DIR ${PKG_MIR_INCLUDE_DIRS})
  set (MIR_LIBRARIES   ${PKG_MIR_LIBRARIES})

endif ()
