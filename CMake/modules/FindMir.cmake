# Try to find Mir on a Unix system
#
# This will define:
#
#   MIR_FOUND       - System has Mir
#   MIR_LIBRARIES   - Link these to use Mir
#   MIR_INCLUDE_DIR - Include directory for Mir
#   MIR_DEFINITIONS - Compiler switches required for using Mir

if (NOT WIN32)

  find_package (PkgConfig)
  pkg_check_modules (PKG_MIR QUIET mirclient)
  set(MIR_DEFINITIONS ${PKG_MIR_CFLAGS_OTHER})

  find_path(MIR_INCLUDE_DIR
      NAMES xkbcommon/xkbcommon.h
      HINTS ${PC_XKBCOMMON_INCLUDE_DIR} ${PC_XKBCOMMON_INCLUDE_DIRS}
  )

  find_library(MIR_LIBRARY
      NAMES mirclient
      HINTS ${PKG_MIR_LIBRARIES} ${MIR_LIBRARY_DIRS}
  )

  set (MIR_INCLUDE_DIR ${PKG_MIR_INCLUDE_DIRS})
  set (MIR_LIBRARIES   ${MIR_LIBRARY})

  include (FindPackageHandleStandardArgs)
  find_package_handle_standard_args (MIR DEFAULT_MSG
      MIR_LIBRARIES
      MIR_INCLUDE_DIR
  )

  mark_as_advanced (MIR_LIBRARIES MIR_INCLUDE_DIR)

endif ()
