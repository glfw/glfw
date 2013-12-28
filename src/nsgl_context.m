//========================================================================
// GLFW 3.0 OS X - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2010 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <pthread.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize OpenGL support
//
int _glfwInitContextAPI(void)
{
    if (pthread_key_create(&_glfw.nsgl.current, NULL) != 0)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "NSGL: Failed to create context TLS");
        return GL_FALSE;
    }

    _glfw.nsgl.framework =
        CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
    if (_glfw.nsgl.framework == NULL)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "NSGL: Failed to locate OpenGL framework");
        return GL_FALSE;
    }

    return GL_TRUE;
}

// Terminate OpenGL support
//
void _glfwTerminateContextAPI(void)
{
    pthread_key_delete(_glfw.nsgl.current);
}

// Create the OpenGL context
//
int _glfwCreateContext(_GLFWwindow* window,
                       const _GLFWwndconfig* wndconfig,
                       const _GLFWfbconfig* fbconfig)
{
    unsigned int attributeCount = 0;

    // OS X needs non-zero color size, so set resonable values
    int colorBits = fbconfig->redBits + fbconfig->greenBits + fbconfig->blueBits;
    if (colorBits == 0)
        colorBits = 24;
    else if (colorBits < 15)
        colorBits = 15;

    if (wndconfig->clientAPI == GLFW_OPENGL_ES_API)
    {
        _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                        "NSGL: This API does not support OpenGL ES");
        return GL_FALSE;
    }

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (wndconfig->glMajor == 3 && wndconfig->glMinor < 2)
    {
        _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                        "NSGL: The targeted version of OS X does not "
                        "support OpenGL 3.0 or 3.1");
        return GL_FALSE;
    }

    if (wndconfig->glMajor > 2)
    {
        if (!wndconfig->glForward)
        {
            _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                            "NSGL: The targeted version of OS X only "
                            "supports OpenGL 3.2 and later versions if they "
                            "are forward-compatible");
            return GL_FALSE;
        }

        if (wndconfig->glProfile != GLFW_OPENGL_CORE_PROFILE)
        {
            _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                            "NSGL: The targeted version of OS X only "
                            "supports OpenGL 3.2 and later versions if they "
                            "use the core profile");
            return GL_FALSE;
        }
    }
#else
    // Fail if OpenGL 3.0 or above was requested
    if (wndconfig->glMajor > 2)
    {
        _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                        "NSGL: The targeted version of OS X does not "
                        "support OpenGL version 3.0 or above");
        return GL_FALSE;
    }
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

    // Fail if a robustness strategy was requested
    if (wndconfig->glRobustness)
    {
        _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                        "NSGL: OS X does not support OpenGL robustness "
                        "strategies");
        return GL_FALSE;
    }

#define ADD_ATTR(x) { attributes[attributeCount++] = x; }
#define ADD_ATTR2(x, y) { ADD_ATTR(x); ADD_ATTR(y); }

    // Arbitrary array size here
    NSOpenGLPixelFormatAttribute attributes[40];

    ADD_ATTR(NSOpenGLPFADoubleBuffer);
    ADD_ATTR(NSOpenGLPFAClosestPolicy);

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (wndconfig->glMajor > 2)
        ADD_ATTR2(NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core);
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

    ADD_ATTR2(NSOpenGLPFAColorSize, colorBits);

    if (fbconfig->alphaBits > 0)
        ADD_ATTR2(NSOpenGLPFAAlphaSize, fbconfig->alphaBits);

    if (fbconfig->depthBits > 0)
        ADD_ATTR2(NSOpenGLPFADepthSize, fbconfig->depthBits);

    if (fbconfig->stencilBits > 0)
        ADD_ATTR2(NSOpenGLPFAStencilSize, fbconfig->stencilBits);

    int accumBits = fbconfig->accumRedBits + fbconfig->accumGreenBits +
                    fbconfig->accumBlueBits + fbconfig->accumAlphaBits;

    if (accumBits > 0)
        ADD_ATTR2(NSOpenGLPFAAccumSize, accumBits);

    if (fbconfig->auxBuffers > 0)
        ADD_ATTR2(NSOpenGLPFAAuxBuffers, fbconfig->auxBuffers);

    if (fbconfig->stereo)
        ADD_ATTR(NSOpenGLPFAStereo);

    if (fbconfig->samples > 0)
    {
        ADD_ATTR2(NSOpenGLPFASampleBuffers, 1);
        ADD_ATTR2(NSOpenGLPFASamples, fbconfig->samples);
    }

    // NOTE: All NSOpenGLPixelFormats on the relevant cards support sRGB
    //       frambuffer, so there's no need (and no way) to request it

    ADD_ATTR(0);

#undef ADD_ATTR
#undef ADD_ATTR2

    window->nsgl.pixelFormat =
        [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    if (window->nsgl.pixelFormat == nil)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "NSGL: Failed to create OpenGL pixel format");
        return GL_FALSE;
    }

    NSOpenGLContext* share = NULL;

    if (wndconfig->share)
        share = wndconfig->share->nsgl.context;

    window->nsgl.context =
        [[NSOpenGLContext alloc] initWithFormat:window->nsgl.pixelFormat
                                   shareContext:share];
    if (window->nsgl.context == nil)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "NSGL: Failed to create OpenGL context");
        return GL_FALSE;
    }

    return GL_TRUE;
}

// Destroy the OpenGL context
//
void _glfwDestroyContext(_GLFWwindow* window)
{
    [window->nsgl.pixelFormat release];
    window->nsgl.pixelFormat = nil;

    [window->nsgl.context release];
    window->nsgl.context = nil;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformMakeContextCurrent(_GLFWwindow* window)
{
    if (window)
        [window->nsgl.context makeCurrentContext];
    else
        [NSOpenGLContext clearCurrentContext];

    pthread_setspecific(_glfw.nsgl.current, window);
}

_GLFWwindow* _glfwPlatformGetCurrentContext(void)
{
    return (_GLFWwindow*) pthread_getspecific(_glfw.nsgl.current);
}

void _glfwPlatformSwapBuffers(_GLFWwindow* window)
{
    // ARP appears to be unnecessary, but this is future-proof
    [window->nsgl.context flushBuffer];
}

void _glfwPlatformSwapInterval(int interval)
{
    _GLFWwindow* window = _glfwPlatformGetCurrentContext();

    GLint sync = interval;
    [window->nsgl.context setValues:&sync forParameter:NSOpenGLCPSwapInterval];
}

int _glfwPlatformExtensionSupported(const char* extension)
{
    // There are no NSGL extensions
    return GL_FALSE;
}

GLFWglproc _glfwPlatformGetProcAddress(const char* procname)
{
    CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                       procname,
                                                       kCFStringEncodingASCII);

    GLFWglproc symbol = CFBundleGetFunctionPointerForName(_glfw.nsgl.framework,
                                                          symbolName);

    CFRelease(symbolName);

    return symbol;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI id glfwGetNSGLContext(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(nil);
    return window->nsgl.context;
}

