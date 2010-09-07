# - Check if X11 RandR extension is available 
# Check if the X11 extension RandR is available. 
# This macro defines : 
#   - X11_RANDR_FOUND, If set to NO RandR is not available.
#   - X11_RANDR_INCLUDE_DIR, includes directory containing the RandR header.
#   - X11_RANDR_LIBRARIES, libraries to link in the library to use RandR.
#
# Created by Olivier Delannoy. 
macro(CHECK_X11_XRANDR)
    message(STATUS "Checking for X11 extension XRandR")
    set(X11_XRANDR_FOUND "NO")
    find_path(X11_XRANDR_INCLUDE_DIR "X11/extensions/Xrandr.h" 
        PATHS
        /usr/local/include
        /usr/local/X11/include
        /usr/local/X11R6/include
        /usr/include
        /usr/X11/include
        /usr/X11R6/include)

    find_library(X11_XRANDR_LIBRARIES NAMES Xrandr 
        PATHS
        /usr/local/lib
        /usr/local/X11/lib
        /usr/local/X11R6/lib
        /usr/lib
        /usr/X11/lib
        /usr/X11R6/lib)
    # Create check if file compiles with randr 

    if (X11_XRANDR_LIBRARIES AND X11_XRANDR_INCLUDE_DIR) 
        set(X11_XRANDR_FOUND "YES")
    endif (X11_XRANDR_LIBRARIES AND X11_XRANDR_INCLUDE_DIR)

    if (X11_XRANDR_FOUND) 
        message(STATUS "Checking for X11 extension XRandR -- found")
    else (X11_XRANDR_FOUND)
        message(STATUS "Checking for X11 extension XRandR -- not found")
    endif (X11_XRANDR_FOUND) 

    mark_as_advanced(X11_XRANDR_LIBRARIES X11_XRANDR_INCLUDE_DIR)
endmacro(CHECK_X11_XRANDR)


# - Check if X11 VidMod extension is available 
# Check if the X11 extension VidMod is available. 
# This macro defines : 
#   - X11_VIDMOD_FOUND, If set to NO VidMod is not available.
#   - X11_VIDMOD_INCLUDE_DIR, includes directory containing the headers.
#   - X11_VIDMOD_LIBRARIES, libraries to link in the libraries.
#
# Created by Olivier Delannoy. 
macro(CHECK_X11_XF86VIDMODE)
    message(STATUS "Checking for X11 extension xf86vidmode")
    set(X11_XF86VIDMODE_FOUND "NO")
    find_path(X11_XF86VIDMODE_INCLUDE_DIR "X11/extensions/xf86vmode.h" 
        PATHS
        /usr/local/include
        /usr/local/X11/include
        /usr/local/X11R6/include
        /usr/include
        /usr/X11/include
        /usr/X11R6/include)

    find_library(X11_XF86VIDMODE_LIBRARIES NAMES Xxf86vm PATHS
        /usr/local/lib
        /usr/local/X11/lib
        /usr/local/X11R6/lib
        /usr/lib
        /usr/X11/lib
        /usr/X11R6/lib)
    # Add a test case here 
    if (X11_XF86VIDMODE_LIBRARIES AND X11_XF86VIDMODE_INCLUDE_DIR)
        set(X11_XF86VIDMODE_FOUND "YES")
    endif (X11_XF86VIDMODE_LIBRARIES AND X11_XF86VIDMODE_INCLUDE_DIR)

    if (X11_XF86VIDMODE_FOUND)
        message(STATUS "Checking for X11 extension xf86vidmode -- found")
    else (X11_XF86VIDMODE_FOUND)
        message(STATUS "Checking for X11 extension xf86vidmode -- not found")
    endif(X11_XF86VIDMODE_FOUND)

    mark_as_advanced(
        X11_XF86VIDMODE_LIBRARIES 
        X11_XF86VIDMODE_INCLUDE_DIR
    )

endmacro(CHECK_X11_XF86VIDMODE)
