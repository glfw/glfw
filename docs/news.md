# Release notes {#news}

[TOC]


## Release notes for version 3.4 {#news_34}

### New features in version 3.4 {#features_34}

#### Runtime platform selection {#runtime_platform_34}

GLFW now supports being compiled for multiple backends and selecting between
them at runtime with the @ref GLFW_PLATFORM init hint.  After initialization the
selected platform can be queried with @ref glfwGetPlatform.  You can check if
support for a given platform is compiled in with @ref glfwPlatformSupported.


#### More standard cursors {#standard_cursors_34}

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


#### Mouse event passthrough {#mouse_passthrough_34}

GLFW now provides the [GLFW_MOUSE_PASSTHROUGH](@ref GLFW_MOUSE_PASSTHROUGH_hint)
window hint for making a window transparent to mouse input, lettings events pass
to whatever window is behind it.  This can also be changed after window
creation with the matching [window attribute](@ref GLFW_MOUSE_PASSTHROUGH_attrib).


#### Wayland libdecor decorations {#wayland_libdecor_34}

GLFW now supports improved fallback window decorations via
[libdecor](https://gitlab.freedesktop.org/libdecor/libdecor).

Support for libdecor can be toggled before GLFW is initialized with the
[GLFW_WAYLAND_LIBDECOR](@ref GLFW_WAYLAND_LIBDECOR_hint) init hint.  It is
enabled by default.


#### Wayland app_id specification {#wayland_app_id_34}

GLFW now supports specifying the app_id for a Wayland window using the
[GLFW_WAYLAND_APP_ID](@ref GLFW_WAYLAND_APP_ID_hint) window hint string.


#### Support for ANGLE rendering backend selection {#features_34_angle_backend}

GLFW now provides the
[GLFW_ANGLE_PLATFORM_TYPE](@ref GLFW_ANGLE_PLATFORM_TYPE_hint) init hint for
requesting a specific rendering backend when using [ANGLE][] to create OpenGL ES
contexts.

[ANGLE]: https://chromium.googlesource.com/angle/angle/


#### Captured cursor mode {#captured_cursor_34}

GLFW now supports confining the cursor to the window content area with the @ref
GLFW_CURSOR_CAPTURED cursor mode.

For more information see @ref cursor_mode.


#### Support for custom memory allocator {#features_34_init_allocator}

GLFW now supports plugging a custom memory allocator at initialization with @ref
glfwInitAllocator.  The allocator is a struct of type @ref GLFWallocator with
function pointers corresponding to the standard library functions `malloc`,
`realloc` and `free`.

For more information see @ref init_allocator.


#### Window hint for framebuffer scaling {#scale_framebuffer_34}

GLFW now allows provides the
[GLFW_SCALE_FRAMEBUFFER](@ref GLFW_SCALE_FRAMEBUFFER_hint) window hint for
controlling framebuffer scaling on platforms that handle scaling by keeping the
window size the same while resizing the framebuffer.  The default value is to
allow framebuffer scaling.

This was already possible on macOS via the
[GLFW_COCOA_RETINA_FRAMEBUFFER](@ref GLFW_COCOA_RETINA_FRAMEBUFFER_hint) window
hint.  This hint is now another name for
[GLFW_SCALE_FRAMEBUFFER](@ref GLFW_SCALE_FRAMEBUFFER_hint).

For more information, see @ref window_scale.


#### Window hints for initial position {#features_34_position_hint}

GLFW now provides the @ref GLFW_POSITION_X and @ref GLFW_POSITION_Y window hints for
specifying the initial position of the window.  This removes the need to create a hidden
window, move it and then show it.  The default value of these hints is
`GLFW_ANY_POSITION`, which selects the previous behavior.


#### Support for keyboard access to Windows window menu {#features_34_win32_keymenu}

GLFW now provides the
[GLFW_WIN32_KEYBOARD_MENU](@ref GLFW_WIN32_KEYBOARD_MENU_hint) window hint for
enabling keyboard access to the window menu via the Alt+Space and
Alt-and-then-Space shortcuts.  This may be useful for more GUI-oriented
applications.


#### Support for applying STARTUPINFO show command {#features_34_win32_showdefault}

GLFW now provides the [GLFW_WIN32_SHOWDEFAULT](@ref GLFW_WIN32_SHOWDEFAULT_hint) window
hint for applying the show command in the program's `STARTUPINFO` when showing the window
for the first time.  This may be useful for the main window of a windowed-mode tool.


### Caveats for version 3.4 {#caveats}

#### Multiple sets of native access functions {#native_34}

Because GLFW now supports runtime selection of platform (window system), a library binary
may export native access functions for multiple platforms.  Starting with version 3.4 you
must not assume that GLFW is running on a platform just because it exports native access
functions for it.  After initialization, you can query the selected platform with @ref
glfwGetPlatform.


#### Version string format has been changed {#version_string_34}

Because GLFW now supports runtime selection of platform (window system), the version
string returned by @ref glfwGetVersionString has been expanded.  It now contains the names
of all APIs for all the platforms that the library binary supports.


#### Joystick support is initialized on demand {#joysticks_34}

The joystick part of GLFW is now initialized when first used, primarily to work
around faulty Windows drivers that cause DirectInput to take up to several
seconds to enumerate devices.

This change will usually not be observable.  However, if your application waits
for events without having first called any joystick function or created any
visible windows, the wait may never unblock as GLFW may not yet have subscribed
to joystick related OS events.

To work around this, call any joystick function before waiting for events, for
example by setting a [joystick callback](@ref joystick_event).


#### Framebuffer may lack alpha channel on older Wayland systems {#wayland_alpha_34}

On Wayland, when creating an EGL context on a machine lacking the new
`EGL_EXT_present_opaque` extension, the @ref GLFW_ALPHA_BITS window hint will be
ignored and the framebuffer will have no alpha channel.  This is because some
Wayland compositors treat any buffer with an alpha channel as per-pixel
transparent.

If you want a per-pixel transparent window, see the
[GLFW_TRANSPARENT_FRAMEBUFFER](@ref GLFW_TRANSPARENT_FRAMEBUFFER_hint) window
hint.


#### Tests and examples are disabled when built as a subproject {#standalone_34}

GLFW now does not build the tests and examples when it is added as
a subdirectory of another CMake project.  To enable these, set the @ref
GLFW_BUILD_TESTS and @ref GLFW_BUILD_EXAMPLES cache variables before adding the
GLFW subdirectory.

```cmake
set(GLFW_BUILD_EXAMPLES ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS ON CACHE BOOL "" FORCE)
add_subdirectory(path/to/glfw)
```


#### macOS main menu now created at initialization {#initmenu_34}

GLFW now creates the main menu and completes the initialization of NSApplication
during initialization.  Programs that do not want a main menu can disable it
with the [GLFW_COCOA_MENUBAR](@ref GLFW_COCOA_MENUBAR_hint) init hint.


#### CoreVideo dependency has been removed {#corevideo_34}

GLFW no longer depends on the CoreVideo framework on macOS and it no longer
needs to be specified during compilation or linking.


#### Framebuffer transparency requires DWM transparency {#caveat_fbtransparency_34}

GLFW no longer supports framebuffer transparency enabled via @ref
GLFW_TRANSPARENT_FRAMEBUFFER on Windows 7 if DWM transparency is off
(the Transparency setting under Personalization > Window Color).


#### Empty events on X11 no longer round-trip to server {#emptyevents_34}

Events posted with @ref glfwPostEmptyEvent now use a separate unnamed pipe
instead of sending an X11 client event to the helper window.


### Deprecations in version 3.4 {#deprecations_34}

### Removals in 3.4 {#removals_34}

#### GLFW_VULKAN_STATIC CMake option has been removed {#vulkan_static_34}

This option was used to compile GLFW directly linked with the Vulkan loader, instead of
using dynamic loading to get hold of `vkGetInstanceProcAddr` at initialization.  This is
now done by calling the @ref glfwInitVulkanLoader function before initialization.

If you need backward compatibility, this macro can still be defined for GLFW 3.4 and will
have no effect.  The call to @ref glfwInitVulkanLoader can be conditionally enabled in
your code by checking the @ref GLFW_VERSION_MAJOR and @ref GLFW_VERSION_MINOR macros.


#### GLFW_USE_OSMESA CMake option has been removed {#osmesa_option_34}

This option was used to compile GLFW for the Null platform.  The Null platform is now
always supported.  To produce a library binary that only supports this platform, the way
this CMake option used to do, you will instead need to disable the default platform for
the target OS.  This means setting the @ref GLFW_BUILD_WIN32, @ref GLFW_BUILD_COCOA or
@ref GLFW_BUILD_X11 CMake option to false.

You can set all of them to false and the ones that don't apply for the target OS will be
ignored.


#### Support for the wl_shell protocol has been removed {#wl_shell_34}

Support for the wl_shell protocol has been removed and GLFW now only supports
the XDG-Shell protocol.  If your Wayland compositor does not support XDG-Shell
then GLFW will fail to initialize.


### New symbols in version 3.4 {#symbols_34}

#### New functions in version 3.4 {#functions_34}

 - @ref glfwInitAllocator
 - @ref glfwGetPlatform
 - @ref glfwPlatformSupported
 - @ref glfwInitVulkanLoader


#### New types in version 3.4 {#types_34}

 - @ref GLFWallocator
 - @ref GLFWallocatefun
 - @ref GLFWreallocatefun
 - @ref GLFWdeallocatefun


#### New constants in version 3.4 {#constants_34}

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

