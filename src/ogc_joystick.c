//========================================================================
// GLFW 3.5 OGC - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2024 Alberto Mardegan <info@mardy.it>
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

#if defined(_GLFW_OGC)

#include <ogc/lwp_watchdog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __wii__
#include <wiiuse/wpad.h>
#endif

#define MAX_FAILED_GC_READS 10

#define MAX_GC_AXES    6
#define MAX_GC_BUTTONS 8
#define MAX_GC_HATS    1

/* PAD_ScanPads() returns 0 if no joystick is connected, but also if querying
 * the hardware failed for whatever reason. This can occasionally happen at
 * runtime; so, we want to ignore these events and only report the joystick
 * disconnection if PAD_ScanPads() consistently returns 0.
 *
 * This function returns true if the scanPads value is reliable.
 */
static GLFWbool readGCJoysticks(u32 *scanPads)
{
    static GLFWbool lastReadFailed = GLFW_FALSE;
    static u64 firstDisconnectedTimestamp = 0;
    u32 currScanPads;
    u64 timestamp;

    currScanPads = PAD_ScanPads();
    if (currScanPads == 0) {
        timestamp = gettime();
        if (lastReadFailed) {
            if (ticks_to_millisecs(timestamp - firstDisconnectedTimestamp) < 500)
                return GLFW_FALSE;
        } else {
            firstDisconnectedTimestamp = timestamp;
            lastReadFailed = GLFW_TRUE;
            return GLFW_FALSE;
        }
    } else {
        lastReadFailed = GLFW_FALSE;
    }

    *scanPads = currScanPads;
    return GLFW_TRUE;
}

static void readGCJoystick(_GLFWjoystick* js)
{
    u32 id = js->ogc.id;
    u16 btns = PAD_ButtonsHeld(id);
    js->buttons[0] = btns & PAD_BUTTON_A ? GLFW_PRESS : GLFW_RELEASE;
    js->buttons[1] = btns & PAD_BUTTON_B ? GLFW_PRESS : GLFW_RELEASE;
    js->buttons[2] = btns & PAD_BUTTON_X ? GLFW_PRESS : GLFW_RELEASE;
    js->buttons[3] = btns & PAD_BUTTON_Y ? GLFW_PRESS : GLFW_RELEASE;
    js->buttons[4] = btns & PAD_TRIGGER_L ? GLFW_PRESS : GLFW_RELEASE;
    js->buttons[5] = btns & PAD_TRIGGER_R ? GLFW_PRESS : GLFW_RELEASE;
    js->buttons[6] = btns & PAD_TRIGGER_Z ? GLFW_PRESS : GLFW_RELEASE;
    js->buttons[7] = btns & PAD_BUTTON_START ? GLFW_PRESS : GLFW_RELEASE;
    js->axes[0] = PAD_StickX(id) / 100.0;
    js->axes[1] = PAD_StickY(id) / 100.0;
    js->axes[2] = PAD_SubStickX(id) / 100.0;
    js->axes[3] = PAD_SubStickY(id) / 100.0;
    js->axes[4] = PAD_TriggerL(id) / 200.0;
    js->axes[5] = PAD_TriggerR(id) / 200.0;
    unsigned char hat = 0;
    if (btns & PAD_BUTTON_UP) hat |= GLFW_HAT_UP;
    if (btns & PAD_BUTTON_DOWN) hat |= GLFW_HAT_DOWN;
    if (btns & PAD_BUTTON_LEFT) hat |= GLFW_HAT_LEFT;
    if (btns & PAD_BUTTON_RIGHT) hat |= GLFW_HAT_RIGHT;
    _glfwInputJoystickHat(js, 0, hat);
}

/* CRC algorithm used by SDL */
static uint16_t crc16_for_byte(uint8_t r)
{
    uint16_t crc = 0;
    int i;
    for (i = 0; i < 8; ++i) {
        crc = ((crc ^ r) & 1 ? 0xA001 : 0) ^ crc >> 1;
        r >>= 1;
    }
    return crc;
}

uint16_t SDL_crc16(uint16_t crc, const void *data, size_t len)
{
    /* As an optimization we can precalculate a 256 entry table for each byte */
    size_t i;
    for (i = 0; i < len; ++i) {
        crc = crc16_for_byte((uint8_t)crc ^ ((const uint8_t *)data)[i]) ^ crc >> 8;
    }
    return crc;
}

static _GLFWjoystick* addJoystick(int deviceId,
                                  const char *name, const char* guid,
                                  int axisCount, int buttonCount, int hatCount)
{
    fprintf(stderr, "Add controller %d %s\n", deviceId, name);
    _GLFWjoystick* js =
        _glfwAllocJoystick(name, guid,
                           axisCount, buttonCount, hatCount);
    js->ogc.id = deviceId;
    _glfw.ogcjs.joystickIndex[deviceId] = js - _glfw.joysticks;
    _glfwInputJoystick(js, GLFW_CONNECTED);
    return js;
}

static void addGCJoystick(int deviceId)
{
    char name[16], guid[33];
    uint16_t nameCrc;

    sprintf(name, "Gamecube %d", deviceId);
    nameCrc = SDL_crc16(0, name, strlen(name));
    sprintf(guid, "0000%02x%02x7e050000000%d000001000000",
            nameCrc & 0xff, nameCrc >> 8, deviceId + 1);
    addJoystick(deviceId, name, guid,
                MAX_GC_AXES, MAX_GC_BUTTONS, MAX_GC_HATS);
}

#ifdef __wii__
static inline float readAxis(uint8_t pos, uint8_t center, uint8_t min, uint8_t max)
{
    if (pos < center) {
        return (pos - center) / (float)(center - min);
    } else {
        return (pos - center) / (float)(max - center);
    }
}

static void readJoystickAxes(_GLFWjoystick* js, int index, const joystick_t *data)
{
    js->axes[index] = readAxis(data->pos.x, data->center.x, data->min.x, data->max.x);
    js->axes[index + 1] = readAxis(data->pos.y, data->center.y, data->min.y, data->max.y);
}

static inline float clampUnity(float value)
{
    if (value < -1.0f) return -1.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

static void readOrientationAxes(_GLFWjoystick* js, int index, const orient_t *data)
{
    /* Orientation fields range from -180 to 180 (they are measured in
     * degrees), but in order to use them as joystick axes it's more reasonable
     * to limit their range to -90/90. */
    js->axes[index] = clampUnity(-data->pitch / 90.0f);
    js->axes[index + 1] = clampUnity(data->roll / 90.0f);
    js->axes[index + 2] = clampUnity(data->yaw / 90.0f);
}

static void readWiimote(_GLFWjoystick* js)
{
    u32 id = js->ogc.id - MAX_GC_JOYSTICKS;
    WPADData *data = WPAD_Data(id);
    u32 btns = data->btns_h | data->btns_d;
    unsigned char hat = 0;

    GLFWbool readWiimote = GLFW_FALSE;
    GLFWbool wiimotePointer = GLFW_FALSE;
    int wiimoteFirstAxis = 0;
    if (js->ogc.expansion == EXP_NUNCHUK) {
        js->buttons[7] = btns & WPAD_NUNCHUK_BUTTON_Z ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[8] = btns & WPAD_NUNCHUK_BUTTON_C ? GLFW_PRESS : GLFW_RELEASE;
        /* When a nunchuk is connected, we report its joystick as the first two axes,
         * followed by three axes from the wiimote orientation, followed by
         * three axes from the nunchuk orientation. */
        readJoystickAxes(js, 0, &data->exp.nunchuk.js);
        readWiimote = GLFW_TRUE;
        wiimotePointer = GLFW_TRUE;
        wiimoteFirstAxis = 2;
        readOrientationAxes(js, wiimoteFirstAxis + 3, &data->exp.nunchuk.orient);
    } else if (js->ogc.expansion == EXP_CLASSIC) {
        if (btns & WPAD_CLASSIC_BUTTON_LEFT) hat |= GLFW_HAT_LEFT;
        if (btns & WPAD_CLASSIC_BUTTON_RIGHT) hat |= GLFW_HAT_RIGHT;
        if (btns & WPAD_CLASSIC_BUTTON_UP) hat |= GLFW_HAT_UP;
        if (btns & WPAD_CLASSIC_BUTTON_DOWN) hat |= GLFW_HAT_DOWN;
        js->buttons[0] = btns & WPAD_CLASSIC_BUTTON_A ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[1] = btns & WPAD_CLASSIC_BUTTON_B ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[2] = btns & WPAD_CLASSIC_BUTTON_X ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[3] = btns & WPAD_CLASSIC_BUTTON_Y ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[4] = btns & WPAD_CLASSIC_BUTTON_FULL_L ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[5] = btns & WPAD_CLASSIC_BUTTON_FULL_R ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[6] = btns & WPAD_CLASSIC_BUTTON_ZL ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[7] = btns & WPAD_CLASSIC_BUTTON_ZR ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[8] = btns & WPAD_CLASSIC_BUTTON_MINUS ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[9] = btns & WPAD_CLASSIC_BUTTON_PLUS ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[10] = btns & WPAD_CLASSIC_BUTTON_HOME ? GLFW_PRESS : GLFW_RELEASE;
        readJoystickAxes(js, 0, &data->exp.classic.ljs);
        readJoystickAxes(js, 2, &data->exp.classic.rjs);
    } else if (js->ogc.expansion == EXP_NONE) {
        readWiimote = GLFW_TRUE;
        wiimotePointer = data->ir.valid;
    }

    if (readWiimote) {
        if (wiimotePointer) {
            if (btns & WPAD_BUTTON_LEFT) hat |= GLFW_HAT_LEFT;
            if (btns & WPAD_BUTTON_RIGHT) hat |= GLFW_HAT_RIGHT;
            if (btns & WPAD_BUTTON_UP) hat |= GLFW_HAT_UP;
            if (btns & WPAD_BUTTON_DOWN) hat |= GLFW_HAT_DOWN;
        } else {
            if (btns & WPAD_BUTTON_LEFT) hat |= GLFW_HAT_DOWN;
            if (btns & WPAD_BUTTON_RIGHT) hat |= GLFW_HAT_UP;
            if (btns & WPAD_BUTTON_UP) hat |= GLFW_HAT_LEFT;
            if (btns & WPAD_BUTTON_DOWN) hat |= GLFW_HAT_RIGHT;
        }

        js->buttons[0] = btns & WPAD_BUTTON_1 ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[1] = btns & WPAD_BUTTON_2 ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[2] = btns & WPAD_BUTTON_A ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[3] = btns & WPAD_BUTTON_B ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[4] = btns & WPAD_BUTTON_MINUS ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[5] = btns & WPAD_BUTTON_PLUS ? GLFW_PRESS : GLFW_RELEASE;
        js->buttons[6] = btns & WPAD_BUTTON_HOME ? GLFW_PRESS : GLFW_RELEASE;
        readOrientationAxes(js, wiimoteFirstAxis, &data->orient);
    }

    _glfwInputJoystickHat(js, 0, hat);

    /* On the Wii, let's close the window when the HOME button is pressed,
     * since that's the default behaviour for homebrew applications. The close
     * event can anyway be overridden in the application, if this behaviour is
     * not desired. */
    if (btns & WPAD_BUTTON_HOME && _glfw.windowListHead) {
        _glfwInputWindowCloseRequest(_glfw.windowListHead);
    }
}

static void addWiimote(int deviceId, uint8_t expansion)
{
    char name[64], guid[40];
    uint16_t nameCrc;
    int axisCount, buttonCount, hatCount;

    /* These counters are for the bare Wiimote */
    axisCount = 3;
    buttonCount = 7;
    hatCount = 1;
    char *name_ptr = name;
    name_ptr += sprintf(name_ptr, "Wiimote %d", deviceId - MAX_GC_JOYSTICKS);
    switch (expansion) {
        case WPAD_EXP_NUNCHUK:
            strcpy(name_ptr, " + Nunchuk");
            axisCount += 5;
            buttonCount += 2;
            break;
        case WPAD_EXP_CLASSIC:
            strcpy(name_ptr, " + Classic");
            axisCount = 4;
            buttonCount = 11;
            hatCount = 1;
            break;
        case WPAD_EXP_GUITARHERO3:
            strcpy(name_ptr, " + Guitar Hero 3");
            break;
        case WPAD_EXP_WIIBOARD:
            strcpy(name_ptr, " + Balance board");
            break;
    }

    nameCrc = SDL_crc16(0, name, strlen(name));
    sprintf(guid, "0500%02x%02x7e0500000%d0%d000001000000",
            nameCrc & 0xff, nameCrc >> 8, expansion + 1, deviceId + 1);
    _GLFWjoystick *js = addJoystick(deviceId, name, guid,
                                    axisCount, buttonCount, hatCount);
    js->ogc.expansion = expansion;
}
#endif /* __wii__ */

static void removeJoystick(int deviceId)
{
    _GLFWjoystick* js = JOYSTICK_FROM_DEVICE(deviceId);
    _glfwInputJoystick(js, GLFW_DISCONNECTED);
    _glfwFreeJoystick(js);
    _glfw.ogcjs.joystickIndex[deviceId] = -1;
    fprintf(stderr, "Removed controller %d\n", deviceId);
}

static GLFWbool updateJoysticks()
{
    u32 scanPads = 0;

    if (_glfw.joysticksInitialized && readGCJoysticks(&scanPads)) {
        for (int i = 0; i < MAX_GC_JOYSTICKS; i++) {
            GLFWbool connected = !!(scanPads & (1 << i));
            int joystickIndex = JOYSTICK_INDEX_FROM_DEVICE(i);
            GLFWbool wasConnected = joystickIndex >= 0 ?
                _glfw.joysticks[joystickIndex].connected : GLFW_FALSE;
            if (connected != wasConnected) {
                if (connected) {
                    addGCJoystick(i);
                    joystickIndex = JOYSTICK_INDEX_FROM_DEVICE(i);
                } else {
                    removeJoystick(i);
                }
            }
            if (connected)
                readGCJoystick(&_glfw.joysticks[joystickIndex]);
        }
    }

#ifdef __wii__
    /* The WPAD data is also used for the mouse, so we read it even if
     * joysticks haven't been initialized */
    WPAD_ReadPending(WPAD_CHAN_ALL, NULL);
    if (_glfw.joysticksInitialized) {
        for (int i = 0; i < MAX_WIIMOTES; i++) {
            int deviceId = i + MAX_GC_JOYSTICKS;
            int joystickIndex = JOYSTICK_INDEX_FROM_DEVICE(deviceId);
            WPADData *data = WPAD_Data(i);
            /* Ignore all reads where an error occurred */
            if (data->err != WPAD_ERR_NONE) continue;
            GLFWbool connected = data->data_present != 0;
            uint8_t expansion = data->data_present & WPAD_DATA_EXPANSION ?
                data->exp.type : EXP_NONE;
            GLFWbool wasConnected = joystickIndex >= 0 ?
                _glfw.joysticks[joystickIndex].connected : GLFW_FALSE;
            uint8_t hadExpansion = joystickIndex >= 0 ?
                _glfw.joysticks[joystickIndex].ogc.expansion : EXP_NONE;
            if (connected != wasConnected || expansion != hadExpansion) {
                if (wasConnected) {
                    removeJoystick(deviceId);
                }
                if (connected) {
                    addWiimote(deviceId, expansion);
                    joystickIndex = JOYSTICK_INDEX_FROM_DEVICE(deviceId);
                }
            }
            if (connected)
                readWiimote(&_glfw.joysticks[joystickIndex]);
        }
    }
#endif
    return GLFW_TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwPollJoysticksOgc(void)
{
    return updateJoysticks();
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwInitJoysticksOgc(void)
{
    /* Hardware initialization done in ogc_init.c, since we want it to happen
     * ASAP */
    for (int i = 0; i < MAX_JOYSTICKS; i++) {
        _glfw.ogcjs.joystickIndex[i] = -1;
    }
    return GLFW_TRUE;
}

void _glfwTerminateJoysticksOgc(void)
{
    // TODO
}

GLFWbool _glfwPollJoystickOgc(_GLFWjoystick* js, int mode)
{
    updateJoysticks();
    return js->connected;
}

const char* _glfwGetMappingNameOgc(void)
{
    return "Ogc";
}

void _glfwUpdateGamepadGUIDOgc(char* guid)
{
}

#endif // _GLFW_OGC

