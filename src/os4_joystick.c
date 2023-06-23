//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016-2017 Camilla Löwy <elmindreda@glfw.org>
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

#include "internal.h"

#include <string.h>
#include <stdio.h>

static struct Library   *GLFW_AIN_Base;
static struct AIN_IFace *GLFW_IAIN;

/*
 * Convert AmigaInput hat data to GLFW hat data.
 */
static inline int
AMIGAINPUT_MapHatData(int hat_data)
{
    switch (hat_data) {
        case 1:  return GLFW_HAT_UP;
        case 2:  return GLFW_HAT_RIGHT_UP;
        case 3:  return GLFW_HAT_RIGHT;
        case 4:  return GLFW_HAT_RIGHT_DOWN;
        case 5:  return GLFW_HAT_DOWN;
        case 6:  return GLFW_HAT_LEFT_DOWN;
        case 7:  return GLFW_HAT_LEFT;
        case 8:  return GLFW_HAT_LEFT_UP;
        default: return GLFW_HAT_CENTERED;
    }
}

static void
AMIGAINPUT_Close(_GLFWjoystick * joystick)
{
    dprintf("Closing joystick #%d (AI ID=%ld)\n", joystick->os4js.instance_id, joystick->os4js.joystickList[joystick->os4js.instance_id].id);
    if (joystick->os4js.hwdata) {
#if OLDSDK
        GLFW_IAIN->AIN_ReleaseDevice(joystick->os4js.hwdata->context, joystick->os4js.hwdata->handle);
#else
        GLFW_IAIN->ReleaseDevice(joystick->os4js.hwdata->context, joystick->os4js.hwdata->handle);
#endif

        _glfw_free(joystick->os4js.hwdata);
        joystick->os4js.hwdata = NULL;
    }
}

/* Function to open a joystick for use.
   The joystick to open is specified by the index field of the joystick.
   This should fill the buttonCount and axisCount fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
static int
AMIGAINPUT_Open(_GLFWjoystick * joysticks, int device_index)
{
    _GLFWjoystick * joystick = joysticks + device_index;
    AIN_DeviceHandle *handle;
    AIN_DeviceID id = joystick->os4js.joystickList[joystick->os4js.instance_id].id;

#if OLDSDK
    handle = GLFW_IAIN->AIN_ObtainDevice(_glfw.os4js.joystickContext, id);
#else
    handle = GLFW_IAIN->ObtainDevice(_glfw.os4js.joystickContext, id);
#endif

    printf("Opening joystick #%ld (AI ID=%ld)\n", joystick->os4js.instance_id, id);

    if (handle) {
        joystick->os4js.hwdata = _glfw_calloc(1, sizeof(struct joystick_hwdata));

        if (joystick->os4js.hwdata) {
            struct joystick_hwdata *hwdata      = joystick->os4js.hwdata;
            unsigned int            num_axes    = 0;
            unsigned int            num_buttons = 0;
            unsigned int            num_hats    = 0;
            TEXT tmpstr[32];
            uint32 tmpoffset;

            int i;
            BOOL result = TRUE;

            joystick->os4js.hwdata->handle  = handle;
            joystick->os4js.hwdata->context = _glfw.os4js.joystickContext;

            strncpy(joystick->name, joystick->os4js.joystickList[joystick->os4js.instance_id].name, sizeof(joystick->name) - 1);

            /* Query number of axes, buttons and hats the device has */
#if OLDSDK
            result = result && GLFW_IAIN->AIN_Query(joystick->os4js.hwdata->context, id, AINQ_NUMAXES,    0, &num_axes, 4);
            result = result && GLFW_IAIN->AIN_Query(joystick->os4js.hwdata->context, id, AINQ_NUMBUTTONS, 0, &num_buttons, 4);
            result = result && GLFW_IAIN->AIN_Query(joystick->os4js.hwdata->context, id, AINQ_NUMHATS,    0, &num_hats, 4);
#else
            result = result && GLFW_IAIN->Query(joystick->os4js.hwdata->context, id, AINQ_NUMAXES,    0, &num_axes, 4);
            result = result && GLFW_IAIN->Query(joystick->os4js.hwdata->context, id, AINQ_NUMBUTTONS, 0, &num_buttons, 4);
            result = result && GLFW_IAIN->Query(joystick->os4js.hwdata->context, id, AINQ_NUMHATS,    0, &num_hats, 4);
#endif

            printf ("Found %d axes, %d buttons, %d hats\n", num_axes, num_buttons, num_hats);

            joystick->axisCount   = num_axes < MAX_AXES       ? num_axes    : MAX_AXES;
            joystick->buttonCount = num_buttons < MAX_BUTTONS ? num_buttons : MAX_BUTTONS;
            joystick->hatCount    = num_hats < MAX_HATS       ? num_hats    : MAX_HATS;

            // Ensure all axis names are null terminated
            for (i = 0; i < MAX_AXES; i++)
                joystick->os4js.hwdata->axisName[i][0] = 0;

            /* Query offsets in ReadDevice buffer for axes' data */
            for (i = 0; i < joystick->axisCount; i++) {
#if OLDSDK
                result = result && GLFW_IAIN->AIN_Query(joystick->os4js.hwdata->context, id, AINQ_AXIS_OFFSET, i, &(hwdata->axisBufferOffset[i]), 4);
                result = result && GLFW_IAIN->AIN_Query(joystick->os4js.hwdata->context, id, AINQ_AXISNAME,    i, &(hwdata->axisName[i][0]), 32 );
#else
                result = result && GLFW_IAIN->Query(joystick->os4js.hwdata->context, id, AINQ_AXIS_OFFSET, i, &(hwdata->axisBufferOffset[i]), 4);
                result = result && GLFW_IAIN->Query(joystick->os4js.hwdata->context, id, AINQ_AXISNAME,    i, &(hwdata->axisName[i][0]), 32 );
#endif
            }

            // Sort the axes so that X and Y come first
            for (i = 0; i < joystick->axisCount; i++) {
                if ( ( strcasecmp( &joystick->os4js.hwdata->axisName[i][0], "X-Axis" ) == 0 ) && ( i != 0 ) ) {
                    // Back up the zero position axis data
                    tmpoffset = joystick->os4js.hwdata->axisBufferOffset[0];
                    strlcpy( tmpstr, joystick->os4js.hwdata->axisName[0], 32 );

                    // Move this one to zero
                    joystick->os4js.hwdata->axisBufferOffset[0] = joystick->os4js.hwdata->axisBufferOffset[i];
                    strlcpy( joystick->os4js.hwdata->axisName[0], joystick->os4js.hwdata->axisName[i], 32 );

                    // Put the old 0 here
                    joystick->os4js.hwdata->axisBufferOffset[i] = tmpoffset;
                    strlcpy( joystick->os4js.hwdata->axisName[i], tmpstr, 32 );

                    continue;
                }

                if ( ( strcasecmp( &joystick->os4js.hwdata->axisName[i][0], "Y-Axis" ) == 0 ) && ( i != 1 ) ) {
                    // Back up the position 1 axis data
                    tmpoffset = joystick->os4js.hwdata->axisBufferOffset[1];
                    strlcpy( tmpstr, joystick->os4js.hwdata->axisName[1], 32 );

                    // Move this one to position 1
                    joystick->os4js.hwdata->axisBufferOffset[1] = joystick->os4js.hwdata->axisBufferOffset[i];
                    strlcpy( joystick->os4js.hwdata->axisName[1], joystick->os4js.hwdata->axisName[i], 32 );

                    // Put the old 1 here
                    joystick->os4js.hwdata->axisBufferOffset[i] = tmpoffset;
                    strlcpy( joystick->os4js.hwdata->axisName[i], tmpstr, 32 );

                    continue;
                }
            }

            /* Query offsets in ReadDevice buffer for buttons' data */
            for (i = 0; i < joystick->buttonCount; i++) {
#if OLDSDK
                result = result && GLFW_IAIN->AIN_Query(joystick->os4js.hwdata->context, id, AINQ_BUTTON_OFFSET, i, &(joystick->os4js.hwdata->buttonBufferOffset[i]), 4);
#else
                result = result && GLFW_IAIN->Query(joystick->os4js.hwdata->context, id, AINQ_BUTTON_OFFSET, i, &(joystick->os4js.hwdata->buttonBufferOffset[i]), 4);
#endif
            }

            /* Query offsets in ReadDevice buffer for hats' data */
            for (i = 0; i < joystick->hatCount; i++) {
#if OLDSDK
                result = result && GLFW_IAIN->AIN_Query(joystick->os4js.hwdata->context, id, AINQ_HAT_OFFSET, i, &(hwdata->hatBufferOffset[i]), 4);
#else
                result = result && GLFW_IAIN->Query(joystick->os4js.hwdata->context, id, AINQ_HAT_OFFSET, i, &(hwdata->hatBufferOffset[i]), 4);
#endif
            }

            if (result) {
                char guid[33];
                // Generate a joystick GUID that matches the SDL 2.0.5+ one
                sprintf(guid, "78696e707574%02x000000000000000000", handle->DeviceID & 0xff);                
                _glfwAllocJoystick(joystick->name, guid, joystick->axisCount, joystick->buttonCount, joystick->hatCount);

                printf("Successful\n");
                return 0;
            }
        }

#if OLDSDK
        GLFW_IAIN->AIN_ReleaseDevice (_glfw.os4js.joystickContext, handle);
#else
        GLFW_IAIN->ReleaseDevice (_glfw.os4js.joystickContext, handle);
#endif
    }

    printf("Failed\n");

    return GLFW_FALSE;
}

/*
 * Callback to enumerate joysticks
 */
static BOOL
AMIGAINPUT_EnumerateJoysticks(AIN_Device *device, void *UserData)
{
    APTR             context =  ((struct enumPacket *)UserData)->context;
    uint32          *count   =  ((struct enumPacket *)UserData)->count;
    struct joystick *joy     = &((struct enumPacket *)UserData)->joyList[*count];

    BOOL result = FALSE;

    if (*count < MAX_JOYSTICKS) {
        printf("ENUMJOY: id=%ld, type=%ld, axes=%ld, buttons=%ld\n",
            *count,
            (int32)device->Type,
            (int32)device->NumAxes,
            (int32)device->NumButtons);

        if (device->Type == AINDT_JOYSTICK) {
            /* AmigaInput can report devices even when there's no
             * physical stick present. We take some steps to try and
             * ignore such bogus devices.
             *
             * First, check whether we have a useful number of axes and buttons
             */
            if ((device->NumAxes > 0) && (device->NumButtons > 0)) {
                /* Then, check whether we can actually obtain the device
                 */
#if OLDSDK
                AIN_DeviceHandle *handle = GLFW_IAIN->AIN_ObtainDevice (context, device->DeviceID);
#else
                AIN_DeviceHandle *handle = GLFW_IAIN->ObtainDevice (context, device->DeviceID);
#endif

                if (handle) {
                    /* Okay. This appears to be a valid device. We'll report it to GLFW.
                     */
                    joy->id   = device->DeviceID;
                    joy->name = _glfw_strdup(device->DeviceName);

                    printf("Found joystick #%d (AI ID=%ld) '%s'\n", *count, joy->id, joy->name);

                    (*count)++;

#if OLDSDK
                    GLFW_IAIN->AIN_ReleaseDevice (context, handle);
#else
                    GLFW_IAIN->ReleaseDevice (context, handle);
#endif

                    result = TRUE;
                }
                else
                    printf("Failed to obtain joystick '%s' (AI ID=%ld) - ignoring.\n", device->DeviceName, device->DeviceID);
            }
            else
                printf("Joystick '%s' (AI ID=%ld) has no axes/buttons - ignoring.\n", device->DeviceName, device->DeviceID);
        }
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwInitJoysticksOS4(void)
{
    printf("_glfwInitJoysticksOS4\n");
    GLFW_AIN_Base = IExec->OpenLibrary("AmigaInput.library", 51);

    if (GLFW_AIN_Base) {
        GLFW_IAIN = (struct AIN_IFace *) IExec->GetInterface(GLFW_AIN_Base, "main", 1, NULL);

        if (!GLFW_IAIN) {
            IExec->CloseLibrary(GLFW_AIN_Base);
            return GLFW_FALSE;
        }
#if OLDSDK
        _glfw.os4js.joystickContext = GLFW_IAIN->AIN_CreateContext(1, NULL);
#else
        _glfw.os4js.joystickContext = GLFW_IAIN->CreateContext(1, NULL);
#endif
        if (_glfw.os4js.joystickContext) {
            struct enumPacket packet = {
                 _glfw.os4js.joystickContext,
                &_glfw.os4js.joystickCount,
                &_glfw.os4js.joystickList[0]
            };

#if OLDSDK
            BOOL result = GLFW_IAIN->AIN_EnumDevices(_glfw.os4js.joystickContext, AMIGAINPUT_EnumerateJoysticks, &packet);
#else
            BOOL result = GLFW_IAIN->EnumDevices(_glfw.os4js.joystickContext, AMIGAINPUT_EnumerateJoysticks, &packet);
#endif
            printf("EnumDevices returned %d\n", result);
            printf("Found %d joysticks\n", _glfw.os4js.joystickCount);

            if (result) {
                /*

                NOTE: AI doesn't seem to handle hotplugged/removed joysticks very well.
                Report only devices detected at startup to GLFW.

                */
                int i;

                for (i = 0; i < _glfw.os4js.joystickCount; i++) {
                    printf("Add joystick %d\n", i);
                    AMIGAINPUT_Open(_glfw.joysticks, i);
                }
            }
            return GLFW_TRUE;
        }
    } else {
        printf("Failed to open AmigaInput.library\n");
    }

    return GLFW_FALSE;
}

void _glfwTerminateJoysticksOS4(void)
{
    printf("_glfwTerminateJoysticksOS4\n");
    uint32 i;
    
    for (i = 0; i < _glfw.os4js.joystickCount; i++) {
        printf("_glfw_free joystickList[i].name\n");
        AMIGAINPUT_Close(_glfw.joysticks + i);
        _glfw_free((char *)_glfw.os4js.joystickList[i].name);
    }

    _glfw.os4js.joystickCount = 0;

    if (_glfw.os4js.joystickContext) {
        printf("AIN_DeleteContext\n");
#if OLDSDK
        GLFW_IAIN->AIN_DeleteContext(_glfw.os4js.joystickContext);
#else
        GLFW_IAIN->DeleteContext(_glfw.os4js.joystickContext);
#endif
        _glfw.os4js.joystickContext = NULL;
    }

    IExec->DropInterface((void *) GLFW_IAIN);
    IExec->CloseLibrary(GLFW_AIN_Base);
}

int _glfwPollJoystickOS4(_GLFWjoystick* joystick, int mode)
{   
    struct joystick_hwdata *hwdata = joystick->os4js.hwdata;
    void                   *buffer;

    //printf("Called %p\n", hwdata);

    /*
     * Poll device for data
     */
#if OLDSDK
    if (hwdata && GLFW_IAIN->AIN_ReadDevice(hwdata->context, hwdata->handle, &buffer))
#else
    if (hwdata && GLFW_IAIN->ReadDevice(hwdata->context, hwdata->handle, &buffer))
#endif
    {
        int i;

        /* Extract axis data from buffer and notify GLFW of any changes
         * in axis state
         */
        for (i = 0; i < joystick->axisCount; i++) {
            int axisdata = BUFFER_OFFSET(buffer, hwdata->axisBufferOffset[i]);

            /* Clamp axis data to 16-bits to work around possible AI driver bugs */
            if (axisdata > 32767)  axisdata =  32767;
            if (axisdata < -32768) axisdata = -32768;

            if (axisdata != hwdata->axisData[i]) {
                _glfwInputJoystickAxis(joystick, i, (float) axisdata);
                hwdata->axisData[i] = axisdata;
            }
        }

        /* Extract button data from buffer and notify GLFW of any changes
         * in button state
         */
        for (i = 0; i < joystick->buttonCount; i++) {
            int buttondata = BUFFER_OFFSET(buffer, hwdata->buttonBufferOffset[i]);

            if (buttondata != hwdata->buttonData[i]) {
                _glfwInputJoystickButton(joystick, i, buttondata ? GLFW_PRESS : GLFW_RELEASE);
                hwdata->buttonData[i] = buttondata;
            }
        }

        /* Extract hat data from buffer and notify GLFW of any changes
         * in hat state
         */
        for (i = 0; i < joystick->hatCount; i++) {
            int hatdata = BUFFER_OFFSET(buffer, hwdata->hatBufferOffset[i]);

            if (hatdata != hwdata->hatData[i]) {
                _glfwInputJoystickHat(joystick, i, AMIGAINPUT_MapHatData(hatdata));
                hwdata->hatData[i] = hatdata;
            }
        }
        return GLFW_TRUE;
    }
    return GLFW_FALSE;
}

const char* _glfwGetMappingNameOS4(void)
{
    return "AmigaOS4";
}

void _glfwUpdateGamepadGUIDOS4(char* guid)
{
}

