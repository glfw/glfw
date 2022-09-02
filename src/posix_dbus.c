//========================================================================
// GLFW 3.4 POSIX - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2022 Camilla Löwy <elmindreda@glfw.org>
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#define _GNU_SOURCE

#include "internal.h"

void _glfwInitDBusPOSIX(void)
{
    //Initialize DBus library functions
    _glfw.dbus.handle = NULL;
    _glfw.dbus.connection = NULL;

    _glfw.dbus.handle = _glfwPlatformLoadModule("libdbus-1.so.3");
    if (_glfw.dbus.handle)
    {
        _glfw.dbus.error_init = (PFN_dbus_error_init)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_error_init");
        _glfw.dbus.error_is_set = (PFN_dbus_error_is_set)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_error_is_set");
        _glfw.dbus.error_free = (PFN_dbus_error_free)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_error_free");
        _glfw.dbus.connection_unref = (PFN_dbus_connection_unref)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_connection_unref");
        _glfw.dbus.connection_send = (PFN_dbus_connection_send)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_connection_send");
        _glfw.dbus.connection_flush = (PFN_dbus_connection_flush)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_connection_flush");
        _glfw.dbus.bus_request_name = (PFN_dbus_bus_request_name)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_bus_request_name");
        _glfw.dbus.bus_get = (PFN_dbus_bus_get)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_bus_get");
        _glfw.dbus.message_unref = (PFN_dbus_message_unref)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_message_unref");
        _glfw.dbus.message_new_signal = (PFN_dbus_message_new_signal)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_message_new_signal");
        _glfw.dbus.message_iter_init_append = (PFN_dbus_message_iter_init_append)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_message_iter_init_append");
        _glfw.dbus.message_iter_append_basic = (PFN_dbus_message_iter_append_basic)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_message_iter_append_basic");
        _glfw.dbus.message_iter_open_container = (PFN_dbus_message_iter_open_container)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_message_iter_open_container");
        _glfw.dbus.message_iter_close_container = (PFN_dbus_message_iter_close_container)
            _glfwPlatformGetModuleSymbol(_glfw.dbus.handle, "dbus_message_iter_close_container");

        //Initialize DBus connection
        dbus_error_init(&_glfw.dbus.error);
        _glfw.dbus.connection = dbus_bus_get(DBUS_BUS_SESSION, &_glfw.dbus.error);

        //Check for errors
        if(dbus_error_is_set(&_glfw.dbus.error) || !_glfw.dbus.connection)
        {
            if(dbus_error_is_set(&_glfw.dbus.error))
                dbus_error_free(&_glfw.dbus.error);

            _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to connect to DBus");

            dbus_connection_unref(_glfw.dbus.connection);
            _glfw.dbus.connection = NULL;
        }
        else
        {
            //Request name
            const int res = dbus_bus_request_name(_glfw.dbus.connection, "org.glfw", DBUS_NAME_FLAG_REPLACE_EXISTING, &_glfw.dbus.error);

            //Check for errors
            if(dbus_error_is_set(&_glfw.dbus.error) || res != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
            {
                if(dbus_error_is_set(&_glfw.dbus.error))
                    dbus_error_free(&_glfw.dbus.error);

                _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to request DBus name");

                dbus_connection_unref(_glfw.dbus.connection);
                _glfw.dbus.connection = NULL;
            }
        }
    }

    if(_glfw.dbus.connection)
    {
        //Window NULL is safe here because it won't get
        //used inside th eSetWindowTaskbarProgress function
        _glfw.platform.setWindowTaskbarProgress(NULL, GLFW_TASKBAR_PROGRESS_NOPROGRESS, 0);
    }
}

void _glfwTerminateDBusPOSIX(void)
{
    if (_glfw.dbus.connection)
    {
        dbus_connection_unref(_glfw.dbus.connection);
        _glfw.dbus.connection = NULL;
    }

    if (_glfw.dbus.handle)
    {
        _glfwPlatformFreeModule(_glfw.dbus.handle);
        _glfw.dbus.handle = NULL;
    }
}
