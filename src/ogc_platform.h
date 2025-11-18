//========================================================================
// GLFW 3.5 OGC - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2024 Alberto Mardegan <info@mardy.it>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include <ogc/gx.h>
#include <ogc/system.h>
#include <ogc/video.h>

#define GLFW_OGC_WINDOW_STATE         _GLFWwindowOgc  ogc;
#define GLFW_OGC_LIBRARY_WINDOW_STATE _GLFWlibraryOgc ogc;
#define GLFW_OGC_MONITOR_STATE        _GLFWmonitorOgc ogc;
#define GLFW_OGC_CURSOR_STATE         _GLFWcursorOgc  ogc;

extern char _glfwUnimplementedFmt[];

#if 0
typedef struct ogc_cursor_theme* (* PFN_ogc_cursor_theme_load)(const char*, int, struct ogc_shm*);
typedef void (* PFN_ogc_cursor_theme_destroy)(struct ogc_cursor_theme*);
typedef struct ogc_cursor* (* PFN_ogc_cursor_theme_get_cursor)(struct ogc_cursor_theme*, const char*);
typedef struct ogc_buffer* (* PFN_ogc_cursor_image_get_buffer)(struct ogc_cursor_image*);
#define ogc_cursor_theme_load _glfw.ogc.cursor.theme_load
#define ogc_cursor_theme_destroy _glfw.ogc.cursor.theme_destroy
#define ogc_cursor_theme_get_cursor _glfw.ogc.cursor.theme_get_cursor
#define ogc_cursor_image_get_buffer _glfw.ogc.cursor.image_get_buffer

typedef struct ogc_egl_window* (* PFN_ogc_egl_window_create)(struct ogc_surface*, int, int);
typedef void (* PFN_ogc_egl_window_destroy)(struct ogc_egl_window*);
typedef void (* PFN_ogc_egl_window_resize)(struct ogc_egl_window*, int, int, int, int);
#define ogc_egl_window_create _glfw.ogc.egl.window_create
#define ogc_egl_window_destroy _glfw.ogc.egl.window_destroy
#define ogc_egl_window_resize _glfw.ogc.egl.window_resize

typedef struct xkb_context* (* PFN_xkb_context_new)(enum xkb_context_flags);
typedef void (* PFN_xkb_context_unref)(struct xkb_context*);
typedef struct xkb_keymap* (* PFN_xkb_keymap_new_from_string)(struct xkb_context*, const char*, enum xkb_keymap_format, enum xkb_keymap_compile_flags);
typedef void (* PFN_xkb_keymap_unref)(struct xkb_keymap*);
typedef xkb_mod_index_t (* PFN_xkb_keymap_mod_get_index)(struct xkb_keymap*, const char*);
typedef int (* PFN_xkb_keymap_key_repeats)(struct xkb_keymap*, xkb_keycode_t);
typedef int (* PFN_xkb_keymap_key_get_syms_by_level)(struct xkb_keymap*,xkb_keycode_t,xkb_layout_index_t,xkb_level_index_t,const xkb_keysym_t**);
typedef struct xkb_state* (* PFN_xkb_state_new)(struct xkb_keymap*);
typedef void (* PFN_xkb_state_unref)(struct xkb_state*);
typedef int (* PFN_xkb_state_key_get_syms)(struct xkb_state*, xkb_keycode_t, const xkb_keysym_t**);
typedef enum xkb_state_component (* PFN_xkb_state_update_mask)(struct xkb_state*, xkb_mod_mask_t, xkb_mod_mask_t, xkb_mod_mask_t, xkb_layout_index_t, xkb_layout_index_t, xkb_layout_index_t);
typedef xkb_layout_index_t (* PFN_xkb_state_key_get_layout)(struct xkb_state*,xkb_keycode_t);
typedef int (* PFN_xkb_state_mod_index_is_active)(struct xkb_state*,xkb_mod_index_t,enum xkb_state_component);
typedef uint32_t (* PFN_xkb_keysym_to_utf32)(xkb_keysym_t);
typedef int (* PFN_xkb_keysym_to_utf8)(xkb_keysym_t, char*, size_t);
#define xkb_context_new _glfw.ogc.xkb.context_new
#define xkb_context_unref _glfw.ogc.xkb.context_unref
#define xkb_keymap_new_from_string _glfw.ogc.xkb.keymap_new_from_string
#define xkb_keymap_unref _glfw.ogc.xkb.keymap_unref
#define xkb_keymap_mod_get_index _glfw.ogc.xkb.keymap_mod_get_index
#define xkb_keymap_key_repeats _glfw.ogc.xkb.keymap_key_repeats
#define xkb_keymap_key_get_syms_by_level _glfw.ogc.xkb.keymap_key_get_syms_by_level
#define xkb_state_new _glfw.ogc.xkb.state_new
#define xkb_state_unref _glfw.ogc.xkb.state_unref
#define xkb_state_key_get_syms _glfw.ogc.xkb.state_key_get_syms
#define xkb_state_update_mask _glfw.ogc.xkb.state_update_mask
#define xkb_state_key_get_layout _glfw.ogc.xkb.state_key_get_layout
#define xkb_state_mod_index_is_active _glfw.ogc.xkb.state_mod_index_is_active
#define xkb_keysym_to_utf32 _glfw.ogc.xkb.keysym_to_utf32
#define xkb_keysym_to_utf8 _glfw.ogc.xkb.keysym_to_utf8

typedef struct xkb_compose_table* (* PFN_xkb_compose_table_new_from_locale)(struct xkb_context*, const char*, enum xkb_compose_compile_flags);
typedef void (* PFN_xkb_compose_table_unref)(struct xkb_compose_table*);
typedef struct xkb_compose_state* (* PFN_xkb_compose_state_new)(struct xkb_compose_table*, enum xkb_compose_state_flags);
typedef void (* PFN_xkb_compose_state_unref)(struct xkb_compose_state*);
typedef enum xkb_compose_feed_result (* PFN_xkb_compose_state_feed)(struct xkb_compose_state*, xkb_keysym_t);
typedef enum xkb_compose_status (* PFN_xkb_compose_state_get_status)(struct xkb_compose_state*);
typedef xkb_keysym_t (* PFN_xkb_compose_state_get_one_sym)(struct xkb_compose_state*);
#define xkb_compose_table_new_from_locale _glfw.ogc.xkb.compose_table_new_from_locale
#define xkb_compose_table_unref _glfw.ogc.xkb.compose_table_unref
#define xkb_compose_state_new _glfw.ogc.xkb.compose_state_new
#define xkb_compose_state_unref _glfw.ogc.xkb.compose_state_unref
#define xkb_compose_state_feed _glfw.ogc.xkb.compose_state_feed
#define xkb_compose_state_get_status _glfw.ogc.xkb.compose_state_get_status
#define xkb_compose_state_get_one_sym _glfw.ogc.xkb.compose_state_get_one_sym
#endif

// Ogc-specific per-window data
//
typedef struct _GLFWwindowOgc
{
    int                         width, height;
    GLFWbool                    maximized;
    GLFWbool                    activated;
    GLFWbool                    fullscreen;
    GLFWbool                    hovered;
    GLFWbool                    transparent;
    GLFWbool                    scaleFramebuffer;
    struct ogc_surface*          surface;
    struct ogc_callback*         callback;

    _GLFWcursor*                currentCursor;
    double                      cursorPosX, cursorPosY;

    char*                       appId;
} _GLFWwindowOgc;

// Ogc-specific global data
//
typedef struct _GLFWlibraryOgc
{
    void *                      xfb[2];
    int                         fbIndex;

    struct ogc_cursor_theme*    cursorTheme;
    struct ogc_cursor_theme*    cursorThemeHiDPI;
    struct ogc_surface*         cursorSurface;
    const char*                 cursorPreviousName;
    int                         cursorTimerfd;
    uint32_t                    serial;
    uint32_t                    pointerEnterSerial;

    int                         keyRepeatTimerfd;
    int32_t                     keyRepeatRate;
    int32_t                     keyRepeatDelay;
    int                         keyRepeatScancode;

    char*                       clipboardString;
    short int                   keycodes[256];
    short int                   scancodes[GLFW_KEY_LAST + 1];
    char                        keynames[GLFW_KEY_LAST + 1][5];

    _GLFWwindow*                pointerFocus;
    _GLFWwindow*                keyboardFocus;
} _GLFWlibraryOgc;

// Ogc-specific per-monitor data
//
typedef struct _GLFWmonitorOgc
{
    int                         currentMode;
    GXRModeObj*                 ogcMode;
} _GLFWmonitorOgc;

// Ogc-specific per-cursor data
//
typedef struct _GLFWcursorOgc
{
    GXTexObj                    texobj;
    short                       xhot, yhot;
    GLFWbool                    canRotate;
} _GLFWcursorOgc;

GLFWbool _glfwConnectOgc(int platformID, _GLFWplatform* platform);
int _glfwInitOgc(void);
void _glfwTerminateOgc(void);

void _glfwCreateMonitorOgc(void);
void _glfwSetVideoModeOgc(_GLFWmonitor* monitor, const GLFWvidmode* desired);

GLFWbool _glfwCreateContextOgc(_GLFWwindow* window,
                               const _GLFWctxconfig* ctxconfig,
                               const _GLFWfbconfig* fbconfig);

GLFWbool _glfwCreateWindowOgc(_GLFWwindow* window, const _GLFWwndconfig* wndconfig, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig);
void _glfwDestroyWindowOgc(_GLFWwindow* window);
void _glfwSetWindowTitleOgc(_GLFWwindow* window, const char* title);
void _glfwSetWindowIconOgc(_GLFWwindow* window, int count, const GLFWimage* images);
void _glfwGetWindowPosOgc(_GLFWwindow* window, int* xpos, int* ypos);
void _glfwSetWindowPosOgc(_GLFWwindow* window, int xpos, int ypos);
void _glfwGetWindowSizeOgc(_GLFWwindow* window, int* width, int* height);
void _glfwSetWindowSizeOgc(_GLFWwindow* window, int width, int height);
void _glfwSetWindowSizeLimitsOgc(_GLFWwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _glfwSetWindowAspectRatioOgc(_GLFWwindow* window, int numer, int denom);
void _glfwGetFramebufferSizeOgc(_GLFWwindow* window, int* width, int* height);
void _glfwGetWindowFrameSizeOgc(_GLFWwindow* window, int* left, int* top, int* right, int* bottom);
void _glfwGetWindowContentScaleOgc(_GLFWwindow* window, float* xscale, float* yscale);
void _glfwIconifyWindowOgc(_GLFWwindow* window);
void _glfwRestoreWindowOgc(_GLFWwindow* window);
void _glfwMaximizeWindowOgc(_GLFWwindow* window);
void _glfwShowWindowOgc(_GLFWwindow* window);
void _glfwHideWindowOgc(_GLFWwindow* window);
void _glfwRequestWindowAttentionOgc(_GLFWwindow* window);
void _glfwFocusWindowOgc(_GLFWwindow* window);
void _glfwSetWindowMonitorOgc(_GLFWwindow* window, _GLFWmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
GLFWbool _glfwWindowFocusedOgc(_GLFWwindow* window);
GLFWbool _glfwWindowIconifiedOgc(_GLFWwindow* window);
GLFWbool _glfwWindowVisibleOgc(_GLFWwindow* window);
GLFWbool _glfwWindowMaximizedOgc(_GLFWwindow* window);
GLFWbool _glfwWindowHoveredOgc(_GLFWwindow* window);
GLFWbool _glfwFramebufferTransparentOgc(_GLFWwindow* window);
void _glfwSetWindowResizableOgc(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowDecoratedOgc(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowFloatingOgc(_GLFWwindow* window, GLFWbool enabled);
float _glfwGetWindowOpacityOgc(_GLFWwindow* window);
void _glfwSetWindowOpacityOgc(_GLFWwindow* window, float opacity);
void _glfwSetWindowMousePassthroughOgc(_GLFWwindow* window, GLFWbool enabled);

void _glfwSetRawMouseMotionOgc(_GLFWwindow* window, GLFWbool enabled);
GLFWbool _glfwRawMouseMotionSupportedOgc(void);

void _glfwPollEventsOgc(void);
void _glfwWaitEventsOgc(void);
void _glfwWaitEventsTimeoutOgc(double timeout);
void _glfwPostEmptyEventOgc(void);

void _glfwGetCursorPosOgc(_GLFWwindow* window, double* xpos, double* ypos);
void _glfwSetCursorPosOgc(_GLFWwindow* window, double xpos, double ypos);
void _glfwSetCursorModeOgc(_GLFWwindow* window, int mode);
const char* _glfwGetScancodeNameOgc(int scancode);
int _glfwGetKeyScancodeOgc(int key);
GLFWbool _glfwCreateCursorOgc(_GLFWcursor* cursor, const GLFWimage* image, int xhot, int yhot);
GLFWbool _glfwCreateStandardCursorOgc(_GLFWcursor* cursor, int shape);
void _glfwDestroyCursorOgc(_GLFWcursor* cursor);
void _glfwSetCursorOgc(_GLFWwindow* window, _GLFWcursor* cursor);
void _glfwSetClipboardStringOgc(const char* string);
const char* _glfwGetClipboardStringOgc(void);

EGLenum _glfwGetEGLPlatformOgc(EGLint** attribs);
EGLNativeDisplayType _glfwGetEGLNativeDisplayOgc(void);
EGLNativeWindowType _glfwGetEGLNativeWindowOgc(_GLFWwindow* window);

void _glfwGetRequiredInstanceExtensionsOgc(char** extensions);
GLFWbool _glfwGetPhysicalDevicePresentationSupportOgc(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
VkResult _glfwCreateWindowSurfaceOgc(VkInstance instance, _GLFWwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

void _glfwFreeMonitorOgc(_GLFWmonitor* monitor);
void _glfwGetMonitorPosOgc(_GLFWmonitor* monitor, int* xpos, int* ypos);
void _glfwGetMonitorContentScaleOgc(_GLFWmonitor* monitor, float* xscale, float* yscale);
void _glfwGetMonitorWorkareaOgc(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
GLFWvidmode* _glfwGetVideoModesOgc(_GLFWmonitor* monitor, int* count);
GLFWbool _glfwGetVideoModeOgc(_GLFWmonitor* monitor, GLFWvidmode* mode);
GLFWbool _glfwGetGammaRampOgc(_GLFWmonitor* monitor, GLFWgammaramp* ramp);
void _glfwSetGammaRampOgc(_GLFWmonitor* monitor, const GLFWgammaramp* ramp);

void _glfwAddOutputOgc(uint32_t name, uint32_t version);
void _glfwUpdateBufferScaleFromOutputsOgc(_GLFWwindow* window);

