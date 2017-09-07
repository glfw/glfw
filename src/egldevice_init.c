//========================================================================
// GLFW 3.3 EGLDevice - www.glfw.org
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

#include "internal.h"

#include <linux/limits.h>

static GLFWbool initializeExtensions(void)
{
    _glfw.egldevice.QueryDevicesEXT =
        (PFNEGLQUERYDEVICESEXTPROC)
            eglGetProcAddress("eglQueryDevicesEXT");
   _glfw.egldevice.QueryDeviceStringEXT =
        (PFNEGLQUERYDEVICESTRINGEXTPROC)
            eglGetProcAddress("eglQueryDeviceStringEXT");
    _glfw.egldevice.GetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)
            eglGetProcAddress("eglGetPlatformDisplayEXT");
    _glfw.egldevice.GetOutputLayersEXT =
        (PFNEGLGETOUTPUTLAYERSEXTPROC)
            eglGetProcAddress("eglGetOutputLayersEXT");
    _glfw.egldevice.CreateStreamKHR =
        (PFNEGLCREATESTREAMKHRPROC)
            eglGetProcAddress("eglCreateStreamKHR");
    _glfw.egldevice.DestroyStreamKHR =
        (PFNEGLDESTROYSTREAMKHRPROC)
            eglGetProcAddress("eglDestroyStreamKHR");
    _glfw.egldevice.StreamConsumerOutputEXT =
        (PFNEGLSTREAMCONSUMEROUTPUTEXTPROC)
            eglGetProcAddress("eglStreamConsumerOutputEXT");
    _glfw.egldevice.CreateStreamProducerSurfaceKHR =
        (PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC)
            eglGetProcAddress("eglCreateStreamProducerSurfaceKHR");

    if(!_glfw.egldevice.QueryDevicesEXT ||
       !_glfw.egldevice.QueryDeviceStringEXT ||
       !_glfw.egldevice.GetPlatformDisplayEXT ||
       !_glfw.egldevice.GetOutputLayersEXT ||
       !_glfw.egldevice.CreateStreamKHR ||
       !_glfw.egldevice.DestroyStreamKHR ||
       !_glfw.egldevice.StreamConsumerOutputEXT ||
       !_glfw.egldevice.CreateStreamProducerSurfaceKHR)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Failed to find required EGL extension functions");
        return GLFW_FALSE;
    }
    return GLFW_TRUE;
}

static EGLDeviceEXT getEGLDevice(void)
{
    int i, num_devs;
    EGLDeviceEXT* egl_devs, eglDevice;
    const char *clientExtensionString;

    eglDevice = EGL_NO_DEVICE_EXT;
    clientExtensionString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!_glfwStringInExtensionString("EGL_EXT_device_base",
                                      clientExtensionString) &&
        (!_glfwStringInExtensionString("EGL_EXT_device_enumeration",
                                       clientExtensionString) ||
         !_glfwStringInExtensionString("EGL_EXT_device_query",
                                       clientExtensionString)))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: EGL_EXT_device base extensions not found");
    }

    if (!eglQueryDevicesEXT(0, NULL, &num_devs))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Falied to query EGLDevice");
    }
    if (num_devs < 1)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: No devices found");
    }

    egl_devs = calloc(sizeof(EGLDeviceEXT), num_devs);
    if (egl_devs == NULL)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Unable to allocate memory for device storage");
    }

    // Select suitable device
    if (!eglQueryDevicesEXT(num_devs, egl_devs, &num_devs))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Failed to query EGL devices");
    }

    for (i = 0; i <= num_devs; i++)
    {
        const char* deviceExtensionString;

        deviceExtensionString = eglQueryDeviceStringEXT(egl_devs[i], EGL_EXTENSIONS);
        if (_glfwStringInExtensionString("EGL_EXT_device_drm",
                                         deviceExtensionString))
        {
            eglDevice = egl_devs[i];
            break;
        }
    }

    free(egl_devs);

    if (eglDevice == EGL_NO_DEVICE_EXT)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required extension EGL_EXT_device_drm");
    }
    return eglDevice;
}

static int getDRMFd(EGLDeviceEXT eglDevice)
{
    int drm_fd;
    const char* drmName;

    drmName = eglQueryDeviceStringEXT(eglDevice, EGL_DRM_DEVICE_FILE_EXT);
    if (!drmName || (strnlen(drmName, PATH_MAX) == 0))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Couldn't obtain device file from 0x%p",
                        (void*)(uintptr_t)eglDevice);
    }

    drm_fd = drmOpen(drmName, NULL);
    if (drm_fd < 0)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Couldn't open device file '%s'", drmName);
    }

    return drm_fd;
}

static GLFWbool initEGLDisplay(EGLDeviceEXT egl_dev, int drm_fd)
{
    const char* displayExtensionString;
    EGLint displayAttribs[] = {
        EGL_DRM_MASTER_FD_EXT,
        drm_fd,
        EGL_NONE
    };

    _glfw.egl.display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT,
                                                 (void*)egl_dev,
                                                 displayAttribs);
    if (_glfw.egl.display == EGL_NO_DISPLAY)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Couldn't obtain EGLDisplay for device");
        return GLFW_FALSE;
    }

    if (!eglInitialize(_glfw.egl.display, &_glfw.egl.major, &_glfw.egl.minor))
    {
        _glfwInputError(GLFW_API_UNAVAILABLE, "EGL: Failed to initialize EGL");
        return GLFW_FALSE;
    }

    // Check for stream_consumer_egloutput + output_drm support
    displayExtensionString = eglQueryString(_glfw.egl.display, EGL_EXTENSIONS);
    if (!_glfwStringInExtensionString("EGL_EXT_output_base",
                                      displayExtensionString))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required extension EGL_EXT_output_base");
        return GLFW_FALSE;
    }

    if (!_glfwStringInExtensionString("EGL_EXT_output_drm",
                                      displayExtensionString))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required extension EGL_EXT_output_drm");
        return GLFW_FALSE;
    }

    if (!_glfwStringInExtensionString("EGL_KHR_stream",
                                      displayExtensionString))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required extension EGL_KHR_stream");
        return GLFW_FALSE;
    }

    if (!_glfwStringInExtensionString("EGL_KHR_stream_producer_eglsurface",
                                      displayExtensionString))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required extension EGL_KHR_stream_producer_eglsurface");
        return GLFW_FALSE;
    }
    if (!_glfwStringInExtensionString("EGL_EXT_stream_consumer_egloutput",
                                      displayExtensionString))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required extension EGL_EXT_stream_consumer_egloutput");
        return GLFW_FALSE;
    }

    return GLFW_TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformInit(void)
{
    EGLDeviceEXT egl_dev;
    int drm_fd;

    // Initialize EGL
    if (!_glfwInitEGL())
        return GLFW_FALSE;

    // Initialize global data and extension function pointers
    if (!initializeExtensions())
        return GLFW_FALSE;

    // Query and Obtain EGLDevice
    egl_dev = getEGLDevice();

    // Obtain and open DRM device file
    drm_fd = getDRMFd(egl_dev);

    // Store for use later
    _glfw.egldevice.drmFd = drm_fd;

    // Obtain EGLDisplay
    if (!initEGLDisplay(egl_dev, drm_fd))
        return GLFW_FALSE;

   _glfwInitTimerPOSIX();

   _glfwPollMonitorsEGLDevice();

    return GLFW_TRUE;
}

void _glfwPlatformTerminate(void)
{
    _glfwTerminateEGL();
    _glfwTerminateJoysticksLinux();
}

const char* _glfwPlatformGetVersionString(void)
{
    return _GLFW_VERSION_NUMBER "EGLDevice EGL"
#if defined(_GLFW_BUILD_DLL)
        " shared"
#endif
        ;
}

