//========================================================================
// GLFW 3.0 OS X - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2010 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <mach/mach.h>
#include <mach/mach_error.h>

#include <CoreFoundation/CoreFoundation.h>
#include <Kernel/IOKit/hidsystem/IOHIDUsageTables.h>


//------------------------------------------------------------------------
// Joystick element information
//------------------------------------------------------------------------
typedef struct
{
    IOHIDElementCookie cookie;

    long min;
    long max;

    long minReport;
    long maxReport;

} _GLFWjoyelement;


static void getElementsCFArrayHandler(const void* value, void* parameter);


// Adds an element to the specified joystick
//
static void addJoystickElement(_GLFWjoy* joystick, CFTypeRef elementRef)
{
    long elementType, usagePage, usage;
    CFMutableArrayRef elementsArray = NULL;

    CFNumberGetValue(CFDictionaryGetValue(elementRef, CFSTR(kIOHIDElementTypeKey)),
                     kCFNumberLongType, &elementType);
    CFNumberGetValue(CFDictionaryGetValue(elementRef, CFSTR(kIOHIDElementUsagePageKey)),
                     kCFNumberLongType, &usagePage);
    CFNumberGetValue(CFDictionaryGetValue(elementRef, CFSTR(kIOHIDElementUsageKey)),
                     kCFNumberLongType, &usage);

    if ((elementType == kIOHIDElementTypeInput_Axis) ||
        (elementType == kIOHIDElementTypeInput_Button) ||
        (elementType == kIOHIDElementTypeInput_Misc))
    {
        switch (usagePage)
        {
            case kHIDPage_GenericDesktop:
            {
                switch (usage)
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
                        elementsArray = joystick->axisElements;
                        break;
                    case kHIDUsage_GD_Hatswitch:
                        elementsArray = joystick->hatElements;
                        break;
                }

                break;
            }

            case kHIDPage_Button:
                elementsArray = joystick->buttonElements;
                break;
            default:
                break;
        }

        if (elementsArray)
        {
            long number;
            CFTypeRef numberRef;
            _GLFWjoyelement* element = calloc(1, sizeof(_GLFWjoyelement));

            CFArrayAppendValue(elementsArray, element);

            numberRef = CFDictionaryGetValue(elementRef, CFSTR(kIOHIDElementCookieKey));
            if (numberRef && CFNumberGetValue(numberRef, kCFNumberLongType, &number))
                element->cookie = (IOHIDElementCookie) number;

            numberRef = CFDictionaryGetValue(elementRef, CFSTR(kIOHIDElementMinKey));
            if (numberRef && CFNumberGetValue(numberRef, kCFNumberLongType, &number))
                element->minReport = element->min = number;

            numberRef = CFDictionaryGetValue(elementRef, CFSTR(kIOHIDElementMaxKey));
            if (numberRef && CFNumberGetValue(numberRef, kCFNumberLongType, &number))
                element->maxReport = element->max = number;
        }
    }
    else
    {
        CFTypeRef array = CFDictionaryGetValue(elementRef, CFSTR(kIOHIDElementKey));
        if (array)
        {
            if (CFGetTypeID(array) == CFArrayGetTypeID())
            {
                CFRange range = { 0, CFArrayGetCount(array) };
                CFArrayApplyFunction(array, range, getElementsCFArrayHandler, joystick);
            }
        }
    }
}

// Adds an element to the specified joystick
//
static void getElementsCFArrayHandler(const void* value, void* parameter)
{
    if (CFGetTypeID(value) == CFDictionaryGetTypeID())
        addJoystickElement((_GLFWjoy*) parameter, (CFTypeRef) value);
}

// Returns the value of the specified element of the specified joystick
//
static long getElementValue(_GLFWjoy* joystick, _GLFWjoyelement* element)
{
    IOReturn result = kIOReturnSuccess;
    IOHIDEventStruct hidEvent;
    hidEvent.value = 0;

    if (joystick && element && joystick->interface)
    {
        result = (*(joystick->interface))->getElementValue(joystick->interface,
                                                           element->cookie,
                                                           &hidEvent);
        if (kIOReturnSuccess == result)
        {
            // Record min and max for auto calibration
            if (hidEvent.value < element->minReport)
                element->minReport = hidEvent.value;
            if (hidEvent.value > element->maxReport)
                element->maxReport = hidEvent.value;
        }
    }

    // Auto user scale
    return (long) hidEvent.value;
}

// Removes the specified joystick
//
static void removeJoystick(_GLFWjoy* joystick)
{
    int i;

    if (!joystick->present)
        return;

    for (i = 0;  i < CFArrayGetCount(joystick->axisElements);  i++)
        free((void*) CFArrayGetValueAtIndex(joystick->axisElements, i));
    CFArrayRemoveAllValues(joystick->axisElements);

    for (i = 0;  i < CFArrayGetCount(joystick->buttonElements);  i++)
        free((void*) CFArrayGetValueAtIndex(joystick->buttonElements, i));
    CFArrayRemoveAllValues(joystick->buttonElements);

    for (i = 0;  i < CFArrayGetCount(joystick->hatElements);  i++)
        free((void*) CFArrayGetValueAtIndex(joystick->hatElements, i));
    CFArrayRemoveAllValues(joystick->hatElements);

    free(joystick->axes);
    free(joystick->buttons);

    (*(joystick->interface))->close(joystick->interface);
    (*(joystick->interface))->Release(joystick->interface);

    memset(joystick, 0, sizeof(_GLFWjoy));
}

// Callback for user-initiated joystick removal
//
static void removalCallback(void* target, IOReturn result, void* refcon, void* sender)
{
    removeJoystick((_GLFWjoy*) refcon);
}

// Polls for joystick events and updates GLFW state
//
static void pollJoystickEvents(void)
{
    int joy;

    for (joy = 0;  joy <= GLFW_JOYSTICK_LAST;  joy++)
    {
        CFIndex i;
        int buttonIndex = 0;
        _GLFWjoy* joystick = _glfw.ns.joysticks + joy;

        if (!joystick->present)
            continue;

        for (i = 0;  i < CFArrayGetCount(joystick->buttonElements);  i++)
        {
            _GLFWjoyelement* button =
                (_GLFWjoyelement*) CFArrayGetValueAtIndex(joystick->buttonElements, i);

            if (getElementValue(joystick, button))
                joystick->buttons[buttonIndex++] = GLFW_PRESS;
            else
                joystick->buttons[buttonIndex++] = GLFW_RELEASE;
        }

        for (i = 0;  i < CFArrayGetCount(joystick->axisElements);  i++)
        {
            _GLFWjoyelement* axis =
                (_GLFWjoyelement*) CFArrayGetValueAtIndex(joystick->axisElements, i);

            long value = getElementValue(joystick, axis);
            long readScale = axis->maxReport - axis->minReport;

            if (readScale == 0)
                joystick->axes[i] = value;
            else
                joystick->axes[i] = (2.f * (value - axis->minReport) / readScale) - 1.f;
        }

        for (i = 0;  i < CFArrayGetCount(joystick->hatElements);  i++)
        {
            _GLFWjoyelement* hat =
                (_GLFWjoyelement*) CFArrayGetValueAtIndex(joystick->hatElements, i);

            // Bit fields of button presses for each direction, including nil
            const int directions[9] = { 1, 3, 2, 6, 4, 12, 8, 9, 0 };

            long j, value = getElementValue(joystick, hat);
            if (value < 0 || value > 8)
                value = 8;

            for (j = 0;  j < 4;  j++)
            {
                if (directions[value] & (1 << j))
                    joystick->buttons[buttonIndex++] = GLFW_PRESS;
                else
                    joystick->buttons[buttonIndex++] = GLFW_RELEASE;
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize joystick interface
//
void _glfwInitJoysticks(void)
{
    int joy = 0;
    IOReturn result = kIOReturnSuccess;
    mach_port_t masterPort = 0;
    io_iterator_t objectIterator = 0;
    CFMutableDictionaryRef hidMatchDictionary = NULL;
    io_object_t ioHIDDeviceObject = 0;

    result = IOMasterPort(bootstrap_port, &masterPort);
    hidMatchDictionary = IOServiceMatching(kIOHIDDeviceKey);
    if (kIOReturnSuccess != result || !hidMatchDictionary)
    {
        if (hidMatchDictionary)
            CFRelease(hidMatchDictionary);

        return;
    }

    result = IOServiceGetMatchingServices(masterPort,
                                          hidMatchDictionary,
                                          &objectIterator);
    if (result != kIOReturnSuccess)
        return;

    if (!objectIterator)
    {
        // There are no joysticks
        return;
    }

    while ((ioHIDDeviceObject = IOIteratorNext(objectIterator)))
    {
        CFMutableDictionaryRef propsRef = NULL;
        CFTypeRef valueRef = NULL;
        kern_return_t result;

        IOCFPlugInInterface** ppPlugInInterface = NULL;
        HRESULT plugInResult = S_OK;
        SInt32 score = 0;

        long usagePage, usage;

        // Check device type
        result = IORegistryEntryCreateCFProperties(ioHIDDeviceObject,
                                                   &propsRef,
                                                   kCFAllocatorDefault,
                                                   kNilOptions);

        if (result != kIOReturnSuccess)
            continue;

        valueRef = CFDictionaryGetValue(propsRef, CFSTR(kIOHIDPrimaryUsagePageKey));
        if (valueRef)
        {
            CFNumberGetValue(valueRef, kCFNumberLongType, &usagePage);
            if (usagePage != kHIDPage_GenericDesktop)
            {
                // This device is not relevant to GLFW
                CFRelease(valueRef);
                continue;
            }

            CFRelease(valueRef);
        }

        valueRef = CFDictionaryGetValue(propsRef, CFSTR(kIOHIDPrimaryUsageKey));
        if (valueRef)
        {
            CFNumberGetValue(valueRef, kCFNumberLongType, &usage);

            if ((usage != kHIDUsage_GD_Joystick &&
                 usage != kHIDUsage_GD_GamePad &&
                 usage != kHIDUsage_GD_MultiAxisController))
            {
                // This device is not relevant to GLFW
                CFRelease(valueRef);
                continue;
            }

            CFRelease(valueRef);
        }

        _GLFWjoy* joystick = _glfw.ns.joysticks + joy;
        joystick->present = GL_TRUE;

        result = IOCreatePlugInInterfaceForService(ioHIDDeviceObject,
                                                   kIOHIDDeviceUserClientTypeID,
                                                   kIOCFPlugInInterfaceID,
                                                   &ppPlugInInterface,
                                                   &score);

        if (kIOReturnSuccess != result)
            return;

        plugInResult = (*ppPlugInInterface)->QueryInterface(
                            ppPlugInInterface,
                            CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID),
                            (void *) &(joystick->interface));

        if (plugInResult != S_OK)
            return;

        (*ppPlugInInterface)->Release(ppPlugInInterface);

        (*(joystick->interface))->open(joystick->interface, 0);
        (*(joystick->interface))->setRemovalCallback(joystick->interface,
                                                     removalCallback,
                                                     joystick,
                                                     joystick);

        // Get product string
        valueRef = CFDictionaryGetValue(propsRef, CFSTR(kIOHIDProductKey));
        if (valueRef)
        {
            CFStringGetCString(valueRef,
                               joystick->name,
                               sizeof(joystick->name),
                               kCFStringEncodingUTF8);
            CFRelease(valueRef);
        }

        joystick->axisElements = CFArrayCreateMutable(NULL, 0, NULL);
        joystick->buttonElements = CFArrayCreateMutable(NULL, 0, NULL);
        joystick->hatElements = CFArrayCreateMutable(NULL, 0, NULL);

        valueRef = CFDictionaryGetValue(propsRef, CFSTR(kIOHIDElementKey));
        if (CFGetTypeID(valueRef) == CFArrayGetTypeID())
        {
            CFRange range = { 0, CFArrayGetCount(valueRef) };
            CFArrayApplyFunction(valueRef,
                                 range,
                                 getElementsCFArrayHandler,
                                 (void*) joystick);
            CFRelease(valueRef);
        }

        joystick->axes = calloc(CFArrayGetCount(joystick->axisElements),
                                         sizeof(float));
        joystick->buttons = calloc(CFArrayGetCount(joystick->buttonElements) +
                                   CFArrayGetCount(joystick->hatElements) * 4, 1);

        joy++;
        if (joy > GLFW_JOYSTICK_LAST)
            break;
    }
}

// Close all opened joystick handles
//
void _glfwTerminateJoysticks(void)
{
    int i;

    for (i = 0;  i < GLFW_JOYSTICK_LAST + 1;  i++)
    {
        _GLFWjoy* joystick = &_glfw.ns.joysticks[i];
        removeJoystick(joystick);
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformJoystickPresent(int joy)
{
    pollJoystickEvents();

    return _glfw.ns.joysticks[joy].present;
}

const float* _glfwPlatformGetJoystickAxes(int joy, int* count)
{
    _GLFWjoy* joystick = _glfw.ns.joysticks + joy;

    pollJoystickEvents();

    if (!joystick->present)
        return NULL;

    *count = (int) CFArrayGetCount(joystick->axisElements);
    return joystick->axes;
}

const unsigned char* _glfwPlatformGetJoystickButtons(int joy, int* count)
{
    _GLFWjoy* joystick = _glfw.ns.joysticks + joy;

    pollJoystickEvents();

    if (!joystick->present)
        return NULL;

    *count = (int) CFArrayGetCount(joystick->buttonElements) +
             (int) CFArrayGetCount(joystick->hatElements) * 4;
    return joystick->buttons;
}

const char* _glfwPlatformGetJoystickName(int joy)
{
    pollJoystickEvents();

    return _glfw.ns.joysticks[joy].name;
}

