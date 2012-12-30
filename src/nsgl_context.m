//========================================================================
// GLFW - An OpenGL library
// Platform:    Cocoa/NSOpenGL
// API Version: 3.0
// WWW:         http://www.glfw.org/
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


//========================================================================
// The per-thread current context/window pointer
//========================================================================
static pthread_key_t _glfwCurrentTLS;


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Initialize OpenGL support
//========================================================================

int _glfwInitOpenGL(void)
{
    if (pthread_key_create(&_glfwCurrentTLS, NULL) != 0)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "NSOpenGL: Failed to create context TLS");
        return GL_FALSE;
    }

    return GL_TRUE;
}


//========================================================================
// Terminate OpenGL support
//========================================================================

void _glfwTerminateOpenGL(void)
{
    pthread_key_delete(_glfwCurrentTLS);
}


//========================================================================
// Create the OpenGL context
//========================================================================

int _glfwCreateContext(_GLFWwindow* window,
                       const _GLFWwndconfig* wndconfig,
                       const _GLFWfbconfig* fbconfig)
{
    unsigned int attributeCount = 0;

    // Mac OS X needs non-zero color size, so set resonable values
    int colorBits = fbconfig->redBits + fbconfig->greenBits + fbconfig->blueBits;
    if (colorBits == 0)
        colorBits = 24;
    else if (colorBits < 15)
        colorBits = 15;

    if (wndconfig->clientAPI == GLFW_OPENGL_ES_API)
    {
        _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                      "NSOpenGL: This API does not support OpenGL ES");
        return GL_FALSE;
    }

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    // Fail if any OpenGL version above 2.1 other than 3.2 was requested
    if (wndconfig->glMajor > 3 ||
        (wndconfig->glMajor == 3 && wndconfig->glMinor != 2))
    {
        _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                      "NSOpenGL: The targeted version of Mac OS X does not "
                      "support any OpenGL version above 2.1 except 3.2");
        return GL_FALSE;
    }

    if (wndconfig->glMajor > 2)
    {
        if (!wndconfig->glForward)
        {
            _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                          "NSOpenGL: The targeted version of Mac OS X only "
                          "supports OpenGL 3.2 contexts if they are "
                          "forward-compatible");
            return GL_FALSE;
        }

        if (wndconfig->glProfile != GLFW_OPENGL_CORE_PROFILE)
        {
            _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                          "NSOpenGL: The targeted version of Mac OS X only "
                          "supports OpenGL 3.2 contexts if they use the "
                          "core profile");
            return GL_FALSE;
        }
    }
#else
    // Fail if OpenGL 3.0 or above was requested
    if (wndconfig->glMajor > 2)
    {
        _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                      "NSOpenGL: The targeted version of Mac OS X does not "
                      "support OpenGL version 3.0 or above");
        return GL_FALSE;
    }
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/

    // Fail if a robustness strategy was requested
    if (wndconfig->glRobustness)
    {
        _glfwSetError(GLFW_VERSION_UNAVAILABLE,
                      "NSOpenGL: Mac OS X does not support OpenGL robustness "
                      "strategies");
        return GL_FALSE;
    }

#define ADD_ATTR(x) { attributes[attributeCount++] = x; }
#define ADD_ATTR2(x, y) { ADD_ATTR(x); ADD_ATTR(y); }

    // Arbitrary array size here
    NSOpenGLPixelFormatAttribute attributes[40];

    ADD_ATTR(NSOpenGLPFADoubleBuffer);

    if (wndconfig->mode == GLFW_FULLSCREEN)
    {
        ADD_ATTR(NSOpenGLPFANoRecovery);
        ADD_ATTR2(NSOpenGLPFAScreenMask,
                  CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID()));
    }

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
    // frambuffer, so there's no need (and no way) to request it

    ADD_ATTR(0);

#undef ADD_ATTR
#undef ADD_ATTR2

    window->NSGL.pixelFormat =
        [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    if (window->NSGL.pixelFormat == nil)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "NSOpenGL: Failed to create OpenGL pixel format");
        return GL_FALSE;
    }

    NSOpenGLContext* share = NULL;

    if (wndconfig->share)
        share = wndconfig->share->NSGL.context;

    window->NSGL.context =
        [[NSOpenGLContext alloc] initWithFormat:window->NSGL.pixelFormat
                                   shareContext:share];
    if (window->NSGL.context == nil)
    {
        _glfwSetError(GLFW_PLATFORM_ERROR,
                      "NSOpenGL: Failed to create OpenGL context");
        return GL_FALSE;
    }

    return GL_TRUE;
}


//========================================================================
// Destroy the OpenGL context
//========================================================================

void _glfwDestroyContext(_GLFWwindow* window)
{
    [window->NSGL.pixelFormat release];
    window->NSGL.pixelFormat = nil;

    [window->NSGL.context release];
    window->NSGL.context = nil;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Make the OpenGL context associated with the specified window current
//========================================================================

void _glfwPlatformMakeContextCurrent(_GLFWwindow* window)
{
    if (window)
        [window->NSGL.context makeCurrentContext];
    else
        [NSOpenGLContext clearCurrentContext];

    pthread_setspecific(_glfwCurrentTLS, window);
}


//========================================================================
// Return the window object whose context is current
//========================================================================

_GLFWwindow* _glfwPlatformGetCurrentContext(void)
{
    return (_GLFWwindow*) pthread_getspecific(_glfwCurrentTLS);
}


//========================================================================
// Swap buffers
//========================================================================

void _glfwPlatformSwapBuffers(_GLFWwindow* window)
{
    // ARP appears to be unnecessary, but this is future-proof
    [window->NSGL.context flushBuffer];
}


//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval(int interval)
{
    _GLFWwindow* window = _glfwPlatformGetCurrentContext();

    GLint sync = interval;
    [window->NSGL.context setValues:&sync forParameter:NSOpenGLCPSwapInterval];
}


//========================================================================
// Check if an OpenGL extension is available at runtime
//========================================================================

int _glfwPlatformExtensionSupported(const char* extension)
{
    // There are no NSGL extensions
    return GL_FALSE;
}


//========================================================================
// Get the function pointer to an OpenGL function
//========================================================================

GLFWglproc _glfwPlatformGetProcAddress(const char* procname)
{
    CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                       procname,
                                                       kCFStringEncodingASCII);

    GLFWglproc symbol = CFBundleGetFunctionPointerForName(_glfwLibrary.NSGL.framework,
                                                          symbolName);

    CFRelease(symbolName);

    return symbol;
}

