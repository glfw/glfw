//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
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
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#include "internal.h"

#include <assert.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

// Copies the contents of a @ref _GLFWvideoMode into a @ref GLFWvidmode.
//
static void copyVideoMode(GLFWvidmode* target, const _GLFWvideoMode* source)
{
    target->width       = source->width;
    target->height      = source->height;
    target->redBits     = source->redBits;
    target->greenBits   = source->greenBits;
    target->blueBits    = source->blueBits;
    
    
    #ifdef _MSC_VER
    // Win32 only provides integers. No need to round.
    target->refreshRate = (int) source->refreshRate;
    #else
    target->refreshRate = (int) round(source->refreshRate);
    #endif
}

// Lexically compare video modes, used by qsort
//
static int compareVideoModes(const void* fp, const void* sp)
{
    double refRateDiff;
    const _GLFWvideoMode* fm = fp;
    const _GLFWvideoMode* sm = sp;
    const int fbpp = fm->redBits + fm->greenBits + fm->blueBits;
    const int sbpp = sm->redBits + sm->greenBits + sm->blueBits;
    const int farea = fm->width * fm->height;
    const int sarea = sm->width * sm->height;

    // First sort on color bits per pixel
    if (fbpp != sbpp)
        return fbpp - sbpp;

    // Then sort on screen area
    if (farea != sarea)
        return farea - sarea;

    // Then sort on width
    if (fm->width != sm->width)
        return fm->width - sm->width;

    // Lastly sort on refresh rate
    refRateDiff = fm->refreshRate - sm->refreshRate;
    
    #ifdef _MSC_VER
    // All Win32 refresh rates are converted from integers
    return (int) refRateDiff;
    #else
    // Limits the minimum absolute value to 1.0, to prevent different modes from
    // evaluating to equal, which happens when the difference between the
    // refresh rates is less than 1.0, and this number is truncated to an integer.
    return (int) copysign(fmax(fabs(refRateDiff), 1.0), refRateDiff);
    #endif
}

// Retrieves the available modes for the specified monitor
//
static GLFWbool refreshVideoModes(_GLFWmonitor* monitor)
{
    int i, modeCount;
    _GLFWvideoMode* modes;

    if (monitor->modes)
        return GLFW_TRUE;

    modes = _glfw.platform.getVideoModes(monitor, &modeCount);
    
    if (!modes)
        return GLFW_FALSE;

    qsort(modes, modeCount, sizeof(_GLFWvideoMode), compareVideoModes);

    _glfw_free(monitor->modesOld);
    monitor->modesOld = NULL;
    
    monitor->modes = modes;
    monitor->modesPointers = _glfw_realloc(monitor->modesPointers,
                                           modeCount * sizeof(GLFWvideoMode*));
    
    for (i = 0; i < modeCount; ++i)
        monitor->modesPointers[i] = (GLFWvideoMode*) &modes[i];
    
    monitor->modeCount = modeCount;

    return GLFW_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                         GLFW event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of a monitor connection or disconnection
//
void _glfwInputMonitor(_GLFWmonitor* monitor, int action, int placement)
{
    assert(monitor != NULL);
    assert(action == GLFW_CONNECTED || action == GLFW_DISCONNECTED);
    assert(placement == _GLFW_INSERT_FIRST || placement == _GLFW_INSERT_LAST);

    if (action == GLFW_CONNECTED)
    {
        _glfw.monitorCount++;
        _glfw.monitors =
            _glfw_realloc(_glfw.monitors,
                          sizeof(_GLFWmonitor*) * _glfw.monitorCount);

        if (placement == _GLFW_INSERT_FIRST)
        {
            memmove(_glfw.monitors + 1,
                    _glfw.monitors,
                    ((size_t) _glfw.monitorCount - 1) * sizeof(_GLFWmonitor*));
            _glfw.monitors[0] = monitor;
        }
        else
            _glfw.monitors[_glfw.monitorCount - 1] = monitor;
    }
    else if (action == GLFW_DISCONNECTED)
    {
        int i;
        _GLFWwindow* window;

        for (window = _glfw.windowListHead;  window;  window = window->next)
        {
            if (window->monitor == monitor)
            {
                int width, height, xoff, yoff;
                _glfw.platform.getWindowSize(window, &width, &height);
                _glfw.platform.setWindowMonitor(window, NULL, 0, 0, width, height, 0);
                _glfw.platform.getWindowFrameSize(window, &xoff, &yoff, NULL, NULL);
                _glfw.platform.setWindowPos(window, xoff, yoff);
            }
        }

        for (i = 0;  i < _glfw.monitorCount;  i++)
        {
            if (_glfw.monitors[i] == monitor)
            {
                _glfw.monitorCount--;
                memmove(_glfw.monitors + i,
                        _glfw.monitors + i + 1,
                        ((size_t) _glfw.monitorCount - i) * sizeof(_GLFWmonitor*));
                break;
            }
        }
    }

    if (_glfw.callbacks.monitor)
        _glfw.callbacks.monitor((GLFWmonitor*) monitor, action);

    if (action == GLFW_DISCONNECTED)
        _glfwFreeMonitor(monitor);
}

// Notifies shared code that a full screen window has acquired or released
// a monitor
//
void _glfwInputMonitorWindow(_GLFWmonitor* monitor, _GLFWwindow* window)
{
    assert(monitor != NULL);
    monitor->window = window;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Allocates and returns a monitor object with the specified name and dimensions
//
_GLFWmonitor* _glfwAllocMonitor(const char* name, int widthMM, int heightMM)
{
    _GLFWmonitor* monitor = _glfw_calloc(1, sizeof(_GLFWmonitor));
    monitor->widthMM = widthMM;
    monitor->heightMM = heightMM;

    strncpy(monitor->name, name, sizeof(monitor->name) - 1);

    return monitor;
}

// Frees a monitor object and any data associated with it
//
void _glfwFreeMonitor(_GLFWmonitor* monitor)
{
    if (monitor == NULL)
        return;

    _glfw.platform.freeMonitor(monitor);

    _glfwFreeGammaArrays(&monitor->originalRamp);
    _glfwFreeGammaArrays(&monitor->currentRamp);

    _glfw_free(monitor->modes);
    _glfw_free(monitor->modesPointers);
    _glfw_free(monitor->modesOld);
    _glfw_free(monitor);
}

// Allocates red, green and blue value arrays of the specified size
//
void _glfwAllocGammaArrays(GLFWgammaramp* ramp, unsigned int size)
{
    ramp->red = _glfw_calloc(size, sizeof(unsigned short));
    ramp->green = _glfw_calloc(size, sizeof(unsigned short));
    ramp->blue = _glfw_calloc(size, sizeof(unsigned short));
    ramp->size = size;
}

// Frees the red, green and blue value arrays and clears the struct
//
void _glfwFreeGammaArrays(GLFWgammaramp* ramp)
{
    _glfw_free(ramp->red);
    _glfw_free(ramp->green);
    _glfw_free(ramp->blue);

    memset(ramp, 0, sizeof(GLFWgammaramp));
}

// Chooses the video mode most closely matching the desired one
//
const _GLFWvideoMode* _glfwChooseVideoMode(_GLFWmonitor* monitor,
                                           const _GLFWvideoMode* desired)
{
    int i;
    unsigned int sizeDiff, leastSizeDiff = UINT_MAX;
    double rateDiff, leastRateDiff = DBL_MAX;
    unsigned int colorDiff, leastColorDiff = UINT_MAX;
    const _GLFWvideoMode* current;
    const _GLFWvideoMode* closest = NULL;

    if (!refreshVideoModes(monitor))
        return NULL;

    for (i = 0;  i < monitor->modeCount;  i++)
    {
        current = monitor->modes + i;

        colorDiff = 0;

        if (desired->redBits != GLFW_DONT_CARE)
            colorDiff += abs(current->redBits - desired->redBits);
        if (desired->greenBits != GLFW_DONT_CARE)
            colorDiff += abs(current->greenBits - desired->greenBits);
        if (desired->blueBits != GLFW_DONT_CARE)
            colorDiff += abs(current->blueBits - desired->blueBits);

        sizeDiff = abs((current->width - desired->width) *
                       (current->width - desired->width) +
                       (current->height - desired->height) *
                       (current->height - desired->height));

        if (desired->refreshRate != GLFW_DONT_CARE)
            rateDiff = fabs(current->refreshRate - desired->refreshRate);
        else
            rateDiff = UINT_MAX - current->refreshRate;

        if ((colorDiff < leastColorDiff) ||
            (colorDiff == leastColorDiff && sizeDiff < leastSizeDiff) ||
            (colorDiff == leastColorDiff && sizeDiff == leastSizeDiff && rateDiff < leastRateDiff))
        {
            closest = current;
            leastSizeDiff = sizeDiff;
            leastRateDiff = rateDiff;
            leastColorDiff = colorDiff;
        }
    }

    return closest;
}

// Performs lexical comparison between two @ref _GLFWvideoMode structures
//
int _glfwCompareVideoModes(const _GLFWvideoMode* fm, const _GLFWvideoMode* sm)
{
    return compareVideoModes(fm, sm);
}

// Splits a color depth into red, green and blue bit depths
//
void _glfwSplitBPP(int bpp, int* red, int* green, int* blue)
{
    int delta;

    // We assume that by 32 the user really meant 24
    if (bpp == 32)
        bpp = 24;

    // Convert "bits per pixel" to red, green & blue sizes

    *red = *green = *blue = bpp / 3;
    delta = bpp - (*red * 3);
    if (delta >= 1)
        *green = *green + 1;

    if (delta == 2)
        *red = *red + 1;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI GLFWmonitor** glfwGetMonitors(int* count)
{
    assert(count != NULL);

    *count = 0;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    *count = _glfw.monitorCount;
    return (GLFWmonitor**) _glfw.monitors;
}

GLFWAPI GLFWmonitor* glfwGetPrimaryMonitor(void)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_glfw.monitorCount)
        return NULL;

    return (GLFWmonitor*) _glfw.monitors[0];
}

GLFWAPI void glfwGetMonitorPos(GLFWmonitor* handle, int* xpos, int* ypos)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;

    _GLFW_REQUIRE_INIT();

    _glfw.platform.getMonitorPos(monitor, xpos, ypos);
}

GLFWAPI void glfwGetMonitorWorkarea(GLFWmonitor* handle,
                                    int* xpos, int* ypos,
                                    int* width, int* height)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;
    if (width)
        *width = 0;
    if (height)
        *height = 0;

    _GLFW_REQUIRE_INIT();

    _glfw.platform.getMonitorWorkarea(monitor, xpos, ypos, width, height);
}

GLFWAPI void glfwGetMonitorPhysicalSize(GLFWmonitor* handle, int* widthMM, int* heightMM)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);

    if (widthMM)
        *widthMM = 0;
    if (heightMM)
        *heightMM = 0;

    _GLFW_REQUIRE_INIT();

    if (widthMM)
        *widthMM = monitor->widthMM;
    if (heightMM)
        *heightMM = monitor->heightMM;
}

GLFWAPI void glfwGetMonitorContentScale(GLFWmonitor* handle,
                                        float* xscale, float* yscale)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);

    if (xscale)
        *xscale = 0.f;
    if (yscale)
        *yscale = 0.f;

    _GLFW_REQUIRE_INIT();
    _glfw.platform.getMonitorContentScale(monitor, xscale, yscale);
}

GLFWAPI const char* glfwGetMonitorName(GLFWmonitor* handle)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->name;
}

GLFWAPI void glfwSetMonitorUserPointer(GLFWmonitor* handle, void* pointer)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);

    _GLFW_REQUIRE_INIT();
    monitor->userPointer = pointer;
}

GLFWAPI void* glfwGetMonitorUserPointer(GLFWmonitor* handle)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->userPointer;
}

GLFWAPI GLFWmonitorfun glfwSetMonitorCallback(GLFWmonitorfun cbfun)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    _GLFW_SWAP(GLFWmonitorfun, _glfw.callbacks.monitor, cbfun);
    return cbfun;
}

GLFWAPI const GLFWvidmode* glfwGetVideoModes(GLFWmonitor* handle, int* count)
{
    int i, j;
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    if (!glfwGetVideoModes2(handle, count))
        return NULL;
    
    if (!monitor->modesOld)
    {
        monitor->modesOld = _glfw_calloc(monitor->modeCount, sizeof(GLFWvidmode));
        
        monitor->modeCountOld = 0;
        for (i = 0, j = 0; i < monitor->modeCount; ++i)
        {
            copyVideoMode(&monitor->modesOld[j], &monitor->modes[i]);
                
            // Because the old video mode structure contains less precision
            // than the new one, different new modes will lead to similar
            // old ones, which must be ignored.
        
            // The modes array is already sorted, so identical video modes
            // will always be adjacent. Therefore, it's only necessary to
            // test for equality with the previous mode in the array.
            
            // The only difference between the old and new GLFW video modes,
            // is that the new one allows higher precision refresh rates, so
            // this is the only field that requires a comparison.
            if (j > 0 && monitor->modesOld[j - 1].refreshRate == monitor->modesOld[j].refreshRate)
                continue;
            
            ++monitor->modeCountOld;
            ++j;
        }
    }
    
    *count = monitor->modeCountOld;
    return monitor->modesOld;
}

GLFWAPI const GLFWvideoMode* const* glfwGetVideoModes2(GLFWmonitor* handle, int* count)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);
    assert(count != NULL);

    *count = 0;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    if (!refreshVideoModes(monitor))
        return NULL;

    *count = monitor->modeCount;
    return (const GLFWvideoMode* const *) monitor->modesPointers;
}

GLFWAPI const GLFWvidmode* glfwGetVideoMode(GLFWmonitor* handle)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    _glfw.platform.getVideoMode(monitor, &monitor->currentMode);
    
    copyVideoMode(&monitor->currentModeOld, &monitor->currentMode);
    
    return &monitor->currentModeOld;
}

GLFWAPI const GLFWvideoMode* glfwGetVideoMode2(GLFWmonitor* handle)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    _glfw.platform.getVideoMode(monitor, &monitor->currentMode);
    return (GLFWvideoMode*) &monitor->currentMode;
}

GLFWAPI void glfwSetGamma(GLFWmonitor* handle, float gamma)
{
    unsigned int i;
    unsigned short* values;
    GLFWgammaramp ramp;
    const GLFWgammaramp* original;
    assert(handle != NULL);
    assert(gamma > 0.f);
    assert(gamma <= FLT_MAX);

    _GLFW_REQUIRE_INIT();

    if (gamma != gamma || gamma <= 0.f || gamma > FLT_MAX)
    {
        _glfwInputError(GLFW_INVALID_VALUE, "Invalid gamma value %f", gamma);
        return;
    }

    original = glfwGetGammaRamp(handle);
    if (!original)
        return;

    values = _glfw_calloc(original->size, sizeof(unsigned short));

    for (i = 0;  i < original->size;  i++)
    {
        float value;

        // Calculate intensity
        value = i / (float) (original->size - 1);
        // Apply gamma curve
        value = powf(value, 1.f / gamma) * 65535.f + 0.5f;
        // Clamp to value range
        value = _glfw_fminf(value, 65535.f);

        values[i] = (unsigned short) value;
    }

    ramp.red = values;
    ramp.green = values;
    ramp.blue = values;
    ramp.size = original->size;

    glfwSetGammaRamp(handle, &ramp);
    _glfw_free(values);
}

GLFWAPI const GLFWgammaramp* glfwGetGammaRamp(GLFWmonitor* handle)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    _glfwFreeGammaArrays(&monitor->currentRamp);
    if (!_glfw.platform.getGammaRamp(monitor, &monitor->currentRamp))
        return NULL;

    return &monitor->currentRamp;
}

GLFWAPI void glfwSetGammaRamp(GLFWmonitor* handle, const GLFWgammaramp* ramp)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    assert(monitor != NULL);
    assert(ramp != NULL);
    assert(ramp->size > 0);
    assert(ramp->red != NULL);
    assert(ramp->green != NULL);
    assert(ramp->blue != NULL);

    _GLFW_REQUIRE_INIT();

    if (ramp->size <= 0)
    {
        _glfwInputError(GLFW_INVALID_VALUE,
                        "Invalid gamma ramp size %i",
                        ramp->size);
        return;
    }

    if (!monitor->originalRamp.size)
    {
        if (!_glfw.platform.getGammaRamp(monitor, &monitor->originalRamp))
            return;
    }

    _glfw.platform.setGammaRamp(monitor, ramp);
}


GLFWAPI int glfwVideoModesEqual(const GLFWvideoMode* first, const GLFWvideoMode* second)
{
    if (memcmp(first, second, sizeof(_GLFWvideoMode)) == 0)
        return GLFW_TRUE;
    
    return GLFW_FALSE;
}

GLFWAPI void glfwVideoModeGetSize(const GLFWvideoMode* videoMode, int* width, int* height)
{
    const _GLFWvideoMode* mode = (_GLFWvideoMode*) videoMode;
    assert(videoMode != NULL);
    
    if (width)
        *width = mode->width;
    if (height)
        *height = mode->height;
}

GLFWAPI void glfwVideoModeGetColorDepth(const GLFWvideoMode* videoMode, int* red, int* green, int* blue)
{
    const _GLFWvideoMode* mode = (_GLFWvideoMode*) videoMode;
    assert(videoMode != NULL);
    
    if (red)
        *red = mode->redBits;
    if (green)
        *green = mode->greenBits;
    if (blue)
        *blue = mode->blueBits;
}

GLFWAPI double glfwVideoModeGetRefreshRate(const GLFWvideoMode* videoMode)
{
    assert(videoMode != NULL);
    
    return ((_GLFWvideoMode*) videoMode)->refreshRate;
}

