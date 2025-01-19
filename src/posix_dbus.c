//========================================================================
// GLFW 3.4 POSIX - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2023 Camilla Löwy <elmindreda@glfw.org>
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

#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

void _glfwInitDBusPOSIX(void)
{
    //Initialize DBus library functions
    _glfw.dbus.handle = NULL;
    _glfw.dbus.connection = NULL;

    _glfw.dbus.handle = _glfwPlatformLoadModule("libdbus-1.so.3");
    if (!_glfw.dbus.handle)
        return;

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

    if (!_glfw.dbus.error_init ||
        !_glfw.dbus.error_is_set ||
        !_glfw.dbus.error_free ||
        !_glfw.dbus.connection_unref ||
        !_glfw.dbus.connection_send ||
        !_glfw.dbus.connection_flush ||
        !_glfw.dbus.bus_request_name ||
        !_glfw.dbus.bus_get ||
        !_glfw.dbus.message_unref ||
        !_glfw.dbus.message_new_signal ||
        !_glfw.dbus.message_iter_init_append ||
        !_glfw.dbus.message_iter_append_basic ||
        !_glfw.dbus.message_iter_open_container ||
        !_glfw.dbus.message_iter_close_container)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "POSIX: Failed to load DBus entry points");
        return;
    }

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
        return;
    }
    else
    {
        //Request name

        _glfwCacheLegalExecutableNameDBusPOSIX();
        if(!_glfw.dbus.legalExecutableName)
            return;

        //"org.glfw.<exe_name>_<pid>"
        char* busName = _glfw_calloc(21 + strlen(_glfw.dbus.legalExecutableName), sizeof(char));
        if(!busName)
        {
            _glfwInputError(GLFW_OUT_OF_MEMORY, "Failed to allocate memory for bus name");
            return;
        }
        memset(busName, '\0', (21 + strlen(_glfw.dbus.legalExecutableName)) * sizeof(char));

        const pid_t pid = getpid();
        sprintf(busName, "org.glfw.%s_%d", _glfw.dbus.legalExecutableName, pid);

        const int res = dbus_bus_request_name(_glfw.dbus.connection, busName, DBUS_NAME_FLAG_REPLACE_EXISTING, &_glfw.dbus.error);

        _glfw_free(busName);

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

    _glfwCacheFullExecutableNameDBusPOSIX();
    _glfwCacheDesktopFilePathDBusPOSIX();
    _glfwCacheSignalNameDBusPOSIX();
}

void _glfwCacheSignalNameDBusPOSIX(void)
{
    if(!_glfw.dbus.legalExecutableName)
        return;

    //"/org/glfw/<exe_name>_<pid>"
    char* signalName = _glfw_calloc(22 + strlen(_glfw.dbus.legalExecutableName), sizeof(char));
    if(!signalName)
    {
        _glfwInputError(GLFW_OUT_OF_MEMORY, "Failed to allocate memory for signal name");
        return;
    }

    memset(signalName, '\0', (22 + strlen(_glfw.dbus.legalExecutableName)) * sizeof(char));

    const pid_t pid = getpid();
    if(sprintf(signalName, "/org/glfw/%s_%d", _glfw.dbus.legalExecutableName, pid) < 0)
    {
        _glfwInputError(GLFW_PLATFORM, "Failed to create signal name");
        _glfw_free(signalName);
        return;
    }

    _glfw.dbus.signalName = signalName;
}

void _glfwCacheFullExecutableNameDBusPOSIX(void)
{
    char exeName[PATH_MAX];
    memset(exeName, 0, sizeof(char) * PATH_MAX);
    if(readlink("/proc/self/exe", exeName, PATH_MAX) == -1)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to get name of the running executable");
        return;
    }
    char* exeNameEnd = strchr(exeName, '\0');
    char* lastFound = strrchr(exeName, '/');
    if(!lastFound || !exeNameEnd)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to get name of the running executable");
        return;
    }
    unsigned int exeNameLength = (exeNameEnd - lastFound) - 1;

    char* exeNameFinal = _glfw_calloc(exeNameLength + 1, sizeof(char));
    if(!exeNameFinal)
    {
        _glfwInputError(GLFW_OUT_OF_MEMORY, "Failed to allocate memory for executable name");
        return;
    }

    memset(exeNameFinal, 0, sizeof(char) * (exeNameLength + 1));

    memcpy(exeNameFinal, (lastFound + 1), exeNameLength);

    _glfw.dbus.fullExecutableName = exeNameFinal;
}

void _glfwCacheLegalExecutableNameDBusPOSIX(void)
{
    //The executable name is stripped of any illegal characters
    //according to the DBus specification

    int i = 0;
    int validExeNameLength = 0;
    int output = 0;
    char exeName[PATH_MAX];
    memset(exeName, 0, sizeof(char) * PATH_MAX);
    if(readlink("/proc/self/exe", exeName, PATH_MAX) == -1)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to get name of the running executable");
        return;
    }
    char* exeNameEnd = strchr(exeName, '\0');
    char* lastFound = strrchr(exeName, '/');
    if(!lastFound || !exeNameEnd)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to get name of the running executable");
        return;
    }
    unsigned int exeNameLength = (exeNameEnd - lastFound) - 1;

    for(i = 0; i < exeNameLength; ++i)
    {
        if(isalnum(*(lastFound + 1 + i)))
            validExeNameLength++;
    }

    char* exeNameFinal = _glfw_calloc(validExeNameLength + 1, sizeof(char));
    if(!exeNameFinal)
    {
        _glfwInputError(GLFW_OUT_OF_MEMORY, "Failed to allocate memory for executable name");
        return;
    }

    memset(exeNameFinal, 0, sizeof(char) * (validExeNameLength + 1));

    for(i = 0; i < exeNameLength; ++i)
    {
        if(isalnum(*(lastFound + 1 + i)))
            exeNameFinal[output++] = *(lastFound + 1 + i);
    }

    _glfw.dbus.legalExecutableName = exeNameFinal;
}

void _glfwCacheDesktopFilePathDBusPOSIX(void)
{
    if(!_glfw.dbus.fullExecutableName)
        return;

    //Cache path of .desktop file

    //Create our final desktop file uri
    //"application://<exe_name>.desktop"
    unsigned int desktopFileLength = strlen("application://") + strlen(_glfw.dbus.fullExecutableName) + strlen(".desktop") + 1;
    _glfw.dbus.desktopFilePath = _glfw_calloc(desktopFileLength, sizeof(char));
    if(!_glfw.dbus.desktopFilePath)
    {
        _glfwInputError(GLFW_OUT_OF_MEMORY, "Failed to allocate memory for .desktop file path");
        return;
    }

    memset(_glfw.dbus.desktopFilePath, 0, sizeof(char) * desktopFileLength);
    strcpy(_glfw.dbus.desktopFilePath, "application://");
    memcpy(_glfw.dbus.desktopFilePath + strlen("application://"), _glfw.dbus.fullExecutableName, strlen(_glfw.dbus.fullExecutableName));
    strcpy(_glfw.dbus.desktopFilePath + strlen("application://") + strlen(_glfw.dbus.fullExecutableName), ".desktop");
    _glfw.dbus.desktopFilePath[desktopFileLength - 1] = '\0';
}

void _glfwTerminateDBusPOSIX(void)
{
    if(_glfw.dbus.signalName)
        _glfw_free(_glfw.dbus.signalName);

    if(_glfw.dbus.legalExecutableName)
        _glfw_free(_glfw.dbus.legalExecutableName);

    if(_glfw.dbus.fullExecutableName)
        _glfw_free(_glfw.dbus.fullExecutableName);

    if(_glfw.dbus.desktopFilePath)
        _glfw_free(_glfw.dbus.desktopFilePath);

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

void _glfwUpdateTaskbarProgressDBusPOSIX(dbus_bool_t progressVisible, double progressValue)
{
    struct DBusMessage* msg = NULL;

    if(!_glfw.dbus.handle || !_glfw.dbus.connection || !_glfw.dbus.desktopFilePath || !_glfw.dbus.signalName)
        return;

    //Signal signature:
    //signal com.canonical.Unity.LauncherEntry.Update (in s app_uri, in a{sv} properties)

    struct DBusMessageIter args;
    memset(&args, 0, sizeof(args));

    if(!_glfwNewMessageSignalDBusPOSIX(_glfw.dbus.signalName, "com.canonical.Unity.LauncherEntry", "Update", &msg))
        return;

    dbus_message_iter_init_append(msg, &args);

    //Setup app_uri parameter
    _glfwAppendDataDBusPOSIX(&args, DBUS_TYPE_STRING, &_glfw.dbus.desktopFilePath);

    //Set properties parameter
    struct DBusMessageIter sub1;
    memset(&sub1, 0, sizeof(sub1));

    _glfwOpenContainerDBusPOSIX(&args, DBUS_TYPE_ARRAY, "{sv}", &sub1);

    //Set progress visible property
    const char* progressVisibleStr = "progress-visible";
    _glfwAppendDictDataDBusPOSIX(&sub1, DBUS_TYPE_STRING, &progressVisibleStr, DBUS_TYPE_BOOLEAN, &progressVisible);

    //Set progress value property
    const char* progressStr = "progress";
    _glfwAppendDictDataDBusPOSIX(&sub1, DBUS_TYPE_STRING, &progressStr, DBUS_TYPE_DOUBLE, &progressValue);

    _glfwCloseContainerDBusPOSIX(&args, &sub1);

    _glfwSendMessageDBusPOSIX(msg);

    //Free the message
    dbus_message_unref(msg);
}

dbus_bool_t _glfwNewMessageSignalDBusPOSIX(const char* objectPath, const char* interfaceName, const char* signalName, struct DBusMessage** outMessage)
{
    if(!outMessage)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to create new DBus message, output message pointer is NULL");
        return GLFW_FALSE;
    }

    *outMessage = dbus_message_new_signal(objectPath, interfaceName, signalName);
    if(!(*outMessage))
    {
        *outMessage = NULL;
        _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to create new DBus message");
        return GLFW_FALSE;
    }

    return GLFW_TRUE;
}

dbus_bool_t _glfwOpenContainerDBusPOSIX(struct DBusMessageIter* iterator, int DBusType, const char* signature, struct DBusMessageIter* subIterator)
{
    if(DBusType != DBUS_TYPE_ARRAY && DBusType != DBUS_TYPE_STRUCT_OPEN &&
       DBusType != DBUS_TYPE_STRUCT_CLOSE && DBusType != DBUS_TYPE_VARIANT &&
       DBusType != DBUS_TYPE_DICT_ENTRY)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Invalid DBUS container type provided");
        return GLFW_FALSE;
    }
    if(!iterator || !subIterator)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "DBus message iterator is NULL");
        return GLFW_FALSE;
    }

    return dbus_message_iter_open_container(iterator, DBusType, signature, subIterator);
}

dbus_bool_t _glfwCloseContainerDBusPOSIX(struct DBusMessageIter* iterator, struct DBusMessageIter* subIterator)
{
    if(!iterator || !subIterator)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "DBus message iterator is NULL");
        return GLFW_FALSE;
    }

    return dbus_message_iter_close_container(iterator, subIterator);
}

dbus_bool_t _glfwAppendDataDBusPOSIX(struct DBusMessageIter* iterator, int DBusType, const void* data)
{
    if(DBusType == DBUS_TYPE_ARRAY || DBusType == DBUS_TYPE_VARIANT || DBusType == DBUS_TYPE_DICT_ENTRY || DBusType == DBUS_TYPE_STRUCT_OPEN || DBusType == DBUS_TYPE_STRUCT_CLOSE)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Invalid DBus type provided");
        return GLFW_FALSE;
    }
    if(!iterator)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "DBus message iterator is NULL");
        return GLFW_FALSE;
    }
    if(!data)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "DBus data to append is NULL");
        return GLFW_FALSE;
    }

    return dbus_message_iter_append_basic(iterator, DBusType, data);
}

dbus_bool_t _glfwAppendDictDataDBusPOSIX(struct DBusMessageIter* iterator, int keyType, const void* keyData, int valueType, const void* valueData)
{
    struct DBusMessageIter keyIterator;
    struct DBusMessageIter valueIterator;
    memset(&keyIterator, 0, sizeof(keyIterator));
    memset(&valueIterator, 0, sizeof(valueIterator));

    if(!_glfwOpenContainerDBusPOSIX(iterator, DBUS_TYPE_DICT_ENTRY, NULL, &keyIterator))
        return GLFW_FALSE;

    //Append key data
    if(!_glfwAppendDataDBusPOSIX(&keyIterator, keyType, keyData))
        return GLFW_FALSE;

    if(!_glfwOpenContainerDBusPOSIX(&keyIterator, DBUS_TYPE_VARIANT, (const char*)&valueType, &valueIterator))
        return GLFW_FALSE;

    //Append value data
    if(!_glfwAppendDataDBusPOSIX(&valueIterator, valueType, valueData))
        return GLFW_FALSE;

    if(!_glfwCloseContainerDBusPOSIX(&keyIterator, &valueIterator))
        return GLFW_FALSE;

    if(!_glfwCloseContainerDBusPOSIX(iterator, &keyIterator))
        return GLFW_FALSE;

    return GLFW_TRUE;
}

dbus_bool_t _glfwSendMessageDBusPOSIX(struct DBusMessage* message)
{
    if(!message)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "DBus message is NULL");
        return GLFW_FALSE;
    }

    unsigned int serial = 0;
    if(!dbus_connection_send(_glfw.dbus.connection, message, &serial))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to send DBus message");
        return GLFW_FALSE;
    }

    dbus_connection_flush(_glfw.dbus.connection);

    return GLFW_TRUE;
}