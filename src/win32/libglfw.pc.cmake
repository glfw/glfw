prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: GLFW
Description: A portable framework for OpenGL development
Version: 3.0
URL: http://www.glfw.org/
Libs: -L${libdir} -lglfw @GLFW_LIBRARIES@
Cflags: -I${includedir} -mwin32
