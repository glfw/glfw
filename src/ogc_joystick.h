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


#define GLFW_OGC_JOYSTICK_STATE         _GLFWjoystickOgc ogc;
#define GLFW_OGC_LIBRARY_JOYSTICK_STATE _GLFWlibraryJoystickOgc  ogcjs;

#define MAX_GC_JOYSTICKS 4
#ifdef __wii__
#define MAX_WIIMOTES 4
#else
#define MAX_WIIMOTES 0
#endif
#define MAX_JOYSTICKS (MAX_GC_JOYSTICKS + MAX_WIIMOTES)

// Ogc-specific joystick data
//
typedef struct _GLFWjoystickOgc
{
    int                     id;
    uint8_t                 expansion;
} _GLFWjoystickOgc;

typedef struct _GLFWlibraryJoystickOgc
{
    char                    joystickIndex[MAX_JOYSTICKS];
} _GLFWlibraryJoystickOgc;

#define JOYSTICK_INDEX_FROM_DEVICE(deviceId) \
    ((int)_glfw.ogcjs.joystickIndex[deviceId])
#define JOYSTICK_FROM_DEVICE(deviceId) \
    (&_glfw.joysticks[JOYSTICK_INDEX_FROM_DEVICE(deviceId)])

GLFWbool _glfwPollJoysticksOgc(void);

GLFWbool _glfwInitJoysticksOgc(void);
void _glfwTerminateJoysticksOgc(void);
GLFWbool _glfwPollJoystickOgc(_GLFWjoystick* js, int mode);
const char* _glfwGetMappingNameOgc(void);
void _glfwUpdateGamepadGUIDOgc(char* guid);

