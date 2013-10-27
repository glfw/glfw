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
    IOHIDElementRef elementRef;

    long min;
    long max;

    long minReport;
    long maxReport;

} _GLFWjoyelement;

static void CFSetApplierFunctionCopyToCFArray(const void *value, void *context)
{
    CFArrayAppendValue( ( CFMutableArrayRef ) context, value );
}



// Returns the value of the specified element of the specified joystick
//
static long getElementValue(_GLFWjoy* joystick, _GLFWjoyelement* element)
{
    if (joystick && element && joystick->device)
    {
        IOHIDValueRef valRef;
        IOReturn result = IOHIDDeviceGetValue(joystick->device,element->elementRef,&valRef);

        if (kIOReturnSuccess == result)
        {
            CFIndex value = IOHIDValueGetIntegerValue(valRef);
            // Record min and max for auto calibration
            if (value < element->minReport)
                element->minReport = value;
            if (value > element->maxReport)
                element->maxReport = value;
        return (long) value;
        }
    }
    return 0;
}

// Removes the specified joystick
//
static void removeJoystick(_GLFWjoy* joystick)
{
    int i;

    if (!joystick->present)
        return;

    for (i = 0;  i < CFArrayGetCount(joystick->axisElements);  i++)
    {
        _GLFWjoyelement* control = (_GLFWjoyelement*) CFArrayGetValueAtIndex(joystick->axisElements, i);
        CFRelease(control->elementRef);
        free(control);
    }
    CFArrayRemoveAllValues(joystick->axisElements);
    //CFRelease(joystick->axisElements);

    for (i = 0;  i < CFArrayGetCount(joystick->buttonElements);  i++)
    {
        _GLFWjoyelement* control = (_GLFWjoyelement*) CFArrayGetValueAtIndex(joystick->buttonElements, i);
        CFRelease(control->elementRef);
        free(control);
    }
    CFArrayRemoveAllValues(joystick->buttonElements);
    //CFRelease(joystick->buttonElements);

    for (i = 0;  i < CFArrayGetCount(joystick->hatElements);  i++)
    {
        _GLFWjoyelement* control = (_GLFWjoyelement*) CFArrayGetValueAtIndex(joystick->hatElements, i);
        CFRelease(control->elementRef);
        free(control);
    }
    CFArrayRemoveAllValues(joystick->hatElements);
    //CFRelease(joystick->hatElements);

    free(joystick->axes);
    free(joystick->buttons);


    memset(joystick, 0, sizeof(_GLFWjoy));
}

// Callback for user-initiated joystick removal
//
static void removalCallback(void *inContext,  IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef)
{
    int joy;

    for (joy = 0;  joy <= GLFW_JOYSTICK_LAST;  joy++)
    {
        _GLFWjoy* joystick = _glfw.ns.joysticks + joy;
        if(joystick->device == inIOHIDDeviceRef)
        {
            removeJoystick(joystick);
            return;
        }
    }
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

            long value = getElementValue(joystick, button);
            if(button->maxReport>1)
                joystick->buttons[buttonIndex++] = value;//analogue button
            else
                joystick->buttons[buttonIndex++] = value?GLFW_PRESS:GLFW_RELEASE;
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

static CFMutableDictionaryRef creates_osx_device_match(int usage)
{
    CFMutableDictionaryRef result = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );

    if ( !result )
        return NULL;

    int page = kHIDPage_GenericDesktop;
    CFNumberRef pageCFNumberRef = CFNumberCreate( NULL, kCFNumberIntType, &page);

    if ( !pageCFNumberRef )
        return NULL;

    CFDictionarySetValue( result, CFSTR( kIOHIDDeviceUsagePageKey ), pageCFNumberRef );
    CFRelease( pageCFNumberRef );

    CFNumberRef usageCFNumberRef = CFNumberCreate( NULL, kCFNumberIntType, &usage);
    if ( !usageCFNumberRef )
        return NULL;

    CFDictionarySetValue( result, CFSTR( kIOHIDDeviceUsageKey ), usageCFNumberRef );
    CFRelease( usageCFNumberRef );

    return result;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize joystick interface
//
void _glfwInitJoysticks(void)
{
    int joy = 0;

    _glfw.ns.ioHIDManagerRef = IOHIDManagerCreate( kCFAllocatorDefault, 0L );
    IOReturn tIOReturn = IOHIDManagerOpen( _glfw.ns.ioHIDManagerRef, 0L);
    if ( kIOReturnSuccess != tIOReturn )
        return;

    CFMutableArrayRef matching = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    {
        CFMutableDictionaryRef result = creates_osx_device_match(kHIDUsage_GD_Joystick);
        if (result)
        {
            CFArrayAppendValue( matching, result );
            CFRelease(result);
            result = creates_osx_device_match(kHIDUsage_GD_GamePad);
        }
        if (result)
        {
            CFArrayAppendValue( matching, result );
            CFRelease(result);
            result = creates_osx_device_match(kHIDUsage_GD_MultiAxisController);
        }
        if (result)
        {
            CFArrayAppendValue( matching, result );
            CFRelease(result);
        }
    }
    IOHIDManagerSetDeviceMatchingMultiple( _glfw.ns.ioHIDManagerRef, matching);
    CFRelease(matching);
    CFSetRef devset = IOHIDManagerCopyDevices( _glfw.ns.ioHIDManagerRef );
    if(!devset)
        return;

    CFMutableArrayRef devarray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    CFSetApplyFunction(devset, CFSetApplierFunctionCopyToCFArray, devarray);
    CFRelease( devset);
    CFIndex idx,countDevices = CFArrayGetCount(devarray);

    for (idx = 0; idx < countDevices; idx++)
    {
        IOHIDDeviceRef tDevice = (IOHIDDeviceRef) CFArrayGetValueAtIndex(devarray, idx);

        CFArrayRef elementCFArrayRef = IOHIDDeviceCopyMatchingElements(tDevice, NULL, 0);

        if (!elementCFArrayRef)
            continue;

        CFTypeRef valueRef = IOHIDDeviceGetProperty(tDevice, CFSTR( kIOHIDPrimaryUsageKey ));
        if (!valueRef)
        {
            CFRelease(elementCFArrayRef);
            continue;
        }
        long primaryUsage;
        CFNumberGetValue(valueRef, kCFNumberLongType, &primaryUsage);
        CFRelease(valueRef);

        if ((primaryUsage != kHIDUsage_GD_Joystick &&
             primaryUsage != kHIDUsage_GD_GamePad &&
             primaryUsage != kHIDUsage_GD_MultiAxisController))
        {
            // This device is not relevant to GLFW
            CFRelease(elementCFArrayRef);
            continue;
        }

        _GLFWjoy* joystick = _glfw.ns.joysticks + joy;
        joystick->present = GL_TRUE;

        CFStringRef strRef = IOHIDDeviceGetProperty(tDevice, CFSTR( kIOHIDProductKey ));
        CFStringGetCString(strRef, joystick->name, sizeof(joystick->name), kCFStringEncodingUTF8);
        CFRelease(strRef);

        joystick->device = tDevice;
        joystick->axisElements = CFArrayCreateMutable(NULL, 0, NULL);
        joystick->buttonElements = CFArrayCreateMutable(NULL, 0, NULL);
        joystick->hatElements = CFArrayCreateMutable(NULL, 0, NULL);

        CFIndex eleidx, elecnt = CFArrayGetCount(elementCFArrayRef);
        for (eleidx=0; eleidx<elecnt; eleidx++)
        {
            IOHIDElementRef tIOHIDElementRef = (IOHIDElementRef)CFArrayGetValueAtIndex(elementCFArrayRef, eleidx);
            int elementType = IOHIDElementGetType(tIOHIDElementRef);
            int usagePage = IOHIDElementGetUsagePage(tIOHIDElementRef);
            int usage = IOHIDElementGetUsage(tIOHIDElementRef);

            if ((elementType != kIOHIDElementTypeInput_Axis) &&
                (elementType != kIOHIDElementTypeInput_Button) &&
                (elementType != kIOHIDElementTypeInput_Misc))
                continue;

            CFMutableArrayRef elementsArray = NULL;
            elementsArray = joystick->buttonElements;
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
            }

            if(!elementsArray)
                continue;

            _GLFWjoyelement* control = calloc(1, sizeof(_GLFWjoyelement));

            CFArrayAppendValue(elementsArray, control);

            control->elementRef = tIOHIDElementRef;
            CFRetain(control->elementRef);

            control->max = control->maxReport = IOHIDElementGetLogicalMax(tIOHIDElementRef);
            control->min = control->minReport = IOHIDElementGetLogicalMin(tIOHIDElementRef);

        }
        CFRelease(elementCFArrayRef);

        joystick->axes = calloc(CFArrayGetCount(joystick->axisElements),
                                         sizeof(float));
        joystick->buttons = calloc(CFArrayGetCount(joystick->buttonElements) +
                                   CFArrayGetCount(joystick->hatElements) * 4, 1);

        joy++;
        if (joy > GLFW_JOYSTICK_LAST)
            break;

    }
    CFRelease(devarray);

    IOHIDManagerRegisterDeviceRemovalCallback(_glfw.ns.ioHIDManagerRef, &removalCallback, NULL);

    IOHIDManagerScheduleWithRunLoop(_glfw.ns.ioHIDManagerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);


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
    IOHIDManagerClose(_glfw.ns.ioHIDManagerRef, kIOHIDOptionsTypeNone);
    CFRelease(_glfw.ns.ioHIDManagerRef);
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
