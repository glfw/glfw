project "glfw3"
kind "StaticLib"
language "C"

targetdir("build/%{cfg.system}/%{cfg.architecture}/%{cfg.shortname}/bin")
objdir("build/%{cfg.system}/%{cfg.architecture}/%{cfg.shortname}/bin-int")

includedirs {
    "include/"
}

files {
    "src/window.**",
    "src/context.**",
    "src/input.**",
    "src/internal.**",
    "src/glfw_config.h.in",
    "src/init.**",
    "src/monitor.**"
}

filter "system:Windows"
    files {
        "src/win32_**.c",
        "src/win32_**.h",
        "src/wgl_**",
        "src/egl_**",
        "src/osmesa_**",
        "src/vulkan.**"
    }
    defines {"_GLFW_WIN32"}