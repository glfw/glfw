# Release notes for version 3.6 {#news}

[TOC]


## New features {#features}

### Wayland glfwSetCursorPos support (#wayland_glfwsetcursorpos_support)

Wayland now supports @ref glfwSetCursorPos, using the [pointer-warp-v1][] protocol.

If the compositor does not support `pointer-warp-v1`, the function emits
@ref GLFW_FEATURE_UNAVAILABLE.

[pointer-warp-v1]: https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/staging/pointer-warp/pointer-warp-v1.xml

## Caveats {#caveats}

## Deprecations {#deprecations}

## Removals {#removals}

## New symbols {#new_symbols}

### New functions {#new_functions}

### New types {#new_types}

### New constants {#new_constants}

## Release notes for earlier versions {#news_archive}

- [Release notes for 3.5](https://www.glfw.org/docs/3.5/news.html)
- [Release notes for 3.4](https://www.glfw.org/docs/3.4/news.html)
- [Release notes for 3.3](https://www.glfw.org/docs/3.3/news.html)
- [Release notes for 3.2](https://www.glfw.org/docs/3.2/news.html)
- [Release notes for 3.1](https://www.glfw.org/docs/3.1/news.html)
- [Release notes for 3.0](https://www.glfw.org/docs/3.0/news.html)

