//========================================================================
// GLFW 3.1 Win32 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2015 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <math.h>

#define _GLFW_UPDATE_BUTTONS 1
#define _GLFW_UPDATE_AXES    2


// Returns a description fitting the specified XInput capabilities
//
static const char* getDeviceDescription(const XINPUT_CAPABILITIES* xic)
{
    switch (xic->SubType)
    {
        case XINPUT_DEVSUBTYPE_WHEEL:
            return "XInput Wheel";
        case XINPUT_DEVSUBTYPE_ARCADE_STICK:
            return "XInput Arcade Stick";
        case XINPUT_DEVSUBTYPE_FLIGHT_STICK:
            return "XInput Flight Stick";
        case XINPUT_DEVSUBTYPE_DANCE_PAD:
            return "XInput Dance Pad";
        case XINPUT_DEVSUBTYPE_GUITAR:
            return "XInput Guitar";
        case XINPUT_DEVSUBTYPE_DRUM_KIT:
            return "XInput Drum Kit";
        case XINPUT_DEVSUBTYPE_GAMEPAD:
        {
            if (xic->Flags & XINPUT_CAPS_WIRELESS)
                return "Wireless Xbox 360 Controller";
            else
                return "Xbox 360 Controller";
        }
    }

    return "Unknown XInput Device";
}

// Attempt to open the specified joystick device
// TODO: Pack state arrays for non-gamepad devices
//
static GLFWbool openJoystickDevice(DWORD index)
{
    int joy;
    XINPUT_CAPABILITIES xic;
    _GLFWjoystickWin32* js;

    for (joy = GLFW_JOYSTICK_1;  joy <= GLFW_JOYSTICK_LAST;  joy++)
    {
        if (_glfw.win32_js[joy].present && _glfw.win32_js[joy].index == index)
            return GLFW_FALSE;
    }

    for (joy = GLFW_JOYSTICK_1;  joy <= GLFW_JOYSTICK_LAST;  joy++)
    {
        if (!_glfw.win32_js[joy].present)
            break;
    }

    if (joy > GLFW_JOYSTICK_LAST)
        return GLFW_FALSE;

    if (_glfw_XInputGetCapabilities(index, 0, &xic) != ERROR_SUCCESS)
        return GLFW_FALSE;

    js = _glfw.win32_js + joy;
    js->axisCount = 6;
    js->buttonCount = 14;
    js->present = GLFW_TRUE;
    js->name = strdup(getDeviceDescription(&xic));
    js->index = index;

    return GLFW_TRUE;
}

// Polls for and processes events the specified joystick
//
static GLFWbool pollJoystickEvents(_GLFWjoystickWin32* js, int flags)
{
    XINPUT_STATE xis;
    DWORD result;

    if (!_glfw.win32.xinput.instance)
        return GLFW_FALSE;

    if (!js->present)
        return GLFW_FALSE;

    result = _glfw_XInputGetState(js->index, &xis);
    if (result != ERROR_SUCCESS)
    {
        if (result == ERROR_DEVICE_NOT_CONNECTED)
        {
            free(js->name);
            memset(js, 0, sizeof(_GLFWjoystickWin32));
        }

        return GLFW_FALSE;
    }

    if (flags & _GLFW_UPDATE_AXES)
    {
        if (sqrtf((float) (xis.Gamepad.sThumbLX * xis.Gamepad.sThumbLX +
                           xis.Gamepad.sThumbLY * xis.Gamepad.sThumbLY)) >
            (float) XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        {
            js->axes[0] = (xis.Gamepad.sThumbLX + 0.5f) / 32767.f;
            js->axes[1] = (xis.Gamepad.sThumbLY + 0.5f) / 32767.f;
        }
        else
        {
            js->axes[0] = 0.f;
            js->axes[1] = 0.f;
        }

        if (sqrtf((float) (xis.Gamepad.sThumbRX * xis.Gamepad.sThumbRX +
                           xis.Gamepad.sThumbRY * xis.Gamepad.sThumbRY)) >
            (float) XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
        {
            js->axes[2] = (xis.Gamepad.sThumbRX + 0.5f) / 32767.f;
            js->axes[3] = (xis.Gamepad.sThumbRY + 0.5f) / 32767.f;
        }
        else
        {
            js->axes[2] = 0.f;
            js->axes[3] = 0.f;
        }

        if (xis.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            js->axes[4] = xis.Gamepad.bLeftTrigger / 127.5f - 1.f;
        else
            js->axes[4] = -1.f;

        if (xis.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            js->axes[5] = xis.Gamepad.bRightTrigger / 127.5f - 1.f;
        else
            js->axes[5] = -1.f;
    }

    if (flags & _GLFW_UPDATE_BUTTONS)
    {
        int i;
        const WORD buttons[14] =
        {
            XINPUT_GAMEPAD_A,
            XINPUT_GAMEPAD_B,
            XINPUT_GAMEPAD_X,
            XINPUT_GAMEPAD_Y,
            XINPUT_GAMEPAD_LEFT_SHOULDER,
            XINPUT_GAMEPAD_RIGHT_SHOULDER,
            XINPUT_GAMEPAD_BACK,
            XINPUT_GAMEPAD_START,
            XINPUT_GAMEPAD_LEFT_THUMB,
            XINPUT_GAMEPAD_RIGHT_THUMB,
            XINPUT_GAMEPAD_DPAD_UP,
            XINPUT_GAMEPAD_DPAD_RIGHT,
            XINPUT_GAMEPAD_DPAD_DOWN,
            XINPUT_GAMEPAD_DPAD_LEFT
        };

        for (i = 0;  i < 14;  i++)
            js->buttons[i] = (xis.Gamepad.wButtons & buttons[i]) ? 1 : 0;
    }

    return GLFW_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize joystick interface
//
void _glfwInitJoysticksWin32(void)
{
    _glfwDetectJoystickConnectionWin32();
}

// Close all opened joystick handles
//
void _glfwTerminateJoysticksWin32(void)
{
    int joy;

    for (joy = GLFW_JOYSTICK_1;  joy <= GLFW_JOYSTICK_LAST;  joy++)
        free(_glfw.win32_js[joy].name);
}

// Looks for new joysticks
//
void _glfwDetectJoystickConnectionWin32(void)
{
    DWORD i;

    if (!_glfw.win32.xinput.instance)
        return;

    for (i = 0;  i < XUSER_MAX_COUNT;  i++)
        openJoystickDevice(i);
}

// Checks if any current joystick has been disconnected
//
void _glfwDetectJoystickDisconnectionWin32(void)
{
    DWORD i;

    if (!_glfw.win32.xinput.instance)
        return;

    for (i = 0;  i < XUSER_MAX_COUNT;  i++)
        pollJoystickEvents(_glfw.win32_js + i, 0);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformJoystickPresent(int joy)
{
    _GLFWjoystickWin32* js = _glfw.win32_js + joy;
    return pollJoystickEvents(js, 0);
}

const float* _glfwPlatformGetJoystickAxes(int joy, int* count)
{
    _GLFWjoystickWin32* js = _glfw.win32_js + joy;
    if (!pollJoystickEvents(js, _GLFW_UPDATE_AXES))
        return NULL;

    *count = js->axisCount;
    return js->axes;
}

const unsigned char* _glfwPlatformGetJoystickButtons(int joy, int* count)
{
    _GLFWjoystickWin32* js = _glfw.win32_js + joy;
    if (!pollJoystickEvents(js, _GLFW_UPDATE_BUTTONS))
        return NULL;

    *count = js->buttonCount;
    return js->buttons;
}

const char* _glfwPlatformGetJoystickName(int joy)
{
    _GLFWjoystickWin32* js = _glfw.win32_js + joy;
    if (!pollJoystickEvents(js, 0))
        return NULL;

    return js->name;
}

