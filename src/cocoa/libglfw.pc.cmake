prefix=@PREFIX@
exec_prefix=@PREFIX@
libdir=@PREFIX@/lib
includedir=@PREFIX@/include

Name: GLFW
Description: A portable library for OpenGL development
Version: 3.0
URL: http://www.glfw.org/
Libs: -L${libdir} -lglfw -framework AGL -framework OpenGL -framework Carbon
Cflags: -I${includedir}
