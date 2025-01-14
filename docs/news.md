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


### Support for custom X11 clipboard functionality {#x11_custom_selection}

This change allows clients to implement custom X11 clipboard
functionality like the copying and pasting of files across
applications.

GLFW itself only allows plain text to be copied to the
clipboard and back on all platforms. On some platforms, like Windows,
you can use platform specific APIs to add extra clipboard
functionality like copying of other data types. However, on X11, this
was previously not fully possible due to the fact that GLFW internal
code has full control over the X11 event queue.

This change exposes several new symbols that allow you to get and set
the handler for X11 selection events that GLFW will use. It also
allows getting the internal display connection and selection helper
window, for use in that kind of code.

## Caveats {#caveats}

## Deprecations {#deprecations}

## Removals {#removals}

## New symbols {#new_symbols}

### New functions {#new_functions}

#### X11-specific
 - @ref getSelectionRequestHandler
 - @ref setSelectionRequestHanddler
 - @ref getGLFWDisplay
 - @ref getGLFWHelperWindow

### New types {#new_types}

### New constants {#new_constants}

- @ref GLFW_UNLIMITED_MOUSE_BUTTONS

## Release notes for earlier versions {#news_archive}

- [Release notes for 3.4](https://www.glfw.org/docs/3.4/news.html)
- [Release notes for 3.3](https://www.glfw.org/docs/3.3/news.html)
- [Release notes for 3.2](https://www.glfw.org/docs/3.2/news.html)
- [Release notes for 3.1](https://www.glfw.org/docs/3.1/news.html)
- [Release notes for 3.0](https://www.glfw.org/docs/3.0/news.html)

