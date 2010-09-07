prefix=@PREFIX@
exec_prefix=@PREFIX@
libdir=@PREFIX@/lib
includedir=@PREFIX@/include

Name: GLFW
Description: A portable framework for OpenGL development
Version: 2.7
URL: http://glfw.sourceforge.net/
Libs: -L${libdir} -lglfw -framework AGL -framework OpenGL -framework Carbon
Cflags: -I${includedir}
