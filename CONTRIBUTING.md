# Contribution Guide

This file is a work in progress and you can report errors or submit patches for
it the same as any other file.


## Reporting a bug

If GLFW is behaving unexpectedly, make sure you have set an error callback.
GLFW will often tell you the cause of an issue via this callback.

If GLFW is crashing or triggering asserts, make sure that all your object
handles and other pointers are valid.

Always include the __operating system name and version__ (i.e. `Windows
7 64-bit`).  If you are using an official release of GLFW, include the __GLFW
release version__ (i.e. `3.1.2`), otherwise include the __GLFW commit ID__ (i.e.
`3795d78b14ef06008889cc422a1fb8d642597751`) from Git.  If possible, please also
include the __GLFW version string__ (`"3.2.0 X11 EGL clock_gettime /dev/js XI
Xf86vm"`), as returned by glfwGetVersionString.


### Reporting a compile or link bug

In addition to the information above, always include the complete build log.
Issue posts are editable so it can always be shortened later.


### Reporting a context creation bug

__Note:__ Windows ships with graphics drivers that do not support OpenGL.  If
GLFW says that your machine lacks support for OpenGL, it very likely does.

__Note:__ AMD only supports OpenGL ES on Windows via EGL, which is not enabled
in GLFW by default.  You need to enable EGL when compiling GLFW to use this.

The `glfwinfo` tool lets you request any kind of context and framebuffer format
supported by the GLFW API without having to recompile.  If context creation
fails in your application, please verify that it also fails with this tool
before reporting it as a bug.

In addition to the information above, always include the __GPU model and driver
version__ (i.e. `GeForce GTX660 with 352.79`) when reporting this kind of bug.


### Reporting a monitor or video mode bug

__Note:__ On headless systems on some platforms, no monitors are reported.  This
causes glfwGetPrimaryMonitor to return `NULL`, which not all applications are
prepared for.

The `monitors` tool lists all information about connected monitors made
available by GLFW.

In addition to the information above, if possible please also include the output
of the `monitors` tool when reporting this kind of bug.


### Reporting a window event bug

The `events` tool prints all information provided to every callback supported by
GLFW as events occur.  Each event is listed with the time and a unique number
to make discussions about event logs easier.  The tool has command-line options
for creating multiple windows and full screen windows.


### Reporting a documentation bug

If you found the error in the generated documentation then it's fine to just
link to that webpage.  You don't need to figure out which documentation source
file the text comes from.


## Contributing a bug fix

There should be text here, but there isn't.


## Contributing a feature

This is not (yet) the text you are looking for.

