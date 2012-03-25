prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
includedir=${prefix}/include
libdir=${exec_prefix}/lib

Name: GLFW
Description: A portable library for OpenGL, window and input
Version: 3.0.0
URL: http://www.glfw.org/
Requires.private: @GLFW_PKGLIBS@
Libs: -L${libdir} -lglfw 
Cflags: -I${includedir}
