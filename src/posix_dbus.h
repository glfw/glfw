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

//Taken from DBus docs (https://dbus.freedesktop.org/doc/api/html/index.html)
typedef struct DBusConnection DBusConnection;
typedef struct DBusMessage DBusMessage;
typedef unsigned int dbus_bool_t;
typedef unsigned int dbus_uint32_t;

enum DBusBusType
{
    DBUS_BUS_SESSION,
    DBUS_BUS_SYSTEM,
    DBUS_BUS_STARTER
};

struct DBusError
{
    const char* name;
    const char* message;
    unsigned int dummy1 : 1;
    unsigned int dummy2 : 1;
    unsigned int dummy3 : 1;
    unsigned int dummy4 : 1;
    unsigned int dummy5 : 1;
    void* padding1;
};

struct DBusMessageIter
{
    void* dummy1;
    void* dummy2;
    dbus_uint32_t dummy3;
    int dummy4, dummy5, dummy6, dummy7, dummy8, dummy9, dummy10, dummy11;
    int pad1;
    void* pad2;
    void* pad3;
};

#define DBUS_NAME_FLAG_REPLACE_EXISTING 0x2
#define DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER 1
#define DBUS_TYPE_STRING (unsigned int)'s'
#define DBUS_TYPE_ARRAY (unsigned int)'a'
#define DBUS_TYPE_DICT_ENTRY (unsigned int)'e'
#define DBUS_TYPE_VARIANT (unsigned int)'v'
#define DBUS_TYPE_BOOLEAN (unsigned int)'b'
#define DBUS_TYPE_DOUBLE (unsigned int)'d'
#define DBUS_TYPE_INT64 (unsigned int)'x'

typedef void (* PFN_dbus_error_init)(struct DBusError*);
typedef dbus_bool_t (* PFN_dbus_error_is_set)(const struct DBusError*);
typedef void (* PFN_dbus_error_free)(struct DBusError*);
typedef void (* PFN_dbus_connection_unref)(DBusConnection*);
typedef dbus_bool_t (* PFN_dbus_connection_send)(DBusConnection*, DBusMessage*, dbus_uint32_t*);
typedef void (* PFN_dbus_connection_flush)(DBusConnection*);
typedef int (* PFN_dbus_bus_request_name)(DBusConnection*, const char*, unsigned int, struct DBusError*);
typedef DBusConnection* (* PFN_dbus_bus_get)(enum DBusBusType, struct DBusError*);
typedef void (* PFN_dbus_message_unref)(DBusMessage*);
typedef DBusMessage* (* PFN_dbus_message_new_signal)(const char*, const char*, const char*);
typedef void (* PFN_dbus_message_iter_init_append)(DBusMessage*, struct DBusMessageIter*);
typedef dbus_bool_t (* PFN_dbus_message_iter_append_basic)(struct DBusMessageIter*, int, const void*);
typedef dbus_bool_t (* PFN_dbus_message_iter_open_container)(struct DBusMessageIter*, int, const char*, struct DBusMessageIter*);
typedef dbus_bool_t (* PFN_dbus_message_iter_close_container)(struct DBusMessageIter*, struct DBusMessageIter*);

#define dbus_error_init _glfw.dbus.error_init
#define dbus_error_is_set _glfw.dbus.error_is_set
#define dbus_error_free _glfw.dbus.error_free
#define dbus_connection_unref _glfw.dbus.connection_unref
#define dbus_connection_send _glfw.dbus.connection_send
#define dbus_connection_flush _glfw.dbus.connection_flush
#define dbus_bus_request_name _glfw.dbus.bus_request_name
#define dbus_bus_get _glfw.dbus.bus_get
#define dbus_message_unref _glfw.dbus.message_unref
#define dbus_message_new_signal _glfw.dbus.message_new_signal
#define dbus_message_iter_init_append _glfw.dbus.message_iter_init_append
#define dbus_message_iter_append_basic _glfw.dbus.message_iter_append_basic
#define dbus_message_iter_open_container _glfw.dbus.message_iter_open_container
#define dbus_message_iter_close_container _glfw.dbus.message_iter_close_container

#define GLFW_POSIX_LIBRARY_DBUS_STATE    _GLFWDBusPOSIX   dbus;

// POSIX-specific dbus data
//
typedef struct _GLFWDBusPOSIX
{
    void* handle;

    PFN_dbus_error_init error_init;
    PFN_dbus_error_is_set error_is_set;
    PFN_dbus_error_free error_free;
    PFN_dbus_connection_unref connection_unref;
    PFN_dbus_connection_send connection_send;
    PFN_dbus_connection_flush connection_flush;
    PFN_dbus_bus_request_name bus_request_name;
    PFN_dbus_bus_get bus_get;
    PFN_dbus_message_unref message_unref;
    PFN_dbus_message_new_signal message_new_signal;
    PFN_dbus_message_iter_init_append message_iter_init_append;
    PFN_dbus_message_iter_append_basic message_iter_append_basic;
    PFN_dbus_message_iter_open_container message_iter_open_container;
    PFN_dbus_message_iter_close_container message_iter_close_container;

    DBusConnection* connection;
    struct DBusError error;

    char* desktopFilePath;
} _GLFWDBusPOSIX;

void _glfwInitDBusPOSIX(void);
void _glfwCacheDesktopFilePathPOSIX(void);
void _glfwTerminateDBusPOSIX(void);
void _glfwUpdateTaskbarProgressDBusPOSIX(dbus_bool_t progressVisible, double progressValue);
void _glfwUpdateBadgeDBusPOSIX(dbus_bool_t badgeVisible, int badgeCount);
