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

#include <mach/mach_time.h>


// Return raw time
//
static uint64_t getRawTime(void)
{
    return mach_absolute_time();
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialise timer
//
void _glfwInitTimer(void)
{
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);

    _glfw.ns.timer.resolution = (double) info.numer / (info.denom * 1.0e9);
    _glfw.ns.timer.base = getRawTime();
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

double _glfwPlatformGetTime(void)
{
    return (double) (getRawTime() - _glfw.ns.timer.base) *
        _glfw.ns.timer.resolution;
}

void _glfwPlatformSetTime(double time)
{
    _glfw.ns.timer.base = getRawTime() -
        (uint64_t) (time / _glfw.ns.timer.resolution);
}

