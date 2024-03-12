# Release notes for version 3.4 {#news}

[TOC]


## New features {#features}

### Runtime platform selection {#runtime_platform_selection}

GLFW now supports being compiled for multiple backends and selecting between
them at runtime with the @ref GLFW_PLATFORM init hint.  After initialization the
selected platform can be queried with @ref glfwGetPlatform.  You can check if
support for a given platform is compiled in with @ref glfwPlatformSupported.

For more information see @ref platform.


### More standard cursor shapes {#more_cursor_shapes}

GLFW now provides the standard cursor shapes @ref GLFW_RESIZE_NWSE_CURSOR and
@ref GLFW_RESIZE_NESW_CURSOR for diagonal resizing, @ref GLFW_RESIZE_ALL_CURSOR
for omnidirectional resizing and @ref GLFW_NOT_ALLOWED_CURSOR for showing an
action is not allowed.

Unlike the original set, these shapes may not be available everywhere and
creation will then fail with the new @ref GLFW_CURSOR_UNAVAILABLE error.

The cursors for horizontal and vertical resizing are now referred to as @ref
GLFW_RESIZE_EW_CURSOR and @ref GLFW_RESIZE_NS_CURSOR, and the pointing hand
cursor is now referred to as @ref GLFW_POINTING_HAND_CURSOR.  The older names
are still available.

For more information see @ref cursor_standard.


### Mouse event passthrough {#mouse_input_passthrough}

GLFW now provides the [GLFW_MOUSE_PASSTHROUGH](@ref GLFW_MOUSE_PASSTHROUGH_hint)
window hint for making a window transparent to mouse input, lettings events pass
to whatever window is behind it.  This can also be changed after window
creation with the matching [window attribute](@ref GLFW_MOUSE_PASSTHROUGH_attrib).


### Ability to get window title {#window_title_function}

GLFW now supports querying the title of a window with the @ref glfwGetWindowTitle
function.

For more information see @ref window_title.


### Captured cursor mode {#captured_cursor_mode}

GLFW now supports confining the cursor to the window content area with the @ref
GLFW_CURSOR_CAPTURED cursor mode.

For more information see @ref cursor_mode.


### Support for custom heap memory allocator {#custom_heap_allocator}

GLFW now supports plugging a custom heap memory allocator at initialization with
@ref glfwInitAllocator.  The allocator is a struct of type @ref GLFWallocator
with function pointers corresponding to the standard library functions `malloc`,
`realloc` and `free`.

For more information see @ref init_allocator.


### Window hint for framebuffer scaling {#scale_framebuffer_hint}

GLFW now allows provides the
[GLFW_SCALE_FRAMEBUFFER](@ref GLFW_SCALE_FRAMEBUFFER_hint) window hint for
controlling framebuffer scaling on platforms that handle scaling by keeping the
window size the same while resizing the framebuffer.  The default value is to
allow framebuffer scaling.

This was already possible on macOS via the
[GLFW_COCOA_RETINA_FRAMEBUFFER](@ref GLFW_COCOA_RETINA_FRAMEBUFFER_hint) window
hint.  This is now another name for the same hint value.

For more information see @ref window_scale.


### Window hints for initial window position {#window_position_hint}

GLFW now provides the @ref GLFW_POSITION_X and @ref GLFW_POSITION_Y window hints for
specifying the initial position of the window.  This removes the need to create a hidden
window, move it and then show it.  The default value of these hints is
`GLFW_ANY_POSITION`, which selects the previous behavior.

For more information see @ref window_pos.


### ANGLE rendering backend hint {#angle_renderer_hint}

GLFW now provides the
[GLFW_ANGLE_PLATFORM_TYPE](@ref GLFW_ANGLE_PLATFORM_TYPE_hint) init hint for
requesting a specific rendering backend when using [ANGLE][] to create OpenGL ES
contexts.

[ANGLE]: https://chromium.googlesource.com/angle/angle/


### Windows window menu keyboard access hint {#win32_keymenu_hint}

GLFW now provides the
[GLFW_WIN32_KEYBOARD_MENU](@ref GLFW_WIN32_KEYBOARD_MENU_hint) window hint for
enabling keyboard access to the window menu via the Alt+Space and
Alt-and-then-Space shortcuts.  This may be useful for more GUI-oriented
applications.


### Windows STARTUPINFO show command hint {#win32_showdefault_hint}

GLFW now provides the [GLFW_WIN32_SHOWDEFAULT](@ref GLFW_WIN32_SHOWDEFAULT_hint) window
hint for applying the show command in the program's `STARTUPINFO` when showing the window
for the first time.  This may be useful for the main window of a windowed-mode tool.


### Cocoa NSView native access function {#cocoa_nsview_function}

GLFW now provides the @ref glfwGetCocoaView native access function
for returning the Cocoa NSView.


### Wayland libdecor decorations {#wayland_libdecor_decorations}

GLFW now supports improved client-side window decorations via
[libdecor](https://gitlab.freedesktop.org/libdecor/libdecor).  This provides
fully featured window decorations on desktop environments like GNOME.

Support for libdecor can be toggled before GLFW is initialized with the
[GLFW_WAYLAND_LIBDECOR](@ref GLFW_WAYLAND_LIBDECOR_hint) init hint.  It is
enabled by default.

This feature has also been available in GLFW 3.3 since 3.3.9.


### Wayland surface app_id hint {#wayland_app_id_hint}

GLFW now supports specifying the app_id for a Wayland window using the
[GLFW_WAYLAND_APP_ID](@ref GLFW_WAYLAND_APP_ID_hint) window hint string.


### X11 Vulkan window surface hint {#x11_xcb_vulkan_surface}

GLFW now supports disabling the use of `VK_KHR_xcb_surface` over
`VK_KHR_xlib_surface` where available, with the
[GLFW_X11_XCB_VULKAN_SURFACE](@ref GLFW_X11_XCB_VULKAN_SURFACE_hint) init hint.
This affects @ref glfwGetRequiredInstanceExtensions and @ref
glfwCreateWindowSurface.


## Caveats {#caveats}

### Multiple sets of native access functions {#multiplatform_caveat}

Because GLFW now supports runtime selection of platform (window system), a library binary
may export native access functions for multiple platforms.  Starting with version 3.4 you
must not assume that GLFW is running on a platform just because it exports native access
functions for it.  After initialization, you can query the selected platform with @ref
glfwGetPlatform.


### Version string format has been changed {#version_string_caveat}

Because GLFW now supports runtime selection of platform (window system), the version
string returned by @ref glfwGetVersionString has been expanded.  It now contains the names
of all APIs for all the platforms that the library binary supports.

The version string is intended for bug reporting and should not be parsed.  See
@ref glfwGetVersion and @ref glfwPlatformSupported instead.


### Joystick support is initialized on demand {#joystick_init_caveat}

The joystick part of GLFW is now initialized when first used, primarily to work
around faulty Windows drivers that cause DirectInput to take up to several
seconds to enumerate devices.

This change is mostly not observable.  However, if your application waits for
events without having first called any joystick function or created any visible
windows, the wait may never unblock as GLFW may not yet have subscribed to
joystick related OS events.

To work around this, call any joystick function before waiting for events, for
example by setting a [joystick callback](@ref joystick_event).


### Tests and examples are disabled when built as a subproject {#standalone_caveat}

GLFW now by default does not build the tests or examples when it is added as
a subdirectory of another CMake project.  If you were setting @ref
GLFW_BUILD_TESTS or @ref GLFW_BUILD_EXAMPLES to false in your CMake files, you
can now remove this.

If you do want these to be built, set @ref GLFW_BUILD_TESTS and @ref
GLFW_BUILD_EXAMPLES in your CMake files before adding the GLFW subdirectory.

```cmake
set(GLFW_BUILD_EXAMPLES ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS ON CACHE BOOL "" FORCE)
add_subdirectory(path/to/glfw)
```


### Configuration header is no longer generated {#config_header_caveat}

The `glfw_config.h` configuration header is no longer generated by CMake and the
platform selection macros are now part of the GLFW CMake target.  The
`_GLFW_USE_CONFIG_H` macro is still supported in case you are generating
a configuration header in a custom build setup.


### Documentation generation requires Doxygen 1.9.8 or later {#docs_target_caveat}

Doxygen 1.9.8 or later is now required for the `docs` CMake target to be
generated.  This is because the documentation now uses more of the Markdown
support in Doxygen and this support has until recently been relatively unstable.


### Windows 7 framebuffer transparency requires DWM transparency {#win7_framebuffer_caveat}

GLFW no longer supports per-pixel framebuffer transparency via @ref
GLFW_TRANSPARENT_FRAMEBUFFER on Windows 7 if DWM transparency is off
(the Transparency setting under Personalization > Window Color).


### macOS main menu now created at initialization {#macos_menu_caveat}

GLFW now creates the main menu and completes the initialization of NSApplication
during initialization.  Programs that do not want a main menu can disable it
with the [GLFW_COCOA_MENUBAR](@ref GLFW_COCOA_MENUBAR_hint) init hint.


### macOS CoreVideo dependency has been removed {#corevideo_caveat}

GLFW no longer depends on the CoreVideo framework on macOS and it no longer
needs to be specified during compilation or linking.


### Wayland framebuffer may lack alpha channel on older systems {#wayland_alpha_caveat}

On Wayland, when creating an EGL context on a machine lacking the new
`EGL_EXT_present_opaque` extension, the @ref GLFW_ALPHA_BITS window hint will be
ignored and the framebuffer will not have an alpha channel.  This is because
some Wayland compositors treat any buffer with an alpha channel as per-pixel
transparent.

If you want a per-pixel transparent window, see the
[GLFW_TRANSPARENT_FRAMEBUFFER](@ref GLFW_TRANSPARENT_FRAMEBUFFER_hint) window
hint.


### X11 empty events no longer round-trip to server {#x11_emptyevent_caveat}

Events posted with @ref glfwPostEmptyEvent now use a separate unnamed pipe
instead of sending an X11 client event to the helper window.


## Deprecations {#deprecations}

### Windows XP and Vista support is deprecated {#winxp_deprecated}

Support for Windows XP and Vista has been deprecated and will be removed in
a future release.  Windows XP has been out of extended support since 2014.


### Original MinGW support is deprecated {#mingw_deprecated}

Support for the now unmaintained original MinGW distribution has been deprecated
and will be removed in a future release.

This does not apply to the much more capable MinGW-w64, which remains fully
supported, actively maintained and available on many platforms.


### OS X Yosemite support is deprecated {#yosemite_deprecated}

Support for OS X 10.10 Yosemite and earlier has been deprecated and will be
removed in a future release.  OS X 10.10 has been out of support since 2017.


## Removals {#removals}

### GLFW_VULKAN_STATIC CMake option has been removed {#vulkan_static_removed}

This option was used to compile GLFW directly linked with the Vulkan loader, instead of
using dynamic loading to get hold of `vkGetInstanceProcAddr` at initialization.  This is
now done by calling the @ref glfwInitVulkanLoader function before initialization.

If you need backward compatibility, this macro can still be defined for GLFW 3.4 and will
have no effect.  The call to @ref glfwInitVulkanLoader can be conditionally enabled in
your code by checking the @ref GLFW_VERSION_MAJOR and @ref GLFW_VERSION_MINOR macros.


### GLFW_USE_WAYLAND CMake option has been removed {#use_wayland_removed}

This option was used to compile GLFW for Wayland instead of X11.  GLFW now
supports selecting the platform at run-time.  By default GLFW is compiled for
both Wayland and X11 on Linux and other Unix-like systems.

To disable Wayland or X11 or both, set the @ref GLFW_BUILD_WAYLAND and @ref
GLFW_BUILD_X11 CMake options.

The `GLFW_USE_WAYLAND` CMake variable must not be present in the CMake cache at
all, or GLFW will fail to configure.  If you are getting this error, delete the
CMake cache for GLFW and configure again.


### GLFW_USE_OSMESA CMake option has been removed {#use_osmesa_removed}

This option was used to compile GLFW for the Null platform.  The Null platform
is now always available.  To produce a library binary that only supports this
platform, the way this CMake option used to do, you will instead need to disable
the default platforms for the target OS.  This means setting the @ref
GLFW_BUILD_WIN32, @ref GLFW_BUILD_COCOA or @ref GLFW_BUILD_WAYLAND and @ref
GLFW_BUILD_X11 CMake options to false.

You can set all of them to false and the ones that don't apply for the target OS
will be ignored.


### wl_shell protocol support has been removed {#wl_shell_removed}

Support for the deprecated wl_shell protocol has been removed and GLFW now only
supports the XDG-Shell protocol.  If your Wayland compositor does not support
XDG-Shell then GLFW will fail to initialize.


## New symbols {#new_symbols}

### New functions {#new_functions}

 - @ref glfwInitAllocator
 - @ref glfwGetPlatform
 - @ref glfwPlatformSupported
 - @ref glfwInitVulkanLoader
 - @ref glfwGetWindowTitle
 - @ref glfwGetCocoaView


### New types {#new_types}

 - @ref GLFWallocator
 - @ref GLFWallocatefun
 - @ref GLFWreallocatefun
 - @ref GLFWdeallocatefun


### New constants {#new_constants}

 - @ref GLFW_PLATFORM
 - @ref GLFW_ANY_PLATFORM
 - @ref GLFW_PLATFORM_WIN32
 - @ref GLFW_PLATFORM_COCOA
 - @ref GLFW_PLATFORM_WAYLAND
 - @ref GLFW_PLATFORM_X11
 - @ref GLFW_PLATFORM_NULL
 - @ref GLFW_PLATFORM_UNAVAILABLE
 - @ref GLFW_POINTING_HAND_CURSOR
 - @ref GLFW_RESIZE_EW_CURSOR
 - @ref GLFW_RESIZE_NS_CURSOR
 - @ref GLFW_RESIZE_NWSE_CURSOR
 - @ref GLFW_RESIZE_NESW_CURSOR
 - @ref GLFW_RESIZE_ALL_CURSOR
 - @ref GLFW_MOUSE_PASSTHROUGH
 - @ref GLFW_NOT_ALLOWED_CURSOR
 - @ref GLFW_CURSOR_UNAVAILABLE
 - @ref GLFW_WIN32_KEYBOARD_MENU
 - @ref GLFW_WIN32_SHOWDEFAULT
 - @ref GLFW_CONTEXT_DEBUG
 - @ref GLFW_FEATURE_UNAVAILABLE
 - @ref GLFW_FEATURE_UNIMPLEMENTED
 - @ref GLFW_ANGLE_PLATFORM_TYPE
 - @ref GLFW_ANGLE_PLATFORM_TYPE_NONE
 - @ref GLFW_ANGLE_PLATFORM_TYPE_OPENGL
 - @ref GLFW_ANGLE_PLATFORM_TYPE_OPENGLES
 - @ref GLFW_ANGLE_PLATFORM_TYPE_D3D9
 - @ref GLFW_ANGLE_PLATFORM_TYPE_D3D11
 - @ref GLFW_ANGLE_PLATFORM_TYPE_VULKAN
 - @ref GLFW_ANGLE_PLATFORM_TYPE_METAL
 - @ref GLFW_X11_XCB_VULKAN_SURFACE
 - @ref GLFW_CURSOR_CAPTURED
 - @ref GLFW_POSITION_X
 - @ref GLFW_POSITION_Y
 - @ref GLFW_ANY_POSITION
 - @ref GLFW_WAYLAND_APP_ID
 - @ref GLFW_WAYLAND_LIBDECOR
 - @ref GLFW_WAYLAND_PREFER_LIBDECOR
 - @ref GLFW_WAYLAND_DISABLE_LIBDECOR
 - @ref GLFW_SCALE_FRAMEBUFFER


## Release notes for earlier versions {#news_archive}

- [Release notes for 3.3](https://www.glfw.org/docs/3.3/news.html)
- [Release notes for 3.2](https://www.glfw.org/docs/3.2/news.html)
- [Release notes for 3.1](https://www.glfw.org/docs/3.1/news.html)
- [Release notes for 3.0](https://www.glfw.org/docs/3.0/news.html)

