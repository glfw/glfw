//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
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

#define OLDSDK 1

#include <amigainput/amigainput.h>
#include <proto/amigainput.h>

#define MAX_JOYSTICKS       32

#define MAX_AXES            8
#define MAX_BUTTONS         16
#define MAX_HATS            8

#define BUFFER_OFFSET(buffer, offset)   (((int32 *)buffer)[offset])

struct joystick
{
    AIN_DeviceID     id;
    const char      *name;
};

/* Per-joystick data private to driver */
struct joystick_hwdata
{
    AIN_DeviceHandle   *handle;
    APTR                context;

    uint32              axisBufferOffset[MAX_AXES];
    int32               axisData[MAX_AXES];
    TEXT                axisName[MAX_AXES][32];

    uint32              buttonBufferOffset[MAX_BUTTONS];
    int32               buttonData[MAX_BUTTONS];

    uint32              hatBufferOffset[MAX_HATS];
    int32               hatData[MAX_HATS];
};

/* A handy container to encapsulate the information we
 * need when enumerating joysticks on the system.
 */
struct enumPacket
{
    APTR             context;
    uint32          *count;
    struct joystick *joyList;
};

GLFWbool _glfwInitJoysticksOS4(void);
void _glfwTerminateJoysticksOS4(void);
int _glfwPollJoystickOS4(_GLFWjoystick* js, int mode);
const char* _glfwGetMappingNameOS4(void);
void _glfwUpdateGamepadGUIDOS4(char* guid);

void _glfwDetectJoystickConnectionOS4(void);