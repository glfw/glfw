//========================================================================
// GLFW 3.3 Cocoa - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2016 Camilla Löwy <elmindreda@glfw.org>
// Copyright (c) 2012 Torsten Walluhn <tw@mad-cad.net>
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

#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include <mach/mach.h>
#include <mach/mach_error.h>

#include <CoreFoundation/CoreFoundation.h>
#include <Kernel/IOKit/hidsystem/IOHIDUsageTables.h>


// Joystick element information
//
typedef struct _GLFWjoyelementNS
{
    IOHIDElementRef native;
    long            minimum;
    long            maximum;

} _GLFWjoyelementNS;


// Returns the value of the specified element of the specified joystick
//
static long getElementValue(_GLFWjoystick* js, _GLFWjoyelementNS* element)
{
    IOReturn result = kIOReturnSuccess;
    IOHIDValueRef valueRef;
    long value = 0;

    if (js && element && js->ns.device)
    {
        result = IOHIDDeviceGetValue(js->ns.device,
                                     element->native,
                                     &valueRef);

        if (kIOReturnSuccess == result)
        {
            value = IOHIDValueGetIntegerValue(valueRef);

            // Record min and max for auto calibration
            if (value < element->minimum)
                element->minimum = value;
            if (value > element->maximum)
                element->maximum = value;
        }
    }

    // Auto user scale
    return value;
}

// Removes the specified joystick
//
static void closeJoystick(_GLFWjoystick* js)
{
    int i;

    if (!js->present)
        return;

    for (i = 0;  i < CFArrayGetCount(js->ns.axes);  i++)
        free((void*) CFArrayGetValueAtIndex(js->ns.axes, i));
    CFRelease(js->ns.axes);

    for (i = 0;  i < CFArrayGetCount(js->ns.buttons);  i++)
        free((void*) CFArrayGetValueAtIndex(js->ns.buttons, i));
    CFRelease(js->ns.buttons);

    for (i = 0;  i < CFArrayGetCount(js->ns.hats);  i++)
        free((void*) CFArrayGetValueAtIndex(js->ns.hats, i));
    CFRelease(js->ns.hats);

    _glfwFreeJoystick(js);
    _glfwInputJoystick(_GLFW_JOYSTICK_ID(js), GLFW_DISCONNECTED);
}

// Callback for user-initiated joystick addition
//
static void matchCallback(void* context,
                          IOReturn result,
                          void* sender,
                          IOHIDDeviceRef device)
{
    int jid;
    char name[256];
    CFIndex i;
    CFStringRef productKey;
    _GLFWjoystick* js;
    CFMutableArrayRef axes, buttons, hats;

    for (jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++)
    {
        if (_glfw.joysticks[jid].ns.device == device)
            return;
    }

    axes    = CFArrayCreateMutable(NULL, 0, NULL);
    buttons = CFArrayCreateMutable(NULL, 0, NULL);
    hats    = CFArrayCreateMutable(NULL, 0, NULL);

    productKey = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
    if (productKey)
    {
        CFStringGetCString(productKey,
                           name,
                           sizeof(name),
                           kCFStringEncodingUTF8);
    }
    else
        strncpy(name, "Unknown", sizeof(name));

    CFArrayRef elements =
        IOHIDDeviceCopyMatchingElements(device, NULL, kIOHIDOptionsTypeNone);

    for (i = 0;  i < CFArrayGetCount(elements);  i++)
    {
        IOHIDElementRef native = (IOHIDElementRef) CFArrayGetValueAtIndex(elements, i);
        if (CFGetTypeID(native) != IOHIDElementGetTypeID())
            continue;

        const IOHIDElementType type = IOHIDElementGetType(native);
        if ((type != kIOHIDElementTypeInput_Axis) &&
            (type != kIOHIDElementTypeInput_Button) &&
            (type != kIOHIDElementTypeInput_Misc))
        {
            continue;
        }

        CFMutableArrayRef target = NULL;

        switch (IOHIDElementGetUsagePage(native))
        {
            case kHIDPage_GenericDesktop:
            {
                switch (IOHIDElementGetUsage(native))
                {
                    case kHIDUsage_GD_X:
                    case kHIDUsage_GD_Y:
                    case kHIDUsage_GD_Z:
                    case kHIDUsage_GD_Rx:
                    case kHIDUsage_GD_Ry:
                    case kHIDUsage_GD_Rz:
                    case kHIDUsage_GD_Slider:
                    case kHIDUsage_GD_Dial:
                    case kHIDUsage_GD_Wheel:
                        target = axes;
                        break;
                    case kHIDUsage_GD_Hatswitch:
                        target = hats;
                        break;
                }

                break;
            }

            case kHIDPage_Button:
                target = buttons;
                break;
            default:
                break;
        }

        if (target)
        {
            _GLFWjoyelementNS* element = calloc(1, sizeof(_GLFWjoyelementNS));
            element->native  = native;
            element->minimum = IOHIDElementGetLogicalMin(native);
            element->maximum = IOHIDElementGetLogicalMax(native);
            CFArrayAppendValue(target, element);
        }
    }

    CFRelease(elements);

    js = _glfwAllocJoystick(name,
                            CFArrayGetCount(axes),
                            CFArrayGetCount(buttons) + CFArrayGetCount(hats) * 4);

    js->ns.device  = device;
    js->ns.axes    = axes;
    js->ns.buttons = buttons;
    js->ns.hats    = hats;

    _glfwInputJoystick(_GLFW_JOYSTICK_ID(js), GLFW_CONNECTED);
}

// Callback for user-initiated joystick removal
//
static void removeCallback(void* context,
                           IOReturn result,
                           void* sender,
                           IOHIDDeviceRef device)
{
    int jid;

    for (jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++)
    {
        if (_glfw.joysticks[jid].ns.device == device)
        {
            closeJoystick(_glfw.joysticks + jid);
            break;
        }
    }
}

// Creates a dictionary to match against devices with the specified usage page
// and usage
//
static CFMutableDictionaryRef createMatchingDictionary(long usagePage,
                                                       long usage)
{
    CFMutableDictionaryRef result =
        CFDictionaryCreateMutable(kCFAllocatorDefault,
                                  0,
                                  &kCFTypeDictionaryKeyCallBacks,
                                  &kCFTypeDictionaryValueCallBacks);

    if (result)
    {
        CFNumberRef pageRef = CFNumberCreate(kCFAllocatorDefault,
                                             kCFNumberLongType,
                                             &usagePage);
        if (pageRef)
        {
            CFDictionarySetValue(result,
                                 CFSTR(kIOHIDDeviceUsagePageKey),
                                 pageRef);
            CFRelease(pageRef);

            CFNumberRef usageRef = CFNumberCreate(kCFAllocatorDefault,
                                                  kCFNumberLongType,
                                                  &usage);
            if (usageRef)
            {
                CFDictionarySetValue(result,
                                     CFSTR(kIOHIDDeviceUsageKey),
                                     usageRef);
                CFRelease(usageRef);
            }
        }
    }

    return result;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize joystick interface
//
void _glfwInitJoysticksNS(void)
{
    CFMutableArrayRef matchingCFArrayRef;

    _glfw.ns.hidManager = IOHIDManagerCreate(kCFAllocatorDefault,
                                             kIOHIDOptionsTypeNone);

    matchingCFArrayRef = CFArrayCreateMutable(kCFAllocatorDefault,
                                              0,
                                              &kCFTypeArrayCallBacks);
    if (matchingCFArrayRef)
    {
        CFDictionaryRef matchingCFDictRef =
            createMatchingDictionary(kHIDPage_GenericDesktop,
                                     kHIDUsage_GD_Joystick);
        if (matchingCFDictRef)
        {
            CFArrayAppendValue(matchingCFArrayRef, matchingCFDictRef);
            CFRelease(matchingCFDictRef);
        }

        matchingCFDictRef = createMatchingDictionary(kHIDPage_GenericDesktop,
                                                     kHIDUsage_GD_GamePad);
        if (matchingCFDictRef)
        {
            CFArrayAppendValue(matchingCFArrayRef, matchingCFDictRef);
            CFRelease(matchingCFDictRef);
        }

        matchingCFDictRef =
            createMatchingDictionary(kHIDPage_GenericDesktop,
                                     kHIDUsage_GD_MultiAxisController);
        if (matchingCFDictRef)
        {
            CFArrayAppendValue(matchingCFArrayRef, matchingCFDictRef);
            CFRelease(matchingCFDictRef);
        }

        IOHIDManagerSetDeviceMatchingMultiple(_glfw.ns.hidManager,
                                              matchingCFArrayRef);
        CFRelease(matchingCFArrayRef);
    }

    IOHIDManagerRegisterDeviceMatchingCallback(_glfw.ns.hidManager,
                                               &matchCallback, NULL);
    IOHIDManagerRegisterDeviceRemovalCallback(_glfw.ns.hidManager,
                                              &removeCallback, NULL);

    IOHIDManagerScheduleWithRunLoop(_glfw.ns.hidManager,
                                    CFRunLoopGetMain(),
                                    kCFRunLoopDefaultMode);

    IOHIDManagerOpen(_glfw.ns.hidManager, kIOHIDOptionsTypeNone);

    // Execute the run loop once in order to register any initially-attached
    // joysticks
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
}

// Close all opened joystick handles
//
void _glfwTerminateJoysticksNS(void)
{
    int jid;

    for (jid = 0;  jid <= GLFW_JOYSTICK_LAST;  jid++)
        closeJoystick(_glfw.joysticks + jid);

    CFRelease(_glfw.ns.hidManager);
    _glfw.ns.hidManager = NULL;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformPollJoystick(int jid, int mode)
{
    _GLFWjoystick* js = _glfw.joysticks + jid;

    if (mode == _GLFW_POLL_AXES)
    {
        CFIndex i;

        for (i = 0;  i < CFArrayGetCount(js->ns.axes);  i++)
        {
            _GLFWjoyelementNS* axis = (_GLFWjoyelementNS*)
                CFArrayGetValueAtIndex(js->ns.axes, i);

            const long value = getElementValue(js, axis);
            const long delta = axis->maximum - axis->minimum;

            if (delta == 0)
                _glfwInputJoystickAxis(jid, i, value);
            else
                _glfwInputJoystickAxis(jid, i, (2.f * (value - axis->minimum) / delta) - 1.f);
        }
    }
    else if (mode == _GLFW_POLL_BUTTONS)
    {
        CFIndex i, bi = 0;

        for (i = 0;  i < CFArrayGetCount(js->ns.buttons);  i++)
        {
            _GLFWjoyelementNS* button = (_GLFWjoyelementNS*)
                CFArrayGetValueAtIndex(js->ns.buttons, i);
            const char value = getElementValue(js, button) ? 1 : 0;
            _glfwInputJoystickButton(jid, bi++, value);
        }

        for (i = 0;  i < CFArrayGetCount(js->ns.hats);  i++)
        {
            _GLFWjoyelementNS* hat = (_GLFWjoyelementNS*)
                CFArrayGetValueAtIndex(js->ns.hats, i);

            // Bit fields of button presses for each direction, including nil
            const int directions[9] = { 1, 3, 2, 6, 4, 12, 8, 9, 0 };

            long j, state = getElementValue(js, hat);
            if (state < 0 || state > 8)
                state = 8;

            for (j = 0;  j < 4;  j++)
            {
                const char value = directions[state] & (1 << j) ? 1 : 0;
                _glfwInputJoystickButton(jid, bi++, value);
            }
        }
    }

    return js->present;
}

