//========================================================================
// GLFW 3.1 XInput - www.glfw.org
//------------------------------------------------------------------------
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

#include <stdlib.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

static const DWORD ORDERED_XINPUT_BUTTONS[XINPUT_BUTTONS_COUNT] = {
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
};

static float normalizeThumb(SHORT pos, SHORT deadzone)
{
    const SHORT THUMB_MIN = -32768;
    const SHORT THUMB_MAX =  32767;

    if (abs(pos) < deadzone)
        return 0.f;

    return 2.f * ((float)(pos - THUMB_MIN) / (float)(THUMB_MAX - THUMB_MIN)) - 1.f;
}

static float normalizeTrigger(BYTE pos)
{
    return (float)pos / 255.f;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize joystick interface
//
void _glfwInitJoysticks(void)
{
}

// Close all opened joystick handles
//
void _glfwTerminateJoysticks(void)
{
    int i;

    for (i = 0;  i < GLFW_JOYSTICK_LAST;  i++)
        free(_glfw.xinput_js[i].name);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformJoystickPresent(int joy)
{
    XINPUT_CAPABILITIES capabilities;
    return _glfw_XInputGetCapabilities(joy, XINPUT_FLAG_GAMEPAD, &capabilities) == ERROR_SUCCESS;
}

const float* _glfwPlatformGetJoystickAxes(int joy, int* count)
{
    XINPUT_STATE state;
    XINPUT_GAMEPAD gamepad;
    float* axes = _glfw.xinput_js[joy].axes;

    ZeroMemory(&state, sizeof(XINPUT_STATE));
    if (_glfw_XInputGetState(joy, &state) != ERROR_SUCCESS)
        return NULL;

    gamepad = state.Gamepad;

    axes[(*count)++] = normalizeTrigger(gamepad.bLeftTrigger);
    axes[(*count)++] = normalizeTrigger(gamepad.bRightTrigger);
    axes[(*count)++] = normalizeThumb(gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    axes[(*count)++] = normalizeThumb(gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    axes[(*count)++] = normalizeThumb(gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    axes[(*count)++] = normalizeThumb(gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

    return axes;
}

const unsigned char* _glfwPlatformGetJoystickButtons(int joy, int* count)
{
    XINPUT_STATE state;
    unsigned char* buttons = _glfw.xinput_js[joy].buttons;

    ZeroMemory(&state, sizeof(XINPUT_STATE));
    if (_glfw_XInputGetState(joy, &state) != ERROR_SUCCESS)
        return NULL;

    *count = XINPUT_BUTTONS_COUNT;
    for (int i = 0; i < XINPUT_BUTTONS_COUNT; i++)
        buttons[i] = (unsigned char)((state.Gamepad.wButtons & ORDERED_XINPUT_BUTTONS[i]) ? GLFW_PRESS : GLFW_RELEASE);

     return buttons;
}

const char* _glfwPlatformGetJoystickName(int joy)
{
    char name[64];
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    if (!_glfwPlatformJoystickPresent(joy))
        return NULL;

    sprintf_s(name, sizeof(name), "XInput Joystick %d", joy + 1);
    free(_glfw.xinput_js[joy].name);
    _glfw.xinput_js[joy].name = strdup(name);
    return _glfw.xinput_js[joy].name;
}
