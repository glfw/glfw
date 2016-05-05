# Contribution Guide

This file is a work in progress and you can report errors or submit patches for
it the same as any other file.


## Reporting a bug

If GLFW is behaving unexpectedly, make sure you have set an error callback.
GLFW will often tell you the cause of an issue via this callback.

If GLFW is crashing or triggering asserts, make sure that all your object
handles and other pointers are valid.

Always include the __operating system name and version__ (i.e. `Windows
7 64-bit` or `Ubuntu 15.10`).  If you are using an official release of GLFW,
include the __GLFW release version__ (i.e. `3.1.2`), otherwise include the
__GLFW commit ID__ (i.e.  `3795d78b14ef06008889cc422a1fb8d642597751`) from Git.
If possible, please also include the __GLFW version string__ (`3.2.0 X11 EGL
clock_gettime /dev/js XI Xf86vm`), as described
[here](http://www.glfw.org/docs/latest/intro.html#intro_version_string).


### Reporting a compile or link bug

__Note:__ GLFW needs many system APIs to do its job.  See the [Building
applications](http://www.glfw.org/docs/latest/build.html) guide for more
information.

In addition to the information above, always include the complete build log from
your compiler and linker.  Issue posts are editable so it can always be
shortened later.


### Reporting a context creation bug

__Note:__ Windows ships with graphics drivers that do not support OpenGL.  If
GLFW says that your machine lacks support for OpenGL, it very likely does.
Install drivers from the computer manufacturer or graphics card manufacturer
([Nvidia](http://www.geforce.com/drivers),
 [AMD](http://support.amd.com/en-us/download),
 [Intel](https://www-ssl.intel.com/content/www/us/en/support/detect.html)) to
fix this.

__Note:__ AMD only supports OpenGL ES on Windows via EGL.  EGL support is not
enabled in GLFW by default.  You need to [enable EGL when
compiling](http://www.glfw.org/docs/latest/compile.html) GLFW to use this.

The `glfwinfo` tool is included in the GLFW source tree as `tests/glfwinfo.c`
and is built along with the library.  It lets you request any kind of context
and framebuffer format supported by the GLFW API without having to recompile.
If context creation fails in your application, please verify that it also fails
with this tool before reporting it as a bug.

In addition to the information above (OS and GLFW version), always include the
__GPU model and driver version__ (i.e. `GeForce GTX660 with 352.79`) when
reporting this kind of bug.


### Reporting a monitor or video mode bug

__Note:__ On headless systems on some platforms, no monitors are reported.  This
causes glfwGetPrimaryMonitor to return `NULL`, which not all applications are
prepared for.

__Note:__ Some third-party tools report more video modes than those approved of
by the OS.  For safety and compatbility, GLFW only reports video modes the OS
wants programs to use.  This is not a bug.

The `monitors` tool is included in the GLFW source tree as `tests/monitors.c`
and is built along with the library.  lists all information about connected
monitors made available by GLFW.

In addition to the information above (OS and GLFW version), please also include
the output of the `monitors` tool when reporting this kind of bug.  If it
doesn't work at all, please mention this.


### Reporting a window event bug

__Note:__ While GLFW tries to provide the exact same behavior between platforms,
the exact ordering of related window events will sometimes differ.

The `events` tool is included in the GLFW source tree as `tests/events.c` and is
built along with the library.  It prints all information provided to every
callback supported by GLFW as events occur.  Each event is listed with the time
and a unique number to make discussions about event logs easier.  The tool has
command-line options for creating multiple windows and full screen windows.


### Reporting a documentation bug

If you found the error in the generated documentation then it's fine to just
link to that webpage.  You don't need to figure out which documentation source
file the text comes from.


## Contributing a bug fix

There should be text here, but there isn't.


## Contributing a feature

This is not (yet) the text you are looking for.

