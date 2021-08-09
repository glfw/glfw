project "GLFW"
    kind "StaticLib"
    language "C"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

    files {
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h",
        "src/glfw_config.h",
        "src/context.c",
        "src/init.c",
        "src/input.c",
        "src/monitor.c",
        "src/window.c",

        "src/win32_init.c",
        "src/win32_joystick.c",
        "src/win32_monitor.c",
        "src/win32_time.c",
        "src/win32_thread.c",
        "src/wgl_context.c",
        "src/egl_context.c",
        "src/osmesa_context.c"
    }

    staticruntime "off"
    systemversion "latest"

    defines {
        "_GLFW_WIN32",
        "_CRT_SECURE_NO_WARNINGS"
    }

    filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

    filter "configurations:Distribution"
		runtime "Release"
		optimize "on"