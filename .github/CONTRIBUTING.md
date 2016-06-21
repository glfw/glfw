# Contribution Guide

## Contents

- [Reporting a bug](#reporting-a-bug)
    - [Reporting a compile or link bug](#reporting-a-compile-or-link-bug)
    - [Reporting a segfault or other crash bug](#reporting-a-segfault-or-other-crash-bug)
    - [Reporting a context creation bug](#reporting-a-context-creation-bug)
    - [Reporting a monitor or video mode bug](#reporting-a-monitor-or-video-mode-bug)
    - [Reporting an input or event bug](#reporting-an-input-or-event-bug)
    - [Reporting some other library bug](#reporting-some-other-library-bug)
    - [Reporting a documentation bug](#reporting-a-documentation-bug)
    - [Reporting a website bug](#reporting-a-website-bug)
- [Requesting a feature](#requesting-a-feature)
- [Contributing a bug fix](#contributing-a-bug-fix)
- [Contributing a feature](#contributing-a-feature)


## Reporting a bug

If GLFW is behaving unexpectedly at run-time, start by setting an [error
callback](http://www.glfw.org/docs/latest/intro_guide.html#error_handling).
GLFW will often tell you the cause of an error via this callback.  If it
doesn't, that might be a separate bug.

If GLFW is crashing or triggering asserts, make sure that all your object
handles and other pointers are valid.

There are issue labels for both platforms and GPU manufacturers, so there is no
need to mention these in the subject line.  If you do, it will be removed when
the issue is labeled.


### Reporting a compile or link bug

__Note:__ GLFW needs many system APIs to do its job, which on some platforms
means linking to many system libraries.  If you are using GLFW as a static
library, that means your application needs to link to these in addition to GLFW.
See the [Build guide](http://www.glfw.org/docs/latest/build.html) for more
information.

Always include the __operating system name and version__ (i.e. `Windows
7 64-bit` or `Ubuntu 15.10`) and the __compiler name and version__ (i.e. `Visual
C++ 2015 Update 2`).  If you are using an official release of GLFW,
include the __GLFW release version__ (i.e. `3.1.2`), otherwise include the
__GLFW commit ID__ (i.e.  `3795d78b14ef06008889cc422a1fb8d642597751`) from Git.

Please also include the __complete build log__ from your compiler and linker,
even if it's long.  It can always be shortened later, if necessary.


#### Quick template

```
OS and version:
Compiler version:
Release or commit:
Build log:
```


### Reporting a segfault or other crash bug

Always include the __operating system name and version__ (i.e. `Windows
7 64-bit` or `Ubuntu 15.10`).  If you are using an official release of GLFW,
include the __GLFW release version__ (i.e. `3.1.2`), otherwise include the
__GLFW commit ID__ (i.e.  `3795d78b14ef06008889cc422a1fb8d642597751`) from Git.

Please also include any __error messages__ provided to your application via the
[error
callback](http://www.glfw.org/docs/latest/intro_guide.html#error_handling) and
the __full call stack__ of the crash, or if the crash does not occur in debug
mode, mention that instead.


#### Quick template

```
OS and version:
Release or commit:
Error messages:
Call stack:
```


### Reporting a context creation bug

__Note:__ Windows ships with graphics drivers that do not support OpenGL.  If
GLFW says that your machine lacks support for OpenGL, it very likely does.
Install drivers from the computer manufacturer or graphics card manufacturer
([Nvidia](http://www.geforce.com/drivers),
[AMD](http://support.amd.com/en-us/download),
[Intel](https://www-ssl.intel.com/content/www/us/en/support/detect.html)) to
fix this.

__Note:__ AMD only supports OpenGL ES on Windows via EGL.  See the
[GLFW\_CONTEXT\_CREATION\_API](http://www.glfw.org/docs/latest/window_guide.html#window_hints_ctx)
hint for how to select EGL.

Please verify that context creation also fails with the `glfwinfo` tool before
reporting it as a bug.  This tool is included in the GLFW source tree as
`tests/glfwinfo.c` and is built along with the library.  It has switches for all
GLFW context and framebuffer hints.  Run `glfwinfo -h` for a complete list.

Always include the __operating system name and version__ (i.e. `Windows
7 64-bit` or `Ubuntu 15.10`).  If you are using an official release of GLFW,
include the __GLFW release version__ (i.e. `3.1.2`), otherwise include the
__GLFW commit ID__ (i.e.  `3795d78b14ef06008889cc422a1fb8d642597751`) from Git.

Please also include the __GLFW version string__ (`3.2.0 X11 EGL clock_gettime
/dev/js XI Xf86vm`), as described
[here](http://www.glfw.org/docs/latest/intro.html#intro_version_string), the
__GPU model and driver version__ (i.e. `GeForce GTX660 with 352.79`), and the
__output of `glfwinfo`__ (with switches matching any hints you set in your
code) when reporting this kind of bug.  If this tool doesn't run on the machine,
mention that instead.


#### Quick template

```
OS and version:
GPU and driver:
Release or commit:
Version string:
`glfwinfo` output:
```


### Reporting a monitor or video mode bug

__Note:__ On headless systems on some platforms, no monitors are reported.  This
causes glfwGetPrimaryMonitor to return `NULL`, which not all applications are
prepared for.

__Note:__ Some third-party tools report more video modes than are approved of
by the OS.  For safety and compatibility, GLFW only reports video modes the OS
wants programs to use.  This is not a bug.

The `monitors` tool is included in the GLFW source tree as `tests/monitors.c`
and is built along with the library.  It lists all information GLFW provides
about monitors it detects.

Always include the __operating system name and version__ (i.e. `Windows
7 64-bit` or `Ubuntu 15.10`).  If you are using an official release of GLFW,
include the __GLFW release version__ (i.e. `3.1.2`), otherwise include the
__GLFW commit ID__ (i.e.  `3795d78b14ef06008889cc422a1fb8d642597751`) from Git.

Please also include any __error messages__ provided to your application via the
[error
callback](http://www.glfw.org/docs/latest/intro_guide.html#error_handling) and
the __output of `monitors`__ when reporting this kind of bug.  If this tool
doesn't run on the machine, mention this instead.


#### Quick template

```
OS and version:
Release or commit:
Error messages:
`monitors` output:
```


### Reporting an input or event bug

__Note:__ While GLFW tries to provide the same behavior across platforms, the
exact ordering of related window events will sometimes differ.

The `events` tool is included in the GLFW source tree as `tests/events.c` and is
built along with the library.  It prints all information provided to every
callback supported by GLFW as events occur.  Each event is listed with the time
and a unique number to make discussions about event logs easier.  The tool has
command-line options for creating multiple windows and full screen windows.

Always include the __operating system name and version__ (i.e. `Windows
7 64-bit` or `Ubuntu 15.10`).  If you are using an official release of GLFW,
include the __GLFW release version__ (i.e. `3.1.2`), otherwise include the
__GLFW commit ID__ (i.e.  `3795d78b14ef06008889cc422a1fb8d642597751`) from Git.

Please also include any __error messages__ provided to your application via the
[error
callback](http://www.glfw.org/docs/latest/intro_guide.html#error_handling) and
if relevant, the __output of `events`__ when reporting this kind of bug.  If
this tool doesn't run on the machine, mention this instead.


#### Quick template

```
OS and version:
Release or commit:
Error messages:
`events` output:
```


### Reporting some other library bug

Always include the __operating system name and version__ (i.e. `Windows
7 64-bit` or `Ubuntu 15.10`).  If you are using an official release of GLFW,
include the __GLFW release version__ (i.e. `3.1.2`), otherwise include the
__GLFW commit ID__ (i.e.  `3795d78b14ef06008889cc422a1fb8d642597751`) from Git.

Please also include any __error messages__ provided to your application via the
[error
callback](http://www.glfw.org/docs/latest/intro_guide.html#error_handling), if
relevant.


#### Quick template

```
OS and version:
Release or commit:
Error messages:
```


### Reporting a documentation bug

If you found a bug in the documentation, including this file, then it's fine to
just link to that web page or mention that source file.  You don't need to match
the source to the output or vice versa.


### Reporting a website bug

If the bug is in the documentation (anything under `/docs/`) then please see the
section above.  Bugs in the rest of the site are reported to to the [website
source repository](https://github.com/glfw/website/issues).


## Requesting a feature

Please explain why you need the feature and how you intend to use it.  If you
have a specific API design in mind, please add that as well.  If you have or are
planning to write code for the feature, see the section below.


## Contributing a bug fix

Adding a complete bug fix involves:

- Change log entry in `README.md`
- Credits entries in `README.md` for all authors of the fix


## Contributing a feature

__Note:__ You _must_ mention if you have taken any code from outside sources,
even if the licenses seem compatible.

Adding a complete feature involves:

- Change log entry in `README.md`
- News entry in `docs/news.dox`
- Guide documentation in `docs/window.dox`, `docs/input.dox`,
    `docs/monitor.dox`, `docs/context.dox` or `docs/vulkan.dox`
- Reference documentation in `include/GLFW/glfw3.h`
- Credits entries in `README.md` for all authors of the feature

If it requires platform-specific code, add at minimum stubs for the new platform
function to all supported platforms.

If it's a new callback, add support for it to `tests/event.c`.

If it's a new monitor property, add support for it to `tests/monitor.c`.

If it's a new OpenGL, OpenGL ES or Vulkan option or extension, add support for
it to `tests/glfwinfo.c` and describe its use in `docs/compat.dox`.

If it requires calling non-default DLLs on Windows, load these at runtime in
`src/win32_init.c`.

Please keep in mind that any part of the public API that has been included in
a release cannot be changed until the next _major_ version.  Features can be
added and existing parts can sometimes be overloaded (in the general sense of
doing more things, not in the C++ sense), but code written to the API of one
minor release should both compile and run on subsequent minor releases.

