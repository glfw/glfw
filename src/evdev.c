//========================================================================
// GLFW 3.3 Linux - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2017 Camilla Löwy <elmindreda@glfw.org>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#ifndef SYN_DROPPED // < v2.6.39 kernel headers
// Workaround for CentOS-6, which is supported till 2020-11-30, but still on v2.6.32
#define SYN_DROPPED 3
#endif

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)
#define set_bit(bit, array)	    (array[LONG(bit)] |= BIT(OFF(bit)))
#define reset_bit(bit, array)	(array[LONG(bit)] &= ~BIT(OFF(bit)))

// Translate a GLFW key code to a char.
static unsigned int translateGlfwKeyCodeToChar( int code, GLFWbool upper )
{
    switch( code )
    {
        case GLFW_KEY_SPACE:            return ' ';
        case GLFW_KEY_APOSTROPHE:       return upper ? '\'' : '\"';
        case GLFW_KEY_COMMA:            return upper ? '<' : ',';
        case GLFW_KEY_MINUS:            return upper ? '_' : '-';
        case GLFW_KEY_PERIOD:           return upper ? '>' : '.';
        case GLFW_KEY_SLASH:            return upper ? '?' : '/';
        case GLFW_KEY_0:                return upper ? ')' : '0';
        case GLFW_KEY_1:                return upper ? '!' : '1';
        case GLFW_KEY_2:                return upper ? '@' : '2';
        case GLFW_KEY_3:                return upper ? '#' : '3';
        case GLFW_KEY_4:                return upper ? '$' : '4';
        case GLFW_KEY_5:                return upper ? '%' : '5';
        case GLFW_KEY_6:                return upper ? '^' : '6';
        case GLFW_KEY_7:                return upper ? '&' : '7';
        case GLFW_KEY_8:                return upper ? '*' : '8';
        case GLFW_KEY_9:                return upper ? '(' : '9';
        case GLFW_KEY_SEMICOLON:        return upper ? ':' : ';';
        case GLFW_KEY_EQUAL:            return upper ? '+' : '=';
        case GLFW_KEY_A:                return upper ? 'A' : 'a';
        case GLFW_KEY_B:                return upper ? 'B' : 'b';
        case GLFW_KEY_C:                return upper ? 'C' : 'c';
        case GLFW_KEY_D:                return upper ? 'D' : 'd';
        case GLFW_KEY_E:                return upper ? 'E' : 'e';
        case GLFW_KEY_F:                return upper ? 'F' : 'f';
        case GLFW_KEY_G:                return upper ? 'G' : 'g';
        case GLFW_KEY_H:                return upper ? 'H' : 'h';
        case GLFW_KEY_I:                return upper ? 'I' : 'i';
        case GLFW_KEY_J:                return upper ? 'J' : 'j';
        case GLFW_KEY_K:                return upper ? 'K' : 'k';
        case GLFW_KEY_L:                return upper ? 'L' : 'l';
        case GLFW_KEY_M:                return upper ? 'M' : 'm';
        case GLFW_KEY_N:                return upper ? 'N' : 'n';
        case GLFW_KEY_O:                return upper ? 'O' : 'o';
        case GLFW_KEY_P:                return upper ? 'P' : 'p';
        case GLFW_KEY_Q:                return upper ? 'Q' : 'q';
        case GLFW_KEY_R:                return upper ? 'R' : 'r';
        case GLFW_KEY_S:                return upper ? 'S' : 's';
        case GLFW_KEY_T:                return upper ? 'T' : 't';
        case GLFW_KEY_U:                return upper ? 'U' : 'u';
        case GLFW_KEY_V:                return upper ? 'V' : 'v';
        case GLFW_KEY_W:                return upper ? 'W' : 'w';
        case GLFW_KEY_X:                return upper ? 'X' : 'x';
        case GLFW_KEY_Y:                return upper ? 'Y' : 'y';
        case GLFW_KEY_Z:                return upper ? 'Z' : 'z';
        case GLFW_KEY_LEFT_BRACKET:     return upper ? '{' : '[';
        case GLFW_KEY_BACKSLASH:        return upper ? '|' : '\\';
        case GLFW_KEY_RIGHT_BRACKET:    return upper ? '}' : ']';
        case GLFW_KEY_GRAVE_ACCENT:     return upper ? '~' : '`';
    }
    return 0u;
}

// Translate an linux/input key code to a GLFW key code.
static int translateKeyCodeToGlfw( int code )
{
    switch( code )
    {
        /* Printable keys */
        case KEY_SPACE: return GLFW_KEY_SPACE;
        case KEY_APOSTROPHE: return GLFW_KEY_APOSTROPHE;
        case KEY_COMMA: return GLFW_KEY_COMMA;
        case KEY_MINUS: return GLFW_KEY_MINUS;
        case KEY_DOT: return GLFW_KEY_PERIOD;
        case KEY_SLASH: return GLFW_KEY_SLASH;
        case KEY_0: return GLFW_KEY_0;
        case KEY_1: return GLFW_KEY_1;
        case KEY_2: return GLFW_KEY_2;
        case KEY_3: return GLFW_KEY_3;
        case KEY_4: return GLFW_KEY_4;
        case KEY_5: return GLFW_KEY_5;
        case KEY_6: return GLFW_KEY_6;
        case KEY_7: return GLFW_KEY_7;
        case KEY_8: return GLFW_KEY_8;
        case KEY_9: return GLFW_KEY_9;
        case KEY_SEMICOLON: return GLFW_KEY_SEMICOLON;
        case KEY_EQUAL: return GLFW_KEY_EQUAL;
        case KEY_A: return GLFW_KEY_A;
        case KEY_B: return GLFW_KEY_B;
        case KEY_C: return GLFW_KEY_C;
        case KEY_D: return GLFW_KEY_D;
        case KEY_E: return GLFW_KEY_E;
        case KEY_F: return GLFW_KEY_F;
        case KEY_G: return GLFW_KEY_G;
        case KEY_H: return GLFW_KEY_H;
        case KEY_I: return GLFW_KEY_I;
        case KEY_J: return GLFW_KEY_J;
        case KEY_K: return GLFW_KEY_K;
        case KEY_L: return GLFW_KEY_L;
        case KEY_M: return GLFW_KEY_M;
        case KEY_N: return GLFW_KEY_N;
        case KEY_O: return GLFW_KEY_O;
        case KEY_P: return GLFW_KEY_P;
        case KEY_Q: return GLFW_KEY_Q;
        case KEY_R: return GLFW_KEY_R;
        case KEY_S: return GLFW_KEY_S;
        case KEY_T: return GLFW_KEY_T;
        case KEY_U: return GLFW_KEY_U;
        case KEY_V: return GLFW_KEY_V;
        case KEY_W: return GLFW_KEY_W;
        case KEY_X: return GLFW_KEY_X;
        case KEY_Y: return GLFW_KEY_Y;
        case KEY_Z: return GLFW_KEY_Z;
        case KEY_LEFTBRACE: return GLFW_KEY_LEFT_BRACKET;
        case KEY_BACKSLASH: return GLFW_KEY_BACKSLASH;
        case KEY_RIGHTBRACE: return GLFW_KEY_RIGHT_BRACKET;
        case KEY_GRAVE: return GLFW_KEY_GRAVE_ACCENT;
        // GLFW_KEY_WORLD_1 not found
        // GLFW_KEY_WORLD_2 not found
        
        /* Function keys */
        case KEY_ESC: return GLFW_KEY_ESCAPE;
        case KEY_ENTER: return GLFW_KEY_ENTER;
        case KEY_TAB: return GLFW_KEY_TAB;
        case KEY_BACKSPACE: return GLFW_KEY_BACKSPACE;
        case KEY_INSERT: return GLFW_KEY_INSERT;
        case KEY_DELETE: return GLFW_KEY_DELETE;
        case KEY_RIGHT: return GLFW_KEY_RIGHT;
        case KEY_LEFT: return GLFW_KEY_LEFT;
        case KEY_DOWN: return GLFW_KEY_DOWN;
        case KEY_UP: return GLFW_KEY_UP;
        case KEY_PAGEUP: return GLFW_KEY_PAGE_UP;
        case KEY_PAGEDOWN: return GLFW_KEY_PAGE_DOWN;
        case KEY_HOME: return GLFW_KEY_HOME;
        case KEY_END: return GLFW_KEY_END;
        case KEY_CAPSLOCK: return GLFW_KEY_CAPS_LOCK;
        case KEY_SCROLLLOCK: return GLFW_KEY_SCROLL_LOCK;
        case KEY_NUMLOCK: return GLFW_KEY_NUM_LOCK;
        case KEY_SYSRQ: return GLFW_KEY_PRINT_SCREEN;
        case KEY_PAUSE: return GLFW_KEY_PAUSE;
        case KEY_F1: return GLFW_KEY_F1;
        case KEY_F2: return GLFW_KEY_F2;
        case KEY_F3: return GLFW_KEY_F3;
        case KEY_F4: return GLFW_KEY_F4;
        case KEY_F5: return GLFW_KEY_F5;
        case KEY_F6: return GLFW_KEY_F6;
        case KEY_F7: return GLFW_KEY_F7;
        case KEY_F8: return GLFW_KEY_F8;
        case KEY_F9: return GLFW_KEY_F9;
        case KEY_F10: return GLFW_KEY_F10;
        case KEY_F11: return GLFW_KEY_F11;
        case KEY_F12: return GLFW_KEY_F12;
        case KEY_F13: return GLFW_KEY_F13;
        case KEY_F14: return GLFW_KEY_F14;
        case KEY_F15: return GLFW_KEY_F15;
        case KEY_F16: return GLFW_KEY_F16;
        case KEY_F17: return GLFW_KEY_F17;
        case KEY_F18: return GLFW_KEY_F18;
        case KEY_F19: return GLFW_KEY_F19;
        case KEY_F20: return GLFW_KEY_F20;
        case KEY_F21: return GLFW_KEY_F21;
        case KEY_F22: return GLFW_KEY_F22;
        case KEY_F23: return GLFW_KEY_F23;
        case KEY_F24: return GLFW_KEY_F24;
        // GLFW_KEY_F25 not found
        case KEY_KP0: return GLFW_KEY_KP_0;
        case KEY_KP1: return GLFW_KEY_KP_1;
        case KEY_KP2: return GLFW_KEY_KP_2;
        case KEY_KP3: return GLFW_KEY_KP_3;
        case KEY_KP4: return GLFW_KEY_KP_4;
        case KEY_KP5: return GLFW_KEY_KP_5;
        case KEY_KP6: return GLFW_KEY_KP_6;
        case KEY_KP7: return GLFW_KEY_KP_7;
        case KEY_KP8: return GLFW_KEY_KP_8;
        case KEY_KP9: return GLFW_KEY_KP_9;
        case KEY_KPDOT: return GLFW_KEY_KP_DECIMAL;
        case KEY_KPSLASH: return GLFW_KEY_KP_DIVIDE;
        case KEY_KPASTERISK: return GLFW_KEY_KP_MULTIPLY;
        case KEY_KPMINUS: return GLFW_KEY_KP_SUBTRACT;
        case KEY_KPPLUS: return GLFW_KEY_KP_ADD;
        case KEY_KPENTER: return GLFW_KEY_KP_ENTER;
        case KEY_KPEQUAL: return GLFW_KEY_KP_EQUAL;
        case KEY_LEFTSHIFT: return GLFW_KEY_LEFT_SHIFT;
        case KEY_LEFTCTRL: return GLFW_KEY_LEFT_CONTROL;
        case KEY_LEFTALT: return GLFW_KEY_LEFT_ALT;
        case KEY_LEFTMETA: return GLFW_KEY_LEFT_SUPER;
        case KEY_RIGHTSHIFT: return GLFW_KEY_RIGHT_SHIFT;
        case KEY_RIGHTCTRL: return GLFW_KEY_RIGHT_CONTROL;
        case KEY_RIGHTALT: return GLFW_KEY_RIGHT_ALT;
        case KEY_RIGHTMETA: return GLFW_KEY_RIGHT_SUPER;
        case KEY_COMPOSE: return GLFW_KEY_MENU;
        default:
            break;
    }
    return GLFW_KEY_UNKNOWN;
}

// Translate an linux/input key code to a GLFW mouse button code.
static int translateMouseButtonCodeToGlfw( int code )
{
    switch( code )
    {
        case BTN_LEFT: return GLFW_MOUSE_BUTTON_1;
        case BTN_RIGHT: return GLFW_MOUSE_BUTTON_2;
        case BTN_MIDDLE: return GLFW_MOUSE_BUTTON_3;
        case BTN_SIDE: return GLFW_MOUSE_BUTTON_4;
        case BTN_EXTRA: return GLFW_MOUSE_BUTTON_5;
        case BTN_FORWARD: return GLFW_MOUSE_BUTTON_6;
        case BTN_BACK: return GLFW_MOUSE_BUTTON_7;
        case BTN_TASK: return GLFW_MOUSE_BUTTON_8;
        default:
            break;
    }
    return GLFW_KEY_UNKNOWN;
}

// Translate an linux/input key action code to a GLFW action code.
static int translateActionCodeToGlfw( int code )
{
    switch( code )
    {
        case 0: return GLFW_RELEASE;
        case 1: return GLFW_PRESS;
        case 2: return GLFW_REPEAT;
        default:
            break;
    }
    return GLFW_RELEASE;
}

static void updateMods()
{
    int edid;
    int mods = 0;
    
    _GLFWeventDevice* ed = NULL;
    
    for (edid = 0;  edid <= GLFW_EVENT_DEVICES_MAX;  edid++)
    {
        ed = _glfw.evdev.devices + edid;
        if (ed->present){
            mods |= ed->mods;
        }
    }
    
    _glfw.evdev.mods = mods;
}

static void updateEventDeviceMods(_GLFWeventDevice* ed)
{
    int mods = 0;
    
    if(test_bit(KEY_LEFTSHIFT, ed->keyState))
        mods |= GLFW_MOD_SHIFT;
    if(test_bit(KEY_RIGHTSHIFT, ed->keyState))
        mods |= GLFW_MOD_SHIFT;
    
    if(test_bit(KEY_LEFTCTRL, ed->keyState))
        mods |= GLFW_MOD_CONTROL;
    if(test_bit(KEY_RIGHTCTRL, ed->keyState))
        mods |= GLFW_MOD_CONTROL;
    
    if(test_bit(KEY_LEFTALT, ed->keyState))
        mods |= GLFW_MOD_ALT;
    if(test_bit(KEY_RIGHTALT, ed->keyState))
        mods |= GLFW_MOD_ALT;
    
    if(test_bit(KEY_LEFTMETA, ed->keyState))
        mods |= GLFW_MOD_SUPER;
    if(test_bit(KEY_RIGHTMETA, ed->keyState))
        mods |= GLFW_MOD_SUPER;
    
    if(test_bit(LED_CAPSL, ed->ledState))
        mods |= GLFW_MOD_CAPS_LOCK;
    
    if(test_bit(LED_NUML, ed->ledState))
        mods |= GLFW_MOD_NUM_LOCK;

    ed->mods = mods;
    updateMods();
}

// Apply an EV_KEY event to the specified event device
//
static void handleKeyEvent(_GLFWeventDevice* ed, int code, int value)
{
    int button, key, action, mods;
    GLFWbool upper;
    unsigned int codepoint;
    if( value )
        set_bit(code, ed->keyState);
    else
        reset_bit(code, ed->keyState);
    updateEventDeviceMods(ed);
    mods = _glfw.evdev.mods;
    button = translateMouseButtonCodeToGlfw(code);
    action = translateActionCodeToGlfw(value);
    
    if (button!=GLFW_KEY_UNKNOWN){
        _glfwEvdevInputMouseClick(button, action, mods);
    }else{
        key = translateKeyCodeToGlfw(code);
        _glfwEvdevInputKey(key, ed->scancode, action, mods);
        
        if (action != GLFW_RELEASE){
            upper = mods & GLFW_MOD_SHIFT;
            if ((codepoint = translateGlfwKeyCodeToChar(key, upper))){
                if( (mods & GLFW_MOD_CAPS_LOCK) && isalpha(codepoint) ){
                    codepoint = isupper(codepoint) ? tolower(codepoint) : toupper(codepoint);
                }
                _glfwEvdevInputChar(codepoint, mods, GLFW_TRUE);
            }
        }
    }
}

// Apply an EV_REL event to the specified event device
//
static void handleRelEvent(_GLFWeventDevice* ed, int code, int value)
{
    if (code == REL_X){
        _glfwEvdevInputCursorMove(value, .0);
    }else if (code == REL_Y)
        _glfwEvdevInputCursorMove(.0, value);
    else if (code == REL_HWHEEL)
        _glfwEvdevInputScroll(value, .0);
    else if (code == REL_WHEEL)
        _glfwEvdevInputScroll(.0, value);
}

// Apply an EV_ABS event to the specified event device
//
static void handleAbsEvent(_GLFWeventDevice* ed, int code, int value)
{
    if (code == ABS_X){
        _glfwEvdevInputCursorPos(value, .0);
    }else if (code == ABS_Y)
        _glfwEvdevInputCursorPos(.0, value);
}

// Apply an EV_LED event to the specified event device
//
static void handleLedEvent(_GLFWeventDevice* ed, int code, int value)
{
    if( value )
        set_bit(code, ed->ledState);
    else
        reset_bit(code, ed->ledState);
    updateEventDeviceMods(ed);
}

// Poll keys and leds state of the specified event device
//
static void pollEventDeviceState(_GLFWeventDevice* ed)
{
    int r;
    
    memset(ed->keyState, 0, sizeof(ed->keyState));
    memset(ed->ledState, 0, sizeof(ed->ledState));
    
    r = ioctl(ed->fd, EVIOCGKEY( KEY_MAX ), ed->keyState);
    if (r == -1){
        _glfwInputError(GLFW_PLATFORM_ERROR, "Evdev: Failed to request keys state");
    }
    
    r = ioctl(ed->fd, EVIOCGLED( LED_MAX ), ed->ledState);
    if (r == -1){
        _glfwInputError(GLFW_PLATFORM_ERROR, "Evdev: Failed to request leds state");
    }
    
    updateEventDeviceMods(ed);
}

// Attempt to open the specified event device
//
static GLFWbool openEventDevice(const char* path)
{
    int fd;
    int edid;
    char name[256] = "";
    _GLFWeventDevice* ed = NULL;
    
    for (edid = 0;  edid <= GLFW_EVENT_DEVICES_MAX;  edid++)
    {
        if (!_glfw.evdev.devices[edid].present)
            continue;
        if (strcmp(_glfw.evdev.devices[edid].path, path) == 0)
            return GLFW_FALSE;
    }

    for (edid = 0;  edid <= GLFW_EVENT_DEVICES_MAX;  edid++)
    {
        if (!_glfw.evdev.devices[edid].present)
            break;
    }

    if (edid > GLFW_EVENT_DEVICES_MAX)
        return GLFW_FALSE;

    ed = _glfw.evdev.devices + edid;
    
    fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd == -1)
        return GLFW_FALSE;

    if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0)
        strncpy(name, "Unknown", sizeof(name));
    
    ed->fd          = fd;
    ed->present     = GLFW_TRUE;
    ed->name        = _glfw_strdup(name);
    strncpy(ed->path, path, sizeof(ed->path));
    
    pollEventDeviceState(ed);
    
    return GLFW_TRUE;
}

// Frees all resources associated with the specified event device
//
static void closeEventDevice(_GLFWeventDevice* ed)
{
    free(ed->name);
    close(ed->fd);
    memset(ed, 0, sizeof(_GLFWeventDevice));
    updateMods();
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize evdev interface
//
GLFWbool _glfwInitEvdev()
{
    DIR* dir;
    int count = 0;
    const char* dirname = "/dev/input";

    _glfw.evdev.inotify = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (_glfw.evdev.inotify > 0)
    {
        // HACK: Register for IN_ATTRIB to get notified when udev is done
        //       This works well in practice but the true way is libudev

        _glfw.evdev.watch = inotify_add_watch(_glfw.evdev.inotify,
                                              dirname,
                                              IN_CREATE | IN_ATTRIB | IN_DELETE);
    }

    // Continue without device connection notifications if inotify fails

    if (regcomp(&_glfw.evdev.regex, "^event[0-9]\\+$", 0) != 0)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "Evdev: Failed to compile regex");
        return GLFW_FALSE;
    }

    dir = opendir(dirname);
    if (dir)
    {
        struct dirent* entry;

        while ((entry = readdir(dir)))
        {
            regmatch_t match;

            if (regexec(&_glfw.evdev.regex, entry->d_name, 1, &match, 0) != 0)
                continue;

            char path[PATH_MAX];

            snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

            if (openEventDevice(path))
                count++;
        }

        closedir(dir);
    }

    // Continue with no devices if enumeration fails
    return GLFW_TRUE;
}

void _glfwTerminateEvdev()
{
    int edid;

    for (edid = 0;  edid <= GLFW_EVENT_DEVICES_MAX;  edid++)
    {
        _GLFWeventDevice* ed = _glfw.evdev.devices + edid;
        if (ed->present)
            closeEventDevice(ed);
    }

    regfree(&_glfw.evdev.regex);

    if (_glfw.evdev.inotify > 0)
    {
        if (_glfw.evdev.watch > 0)
            inotify_rm_watch(_glfw.evdev.inotify, _glfw.evdev.watch);

        close(_glfw.evdev.inotify);
    }
}

void _glfwDetectEvdevConnection()
{
    ssize_t offset = 0;
    char buffer[16384];
    
    if (_glfw.evdev.inotify <= 0)
        return;

    const ssize_t size = read(_glfw.evdev.inotify, buffer, sizeof(buffer));

    while (size > offset)
    {
        regmatch_t match;
        const struct inotify_event* e = (struct inotify_event*) (buffer + offset);

        offset += sizeof(struct inotify_event) + e->len;

        if (regexec(&_glfw.evdev.regex, e->name, 1, &match, 0) != 0)
            continue;

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "/dev/input/%s", e->name);

        if (e->mask & (IN_CREATE | IN_ATTRIB))
        {
            openEventDevice(path);
        }
        else if (e->mask & IN_DELETE)
        {
            int edid;
            for (edid = 0;  edid <= GLFW_EVENT_DEVICES_MAX;  edid++)
            {
                if (strcmp(_glfw.evdev.devices[edid].path, path) == 0)
                {
                    closeEventDevice(_glfw.evdev.devices + edid);
                    break;
                }
            }
        }
    }
}

int _glfwPollEvdevDevice( _GLFWeventDevice* ed)
{
    // Read all queued events (non-blocking)
    for (;;)
    {
        struct input_event e;

        errno = 0;
        if (read(ed->fd, &e, sizeof(e)) < 0)
        {
            // Reset the event device slot if the device was disconnected
            if (errno == ENODEV)
                closeEventDevice(ed);

            break;
        }

        if (e.type == EV_SYN)
        {
            if (e.code == SYN_DROPPED)
                ed->dropped = GLFW_TRUE;
            else if (e.code == SYN_REPORT)
            {
                ed->dropped = GLFW_FALSE;
                ed->scancode = 0;
                pollEventDeviceState(ed);
            }
        }

        if (ed->dropped){
            continue;
        }

        if (e.type == EV_MSC){
            if (e.code == MSC_SCAN){
                ed->scancode = e.value;
            }
        }else if (e.type == EV_KEY)
            handleKeyEvent(ed, e.code, e.value);
        else if (e.type == EV_REL)
            handleRelEvent(ed, e.code, e.value);
        else if (e.type == EV_ABS)
            handleAbsEvent(ed, e.code, e.value);
        else if (e.type == EV_LED)
            handleLedEvent(ed, e.code, e.value);
    }

    return ed->present;
}

void _glfwPollEvdevDevices()
{
    int edid;
    _GLFWeventDevice* ed = NULL;
    
    for (edid = 0;  edid <= GLFW_EVENT_DEVICES_MAX;  edid++)
    {
        ed = _glfw.evdev.devices + edid;
        if (ed->present)
            _glfwPollEvdevDevice(ed);
    }
}
