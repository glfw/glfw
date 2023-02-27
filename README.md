# GLFW

This software was not created by me. The original repository is linked down below.

[Original repository](https://github.com/glfw/glfw)

## Borderless features


### New hint for Windows:
- `GLFW_BORDERLESS_AREO` - When window is created it automatically becomes borderless but has all the area and animation features.
### New functions:

- `glfwSetWindowBorderlessResizeBorderSize(windowPtr, sizeOfResizeBorder)` - Changes the size of resize borders in the window.
- `glfwSetWindowBorderlessGrabArea(windowPtr, xpos, ypos, width, height)` - Sets the grab area size and position.
