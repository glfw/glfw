//========================================================================
// GLFW - An OpenGL library
// Platform:    X11 (Unix)
// API version: 3.0
// WWW:         http://www.glfw.org/
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

#include <stdlib.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

_GLFWmonitor** _glfwCreateMonitor(_GLFWmonitor** current, DISPLAY_DEVICE* adapter, DISPLAY_DEVICE* monitor, DEVMODE* setting)
{
    HDC dc = NULL;

    *current = _glfwMalloc(sizeof(_GLFWmonitor));
    memset(*current, 0, sizeof(_GLFWmonitor));

    dc = CreateDC("DISPLAY", monitor->DeviceString, NULL, NULL);

    (*current)->physicalWidth  = GetDeviceCaps(dc, HORZSIZE);
    (*current)->physicalHeight = GetDeviceCaps(dc, VERTSIZE);

    DeleteDC(dc);

    (*current)->deviceName = _glfwMalloc(strlen(monitor->DeviceName) + 1);
    memcpy((*current)->deviceName, monitor->DeviceName, strlen(monitor->DeviceName) + 1);
    (*current)->deviceName[strlen(monitor->DeviceName)] = '\0';

    (*current)->screenXPosition = setting->dmPosition.x;
    (*current)->screenYPosition = setting->dmPosition.y;

    memcpy((*current)->Win32.DeviceName, adapter->DeviceName, 32);
    return &((*current)->next);
}

_GLFWmonitor* _glfwDestroyMonitor(_GLFWmonitor* monitor)
{
    _GLFWmonitor* result;

    result = monitor->next;

    _glfwFree(monitor->deviceName);
    _glfwFree(monitor);

    return result;
}

void _glfwInitMonitors(void)
{
    _GLFWmonitor** curMonitor;

    DISPLAY_DEVICE adapter;
    DWORD adapterNum;

    DISPLAY_DEVICE monitor;

    DEVMODE setting;
    DWORD settingNum;

    curMonitor = &_glfwLibrary.monitorListHead;

    adapter.cb = sizeof(DISPLAY_DEVICE);
    adapterNum = 0;

    monitor.cb = sizeof(DISPLAY_DEVICE);
    setting.dmSize = sizeof(DEVMODE);
    settingNum = 0;

    while(EnumDisplayDevices(NULL, adapterNum++, &adapter, 0))
    {
        if(adapter.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
        {
            continue;
        }

        EnumDisplaySettingsEx(adapter.DeviceName, ENUM_CURRENT_SETTINGS, &setting, EDS_ROTATEDMODE);

        EnumDisplayDevices(adapter.DeviceName, 0, &monitor, 0);

        curMonitor = _glfwCreateMonitor(curMonitor, &adapter, &monitor, &setting);
    }
}

void _glfwTerminateMonitors(void)
{
    while(_glfwLibrary.monitorListHead)
        _glfwLibrary.monitorListHead = _glfwDestroyMonitor(_glfwLibrary.monitorListHead);
}

