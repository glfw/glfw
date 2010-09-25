/* Configure build time options of GLFW */

/* Define this to 1 if XRandR is available */
#cmakedefine _GLFW_HAS_XRANDR      1
/* Define this to 1 if Xf86VidMode is available */
#cmakedefine _GLFW_HAS_XF86VIDMODE 1

/* Define this to 1 if glXGetProcAddress is available */
#cmakedefine _GLFW_HAS_GLXGETPROCADDRESS 1
/* Define this to 1 if glXGetProcAddressARB is available */
#cmakedefine _GLFW_HAS_GLXGETPROCADDRESSARB 1
/* Define this to 1 if glXGetProcAddressEXT is available */
#cmakedefine _GLFW_HAS_GLXGETPROCADDRESSEXT 1

/* Define this to 1 if the Linux joystick API is available */
#cmakedefine _GLFW_USE_LINUX_JOYSTICKS 1

/* Define this to 1 to not load gdi32.dll dynamically */
#cmakedefine _GLFW_NO_DLOAD_GDI32 1
/* Define this to 1 to not load winmm.dll dynamically */
#cmakedefine _GLFW_NO_DLOAD_WINMM 1

#define _GLFW_VERSION_FULL "@GLFW_VERSION_FULL@"

