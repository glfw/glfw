//========================================================================
// GLFW - An OpenGL library
// Platform:    Cocoa/NSOpenGL
// API Version: 3.0
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2009-2010 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <Kernel/IOKit/hidsystem/IOHIDUsageTables.h>


//------------------------------------------------------------------------
// Joystick element information
//------------------------------------------------------------------------

typedef struct
{
    IOHIDElementCookie cookie;

    long value;

    long min;
    long max;

    long minReport;
    long maxReport;

} _glfwJoystickElement;


//------------------------------------------------------------------------
// Joystick information & state
//------------------------------------------------------------------------

typedef struct
{
    int present;
    char product[256];

    IOHIDDeviceInterface** interface;

    int numAxes;
    int numButtons;
    int numHats;

    CFMutableArrayRef axes;
    CFMutableArrayRef buttons;
    CFMutableArrayRef hats;

} _glfwJoystick;

static _glfwJoystick _glfwJoysticks[GLFW_JOYSTICK_LAST + 1];


void GetElementsCFArrayHandler(const void* value, void* parameter);


//========================================================================
// Adds an element to the specified joystick
//========================================================================

static void addJoystickElement(_glfwJoystick* joystick, CFTypeRef refElement)
{
    long elementType, usagePage, usage;
    CFTypeRef refElementType, refUsagePage, refUsage;

    refElementType = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementTypeKey));
    refUsagePage = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementUsagePageKey));
    refUsage = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementUsageKey));

    CFMutableArrayRef elementsArray = NULL;

    CFNumberGetValue(refElementType, kCFNumberLongType, &elementType);
    CFNumberGetValue(refUsagePage, kCFNumberLongType, &usagePage);
    CFNumberGetValue(refUsage, kCFNumberLongType, &usage);

    if ((elementType == kIOHIDElementTypeInput_Axis) ||
        (elementType == kIOHIDElementTypeInput_Button) ||
        (elementType == kIOHIDElementTypeInput_Misc))
    {
        switch (usagePage) /* only interested in kHIDPage_GenericDesktop and kHIDPage_Button */
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
                        joystick->numAxes++;
                        elementsArray = joystick->axes;
                        break;
                    case kHIDUsage_GD_Hatswitch:
                        joystick->numHats++;
                        elementsArray = joystick->hats;
                        break;
                }

                break;
            }

            case kHIDPage_Button:
                joystick->numButtons++;
                elementsArray = joystick->buttons;
                break;
            default:
                break;
        }

        if (elementsArray)
        {
            long number;
            CFTypeRef refType;

            _glfwJoystickElement* element = (_glfwJoystickElement*) malloc(sizeof(_glfwJoystickElement));

            CFArrayAppendValue(elementsArray, element);

            refType = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementCookieKey));
            if (refType && CFNumberGetValue(refType, kCFNumberLongType, &number))
                element->cookie = (IOHIDElementCookie) number;

            refType = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementMinKey));
            if (refType && CFNumberGetValue(refType, kCFNumberLongType, &number))
                element->minReport = element->min = number;

            refType = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementMaxKey));
            if (refType && CFNumberGetValue(refType, kCFNumberLongType, &number))
                element->maxReport = element->max = number;
        }
    }
    else
    {
        CFTypeRef refElementTop = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementKey));
        if (refElementTop)
        {
            CFTypeID type = CFGetTypeID (refElementTop);
            if (type == CFArrayGetTypeID()) /* if element is an array */
            {
                CFRange range = {0, CFArrayGetCount (refElementTop)};
                CFArrayApplyFunction(refElementTop, range, GetElementsCFArrayHandler, joystick);
            }
        }
    }
}


//========================================================================
// Adds an element to the specified joystick
//========================================================================

void GetElementsCFArrayHandler(const void* value, void* parameter)
{
    if (CFGetTypeID(value) == CFDictionaryGetTypeID())
        addJoystickElement((_glfwJoystick*) parameter, (CFTypeRef) value);
}


//========================================================================
// Returns the value of the specified element of the specified joystick
//========================================================================

static long getElementValue(_glfwJoystick* joystick, _glfwJoystickElement* element)
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
            /* record min and max for auto calibration */
            if (hidEvent.value < element->minReport)
                element->minReport = hidEvent.value;
            if (hidEvent.value > element->maxReport)
                element->maxReport = hidEvent.value;
        }
    }

    /* auto user scale */
    return (long) hidEvent.value;
}


//========================================================================
// Removes the specified joystick
//========================================================================

static void removeJoystick(_glfwJoystick* joystick)
{
    int i;

    if (joystick->present)
    {
        joystick->present = GL_FALSE;

        for (i = 0;  i < joystick->numAxes;  i++)
        {
            _glfwJoystickElement* axes =
                (_glfwJoystickElement*) CFArrayGetValueAtIndex(joystick->axes, i);
            free(axes);
        }
        CFArrayRemoveAllValues(joystick->axes);
        joystick->numAxes = 0;

        for (i = 0;  i < joystick->numButtons;  i++)
        {
            _glfwJoystickElement* button =
                (_glfwJoystickElement*) CFArrayGetValueAtIndex(joystick->buttons, i);
            free(button);
        }
        CFArrayRemoveAllValues(joystick->buttons);
        joystick->numButtons = 0;

        for (i = 0;  i < joystick->numHats;  i++)
        {
            _glfwJoystickElement* hat =
                (_glfwJoystickElement*) CFArrayGetValueAtIndex(joystick->hats, i);
            free(hat);
        }
        CFArrayRemoveAllValues(joystick->hats);
        joystick->hats = 0;

        (*(joystick->interface))->close(joystick->interface);
        (*(joystick->interface))->Release(joystick->interface);

        joystick->interface = NULL;
    }
}


//========================================================================
// Callback for user-initiated joystick removal
//========================================================================

static void removalCallback(void* target, IOReturn result, void* refcon, void* sender)
{
    removeJoystick((_glfwJoystick*) refcon);
}


//========================================================================
// Polls for joystick events and updates GFLW state
//========================================================================

static void pollJoystickEvents(void)
{
    int i;
    CFIndex j;

    for (i = 0;  i < GLFW_JOYSTICK_LAST + 1;  i++)
    {
        _glfwJoystick* joystick = &_glfwJoysticks[i];

        if (joystick->present)
        {
            for (j = 0;  j < joystick->numButtons;  j++)
            {
                _glfwJoystickElement* button =
                    (_glfwJoystickElement*) CFArrayGetValueAtIndex(joystick->buttons, j);
                button->value = getElementValue(joystick, button);
            }

            for (j = 0;  j < joystick->numAxes;  j++)
            {
                _glfwJoystickElement* axes =
                    (_glfwJoystickElement*) CFArrayGetValueAtIndex(joystick->axes, j);
                axes->value = getElementValue(joystick, axes);
            }

            for (j = 0;  j < joystick->numHats;  j++)
            {
                _glfwJoystickElement* hat =
                    (_glfwJoystickElement*) CFArrayGetValueAtIndex(joystick->hats, j);
                hat->value = getElementValue(joystick, hat);
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Initialize joystick interface
//========================================================================

void _glfwInitJoysticks(void)
{
    int deviceCounter = 0;
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

    if (!objectIterator) /* there are no joysticks */
        return;

    while ((ioHIDDeviceObject = IOIteratorNext(objectIterator)))
    {
        CFMutableDictionaryRef hidProperties = 0;
        kern_return_t result;
        CFTypeRef refCF = 0;

        IOCFPlugInInterface** ppPlugInInterface = NULL;
        HRESULT plugInResult = S_OK;
        SInt32 score = 0;

        long usagePage, usage;

        result = IORegistryEntryCreateCFProperties(ioHIDDeviceObject,
                                                   &hidProperties,
                                                   kCFAllocatorDefault,
                                                   kNilOptions);

        if (result != kIOReturnSuccess)
            continue;

        /* Check device type */
        refCF = CFDictionaryGetValue(hidProperties, CFSTR(kIOHIDPrimaryUsagePageKey));
        if (refCF)
        {
            CFNumberGetValue(refCF, kCFNumberLongType, &usagePage);
            if (usagePage != kHIDPage_GenericDesktop)
            {
                /* We are not interested in this device */
                continue;
            }
        }

        refCF = CFDictionaryGetValue(hidProperties, CFSTR(kIOHIDPrimaryUsageKey));
        if (refCF)
        {
            CFNumberGetValue(refCF, kCFNumberLongType, &usage);
            
            if ((usage != kHIDUsage_GD_Joystick &&
                 usage != kHIDUsage_GD_GamePad &&
                 usage != kHIDUsage_GD_MultiAxisController))
            {
                /* We are not interested in this device */
                continue;
            }
        }

        _glfwJoystick* joystick = &_glfwJoysticks[deviceCounter];

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

        /* Get product string */
        refCF = CFDictionaryGetValue(hidProperties, CFSTR(kIOHIDProductKey));
        if (refCF)
        {
            CFStringGetCString(refCF,
                               (char*) &(joystick->product),
                               256,
                               CFStringGetSystemEncoding());
        }

        joystick->numAxes = 0;
        joystick->numButtons = 0;
        joystick->numHats = 0;
        joystick->axes = CFArrayCreateMutable(NULL, 0, NULL);
        joystick->buttons = CFArrayCreateMutable(NULL, 0, NULL);
        joystick->hats = CFArrayCreateMutable(NULL, 0, NULL);

        CFTypeRef refTopElement = CFDictionaryGetValue(hidProperties,
                                                       CFSTR(kIOHIDElementKey));
        CFTypeID type = CFGetTypeID(refTopElement);
        if (type == CFArrayGetTypeID())
        {
            CFRange range = { 0, CFArrayGetCount(refTopElement) };
            CFArrayApplyFunction(refTopElement,
                                 range,
                                 GetElementsCFArrayHandler,
                                 (void*) joystick);
        }

        deviceCounter++;
    }
}


//========================================================================
// Close all opened joystick handles
//========================================================================

void _glfwTerminateJoysticks(void)
{
    int i;

    for (i = 0;  i < GLFW_JOYSTICK_LAST + 1;  i++)
    {
        _glfwJoystick* joystick = &_glfwJoysticks[i];
        removeJoystick(joystick);
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Determine joystick capabilities
//========================================================================

int _glfwPlatformGetJoystickParam(int joy, int param)
{
    if (!_glfwJoysticks[joy].present)
    {
        // TODO: Figure out if this is an error
        return GL_FALSE;
    }

    switch (param)
    {
        case GLFW_PRESENT:
            return GL_TRUE;

        case GLFW_AXES:
            return (int) CFArrayGetCount(_glfwJoysticks[joy].axes);

        case GLFW_BUTTONS:
            return (int) CFArrayGetCount(_glfwJoysticks[joy].buttons) + ((int) CFArrayGetCount(_glfwJoysticks[joy].hats)) * 4;

        default:
            break;
    }

    return GL_FALSE;
}


//========================================================================
// Get joystick axis positions
//========================================================================

int _glfwPlatformGetJoystickPos(int joy, float* pos, int numaxes)
{
    int i;

    if (joy < GLFW_JOYSTICK_1 || joy > GLFW_JOYSTICK_LAST)
        return 0;

    _glfwJoystick joystick = _glfwJoysticks[joy];

    if (!joystick.present)
    {
        // TODO: Figure out if this is an error
        return 0;
    }

    numaxes = numaxes < joystick.numAxes ? numaxes : joystick.numAxes;

    // Update joystick state
    pollJoystickEvents();

    for (i = 0;  i < numaxes;  i++)
    {
        _glfwJoystickElement* axes =
            (_glfwJoystickElement*) CFArrayGetValueAtIndex(joystick.axes, i);

        long readScale = axes->maxReport - axes->minReport;

        if (readScale == 0)
            pos[i] = axes->value;
        else
            pos[i] = (2.0f * (axes->value - axes->minReport) / readScale) - 1.0f;

        //printf("%ld, %ld, %ld\n", axes->value, axes->minReport, axes->maxReport);

        if (i & 1)
            pos[i] = -pos[i];
    }

    return numaxes;
}


//========================================================================
// Get joystick button states
//========================================================================

int _glfwPlatformGetJoystickButtons(int joy, unsigned char* buttons,
                                    int numbuttons)
{
    int i, j, button;

    if (joy < GLFW_JOYSTICK_1 || joy > GLFW_JOYSTICK_LAST)
        return 0;

    _glfwJoystick joystick = _glfwJoysticks[joy];

    if (!joystick.present)
    {
        // TODO: Figure out if this is an error
        return 0;
    }

    // Update joystick state
    pollJoystickEvents();

    for (button = 0;  button < numbuttons && button < joystick.numButtons;  button++)
    {
        _glfwJoystickElement* element = (_glfwJoystickElement*) CFArrayGetValueAtIndex(joystick.buttons, button);
        buttons[button] = element->value ? GLFW_PRESS : GLFW_RELEASE;
    }

    // Virtual buttons - Inject data from hats
    // Each hat is exposed as 4 buttons which exposes 8 directions with concurrent button presses

    const int directions[9] = { 1, 3, 2, 6, 4, 12, 8, 9, 0 }; // Bit fields of button presses for each direction, including nil

    for (i = 0;  i < joystick.numHats;  i++)
    {
        _glfwJoystickElement* hat = (_glfwJoystickElement*) CFArrayGetValueAtIndex(joystick.hats, i);
        int value = hat->value;
        if (value < 0 || value > 8) value = 8;

        for (j = 0; j < 4 && button < numbuttons; j++)
        {
            buttons[button++] = directions[value] & (1 << j) ? GLFW_PRESS : GLFW_RELEASE;
        }
    }

    return button;
}
