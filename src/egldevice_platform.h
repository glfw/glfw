//========================================================================
// GLFW 3.2 EGLDevice - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
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

#ifndef _glfw3_egldevice_h_
#define _glfw3_egldevice_h_

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <dlfcn.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include "posix_time.h"
#include "linux_joystick.h"
#include "posix_tls.h"

#include "egl_context.h"

#define _glfw_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define _glfw_dlclose(handle) dlclose(handle)
#define _glfw_dlsym(handle, name) dlsym(handle, name)

#define _GLFW_EGL_NATIVE_DISPLAY EGL_DEFAULT_DISPLAY
#define _GLFW_EGL_NATIVE_WINDOW ((EGLNativeWindowType)window->egldevice.handle)

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowEgldevice egldevice
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryEgldevice egldevice
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorEgldevice egldevice
#define _GLFW_PLATFORM_CURSOR_STATE         _GLFWcursorEgldevice egldevice

#define _GLFW_PLATFORM_CONTEXT_STATE
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE

// EGLDEVICE-specific per-window data
//
typedef struct _GLFWwindowEgldevice
{
    int xsurfsize, ysurfsize;
    int xoffset, yoffset;
    int fifo;

    uint32_t planeId;

    EGLDisplay handle;
    EGLOutputLayerEXT eglLayer;
    EGLStreamKHR eglStream;
} _GLFWwindowEgldevice;

// EGLDEVICE-specific global data
//
typedef struct _GLFWlibraryEgldevice
{
    int drmFd;

    PFNEGLQUERYDEVICESEXTPROC                eglQueryDevicesEXT;
    PFNEGLQUERYDEVICESTRINGEXTPROC           eglQueryDeviceStringEXT;
    PFNEGLGETPLATFORMDISPLAYEXTPROC          eglGetPlatformDisplayEXT;
    PFNEGLGETOUTPUTLAYERSEXTPROC             eglGetOutputLayersEXT;
    PFNEGLCREATESTREAMKHRPROC                eglCreateStreamKHR;
    PFNEGLDESTROYSTREAMKHRPROC               eglDestroyStreamKHR;
    PFNEGLSTREAMCONSUMEROUTPUTEXTPROC        eglStreamConsumerOutputEXT;
    PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC eglCreateStreamProducerSurfaceKHR;
    PFNEGLSTREAMATTRIBKHRPROC                eglStreamAttribKHR;
    PFNEGLSTREAMCONSUMERACQUIREATTRIBEXTPROC eglStreamConsumerAcquireAttribEXT;
} _GLFWlibraryEgldevice;

// EGLDEVICE-specific per-monitor data
//
typedef struct _GLFWmonitorEgldevice {
    int crtcIndex;
    uint32_t connId, encId, crtcId;
} _GLFWmonitorEgldevice;

// EGLDEVICE-specific per-cursor data
//
typedef struct _GLFWcursorEgldevice {
} _GLFWcursorEgldevice;

#endif // _glfw3_egldevice_platform_h_
