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

#include <dlfcn.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#include "posix_time.h"
#include "linux_joystick.h"
#include "posix_thread.h"
#include "egl_context.h"
#include "osmesa_context.h"

#define EGL_NO_DEVICE_EXT ((EGLDeviceEXT) 0)
#define EGL_DRM_DEVICE_FILE_EXT 0x3233
#define EGL_PLATFORM_DEVICE_EXT 0x313f
#define EGL_DRM_MASTER_FD_EXT 0x333c
#define EGL_STREAM_FIFO_LENGTH_KHR 0x31fc
#define EGL_DRM_CRTC_EXT 0x3234
#define EGL_NO_STREAM_KHR ((EGLStreamKHR) 0)
#define EGL_STREAM_BIT_KHR 0x0800

typedef void* EGLOutputLayerEXT;
typedef void* EGLStreamKHR;
typedef void* EGLDeviceEXT;

typedef EGLBoolean (EGLAPIENTRY * PFNEGLQUERYDEVICESEXTPROC)(EGLint,EGLDeviceEXT*,EGLint*);
typedef const char *(EGLAPIENTRY * PFNEGLQUERYDEVICESTRINGEXTPROC)(EGLDeviceEXT,EGLint);
typedef EGLDisplay (EGLAPIENTRY * PFNEGLGETPLATFORMDISPLAYEXTPROC)(EGLenum,void*,const EGLint*);
typedef EGLBoolean (EGLAPIENTRY * PFNEGLGETOUTPUTLAYERSEXTPROC)(EGLDisplay,const EGLAttrib*,EGLOutputLayerEXT*,EGLint,EGLint*);
typedef EGLStreamKHR (EGLAPIENTRY * PFNEGLCREATESTREAMKHRPROC)(EGLDisplay,const EGLint*);
typedef EGLBoolean (EGLAPIENTRY * PFNEGLDESTROYSTREAMKHRPROC)(EGLDisplay,EGLStreamKHR);
typedef EGLBoolean (EGLAPIENTRY * PFNEGLSTREAMCONSUMEROUTPUTEXTPROC)(EGLDisplay,EGLStreamKHR,EGLOutputLayerEXT);
typedef EGLSurface (EGLAPIENTRY * PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC)(EGLDisplay,EGLConfig,EGLStreamKHR,const EGLint*);
typedef EGLBoolean (EGLAPIENTRY * PFNEGLSTREAMATTRIBKHRPROC)(EGLDisplay,EGLStreamKHR,EGLenum,EGLint);
typedef EGLBoolean (EGLAPIENTRY * PFNEGLSTREAMCONSUMERACQUIREATTRIBKHRPROC)(EGLDisplay,EGLStreamKHR,const EGLAttrib*);

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
    PFNEGLSTREAMCONSUMERACQUIREATTRIBKHRPROC eglStreamConsumerAcquireAttribKHR;
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

void _glfwPollMonitorsEGLDevice(void);

#endif // _glfw3_egldevice_platform_h_
