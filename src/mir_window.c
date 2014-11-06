#include "internal.h"

MirPixelFormat FindValidPixelFormat()
{
    unsigned int pf_size = 32;
    unsigned int valid_formats;
    unsigned int f;

    MirPixelFormat formats[pf_size];
    mir_connection_get_available_surface_formats(_glfw.mir.connection, formats,
                                                 pf_size, &valid_formats);

    for (f = 0; f < valid_formats; f++)
    {
        MirPixelFormat cur_pf = formats[f];

        if (cur_pf == mir_pixel_format_abgr_8888 ||
            cur_pf == mir_pixel_format_xbgr_8888 ||
            cur_pf == mir_pixel_format_argb_8888 ||
            cur_pf == mir_pixel_format_xrgb_8888)
        {
            return cur_pf;
        }
    }

    return mir_pixel_format_invalid;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWctxconfig* ctxconfig,
                              const _GLFWfbconfig* fbconfig)
{
    if (!_glfwCreateContext(window, ctxconfig, fbconfig))
        return GL_FALSE;

    // FIXME Add a check here to ensure we are within our max width/height
    window->mir.width  = wndconfig->width;
    window->mir.height = wndconfig->height;

    MirSurfaceParameters params = 
    {
        .name         = "MirSurface",
        .width        = wndconfig->width,
        .height       = wndconfig->height,
        .pixel_format = mir_pixel_format_invalid,
        .buffer_usage = mir_buffer_usage_hardware,
        .output_id    = mir_display_output_id_invalid
    };

/*  // Add the HandleInput function somewhere... to handle events from the windows
    MirEventDelegate delegate =
    {
        HandleInput,
        NULL
    };

    mir_surface_set_event_handler(window->mir.surface, &delegate);
*/

    params.pixel_format = FindValidPixelFormat();
    if (params.pixel_format == mir_pixel_format_invalid)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Mir: Unable to find a correct pixel format!\n");
        return GL_FALSE;
    }

    window->mir.surface = mir_connection_create_surface_sync(_glfw.mir.connection, &params);
    if (!mir_surface_is_valid(window->mir.surface))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Mir: Unable to create surface!\n");
        return GL_FALSE;
    }

    window->mir.native_window = mir_surface_get_egl_native_window(window->mir.surface);

    return GL_TRUE;
}

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
  if (mir_surface_is_valid(window->mir.surface))
  {
    mir_surface_release_sync(window->mir.surface);
    window->mir.surface = NULL;
  }
}

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported Function %s!\n", __PRETTY_FUNCTION__);
}

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int xpos, int ypos)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported Function %s!\n", __PRETTY_FUNCTION__);
}

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported Function %s!\n", __PRETTY_FUNCTION__);
}

void _glfwPlatformGetWindowSize(_GLFWwindow* window, int* width, int* height)
{
    *width  = window->mir.width;
    *height = window->mir.height;
}

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported Function %s!\n", __PRETTY_FUNCTION__);
}

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
}

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
}

// Mir does event handling in a different thread, so windows get events directly as they happen
void _glfwPlatformPollEvents(void)
{
}

void _glfwPlatformWaitEvents(void)
{
}

void _glfwPlatformPostEmptyEvent(void)
{
}

void _glfwPlatformGetFramebufferSize(_GLFWwindow* window, int* width, int* height)
{
    *width  = window->mir.width;
    *height = window->mir.height;
}

void _glfwPlatformGetWindowFrameSize(_GLFWwindow* window, int* left, int* top, int* right, int* bottom)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported Function %s!\n", __PRETTY_FUNCTION__);
}

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
}

void _glfwPlatformUnhideWindow(_GLFWwindow* window)
{
}

void _glfwPlatformGetWindowPos(_GLFWwindow* window, int* xpos, int* ypos)
{
}

int _glfwPlatformCreateCursor(_GLFWcursor* cursor, const GLFWimage* image, int xhot, int yhot)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported Function %s!\n", __PRETTY_FUNCTION__);

    return 0;
}

void _glfwPlatformDestroyCursor(_GLFWcursor* cursor)
{
}

void _glfwPlatformSetCursor(_GLFWwindow* window, _GLFWcursor* cursor)
{
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double xpos, double ypos)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported Function %s!\n", __PRETTY_FUNCTION__);
}

void _glfwPlatformApplyCursorMode(_GLFWwindow* window)
{
}

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported Function %s!\n", __PRETTY_FUNCTION__);
}

const char* _glfwPlatformGetClipboardString(_GLFWwindow* window)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported Function %s!\n", __PRETTY_FUNCTION__);

    return NULL;
}
