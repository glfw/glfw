# Release notes for version 3.5 {#news}

[TOC]


## New features {#features}

### Unlimited mouse buttons {#unlimited_mouse_buttons}

GLFW now has an input mode which allows an unlimited number of mouse buttons to
be reported by the mouse buttton callback, rather than just the associated
[mouse button tokens](@ref buttons). This allows using mouse buttons with
values over 8. For compatibility with older versions, the
@ref GLFW_UNLIMITED_MOUSE_BUTTONS input mode needs to be set to make use of
this.

## Caveats {#caveats}

## Deprecations {#deprecations}

## Removals {#removals}

### Windows XP and Vista support has been removed {#winxp_vista}

Support for Windows XP and Vista has been removed.  Windows XP has been out of extended
support since 2014.


### Original MinGW support has been removed {#original_mingw}

Support for the now unmaintained original MinGW distribution has been removed.

This does not apply to the much more capable [MinGW-w64](https://www.mingw-w64.org/),
which remains fully supported.  MinGW-w64 can build both 32- and 64-bit binaries, is
actively maintained and available on many platforms.


## New symbols {#new_symbols}

### New functions {#new_functions}

### New types {#new_types}

### New constants {#new_constants}

- @ref GLFW_UNLIMITED_MOUSE_BUTTONS

## Release notes for earlier versions {#news_archive}

- [Release notes for 3.4](https://www.glfw.org/docs/3.4/news.html)
- [Release notes for 3.3](https://www.glfw.org/docs/3.3/news.html)
- [Release notes for 3.2](https://www.glfw.org/docs/3.2/news.html)
- [Release notes for 3.1](https://www.glfw.org/docs/3.1/news.html)
- [Release notes for 3.0](https://www.glfw.org/docs/3.0/news.html)

