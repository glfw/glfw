
#include "internal.h"

#include <assert.h>

int createWindow(_GLFWwindow* window, const _GLFWwndconfig* wndconfig)
{
    window->osmesa.width = wndconfig->width;
    window->osmesa.height = wndconfig->height;

    return GLFW_TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWctxconfig* ctxconfig,
                              const _GLFWfbconfig* fbconfig)
{
    if (!_glfwInitOSMesa())
        return GLFW_FALSE;

    if (!_glfwCreateContextOSMesa(window, ctxconfig, fbconfig))
        return GLFW_FALSE;

    if (!createWindow(window, wndconfig))
        return GLFW_FALSE;

    return GLFW_TRUE;
}

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
    if (window->context.destroy)
        window->context.destroy(window);
}

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title) {}

void _glfwPlatformSetWindowIcon(_GLFWwindow* window, int count,
                                const GLFWimage* images) {}

void _glfwPlatformSetWindowMonitor(_GLFWwindow* window,
                                   _GLFWmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate) {}

void _glfwPlatformGetWindowPos(_GLFWwindow* window, int* xpos, int* ypos)
{
    if (xpos != NULL) *xpos = 0;
    if (ypos != NULL) *ypos = 0;
}

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int xpos, int ypos) {}

void _glfwPlatformGetWindowSize(_GLFWwindow* window, int* width, int* height)
{
    if (width != NULL) *width = window->osmesa.width;
    if (height != NULL) *height = window->osmesa.height;
}

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    window->osmesa.width = width;
    window->osmesa.height = height;
}

void _glfwPlatformSetWindowSizeLimits(_GLFWwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight) {}

void _glfwPlatformSetWindowAspectRatio(_GLFWwindow* window, int n, int d) {}

void _glfwPlatformGetFramebufferSize(_GLFWwindow* window, int* width,
                                     int* height)
{
    if (width != NULL) *width = window->osmesa.width;
    if (height != NULL) *height = window->osmesa.height;
}

void _glfwPlatformGetWindowFrameSize(_GLFWwindow* window, int* left, int* top,
                                     int* right, int* bottom)
{
    if (left != NULL) *left = 0;
    if (top != NULL) *top = 0;
    if (right != NULL) *right = window->osmesa.width;
    if (bottom != NULL) *top = window->osmesa.height;
}

void _glfwPlatformIconifyWindow(_GLFWwindow* window) {}

void _glfwPlatformRestoreWindow(_GLFWwindow* window) {}

void _glfwPlatformMaximizeWindow(_GLFWwindow* window) {}

int _glfwPlatformWindowMaximized(_GLFWwindow* window) {
  return 0;
}

void _glfwPlatformShowWindow(_GLFWwindow* window) {}

void _glfwPlatformUnhideWindow(_GLFWwindow* window) {}

void _glfwPlatformHideWindow(_GLFWwindow* window) {}

void _glfwPlatformFocusWindow(_GLFWwindow* window) {}

int _glfwPlatformWindowFocused(_GLFWwindow* window) { return GLFW_FALSE; }

int _glfwPlatformWindowIconified(_GLFWwindow* window) { return GLFW_FALSE; }

int _glfwPlatformWindowVisible(_GLFWwindow* window) { return GLFW_FALSE; }

void _glfwPlatformPollEvents(void) {}

void _glfwPlatformWaitEvents(void) {}

void _glfwPlatformWaitEventsTimeout(double timeout) {}

void _glfwPlatformPostEmptyEvent(void) {}

void _glfwPlatformGetCursorPos(_GLFWwindow* window, double* xpos, double* ypos) {
    if (xpos != NULL) *xpos = 0;
    if (ypos != NULL) *ypos = 0;
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double x, double y) {}

void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode) {}

void _glfwPlatformApplyCursorMode(_GLFWwindow* window) {}

int _glfwPlatformCreateCursor(_GLFWcursor* cursor, const GLFWimage* image,
                              int xhot, int yhot)
{
    return GLFW_FALSE;
}

int _glfwPlatformCreateStandardCursor(_GLFWcursor* cursor, int shape)
{
    return GLFW_FALSE;
}

void _glfwPlatformDestroyCursor(_GLFWcursor* cursor) {}

void _glfwPlatformSetCursor(_GLFWwindow* window, _GLFWcursor* cursor) {}

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string) {}

const char* _glfwPlatformGetClipboardString(_GLFWwindow* window)
{
    return NULL;
}

const char* _glfwPlatformGetKeyName(int key, int scancode) { return ""; }

int _glfwPlatformJoystickPresent(int joy) { return 0; }

const float* _glfwPlatformGetJoystickAxes(int joy, int* count)
{
    if (count != NULL) *count = 0;
    return NULL;
}

const unsigned char* _glfwPlatformGetJoystickButtons(int joy, int* count)
{
    if (count != NULL) *count = 0;
    return NULL;
}

const char* _glfwPlatformGetJoystickName(int joy) { return NULL; }

char** _glfwPlatformGetRequiredInstanceExtensions(uint32_t* count)
{
    if (count != NULL) *count = 0;
    return NULL;
}

int _glfwPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    return GLFW_FALSE;
}

VkResult _glfwPlatformCreateWindowSurface(VkInstance instance,
                                          _GLFWwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface)
{
    // This seems like the most appropriate error to return here.
    return VK_ERROR_INITIALIZATION_FAILED;
}

//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI int glfwGetOSMesaColorBuffer(GLFWwindow* window, int* width,
                                     int* height, int* format, void** buffer)
{
    GLint mesaWidth;
    GLint mesaHeight;
    GLint mesaFormat;
    void* mesaBuffer;

    assert(window != NULL);

    OSMesaContext ctx = ((_GLFWwindow*) window)->context.osmesa.handle;

    // Query OSMesa for the color buffer data.
    int result = OSMesaGetColorBuffer(
        ctx, &mesaWidth, &mesaHeight, &mesaFormat, &mesaBuffer);
    if (result) {
        // Copy the values returned by OSMesa.
        if (width != NULL) *width = mesaWidth;
        if (height != NULL) *height = mesaHeight;
        if (format != NULL) *format = mesaFormat;
        if (buffer != NULL) *buffer = mesaBuffer;
    }

    return result;
}

GLFWAPI int glfwGetOSMesaDepthBuffer(GLFWwindow* window, int* width,
                                     int* height, int* bytesPerValue,
                                     void** buffer)
{
    GLint mesaWidth;
    GLint mesaHeight;
    GLint mesaBytes;
    void* mesaBuffer;

    assert(window != NULL);

    OSMesaContext ctx = ((_GLFWwindow*) window)->context.osmesa.handle;

    // Query OSMesa for the color buffer data.
    int result = OSMesaGetDepthBuffer(
        ctx, &mesaWidth, &mesaHeight, &mesaBytes, &mesaBuffer);
    if (result) {
        // Copy the values returned by OSMesa.
        if (width != NULL) *width = mesaWidth;
        if (height != NULL) *height = mesaHeight;
        if (bytesPerValue != NULL) *bytesPerValue = mesaBytes;
        if (buffer != NULL) *buffer = mesaBuffer;
    }

    return result;
}

