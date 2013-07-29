//========================================================================
// GLFW 3.0 Win32 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
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


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialise timer
//
void _glfwInitTimer(void)
{
    __int64 freq;

    if (QueryPerformanceFrequency((LARGE_INTEGER*) &freq))
    {
        _glfw.win32.timer.hasPC = GL_TRUE;
        _glfw.win32.timer.resolution = 1.0 / (double) freq;
        QueryPerformanceCounter((LARGE_INTEGER*) &_glfw.win32.timer.t0_64);
    }
    else
    {
        _glfw.win32.timer.hasPC = GL_FALSE;
        _glfw.win32.timer.resolution = 0.001; // winmm resolution is 1 ms
        _glfw.win32.timer.t0_32 = _glfw_timeGetTime();
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

double _glfwPlatformGetTime(void)
{
    double t;
    __int64 t_64;

    if (_glfw.win32.timer.hasPC)
    {
        QueryPerformanceCounter((LARGE_INTEGER*) &t_64);
        t =  (double)(t_64 - _glfw.win32.timer.t0_64);
    }
    else
        t = (double)(_glfw_timeGetTime() - _glfw.win32.timer.t0_32);

    return t * _glfw.win32.timer.resolution;
}

void _glfwPlatformSetTime(double t)
{
    __int64 t_64;

    if (_glfw.win32.timer.hasPC)
    {
        QueryPerformanceCounter((LARGE_INTEGER*) &t_64);
        _glfw.win32.timer.t0_64 = t_64 - (__int64) (t / _glfw.win32.timer.resolution);
    }
    else
        _glfw.win32.timer.t0_32 = _glfw_timeGetTime() - (int)(t * 1000.0);
}

