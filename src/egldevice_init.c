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

#include "internal.h"

#include <linux/limits.h>
#include "egl_context.h"

static GLFWbool initializeExtensions()
{
    _glfw.egldevice.eglQueryDevicesEXT =
        (PFNEGLQUERYDEVICESEXTPROC)
            _glfw.egl.GetProcAddress("eglQueryDevicesEXT");
   _glfw.egldevice.eglQueryDeviceStringEXT =
        (PFNEGLQUERYDEVICESTRINGEXTPROC)
            _glfw.egl.GetProcAddress("eglQueryDeviceStringEXT");
    _glfw.egldevice.eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)
            _glfw.egl.GetProcAddress("eglGetPlatformDisplayEXT");
    _glfw.egldevice.eglGetOutputLayersEXT =
        (PFNEGLGETOUTPUTLAYERSEXTPROC)
            _glfw.egl.GetProcAddress("eglGetOutputLayersEXT");
    _glfw.egldevice.eglCreateStreamKHR =
        (PFNEGLCREATESTREAMKHRPROC)
            _glfw.egl.GetProcAddress("eglCreateStreamKHR");
    _glfw.egldevice.eglDestroyStreamKHR =
        (PFNEGLDESTROYSTREAMKHRPROC)
            _glfw.egl.GetProcAddress("eglDestroyStreamKHR");
    _glfw.egldevice.eglStreamConsumerOutputEXT =
        (PFNEGLSTREAMCONSUMEROUTPUTEXTPROC)
            _glfw.egl.GetProcAddress("eglStreamConsumerOutputEXT");
    _glfw.egldevice.eglCreateStreamProducerSurfaceKHR =
        (PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC)
            _glfw.egl.GetProcAddress("eglCreateStreamProducerSurfaceKHR");

    if(!_glfw.egldevice.eglQueryDevicesEXT ||
       !_glfw.egldevice.eglQueryDeviceStringEXT ||
       !_glfw.egldevice.eglGetPlatformDisplayEXT ||
       !_glfw.egldevice.eglGetOutputLayersEXT ||
       !_glfw.egldevice.eglCreateStreamKHR ||
       !_glfw.egldevice.eglDestroyStreamKHR ||
       !_glfw.egldevice.eglStreamConsumerOutputEXT ||
       !_glfw.egldevice.eglCreateStreamProducerSurfaceKHR)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required function(s)");
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

    if (!_glfw.egldevice.eglQueryDevicesEXT(0, NULL, &num_devs))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Falied to query EGLDevice");
    }
    if (num_devs < 1)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: No Devices Found");
    }

    egl_devs = calloc(sizeof(EGLDeviceEXT), num_devs);
    if (egl_devs == NULL)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Unable to allocate memory for device storage");
    }

    // Select suitable device
    if (!_glfw.egldevice.eglQueryDevicesEXT(num_devs, egl_devs, &num_devs))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Failed to query EGL devices");
    }

    for (i = 0; i <= num_devs; i++)
    {
        const char* deviceExtensionString;

        deviceExtensionString = 
            _glfw.egldevice.eglQueryDeviceStringEXT(egl_devs[i], EGL_EXTENSIONS);
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
                        "EGLDevice: Missing required extension:"
                        " EGL_EXT_device_drm");
    }
    return eglDevice;
}

static int getDRMFd(EGLDeviceEXT eglDevice)
{
    int drm_fd;
    const char* drmName;

    drmName = _glfw.egldevice.eglQueryDeviceStringEXT(eglDevice,
                                                      EGL_DRM_DEVICE_FILE_EXT);
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

    _glfw.egl.display =
        _glfw.egldevice.eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT,
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
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "EGL: Failed to initialize EGL: %s",
                        eglGetError());

        return GLFW_FALSE;
    }

    // Check for stream_consumer_egloutput + output_drm support
    displayExtensionString = eglQueryString(_glfw.egl.display, EGL_EXTENSIONS);
    if (!_glfwStringInExtensionString("EGL_EXT_output_base",
                                      displayExtensionString))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required extension:"
                        " EGL_EXT_output_base");
        return GLFW_FALSE;
    }

    if (!_glfwStringInExtensionString("EGL_EXT_output_drm",
                                      displayExtensionString))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required extension:"
                        " EGL_EXT_output_drm");
        return GLFW_FALSE;
    }

    if (!_glfwStringInExtensionString("EGL_KHR_stream",
                                      displayExtensionString))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required extension:"
                        " EGL_KHR_stream");
        return GLFW_FALSE;
    }

    if (!_glfwStringInExtensionString("EGL_KHR_stream_producer_eglsurface",
                                      displayExtensionString))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required extension:"
                        " EGL_KHR_stream_producer_eglsurface");
        return GLFW_FALSE;
    }
    if (!_glfwStringInExtensionString("EGL_EXT_stream_consumer_egloutput",
                                      displayExtensionString))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "EGLDevice: Missing required extension:"
                        " EGL_EXT_stream_consumer_egloutput");
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

    if (!_glfwInitThreadLocalStoragePOSIX())
        return GLFW_FALSE;

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

    return GLFW_TRUE;
}

void _glfwPlatformTerminate(void)
{
    _glfwTerminateEGL();
    _glfwTerminateJoysticksLinux();
    _glfwTerminateThreadLocalStoragePOSIX();
}

const char* _glfwPlatformGetVersionString(void)
{
    return _GLFW_VERSION_NUMBER "EGLDEVICE"
#if defined(_GLFW_EGL)
        " EGL"
#endif
#if defined(_GLFW_BUILD_DLL)
        " shared"
#endif
        ;
}
