project "GLFW"
	kind "StaticLib"
	language "C"
	cdialect "C99"
	staticruntime "off"
	warnings "off"

	targetdir ("%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")
	objdir ("%{wks.location}/bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")

	includedirs
	{
		"include",
		"src",
	}

	files
	{
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

	    	"src/null_init.c",
	    	"src/null_monitor.c",
	    	"src/null_window.c",
	    	"src/null_joystick.c",

		"src/null_platform.h",
		"src/null_joystick.h",
	}

	filter "system:linux"
		pic "On"

		systemversion "latest"
		
		files
		{
			"src/x11_platform.h",
			"src/x11_init.c",
			"src/x11_monitor.c",
			"src/x11_window.c",
			"src/xkb_unicode.c",
			"src/posix_module.c",
			"src/posix_time.c",
			"src/posix_thread.c",
			"src/posix_poll.c",
			"src/glx_context.c",
			"src/linux_joystick.c"
		}

		defines
		{
			"_GLFW_X11",
			"_DEFAULT_SOURCE"
		}

	filter "system:macosx"
		pic "On"

		files
		{
			"src/cocoa_platform.h",
			"src/cocoa_joystick.h",
			"src/cocoa_init.m",
			"src/cocoa_monitor.m",
			"src/cocoa_window.m",
			"src/cocoa_joystick.m",
			"src/macos_time.c",
			"src/nsgl_context.m",
			"src/posix_thread.c",
			"src/posix_module.c",
		}

		defines
		{
			"_GLFW_COCOA"
		}

	filter "system:windows"
		systemversion "latest"

		files
		{
			"src/win32_platform.h",
			"src/win32_joystick.h",
			"src/win32_init.c",
			"src/win32_joystick.c",
			"src/win32_module.c",
			"src/win32_monitor.c",
			"src/win32_time.c",
			"src/win32_thread.c",
			"src/win32_window.c",
			"src/wgl_context.c",
		}

		defines 
		{ 
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS",
			"UNICODE",
			"_UNICODE"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter { "system:windows", "configurations:Debug-AS" }	
		runtime "Debug"
		symbols "on"
		editandcontinue "off"
		sanitize { "Address" }
		incrementallink "off"
		runtimechecks "off"

	filter "configurations:Release"
		runtime "Release"
		optimize "speed"

        filter "configurations:Dist"
	        runtime "Release"
	        optimize "speed"
                symbols "off"

	filter {}