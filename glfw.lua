-- Building libglfw for test consumption is done on an as-needed basis.
-- In order to make this as easy as possible, RTC builds the library using the premake build system to utilize the
-- advancements we've made in compilers and build scripts to simplify things without requiring many changes to the
-- glfw project itself. For every platform, take a look at the configuration documentation for setup information.
project "glfw"

dofile(_BUILD_DIR .. "/shared_library.lua")

configuration { "*" }

uuid "11119b90-6c6e-4493-9860-86f7c2b581dc"

defines {
  "_GLFW_BUILD_DLL",
}

files {
  "include/GLFW/glfw3.h",
  "include/GLFW/glfw3native.h",
  "src/internal.h",
  "src/platform.h",
  "src/mappings.h",
  "src/context.c",
  "src/init.c",
  "src/input.c",
  "src/monitor.c",
  "src/platform.c",
  "src/vulkan.c",
  "src/window.c",
  "src/egl_context.c",
  "src/osmesa_context.c",
  "src/null_platform.h",
  "src/null_joystick.h",
  "src/null_init.c",
  "src/null_monitor.c",
  "src/null_window.c",
  "src/null_joystick.c",
}

includedirs {
  "include/GLFW",
}

if (_PLATFORM_LINUX) then

  buildoptions {
    "-Wall",
    "-fvisibility=hidden",
  }

  defines {
    "_GLFW_X11",
  }

  files {
    "src/posix_time.h",
    "src/posix_thread.h",
    "src/posix_module.c",
    "src/posix_time.c",
    "src/posix_thread.c",
    "src/x11_platform.h",
    "src/xkb_unicode.h",
    "src/x11_init.c",
    "src/x11_monitor.c",
    "src/x11_window.c",
    "src/xkb_unicode.c",
    "src/glx_context.c",
    "src/linux_joystick.h",
    "src/linux_joystick.c",
    "src/posix_poll.h",
    "src/posix_poll.c"
  }

  links {
    "rt",
    "m",
  }

end

if (_PLATFORM_MACOS) then

  buildoptions {
    "-Wall",
    "-fvisibility=hidden",
  }

  defines {
    "_GLFW_COCOA",
  }

  files {
    "src/cocoa_time.h",
    "src/cocoa_time.c",
    "src/posix_thread.h",
    "src/posix_module.c",
    "src/posix_thread.c",
    "src/cocoa_platform.h",
    "src/cocoa_joystick.h",
    "src/cocoa_init.m",
    "src/cocoa_joystick.m",
    "src/cocoa_monitor.m",
    "src/cocoa_window.m",
    "src/nsgl_context.m",
  }

  linkoptions {
    "-Wl,-rpath,@executable_path",
    "-framework Cocoa",
    "-framework IOKit",
    "-framework CoreFoundation",
  }

end

if (_PLATFORM_WINDOWS) then

  buildoptions {
    "/W3",
  }

  defines {
    "UNICODE",
    "_GLFW_WIN32",
    "_UNICODE",
  }

  files {
    "src/win32_time.h",
    "src/win32_thread.h",
    "src/win32_module.c",
    "src/win32_time.c",
    "src/win32_thread.c",
    "src/win32_platform.h",
    "src/win32_joystick.h",
    "src/win32_init.c",
    "src/win32_joystick.c",
    "src/win32_monitor.c",
    "src/win32_window.c",
    "src/wgl_context.c",
  }

  links {
    "gdi32",
  }

end
