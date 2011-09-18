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
#include <sysexits.h>
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

typedef struct {
    IOHIDElementCookie Cookie;
    
    long Value;
    
    long Min;
    long Max;
    
    long MinReport;
    long MaxReport;
} joystick_element_t;


//------------------------------------------------------------------------
// Joystick information & state
//------------------------------------------------------------------------

typedef struct {
    int Present;
    char Product[256];
    
    IOHIDDeviceInterface ** Interface;
    
    int NumAxes;
    int NumButtons;
    int NumHats;
    
    CFMutableArrayRef Axes;
	CFMutableArrayRef Buttons;
	CFMutableArrayRef Hats;
} joystick_t;

joystick_t _glfwJoysticks[GLFW_JOYSTICK_LAST + 1];


void GetElementsCFArrayHandler(const void * value, void * parameter);

static void JoystickAddElemet(joystick_t * joystick, CFTypeRef refElement)
{
    long elementType, usagePage, usage;
    CFTypeRef refElementType, refUsagePage, refUsage;
    
    refElementType = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementTypeKey));
	refUsagePage   = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementUsagePageKey));
	refUsage       = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementUsageKey));
    
    CFMutableArrayRef elementsArray = NULL;
    
    CFNumberGetValue(refElementType, kCFNumberLongType, &elementType);
    CFNumberGetValue(refUsagePage, kCFNumberLongType, &usagePage);
    CFNumberGetValue(refUsage, kCFNumberLongType, &usage);
    
    if ((elementType == kIOHIDElementTypeInput_Axis)   ||
        (elementType == kIOHIDElementTypeInput_Button) ||
        (elementType == kIOHIDElementTypeInput_Misc))
    {
        switch (usagePage) /* only interested in kHIDPage_GenericDesktop and kHIDPage_Button */
        {
            case kHIDPage_GenericDesktop:
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
                        joystick->NumAxes++;
                        elementsArray = joystick->Axes;
                        break;
                    case kHIDUsage_GD_Hatswitch:
                        joystick->NumHats++;
                        elementsArray = joystick->Hats;
                        break;
                }
                break;
            case kHIDPage_Button:
                joystick->NumButtons++;
                elementsArray = joystick->Buttons;
                break;
            default:
                break;
        }
        
        if (elementsArray)
        {
            long number;
            CFTypeRef refType;
            
            joystick_element_t * element = (joystick_element_t *) malloc(sizeof(joystick_element_t));
            
            CFArrayAppendValue(elementsArray, element);
            
            refType = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementCookieKey));
            if (refType && CFNumberGetValue(refType, kCFNumberLongType, &number))
                element->Cookie = (IOHIDElementCookie) number;
            
            refType = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementMinKey));
            if (refType && CFNumberGetValue(refType, kCFNumberLongType, &number))
                element->MinReport = element->Min = number;
            
            refType = CFDictionaryGetValue(refElement, CFSTR(kIOHIDElementMaxKey));
            if (refType && CFNumberGetValue(refType, kCFNumberLongType, &number))
                element->MaxReport = element->Max = number;
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

void GetElementsCFArrayHandler(const void * value, void * parameter)
{
	if (CFGetTypeID(value) == CFDictionaryGetTypeID())
		JoystickAddElemet((joystick_t *) parameter, (CFTypeRef) value);
}


static long GetElementValue(joystick_t * joystick, joystick_element_t * element)
{
	IOReturn result = kIOReturnSuccess;
	IOHIDEventStruct hidEvent;
	hidEvent.value = 0;
	
	if (NULL != joystick && NULL != element && NULL != joystick->Interface)
	{
		result = (*(joystick->Interface))->getElementValue(joystick->Interface, element->Cookie, &hidEvent);
		if (kIOReturnSuccess == result)
		{
            /* record min and max for auto calibration */
            if (hidEvent.value < element->MinReport)
             element->MinReport = hidEvent.value;
            if (hidEvent.value > element->MaxReport)
             element->MaxReport = hidEvent.value;
		}
	}
    
	/* auto user scale */
	return (long) hidEvent.value;
}


static void RemoveJoystick(joystick_t * joystick)
{
    if (joystick->Present)
    {
        joystick->Present = GL_FALSE;
        
        for (int i = 0; i < joystick->NumAxes; i++)
        {
            joystick_element_t * axes = 
                (joystick_element_t *) CFArrayGetValueAtIndex(joystick->Axes, i);
            free(axes);
        }
        CFArrayRemoveAllValues(joystick->Axes);
        joystick->NumAxes = 0;
        
        for (int i = 0; i < joystick->NumButtons; i++)
        {
            joystick_element_t * button =
                (joystick_element_t *) CFArrayGetValueAtIndex(joystick->Buttons, i);
            free(button);
        }
        CFArrayRemoveAllValues(joystick->Buttons);
        joystick->NumButtons = 0;
        
        for (int i = 0; i < joystick->NumHats; i++)
        {
            joystick_element_t * hat =
                (joystick_element_t *) CFArrayGetValueAtIndex(joystick->Hats, i);
            free(hat);
        }
        CFArrayRemoveAllValues(joystick->Hats);
        joystick->Hats = 0;
        
        (*(joystick->Interface))->close(joystick->Interface);
        (*(joystick->Interface))->Release(joystick->Interface);
        
        joystick->Interface = NULL;
    }
}


static void RemovalCallback(void * target, IOReturn result, void * refcon, void * sender)
{
    RemoveJoystick((joystick_t *) refcon);
}


static void PollJoystickEvents(void)
{
    for (int i = 0; i < GLFW_JOYSTICK_LAST + 1; i++)
    {
        joystick_t * joystick = &_glfwJoysticks[i];
        
        if (joystick->Present)
        {
            for (CFIndex i = 0; i < joystick->NumButtons; i++)
            {
                joystick_element_t * button =
                    (joystick_element_t *) CFArrayGetValueAtIndex(joystick->Buttons, i);
                button->Value = GetElementValue(joystick, button);
            }
            
            for (CFIndex i = 0; i < joystick->NumAxes; i++)
            {
                joystick_element_t * axes = 
                    (joystick_element_t *) CFArrayGetValueAtIndex(joystick->Axes, i);   
                axes->Value = GetElementValue(joystick, axes);
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
    int                     deviceCounter       = 0;
    IOReturn                result              = kIOReturnSuccess;
    mach_port_t             masterPort          = 0;
    io_iterator_t           objectIterator      = 0;
    CFMutableDictionaryRef  hidMatchDictionary  = NULL;
    io_object_t             ioHIDDeviceObject   = 0;
    
    result = IOMasterPort(bootstrap_port, &masterPort);
    hidMatchDictionary = IOServiceMatching(kIOHIDDeviceKey);
    if (kIOReturnSuccess != result || !hidMatchDictionary)
        return;
    
    result = IOServiceGetMatchingServices(masterPort, hidMatchDictionary, &objectIterator);
    if (kIOReturnSuccess != result)
        return;
    
    if (!objectIterator) /* there are no joysticks */
        return;
    
    while ((ioHIDDeviceObject = IOIteratorNext(objectIterator)))
    {
        CFMutableDictionaryRef  hidProperties = 0;
        kern_return_t           result;
        CFTypeRef               refCF = 0;
        
        IOCFPlugInInterface ** ppPlugInInterface = NULL;
        HRESULT plugInResult = S_OK;
        SInt32 score = 0;
        
        long usagePage, usage;
        
        
        result = IORegistryEntryCreateCFProperties(
                                                   ioHIDDeviceObject,
                                                   &hidProperties,
                                                   kCFAllocatorDefault,
                                                   kNilOptions);
        
        if (result != kIOReturnSuccess) continue;
        
        /* Check device type */
        refCF = CFDictionaryGetValue(hidProperties, CFSTR(kIOHIDPrimaryUsagePageKey));
        if (refCF)
            CFNumberGetValue(refCF, kCFNumberLongType, &usagePage);
        
        refCF = CFDictionaryGetValue(hidProperties, CFSTR(kIOHIDPrimaryUsageKey));
        if (refCF)
            CFNumberGetValue (refCF, kCFNumberLongType, &usage);
        
        if ((usagePage   != kHIDPage_GenericDesktop) ||
            (usage       != kHIDUsage_GD_Joystick    &&
             usage       != kHIDUsage_GD_GamePad     &&
             usage       != kHIDUsage_GD_MultiAxisController))
        {
            /* We don't interested in this device */
            continue;
        }
        
        
        joystick_t * joystick = &_glfwJoysticks[deviceCounter];
        
        joystick->Present = GL_TRUE;
        
        
        result = IOCreatePlugInInterfaceForService(
                            ioHIDDeviceObject, 
                            kIOHIDDeviceUserClientTypeID, 
                            kIOCFPlugInInterfaceID, 
                            &ppPlugInInterface,
                            &score);
        
        if (kIOReturnSuccess != result)
            exit(EXIT_SUCCESS);
        
        plugInResult = (*ppPlugInInterface)->QueryInterface(
                            ppPlugInInterface, 
                            CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID), 
                            (void *) &(joystick->Interface));
        
        if (S_OK != plugInResult)
            exit(EXIT_FAILURE);
        
        (*ppPlugInInterface)->Release(ppPlugInInterface);
        
        (*(joystick->Interface))->open(joystick->Interface, 0);
        (*(joystick->Interface))->setRemovalCallback(joystick->Interface, RemovalCallback, joystick, joystick);
        
        /* Get product string */
        refCF = CFDictionaryGetValue(hidProperties, CFSTR(kIOHIDProductKey));
        if (refCF)
            CFStringGetCString(refCF, (char *) &(joystick->Product), 256, CFStringGetSystemEncoding());
        
        joystick->NumAxes    = 0;
        joystick->NumButtons = 0;
        joystick->NumHats    = 0;
        joystick->Axes    = CFArrayCreateMutable(NULL, 0, NULL);
        joystick->Buttons = CFArrayCreateMutable(NULL, 0, NULL);
        joystick->Hats    = CFArrayCreateMutable(NULL, 0, NULL);
        
        CFTypeRef refTopElement = CFDictionaryGetValue(hidProperties, CFSTR(kIOHIDElementKey));
        CFTypeID type = CFGetTypeID(refTopElement);
        if (type == CFArrayGetTypeID())
        {
            CFRange range = {0, CFArrayGetCount(refTopElement)};
            CFArrayApplyFunction(refTopElement, range, GetElementsCFArrayHandler, (void *) joystick);
        }
        
        deviceCounter++;
    }
}


//========================================================================
// Close all opened joystick handles
//========================================================================

void _glfwTerminateJoysticks(void)
{
    for (int i = 0; i < GLFW_JOYSTICK_LAST + 1; i++)
    {
        joystick_t * joystick = &_glfwJoysticks[i];
        RemoveJoystick(joystick);
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
    if (!_glfwJoysticks[joy].Present)
    {
        // TODO: Figure out if this is an error
        return 0;
    }
    
    switch (param)
    {
        case GLFW_PRESENT:
            return GL_TRUE;
            
        case GLFW_AXES:
            return (int) CFArrayGetCount(_glfwJoysticks[joy].Axes);
            
        case GLFW_BUTTONS:
            return (int) CFArrayGetCount(_glfwJoysticks[joy].Buttons);
            
        default:
            break;
    }
    
    return 0;
}


//========================================================================
// Get joystick axis positions
//========================================================================

int _glfwPlatformGetJoystickPos(int joy, float *pos, int numaxes)
{
    if (joy < GLFW_JOYSTICK_1 || joy > GLFW_JOYSTICK_LAST)
        return 0;
    
    joystick_t joystick = _glfwJoysticks[joy];
    
    if (!joystick.Present)
    {
        // TODO: Figure out if this is an error
        return 0;
    }
    
    numaxes = numaxes < joystick.NumAxes ? numaxes : joystick.NumAxes;
    
    // Update joystick state
    PollJoystickEvents();
    
    for (int i = 0; i < numaxes; i++)
    {
        joystick_element_t * axes = 
            (joystick_element_t *) CFArrayGetValueAtIndex(joystick.Axes, i);
        
    	long readScale = axes->MaxReport - axes->MinReport;
    	
    	if (readScale == 0)
    		pos[i] = axes->Value;
    	else
    	    pos[i] = (2.0f * (axes->Value - axes->MinReport) / readScale) - 1.0f;
    	
        printf("%ld, %ld, %ld\n", axes->Value, axes->MinReport, axes->MaxReport);
    	
    	if (i & 1)
            pos[i] = -pos[i];
    }
    
    return numaxes;
}


//========================================================================
// Get joystick button states
//========================================================================

int _glfwPlatformGetJoystickButtons(int joy, unsigned char *buttons,
                                    int numbuttons)
{
    if (joy < GLFW_JOYSTICK_1 || joy > GLFW_JOYSTICK_LAST)
        return 0;
    
    joystick_t joystick = _glfwJoysticks[joy];
    
    if (!joystick.Present)
    {
        // TODO: Figure out if this is an error
        return 0;
    }
    
    numbuttons = numbuttons < joystick.NumButtons ? numbuttons : joystick.NumButtons;
    
    // Update joystick state
    PollJoystickEvents();
    
    
    for (int i = 0; i < numbuttons; i++)
    {
        joystick_element_t * button = (joystick_element_t *) CFArrayGetValueAtIndex(joystick.Buttons, i);
        buttons[i] = button->Value ? GLFW_PRESS : GLFW_RELEASE;
    }
    
    return numbuttons;
}


