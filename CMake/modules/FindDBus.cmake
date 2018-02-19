# Find libdbus
#
# DBUS_INCLUDE_DIRS
# DBUS_LIBRARY
# DBUS_FOUND

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)

    pkg_check_modules(PC_DBUS dbus-1 QUIET)
    if (PC_DBUS_FOUND)
        set(DBUS_INCLUDE_DIRS ${PC_DBUS_INCLUDE_DIRS})
        find_library(DBUS_LIBRARY dbus-1 PATHS ${PC_DBUS_LIBRARY_DIRS})

        include(FindPackageHandleStandardArgs)
        find_package_handle_standard_args(DBus DEFAULT_MSG DBUS_LIBRARY DBUS_INCLUDE_DIRS)
        mark_as_advanced(DBUS_INCLUDE_DIRS DBUS_LIBRARY)
    endif()

endif()

