project "GLFW"
    kind "StaticLib"
    language "C++"
        
        targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
       
       files
       {
       "src/mappings.h.in"
       }