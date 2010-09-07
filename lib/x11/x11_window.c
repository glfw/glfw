//========================================================================
// GLFW - An OpenGL framework
// Platform:    X11/GLX
// API version: 2.7
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <limits.h>


/* Define GLX 1.4 FSAA tokens if not already defined */
#ifndef GLX_VERSION_1_4

#define GLX_SAMPLE_BUFFERS  100000
#define GLX_SAMPLES         100001

#endif /*GLX_VERSION_1_4*/

// Action for EWMH client messages
#define _NET_WM_STATE_REMOVE        0
#define _NET_WM_STATE_ADD           1
#define _NET_WM_STATE_TOGGLE        2


//************************************************************************
//****                  GLFW internal functions                       ****
//************************************************************************

//========================================================================
// Checks whether the event is a MapNotify for the specified window
//========================================================================

static Bool isMapNotify( Display *d, XEvent *e, char *arg )
{
    return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}


//========================================================================
// Retrieve a single window property of the specified type
// Inspired by fghGetWindowProperty from freeglut
//========================================================================

static unsigned long getWindowProperty( Window window,
                                        Atom property,
                                        Atom type,
                                        unsigned char** value )
{
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty( _glfwLibrary.display,
                        window,
                        property,
                        0,
                        LONG_MAX,
                        False,
                        type,
                        &actualType,
                        &actualFormat,
                        &itemCount,
                        &bytesAfter,
                        value );

    if( actualType != type )
    {
        return 0;
    }

    return itemCount;
}


//========================================================================
// Check whether the specified atom is supported
//========================================================================

static Atom getSupportedAtom( Atom* supportedAtoms,
                              unsigned long atomCount,
                              const char* atomName )
{
    Atom atom = XInternAtom( _glfwLibrary.display, atomName, True );
    if( atom != None )
    {
        unsigned long i;

        for( i = 0;  i < atomCount;  i++ )
        {
            if( supportedAtoms[i] == atom )
            {
                return atom;
            }
        }
    }

    return None;
}


//========================================================================
// Check whether the running window manager is EWMH-compliant
//========================================================================

static GLboolean checkForEWMH( void )
{
    Window *windowFromRoot = NULL;
    Window *windowFromChild = NULL;

    // Hey kids; let's see if the window manager supports EWMH!

    // First we need a couple of atoms, which should already be there
    Atom supportingWmCheck = XInternAtom( _glfwLibrary.display,
                                          "_NET_SUPPORTING_WM_CHECK",
                                          True );
    Atom wmSupported = XInternAtom( _glfwLibrary.display,
                                    "_NET_SUPPORTED",
                                    True );
    if( supportingWmCheck == None || wmSupported == None )
    {
        return GL_FALSE;
    }

    // Then we look for the _NET_SUPPORTING_WM_CHECK property of the root window
    if( getWindowProperty( _glfwWin.root,
                           supportingWmCheck,
                           XA_WINDOW,
                           (unsigned char**) &windowFromRoot ) != 1 )
    {
        XFree( windowFromRoot );
        return GL_FALSE;
    }

    // It should be the ID of a child window (of the root)
    // Then we look for the same property on the child window
    if( getWindowProperty( *windowFromRoot,
                           supportingWmCheck,
                           XA_WINDOW,
                           (unsigned char**) &windowFromChild ) != 1 )
    {
        XFree( windowFromRoot );
        XFree( windowFromChild );
        return GL_FALSE;
    }

    // It should be the ID of that same child window
    if( *windowFromRoot != *windowFromChild )
    {
        XFree( windowFromRoot );
        XFree( windowFromChild );
        return GL_FALSE;
    }

    XFree( windowFromRoot );
    XFree( windowFromChild );

    // We are now fairly sure that an EWMH-compliant window manager is running

    Atom *supportedAtoms;
    unsigned long atomCount;

    // Now we need to check the _NET_SUPPORTED property of the root window
    atomCount = getWindowProperty( _glfwWin.root,
                                   wmSupported,
                                   XA_ATOM,
                                   (unsigned char**) &supportedAtoms );

    // See which of the atoms we support that are supported by the WM

    _glfwWin.wmState = getSupportedAtom( supportedAtoms,
                                         atomCount,
                                         "_NET_WM_STATE" );

    _glfwWin.wmStateFullscreen = getSupportedAtom( supportedAtoms,
                                                   atomCount,
                                                   "_NET_WM_STATE_FULLSCREEN" );

    _glfwWin.wmPing = getSupportedAtom( supportedAtoms,
                                        atomCount,
                                        "_NET_WM_PING" );

    _glfwWin.wmActiveWindow = getSupportedAtom( supportedAtoms,
                                                atomCount,
                                                "_NET_ACTIVE_WINDOW" );

    XFree( supportedAtoms );

    return GL_TRUE;
}

//========================================================================
// Translates an X Window key to internal coding
//========================================================================

static int translateKey( int keycode )
{
    KeySym key, key_lc, key_uc;

    // Try secondary keysym, for numeric keypad keys
    // Note: This way we always force "NumLock = ON", which at least
    // enables GLFW users to detect numeric keypad keys
    key = XKeycodeToKeysym( _glfwLibrary.display, keycode, 1 );
    switch( key )
    {
        // Numeric keypad
        case XK_KP_0:         return GLFW_KEY_KP_0;
        case XK_KP_1:         return GLFW_KEY_KP_1;
        case XK_KP_2:         return GLFW_KEY_KP_2;
        case XK_KP_3:         return GLFW_KEY_KP_3;
        case XK_KP_4:         return GLFW_KEY_KP_4;
        case XK_KP_5:         return GLFW_KEY_KP_5;
        case XK_KP_6:         return GLFW_KEY_KP_6;
        case XK_KP_7:         return GLFW_KEY_KP_7;
        case XK_KP_8:         return GLFW_KEY_KP_8;
        case XK_KP_9:         return GLFW_KEY_KP_9;
        case XK_KP_Separator:
        case XK_KP_Decimal:   return GLFW_KEY_KP_DECIMAL;
        case XK_KP_Equal:     return GLFW_KEY_KP_EQUAL;
        case XK_KP_Enter:     return GLFW_KEY_KP_ENTER;
        default:              break;
    }

    // Now try pimary keysym
    key = XKeycodeToKeysym( _glfwLibrary.display, keycode, 0 );
    switch( key )
    {
        // Special keys (non character keys)
        case XK_Escape:       return GLFW_KEY_ESC;
        case XK_Tab:          return GLFW_KEY_TAB;
        case XK_Shift_L:      return GLFW_KEY_LSHIFT;
        case XK_Shift_R:      return GLFW_KEY_RSHIFT;
        case XK_Control_L:    return GLFW_KEY_LCTRL;
        case XK_Control_R:    return GLFW_KEY_RCTRL;
        case XK_Meta_L:
        case XK_Alt_L:        return GLFW_KEY_LALT;
        case XK_Mode_switch:  // Mapped to Alt_R on many keyboards
        case XK_Meta_R:
        case XK_ISO_Level3_Shift: // AltGr on at least some machines
        case XK_Alt_R:        return GLFW_KEY_RALT;
        case XK_Super_L:      return GLFW_KEY_LSUPER;
        case XK_Super_R:      return GLFW_KEY_RSUPER;
        case XK_Menu:         return GLFW_KEY_MENU;
        case XK_Num_Lock:     return GLFW_KEY_KP_NUM_LOCK;
        case XK_Caps_Lock:    return GLFW_KEY_CAPS_LOCK;
        case XK_Scroll_Lock:  return GLFW_KEY_SCROLL_LOCK;
        case XK_Pause:        return GLFW_KEY_PAUSE;
        case XK_KP_Delete:
        case XK_Delete:       return GLFW_KEY_DEL;
        case XK_BackSpace:    return GLFW_KEY_BACKSPACE;
        case XK_Return:       return GLFW_KEY_ENTER;
        case XK_KP_Home:
        case XK_Home:         return GLFW_KEY_HOME;
        case XK_KP_End:
        case XK_End:          return GLFW_KEY_END;
        case XK_KP_Page_Up:
        case XK_Page_Up:      return GLFW_KEY_PAGEUP;
        case XK_KP_Page_Down:
        case XK_Page_Down:    return GLFW_KEY_PAGEDOWN;
        case XK_KP_Insert:
        case XK_Insert:       return GLFW_KEY_INSERT;
        case XK_KP_Left:
        case XK_Left:         return GLFW_KEY_LEFT;
        case XK_KP_Right:
        case XK_Right:        return GLFW_KEY_RIGHT;
        case XK_KP_Down:
        case XK_Down:         return GLFW_KEY_DOWN;
        case XK_KP_Up:
        case XK_Up:           return GLFW_KEY_UP;
        case XK_F1:           return GLFW_KEY_F1;
        case XK_F2:           return GLFW_KEY_F2;
        case XK_F3:           return GLFW_KEY_F3;
        case XK_F4:           return GLFW_KEY_F4;
        case XK_F5:           return GLFW_KEY_F5;
        case XK_F6:           return GLFW_KEY_F6;
        case XK_F7:           return GLFW_KEY_F7;
        case XK_F8:           return GLFW_KEY_F8;
        case XK_F9:           return GLFW_KEY_F9;
        case XK_F10:          return GLFW_KEY_F10;
        case XK_F11:          return GLFW_KEY_F11;
        case XK_F12:          return GLFW_KEY_F12;
        case XK_F13:          return GLFW_KEY_F13;
        case XK_F14:          return GLFW_KEY_F14;
        case XK_F15:          return GLFW_KEY_F15;
        case XK_F16:          return GLFW_KEY_F16;
        case XK_F17:          return GLFW_KEY_F17;
        case XK_F18:          return GLFW_KEY_F18;
        case XK_F19:          return GLFW_KEY_F19;
        case XK_F20:          return GLFW_KEY_F20;
        case XK_F21:          return GLFW_KEY_F21;
        case XK_F22:          return GLFW_KEY_F22;
        case XK_F23:          return GLFW_KEY_F23;
        case XK_F24:          return GLFW_KEY_F24;
        case XK_F25:          return GLFW_KEY_F25;

        // Numeric keypad (should have been detected in secondary keysym!)
        case XK_KP_Divide:    return GLFW_KEY_KP_DIVIDE;
        case XK_KP_Multiply:  return GLFW_KEY_KP_MULTIPLY;
        case XK_KP_Subtract:  return GLFW_KEY_KP_SUBTRACT;
        case XK_KP_Add:       return GLFW_KEY_KP_ADD;
        case XK_KP_Equal:     return GLFW_KEY_KP_EQUAL;
        case XK_KP_Enter:     return GLFW_KEY_KP_ENTER;

        // The rest (should be printable keys)
        default:
            // Make uppercase
            XConvertCase( key, &key_lc, &key_uc );
            key = key_uc;

            // Valid ISO 8859-1 character?
            if( (key >=  32 && key <= 126) ||
                (key >= 160 && key <= 255) )
            {
                return (int) key;
            }
            return GLFW_KEY_UNKNOWN;
    }
}


//========================================================================
// Translates an X Window event to Unicode
//========================================================================

static int translateChar( XKeyEvent *event )
{
    KeySym keysym;

    // Get X11 keysym
    XLookupString( event, NULL, 0, &keysym, NULL );

    // Convert to Unicode (see x11_keysym2unicode.c)
    return (int) _glfwKeySym2Unicode( keysym );
}


//========================================================================
// Create a blank cursor (for locked mouse mode)
//========================================================================

static Cursor createNULLCursor( Display *display, Window root )
{
    Pixmap    cursormask;
    XGCValues xgc;
    GC        gc;
    XColor    col;
    Cursor    cursor;

    cursormask = XCreatePixmap( display, root, 1, 1, 1 );
    xgc.function = GXclear;
    gc = XCreateGC( display, cursormask, GCFunction, &xgc );
    XFillRectangle( display, cursormask, gc, 0, 0, 1, 1 );
    col.pixel = 0;
    col.red = 0;
    col.flags = 4;
    cursor = XCreatePixmapCursor( display, cursormask, cursormask,
                                  &col,&col, 0,0 );
    XFreePixmap( display, cursormask );
    XFreeGC( display, gc );

    return cursor;
}


//========================================================================
// Returns the specified attribute of the specified GLXFBConfig
// NOTE: Do not call this unless we have found GLX 1.3+ or GLX_SGIX_fbconfig
//========================================================================

static int getFBConfigAttrib( GLXFBConfig fbconfig, int attrib )
{
    int value;

    if( _glfwWin.has_GLX_SGIX_fbconfig )
    {
        _glfwWin.GetFBConfigAttribSGIX( _glfwLibrary.display, fbconfig, attrib, &value );
    }
    else
    {
        glXGetFBConfigAttrib( _glfwLibrary.display, fbconfig, attrib, &value );
    }

    return value;
}


//========================================================================
// Return a list of available and usable framebuffer configs
//========================================================================

static _GLFWfbconfig *getFBConfigs( unsigned int *found )
{
    GLXFBConfig *fbconfigs;
    _GLFWfbconfig *result;
    int i, count = 0;

    *found = 0;

    if( _glfwLibrary.glxMajor == 1 && _glfwLibrary.glxMinor < 3 )
    {
        if( !_glfwWin.has_GLX_SGIX_fbconfig )
        {
            fprintf( stderr, "GLXFBConfigs are not supported by the X server\n" );
            return NULL;
        }
    }

    if( _glfwWin.has_GLX_SGIX_fbconfig )
    {
        fbconfigs = _glfwWin.ChooseFBConfigSGIX( _glfwLibrary.display,
                                                 _glfwWin.screen,
                                                 NULL,
                                                 &count );
        if( !count )
        {
            fprintf( stderr, "No GLXFBConfigs returned\n" );
            return NULL;
        }
    }
    else
    {
        fbconfigs = glXGetFBConfigs( _glfwLibrary.display, _glfwWin.screen, &count );
        if( !count )
        {
            fprintf( stderr, "No GLXFBConfigs returned\n" );
            return NULL;
        }
    }

    result = (_GLFWfbconfig*) malloc( sizeof(_GLFWfbconfig) * count );
    if( !result )
    {
        fprintf( stderr, "Out of memory\n" );
        return NULL;
    }

    for( i = 0;  i < count;  i++ )
    {
        if( !getFBConfigAttrib( fbconfigs[i], GLX_DOUBLEBUFFER ) ||
            !getFBConfigAttrib( fbconfigs[i], GLX_VISUAL_ID ) )
        {
            // Only consider double-buffered GLXFBConfigs with associated visuals
            continue;
        }

        if( !( getFBConfigAttrib( fbconfigs[i], GLX_RENDER_TYPE ) & GLX_RGBA_BIT ) )
        {
            // Only consider RGBA GLXFBConfigs
            continue;
        }

        if( !( getFBConfigAttrib( fbconfigs[i], GLX_DRAWABLE_TYPE ) & GLX_WINDOW_BIT ) )
        {
            // Only consider window GLXFBConfigs
            continue;
        }

        result[*found].redBits = getFBConfigAttrib( fbconfigs[i], GLX_RED_SIZE );
        result[*found].greenBits = getFBConfigAttrib( fbconfigs[i], GLX_GREEN_SIZE );
        result[*found].blueBits = getFBConfigAttrib( fbconfigs[i], GLX_BLUE_SIZE );

        result[*found].alphaBits = getFBConfigAttrib( fbconfigs[i], GLX_ALPHA_SIZE );
        result[*found].depthBits = getFBConfigAttrib( fbconfigs[i], GLX_DEPTH_SIZE );
        result[*found].stencilBits = getFBConfigAttrib( fbconfigs[i], GLX_STENCIL_SIZE );

        result[*found].accumRedBits = getFBConfigAttrib( fbconfigs[i], GLX_ACCUM_RED_SIZE );
        result[*found].accumGreenBits = getFBConfigAttrib( fbconfigs[i], GLX_ACCUM_GREEN_SIZE );
        result[*found].accumBlueBits = getFBConfigAttrib( fbconfigs[i], GLX_ACCUM_BLUE_SIZE );
        result[*found].accumAlphaBits = getFBConfigAttrib( fbconfigs[i], GLX_ACCUM_ALPHA_SIZE );

        result[*found].auxBuffers = getFBConfigAttrib( fbconfigs[i], GLX_AUX_BUFFERS );
        result[*found].stereo = getFBConfigAttrib( fbconfigs[i], GLX_STEREO );

        if( _glfwWin.has_GLX_ARB_multisample )
        {
            result[*found].samples = getFBConfigAttrib( fbconfigs[i], GLX_SAMPLES );
        }
        else
        {
            result[*found].samples = 0;
        }

        result[*found].platformID = (GLFWintptr) getFBConfigAttrib( fbconfigs[i], GLX_FBCONFIG_ID );

        (*found)++;
    }

    XFree( fbconfigs );

    return result;
}


//========================================================================
// Create the OpenGL context
//========================================================================

#define setGLXattrib( attribs, index, attribName, attribValue ) \
    attribs[index++] = attribName; \
    attribs[index++] = attribValue;

static int createContext( const _GLFWwndconfig *wndconfig, GLXFBConfigID fbconfigID )
{
    int attribs[40];
    int flags, dummy, index;
    GLXFBConfig *fbconfig;

    // Retrieve the previously selected GLXFBConfig
    {
        index = 0;

        setGLXattrib( attribs, index, GLX_FBCONFIG_ID, (int) fbconfigID );
        setGLXattrib( attribs, index, None, None );

        if( _glfwWin.has_GLX_SGIX_fbconfig )
        {
            fbconfig = _glfwWin.ChooseFBConfigSGIX( _glfwLibrary.display,
                                                    _glfwWin.screen,
                                                    attribs,
                                                    &dummy );
        }
        else
        {
            fbconfig = glXChooseFBConfig( _glfwLibrary.display,
                                          _glfwWin.screen,
                                          attribs,
                                          &dummy );
        }

        if( fbconfig == NULL )
        {
            fprintf(stderr, "Unable to retrieve the selected GLXFBConfig\n");
            return GL_FALSE;
        }
    }

    // Retrieve the corresponding visual
    if( _glfwWin.has_GLX_SGIX_fbconfig )
    {
        _glfwWin.visual = _glfwWin.GetVisualFromFBConfigSGIX( _glfwLibrary.display,
                                                              *fbconfig );
    }
    else
    {
        _glfwWin.visual = glXGetVisualFromFBConfig( _glfwLibrary.display, *fbconfig );
    }

    if( _glfwWin.visual == NULL )
    {
        XFree( fbconfig );

        fprintf(stderr, "Unable to retrieve visual for GLXFBconfig\n");
        return GL_FALSE;
    }

    if( _glfwWin.has_GLX_ARB_create_context )
    {
        index = 0;

        if( wndconfig->glMajor != 1 || wndconfig->glMinor != 0 )
        {
            // Request an explicitly versioned context

            setGLXattrib( attribs, index, GLX_CONTEXT_MAJOR_VERSION_ARB, wndconfig->glMajor );
            setGLXattrib( attribs, index, GLX_CONTEXT_MINOR_VERSION_ARB, wndconfig->glMinor );
        }

        if( wndconfig->glForward || wndconfig->glDebug )
        {
            flags = 0;

            if( wndconfig->glForward )
            {
                flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
            }

            if( wndconfig->glDebug )
            {
                flags |= GLX_CONTEXT_DEBUG_BIT_ARB;
            }

            setGLXattrib( attribs, index, GLX_CONTEXT_FLAGS_ARB, flags );
        }

        if( wndconfig->glProfile )
        {
            if( !_glfwWin.has_GLX_ARB_create_context_profile )
            {
                fprintf( stderr, "OpenGL profile requested but GLX_ARB_create_context_profile "
                                 "is unavailable\n" );
                return GL_FALSE;
            }

            if( wndconfig->glProfile == GLFW_OPENGL_CORE_PROFILE )
            {
                flags = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
            }
            else
            {
                flags = GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            }

            setGLXattrib( attribs, index, GLX_CONTEXT_PROFILE_MASK_ARB, flags );
        }

        setGLXattrib( attribs, index, None, None );

        _glfwWin.context = _glfwWin.CreateContextAttribsARB( _glfwLibrary.display,
                                                             *fbconfig,
                                                             NULL,
                                                             True,
                                                             attribs );
    }
    else
    {
        if( _glfwWin.has_GLX_SGIX_fbconfig )
        {
            _glfwWin.context = _glfwWin.CreateContextWithConfigSGIX( _glfwLibrary.display,
                                                                     *fbconfig,
                                                                     GLX_RGBA_TYPE,
                                                                     NULL,
                                                                     True );
        }
        else
        {
            _glfwWin.context = glXCreateNewContext( _glfwLibrary.display,
                                                    *fbconfig,
                                                    GLX_RGBA_TYPE,
                                                    NULL,
                                                    True );
        }
    }

    XFree( fbconfig );

    if( _glfwWin.context == NULL )
    {
        fprintf(stderr, "Unable to create OpenGL context\n");
        return GL_FALSE;
    }

    _glfwWin.fbconfigID = fbconfigID;

    return GL_TRUE;
}

#undef setGLXattrib


//========================================================================
// Initialize GLX-specific extensions
//========================================================================

static void initGLXExtensions( void )
{
    // This needs to include every function pointer loaded below
    _glfwWin.SwapIntervalSGI             = NULL;
    _glfwWin.GetFBConfigAttribSGIX       = NULL;
    _glfwWin.ChooseFBConfigSGIX          = NULL;
    _glfwWin.CreateContextWithConfigSGIX = NULL;
    _glfwWin.GetVisualFromFBConfigSGIX   = NULL;
    _glfwWin.CreateContextAttribsARB     = NULL;

    // This needs to include every extension used below
    _glfwWin.has_GLX_SGIX_fbconfig              = GL_FALSE;
    _glfwWin.has_GLX_SGI_swap_control           = GL_FALSE;
    _glfwWin.has_GLX_ARB_multisample            = GL_FALSE;
    _glfwWin.has_GLX_ARB_create_context         = GL_FALSE;
    _glfwWin.has_GLX_ARB_create_context_profile = GL_FALSE;

    if( _glfwPlatformExtensionSupported( "GLX_SGI_swap_control" ) )
    {
        _glfwWin.SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
            _glfwPlatformGetProcAddress( "glXSwapIntervalSGI" );

        if( _glfwWin.SwapIntervalSGI )
        {
            _glfwWin.has_GLX_SGI_swap_control = GL_TRUE;
        }
    }

    if( _glfwPlatformExtensionSupported( "GLX_SGIX_fbconfig" ) )
    {
        _glfwWin.GetFBConfigAttribSGIX = (PFNGLXGETFBCONFIGATTRIBSGIXPROC)
            _glfwPlatformGetProcAddress( "glXGetFBConfigAttribSGIX" );
        _glfwWin.ChooseFBConfigSGIX = (PFNGLXCHOOSEFBCONFIGSGIXPROC)
            _glfwPlatformGetProcAddress( "glXChooseFBConfigSGIX" );
        _glfwWin.CreateContextWithConfigSGIX = (PFNGLXCREATECONTEXTWITHCONFIGSGIXPROC)
            _glfwPlatformGetProcAddress( "glXCreateContextWithConfigSGIX" );
        _glfwWin.GetVisualFromFBConfigSGIX = (PFNGLXGETVISUALFROMFBCONFIGSGIXPROC)
            _glfwPlatformGetProcAddress( "glXGetVisualFromFBConfigSGIX" );

        if( _glfwWin.GetFBConfigAttribSGIX &&
            _glfwWin.ChooseFBConfigSGIX &&
            _glfwWin.CreateContextWithConfigSGIX &&
            _glfwWin.GetVisualFromFBConfigSGIX )
        {
            _glfwWin.has_GLX_SGIX_fbconfig = GL_TRUE;
        }
    }

    if( _glfwPlatformExtensionSupported( "GLX_ARB_multisample" ) )
    {
        _glfwWin.has_GLX_ARB_multisample = GL_TRUE;
    }

    if( _glfwPlatformExtensionSupported( "GLX_ARB_create_context" ) )
    {
        _glfwWin.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            _glfwPlatformGetProcAddress( "glXCreateContextAttribsARB" );

        if( _glfwWin.CreateContextAttribsARB )
        {
            _glfwWin.has_GLX_ARB_create_context = GL_TRUE;
        }
    }

    if( _glfwPlatformExtensionSupported( "GLX_ARB_create_context_profile" ) )
    {
        _glfwWin.has_GLX_ARB_create_context_profile = GL_TRUE;
    }
}


//========================================================================
// Create the X11 window (and its colormap)
//========================================================================

static GLboolean createWindow( int width, int height,
                               const _GLFWwndconfig *wndconfig )
{
    XEvent event;
    unsigned long wamask;
    XSetWindowAttributes wa;

    // Every window needs a colormap
    // Create one based on the visual used by the current context

    _glfwWin.colormap = XCreateColormap( _glfwLibrary.display,
                                         _glfwWin.root,
                                         _glfwWin.visual->visual,
                                         AllocNone );

    // Create the actual window
    {
        wamask = CWBorderPixel | CWColormap | CWEventMask;

        wa.colormap = _glfwWin.colormap;
        wa.border_pixel = 0;
        wa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
            PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
            ExposureMask | FocusChangeMask | VisibilityChangeMask;

        if( wndconfig->mode == GLFW_WINDOW )
        {
            // The /only/ reason we are setting the background pixel here is
            // that otherwise our window wont get any decorations on systems
            // using Compiz on Intel hardware
            wa.background_pixel = BlackPixel( _glfwLibrary.display, _glfwWin.screen );
            wamask |= CWBackPixel;
        }

        _glfwWin.window = XCreateWindow(
            _glfwLibrary.display,
            _glfwWin.root,
            0, 0,                            // Upper left corner of this window on root
            _glfwWin.width, _glfwWin.height,
            0,                               // Border width
            _glfwWin.visual->depth,          // Color depth
            InputOutput,
            _glfwWin.visual->visual,
            wamask,
            &wa
        );
        if( !_glfwWin.window )
        {
            _glfwPlatformCloseWindow();
            return GL_FALSE;
        }
    }

    // Check whether an EWMH-compliant window manager is running
    _glfwWin.hasEWMH = checkForEWMH();

    if( _glfwWin.fullscreen && !_glfwWin.hasEWMH )
    {
        // This is the butcher's way of removing window decorations
        // Setting the override-redirect attribute on a window makes the window
        // manager ignore the window completely (ICCCM, section 4)
        // The good thing is that this makes undecorated fullscreen windows
        // easy to do; the bad thing is that we have to do everything manually
        // and some things (like iconify/restore) won't work at all, as they're
        // usually performed by the window manager

        XSetWindowAttributes attributes;
        attributes.override_redirect = True;
        XChangeWindowAttributes( _glfwLibrary.display,
                                 _glfwWin.window,
                                 CWOverrideRedirect,
                                 &attributes );

        _glfwWin.overrideRedirect = GL_TRUE;
    }

    // Find or create the protocol atom for window close notifications
    _glfwWin.wmDeleteWindow = XInternAtom( _glfwLibrary.display,
                                            "WM_DELETE_WINDOW",
                                            False );

    // Declare the WM protocols we support
    {
        int count = 0;
        Atom protocols[2];

        // The WM_DELETE_WINDOW ICCCM protocol
        // Basic window close notification protocol
        if( _glfwWin.wmDeleteWindow != None )
        {
            protocols[count++] = _glfwWin.wmDeleteWindow;
        }

        // The _NET_WM_PING EWMH protocol
        // Tells the WM to ping our window and flag us as unresponsive if we
        // don't reply within a few seconds
        if( _glfwWin.wmPing != None )
        {
            protocols[count++] = _glfwWin.wmPing;
        }

        if( count > 0 )
        {
            XSetWMProtocols( _glfwLibrary.display, _glfwWin.window,
                             protocols, count );
        }
    }

    // Set ICCCM WM_HINTS property
    {
        XWMHints *hints = XAllocWMHints();
        if( !hints )
        {
            _glfwPlatformCloseWindow();
            return GL_FALSE;
        }

        hints->flags = StateHint;
        hints->initial_state = NormalState;

        XSetWMHints( _glfwLibrary.display, _glfwWin.window, hints );
        XFree( hints );
    }

    // Set ICCCM WM_NORMAL_HINTS property (even if no parts are set)
    {
        XSizeHints *hints = XAllocSizeHints();
        if( !hints )
        {
            _glfwPlatformCloseWindow();
            return GL_FALSE;
        }

        hints->flags = 0;

        if( wndconfig->windowNoResize )
        {
            hints->flags |= (PMinSize | PMaxSize);
            hints->min_width  = hints->max_width  = _glfwWin.width;
            hints->min_height = hints->max_height = _glfwWin.height;
        }

        XSetWMNormalHints( _glfwLibrary.display, _glfwWin.window, hints );
        XFree( hints );
    }

    _glfwPlatformSetWindowTitle( "GLFW Window" );

    // Make sure the window is mapped before proceeding
    XMapWindow( _glfwLibrary.display, _glfwWin.window );
    XPeekIfEvent( _glfwLibrary.display, &event, isMapNotify,
                  (char*)_glfwWin.window );

    return GL_TRUE;
}


//========================================================================
// Enter fullscreen mode
//========================================================================

static void enterFullscreenMode( void )
{
    if( !_glfwWin.Saver.changed )
    {
        // Remember old screen saver settings
        XGetScreenSaver( _glfwLibrary.display,
                         &_glfwWin.Saver.timeout, &_glfwWin.Saver.interval,
                         &_glfwWin.Saver.blanking, &_glfwWin.Saver.exposure );

        // Disable screen saver
        XSetScreenSaver( _glfwLibrary.display, 0, 0, DontPreferBlanking,
                        DefaultExposures );

        _glfwWin.Saver.changed = GL_TRUE;
    }

    _glfwSetVideoMode( _glfwWin.screen,
                       &_glfwWin.width, &_glfwWin.height,
                       &_glfwWin.refreshRate );

    if( _glfwWin.hasEWMH &&
        _glfwWin.wmState != None &&
        _glfwWin.wmStateFullscreen != None )
    {
        if( _glfwWin.wmActiveWindow != None )
        {
            // Ask the window manager to raise and focus the GLFW window
            // Only focused windows with the _NET_WM_STATE_FULLSCREEN state end
            // up on top of all other windows ("Stacking order" in EWMH spec)

            XEvent event;
            memset( &event, 0, sizeof(event) );

            event.type = ClientMessage;
            event.xclient.window = _glfwWin.window;
            event.xclient.format = 32; // Data is 32-bit longs
            event.xclient.message_type = _glfwWin.wmActiveWindow;
            event.xclient.data.l[0] = 1; // Sender is a normal application
            event.xclient.data.l[1] = 0; // We don't really know the timestamp

            XSendEvent( _glfwLibrary.display,
                        _glfwWin.root,
                        False,
                        SubstructureNotifyMask | SubstructureRedirectMask,
                        &event );
        }

        // Ask the window manager to make the GLFW window a fullscreen window
        // Fullscreen windows are undecorated and, when focused, are kept
        // on top of all other windows

        XEvent event;
        memset( &event, 0, sizeof(event) );

        event.type = ClientMessage;
        event.xclient.window = _glfwWin.window;
        event.xclient.format = 32; // Data is 32-bit longs
        event.xclient.message_type = _glfwWin.wmState;
        event.xclient.data.l[0] = _NET_WM_STATE_ADD;
        event.xclient.data.l[1] = _glfwWin.wmStateFullscreen;
        event.xclient.data.l[2] = 0; // No secondary property
        event.xclient.data.l[3] = 1; // Sender is a normal application

        XSendEvent( _glfwLibrary.display,
                    _glfwWin.root,
                    False,
                    SubstructureNotifyMask | SubstructureRedirectMask,
                    &event );
    }
    else if( _glfwWin.overrideRedirect )
    {
        // In override-redirect mode, we have divorced ourselves from the
        // window manager, so we need to do everything manually

        XRaiseWindow( _glfwLibrary.display, _glfwWin.window );
        XSetInputFocus( _glfwLibrary.display, _glfwWin.window,
                        RevertToParent, CurrentTime );
        XMoveWindow( _glfwLibrary.display, _glfwWin.window, 0, 0 );
        XResizeWindow( _glfwLibrary.display, _glfwWin.window,
                       _glfwWin.width, _glfwWin.height );
    }

    if( _glfwWin.mouseLock )
    {
        _glfwPlatformHideMouseCursor();
    }

    // HACK: Try to get window inside viewport (for virtual displays) by moving
    // the mouse cursor to the upper left corner (and then to the center)
    // This hack should be harmless on saner systems as well
    XWarpPointer( _glfwLibrary.display, None, _glfwWin.window, 0,0,0,0, 0,0 );
    XWarpPointer( _glfwLibrary.display, None, _glfwWin.window, 0,0,0,0,
                  _glfwWin.width / 2, _glfwWin.height / 2 );
}

//========================================================================
// Leave fullscreen mode
//========================================================================

static void leaveFullscreenMode( void )
{
    _glfwRestoreVideoMode();

    // Did we change the screen saver setting?
    if( _glfwWin.Saver.changed )
    {
        // Restore old screen saver settings
        XSetScreenSaver( _glfwLibrary.display,
                         _glfwWin.Saver.timeout,
                         _glfwWin.Saver.interval,
                         _glfwWin.Saver.blanking,
                         _glfwWin.Saver.exposure );

        _glfwWin.Saver.changed = GL_FALSE;
    }

    if( _glfwWin.hasEWMH &&
        _glfwWin.wmState != None &&
        _glfwWin.wmStateFullscreen != None )
    {
        // Ask the window manager to make the GLFW window a normal window
        // Normal windows usually have frames and other decorations

        XEvent event;
        memset( &event, 0, sizeof(event) );

        event.type = ClientMessage;
        event.xclient.window = _glfwWin.window;
        event.xclient.format = 32; // Data is 32-bit longs
        event.xclient.message_type = _glfwWin.wmState;
        event.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
        event.xclient.data.l[1] = _glfwWin.wmStateFullscreen;
        event.xclient.data.l[2] = 0; // No secondary property
        event.xclient.data.l[3] = 1; // Sender is a normal application

        XSendEvent( _glfwLibrary.display,
                    _glfwWin.root,
                    False,
                    SubstructureNotifyMask | SubstructureRedirectMask,
                    &event );
    }

    if( _glfwWin.mouseLock )
    {
        _glfwPlatformShowMouseCursor();
    }
}

//========================================================================
// Get and process next X event (called by _glfwPlatformPollEvents)
// Returns GL_TRUE if a window close request was received
//========================================================================

static GLboolean processSingleEvent( void )
{
    XEvent event;
    XNextEvent( _glfwLibrary.display, &event );

    switch( event.type )
    {
        case KeyPress:
        {
            // A keyboard key was pressed

            // Translate and report key press
            _glfwInputKey( translateKey( event.xkey.keycode ), GLFW_PRESS );

            // Translate and report character input
            if( _glfwWin.charCallback )
            {
                _glfwInputChar( translateChar( &event.xkey ), GLFW_PRESS );
            }
            break;
        }

        case KeyRelease:
        {
            // A keyboard key was released

            // Do not report key releases for key repeats. For key repeats we
            // will get KeyRelease/KeyPress pairs with similar or identical
            // time stamps. User selected key repeat filtering is handled in
            // _glfwInputKey()/_glfwInputChar().
            if( XEventsQueued( _glfwLibrary.display, QueuedAfterReading ) )
            {
                XEvent nextEvent;
                XPeekEvent( _glfwLibrary.display, &nextEvent );

                if( nextEvent.type == KeyPress &&
                    nextEvent.xkey.window == event.xkey.window &&
                    nextEvent.xkey.keycode == event.xkey.keycode )
                {
                    // This last check is a hack to work around key repeats
                    // leaking through due to some sort of time drift
                    // Toshiyuki Takahashi can press a button 16 times per
                    // second so it's fairly safe to assume that no human is
                    // pressing the key 50 times per second (value is ms)
                    if( ( nextEvent.xkey.time - event.xkey.time ) < 20 )
                    {
                        // Do not report anything for this event
                        break;
                    }
                }
            }

            // Translate and report key release
            _glfwInputKey( translateKey( event.xkey.keycode ), GLFW_RELEASE );

            // Translate and report character input
            if( _glfwWin.charCallback )
            {
                _glfwInputChar( translateChar( &event.xkey ), GLFW_RELEASE );
            }
            break;
        }

        case ButtonPress:
        {
            // A mouse button was pressed or a scrolling event occurred

            if( event.xbutton.button == Button1 )
            {
                _glfwInputMouseClick( GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS );
            }
            else if( event.xbutton.button == Button2 )
            {
                _glfwInputMouseClick( GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS );
            }
            else if( event.xbutton.button == Button3 )
            {
                _glfwInputMouseClick( GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS );
            }

            // XFree86 3.3.2 and later translates mouse wheel up/down into
            // mouse button 4 & 5 presses
            else if( event.xbutton.button == Button4 )
            {
                _glfwInput.WheelPos++;  // To verify: is this up or down?
                if( _glfwWin.mouseWheelCallback )
                {
                    _glfwWin.mouseWheelCallback( _glfwInput.WheelPos );
                }
            }
            else if( event.xbutton.button == Button5 )
            {
                _glfwInput.WheelPos--;
                if( _glfwWin.mouseWheelCallback )
                {
                    _glfwWin.mouseWheelCallback( _glfwInput.WheelPos );
                }
            }
            break;
        }

        case ButtonRelease:
        {
            // A mouse button was released

            if( event.xbutton.button == Button1 )
            {
                _glfwInputMouseClick( GLFW_MOUSE_BUTTON_LEFT,
                                      GLFW_RELEASE );
            }
            else if( event.xbutton.button == Button2 )
            {
                _glfwInputMouseClick( GLFW_MOUSE_BUTTON_MIDDLE,
                                      GLFW_RELEASE );
            }
            else if( event.xbutton.button == Button3 )
            {
                _glfwInputMouseClick( GLFW_MOUSE_BUTTON_RIGHT,
                                      GLFW_RELEASE );
            }
            break;
        }

        case MotionNotify:
        {
            // The mouse cursor was moved

            if( event.xmotion.x != _glfwInput.CursorPosX ||
                event.xmotion.y != _glfwInput.CursorPosY )
            {
                // The mouse cursor was moved and we didn't do it

                if( _glfwWin.mouseLock )
                {
                    if( _glfwWin.pointerHidden )
                    {
                        _glfwInput.MousePosX += event.xmotion.x -
                                                _glfwInput.CursorPosX;
                        _glfwInput.MousePosY += event.xmotion.y -
                                                _glfwInput.CursorPosY;
                    }
                }
                else
                {
                    _glfwInput.MousePosX = event.xmotion.x;
                    _glfwInput.MousePosY = event.xmotion.y;
                }

                _glfwInput.CursorPosX = event.xmotion.x;
                _glfwInput.CursorPosY = event.xmotion.y;
                _glfwInput.MouseMoved = GL_TRUE;

                if( _glfwWin.mousePosCallback )
                {
                    _glfwWin.mousePosCallback( _glfwInput.MousePosX,
                                               _glfwInput.MousePosY );
                }
            }
            break;
        }

        case ConfigureNotify:
        {
            if( event.xconfigure.width != _glfwWin.width ||
                event.xconfigure.height != _glfwWin.height )
            {
                // The window was resized

                _glfwWin.width = event.xconfigure.width;
                _glfwWin.height = event.xconfigure.height;
                if( _glfwWin.windowSizeCallback )
                {
                    _glfwWin.windowSizeCallback( _glfwWin.width,
                                                 _glfwWin.height );
                }
            }
            break;
        }

        case ClientMessage:
        {
            if( (Atom) event.xclient.data.l[ 0 ] == _glfwWin.wmDeleteWindow )
            {
                // The window manager was asked to close the window, for example by
                // the user pressing a 'close' window decoration button

                return GL_TRUE;
            }
            else if( _glfwWin.wmPing != None &&
                     (Atom) event.xclient.data.l[ 0 ] == _glfwWin.wmPing )
            {
                // The window manager is pinging us to make sure we are still
                // responding to events

                event.xclient.window = _glfwWin.root;
                XSendEvent( _glfwLibrary.display,
                            event.xclient.window,
                            False,
                            SubstructureNotifyMask | SubstructureRedirectMask,
                            &event );
            }

            break;
        }

        case MapNotify:
        {
            // The window was mapped

            _glfwWin.iconified = GL_FALSE;
            break;
        }

        case UnmapNotify:
        {
            // The window was unmapped

            _glfwWin.iconified = GL_TRUE;
            break;
        }

        case FocusIn:
        {
            // The window gained focus

            _glfwWin.active = GL_TRUE;

            if( _glfwWin.mouseLock )
            {
                _glfwPlatformHideMouseCursor();
            }

            break;
        }

        case FocusOut:
        {
            // The window lost focus

            _glfwWin.active = GL_FALSE;
            _glfwInputDeactivation();

            if( _glfwWin.mouseLock )
            {
                _glfwPlatformShowMouseCursor();
            }

            break;
        }

        case Expose:
        {
            // The window's contents was damaged

            if( _glfwWin.windowRefreshCallback )
            {
                _glfwWin.windowRefreshCallback();
            }
            break;
        }

        // Was the window destroyed?
        case DestroyNotify:
            return GL_FALSE;

        default:
        {
#if defined( _GLFW_HAS_XRANDR )
            switch( event.type - _glfwLibrary.XRandR.eventBase )
            {
                case RRScreenChangeNotify:
                {
                    // Show XRandR that we really care
                    XRRUpdateConfiguration( &event );
                    break;
                }
            }
#endif
            break;
        }
    }

    // The window was not destroyed
    return GL_FALSE;
}



//************************************************************************
//****               Platform implementation functions                ****
//************************************************************************

//========================================================================
// Here is where the window is created, and
// the OpenGL rendering context is created
//========================================================================

int _glfwPlatformOpenWindow( int width, int height,
                             const _GLFWwndconfig* wndconfig,
                             const _GLFWfbconfig* fbconfig )
{
    _GLFWfbconfig closest;

    // Clear platform specific GLFW window state
    _glfwWin.visual           = (XVisualInfo*)NULL;
    _glfwWin.colormap         = (Colormap)0;
    _glfwWin.context          = (GLXContext)NULL;
    _glfwWin.window           = (Window)0;
    _glfwWin.pointerGrabbed   = GL_FALSE;
    _glfwWin.pointerHidden    = GL_FALSE;
    _glfwWin.keyboardGrabbed  = GL_FALSE;
    _glfwWin.overrideRedirect = GL_FALSE;
    _glfwWin.FS.modeChanged   = GL_FALSE;
    _glfwWin.Saver.changed    = GL_FALSE;
    _glfwWin.refreshRate      = wndconfig->refreshRate;
    _glfwWin.windowNoResize   = wndconfig->windowNoResize;

    _glfwWin.wmDeleteWindow    = None;
    _glfwWin.wmPing            = None;
    _glfwWin.wmState           = None;
    _glfwWin.wmStateFullscreen = None;
    _glfwWin.wmActiveWindow    = None;

    // As the 2.x API doesn't understand multiple display devices, we hardcode
    // this choice and hope for the best
    _glfwWin.screen = DefaultScreen( _glfwLibrary.display );
    _glfwWin.root = RootWindow( _glfwLibrary.display, _glfwWin.screen );

    // Create the invisible cursor for hidden cursor mode
    _glfwWin.cursor = createNULLCursor( _glfwLibrary.display, _glfwWin.root );

    initGLXExtensions();

    // Choose the best available fbconfig
    {
        unsigned int fbcount;
        _GLFWfbconfig *fbconfigs;
        const _GLFWfbconfig *result;

        fbconfigs = getFBConfigs( &fbcount );
        if( !fbconfigs )
        {
            _glfwPlatformCloseWindow();
            return GL_FALSE;
        }

        result = _glfwChooseFBConfig( fbconfig, fbconfigs, fbcount );
        if( !result )
        {
            free( fbconfigs );
            _glfwPlatformCloseWindow();
            return GL_FALSE;
        }

        closest = *result;
        free( fbconfigs );
    }

    if( !createContext( wndconfig, (GLXFBConfigID) closest.platformID ) )
    {
        _glfwPlatformCloseWindow();
        return GL_FALSE;
    }

    if( !createWindow( width, height, wndconfig ) )
    {
        _glfwPlatformCloseWindow();
        return GL_FALSE;
    }

    if( wndconfig->mode == GLFW_FULLSCREEN )
    {
#if defined( _GLFW_HAS_XRANDR )
        // Request screen change notifications
        if( _glfwLibrary.XRandR.available )
        {
            XRRSelectInput( _glfwLibrary.display,
                            _glfwWin.window,
                            RRScreenChangeNotifyMask );
        }
#endif
        enterFullscreenMode();
    }

    // Process the window map event and any other that may have arrived
    _glfwPlatformPollEvents();

    // Retrieve and set initial cursor position
    {
        Window window, root;
        int windowX, windowY, rootX, rootY;
        unsigned int mask;

        XQueryPointer( _glfwLibrary.display,
                       _glfwWin.window,
                       &root,
                       &window,
                       &rootX, &rootY,
                       &windowX, &windowY,
                       &mask );

        // TODO: Probably check for some corner cases here.

        _glfwInput.MousePosX = windowX;
        _glfwInput.MousePosY = windowY;
    }

    // Connect the context to the window
    glXMakeCurrent( _glfwLibrary.display, _glfwWin.window, _glfwWin.context );

    return GL_TRUE;
}


//========================================================================
// Properly kill the window/video display
//========================================================================

void _glfwPlatformCloseWindow( void )
{
    if( _glfwWin.fullscreen )
    {
        leaveFullscreenMode();
    }

    if( _glfwWin.context )
    {
        // Release and destroy the context
        glXMakeCurrent( _glfwLibrary.display, None, NULL );
        glXDestroyContext( _glfwLibrary.display, _glfwWin.context );
        _glfwWin.context = NULL;
    }

    if( _glfwWin.visual )
    {
        XFree( _glfwWin.visual );
        _glfwWin.visual = NULL;
    }

    if( _glfwWin.window )
    {
        XUnmapWindow( _glfwLibrary.display, _glfwWin.window );
        XDestroyWindow( _glfwLibrary.display, _glfwWin.window );
        _glfwWin.window = (Window) 0;
    }

    if( _glfwWin.colormap )
    {
        XFreeColormap( _glfwLibrary.display, _glfwWin.colormap );
        _glfwWin.colormap = (Colormap) 0;
    }

    if( _glfwWin.cursor )
    {
        XFreeCursor( _glfwLibrary.display, _glfwWin.cursor );
        _glfwWin.cursor = (Cursor) 0;
    }
}


//========================================================================
// Set the window title
//========================================================================

void _glfwPlatformSetWindowTitle( const char *title )
{
    // Set window & icon title
    XStoreName( _glfwLibrary.display, _glfwWin.window, title );
    XSetIconName( _glfwLibrary.display, _glfwWin.window, title );
}


//========================================================================
// Set the window size
//========================================================================

void _glfwPlatformSetWindowSize( int width, int height )
{
    int     mode = 0, rate, sizeChanged = GL_FALSE;
    XSizeHints *sizehints;

    rate = _glfwWin.refreshRate;

    if( _glfwWin.fullscreen )
    {
        // Get the closest matching video mode for the specified window size
        mode = _glfwGetClosestVideoMode( _glfwWin.screen, &width, &height, &rate );
    }

    if( _glfwWin.windowNoResize )
    {
        // Update window size restrictions to match new window size

        sizehints = XAllocSizeHints();
        sizehints->flags = 0;

        sizehints->min_width  = sizehints->max_width  = width;
        sizehints->min_height = sizehints->max_height = height;

        XSetWMNormalHints( _glfwLibrary.display, _glfwWin.window, sizehints );
        XFree( sizehints );
    }

    // Change window size before changing fullscreen mode?
    if( _glfwWin.fullscreen && (width > _glfwWin.width) )
    {
        XResizeWindow( _glfwLibrary.display, _glfwWin.window, width, height );
        sizeChanged = GL_TRUE;
    }

    if( _glfwWin.fullscreen )
    {
        // Change video mode, keeping current refresh rate
        _glfwSetVideoModeMODE( _glfwWin.screen, mode, _glfwWin.refreshRate );
    }

    // Set window size (if not already changed)
    if( !sizeChanged )
    {
        XResizeWindow( _glfwLibrary.display, _glfwWin.window, width, height );
    }
}


//========================================================================
// Set the window position.
//========================================================================

void _glfwPlatformSetWindowPos( int x, int y )
{
    XMoveWindow( _glfwLibrary.display, _glfwWin.window, x, y );
}


//========================================================================
// Window iconification
//========================================================================

void _glfwPlatformIconifyWindow( void )
{
    if( _glfwWin.overrideRedirect )
    {
        // We can't iconify/restore override-redirect windows, as that's
        // performed by the window manager
        return;
    }

    XIconifyWindow( _glfwLibrary.display, _glfwWin.window, _glfwWin.screen );
}


//========================================================================
// Window un-iconification
//========================================================================

void _glfwPlatformRestoreWindow( void )
{
    if( _glfwWin.overrideRedirect )
    {
        // We can't iconify/restore override-redirect windows, as that's
        // performed by the window manager
        return;
    }

    XMapWindow( _glfwLibrary.display, _glfwWin.window );
}


//========================================================================
// Swap OpenGL buffers and poll any new events
//========================================================================

void _glfwPlatformSwapBuffers( void )
{
    // Update display-buffer
    glXSwapBuffers( _glfwLibrary.display, _glfwWin.window );
}


//========================================================================
// Set double buffering swap interval
//========================================================================

void _glfwPlatformSwapInterval( int interval )
{
    if( _glfwWin.has_GLX_SGI_swap_control )
    {
        _glfwWin.SwapIntervalSGI( interval );
    }
}


//========================================================================
// Read back framebuffer parameters from the context
//========================================================================

void _glfwPlatformRefreshWindowParams( void )
{
    int dummy;
    GLXFBConfig *fbconfig;
#if defined( _GLFW_HAS_XRANDR )
    XRRScreenConfiguration *sc;
#elif defined( _GLFW_HAS_XF86VIDMODE )
    XF86VidModeModeLine modeline;
    int dotclock;
    float pixels_per_second, pixels_per_frame;
#endif
    int attribs[] = { GLX_FBCONFIG_ID, _glfwWin.fbconfigID, None };

    if( _glfwWin.has_GLX_SGIX_fbconfig )
    {
        fbconfig = _glfwWin.ChooseFBConfigSGIX( _glfwLibrary.display,
                                                _glfwWin.screen,
                                                attribs,
                                                &dummy );
    }
    else
    {
        fbconfig = glXChooseFBConfig( _glfwLibrary.display,
                                    _glfwWin.screen,
                                    attribs,
                                    &dummy );
    }

    if( fbconfig == NULL )
    {
        // This should never ever happen
        // TODO: Figure out what to do when this happens
        fprintf( stderr, "Cannot find known GLXFBConfig by ID. "
                         "This cannot happen. Have a nice day.\n");
        abort();
    }

    // There is no clear definition of an "accelerated" context on X11/GLX, and
    // true sounds better than false, so we hardcode true here
    _glfwWin.accelerated = GL_TRUE;

    _glfwWin.redBits = getFBConfigAttrib( *fbconfig, GLX_RED_SIZE );
    _glfwWin.greenBits = getFBConfigAttrib( *fbconfig, GLX_GREEN_SIZE );
    _glfwWin.blueBits = getFBConfigAttrib( *fbconfig, GLX_BLUE_SIZE );

    _glfwWin.alphaBits = getFBConfigAttrib( *fbconfig, GLX_ALPHA_SIZE );
    _glfwWin.depthBits = getFBConfigAttrib( *fbconfig, GLX_DEPTH_SIZE );
    _glfwWin.stencilBits = getFBConfigAttrib( *fbconfig, GLX_STENCIL_SIZE );

    _glfwWin.accumRedBits = getFBConfigAttrib( *fbconfig, GLX_ACCUM_RED_SIZE );
    _glfwWin.accumGreenBits = getFBConfigAttrib( *fbconfig, GLX_ACCUM_GREEN_SIZE );
    _glfwWin.accumBlueBits = getFBConfigAttrib( *fbconfig, GLX_ACCUM_BLUE_SIZE );
    _glfwWin.accumAlphaBits = getFBConfigAttrib( *fbconfig, GLX_ACCUM_ALPHA_SIZE );

    _glfwWin.auxBuffers = getFBConfigAttrib( *fbconfig, GLX_AUX_BUFFERS );
    _glfwWin.stereo = getFBConfigAttrib( *fbconfig, GLX_STEREO ) ? 1 : 0;

    // Get FSAA buffer sample count
    if( _glfwWin.has_GLX_ARB_multisample )
    {
        _glfwWin.samples = getFBConfigAttrib( *fbconfig, GLX_SAMPLES );
    }
    else
    {
        _glfwWin.samples = 0;
    }

    // Default to refresh rate unknown (=0 according to GLFW spec)
    _glfwWin.refreshRate = 0;

    // Retrieve refresh rate if possible
#if defined( _GLFW_HAS_XRANDR )
    if( _glfwLibrary.XRandR.available )
    {
        sc = XRRGetScreenInfo( _glfwLibrary.display, _glfwWin.root );
        _glfwWin.refreshRate = XRRConfigCurrentRate( sc );
        XRRFreeScreenConfigInfo( sc );
    }
#elif defined( _GLFW_HAS_XF86VIDMODE )
    if( _glfwLibrary.XF86VidMode.available )
    {
        // Use the XF86VidMode extension to get current video mode
        XF86VidModeGetModeLine( _glfwLibrary.display, _glfwWin.screen,
                                &dotclock, &modeline );
        pixels_per_second = 1000.0f * (float) dotclock;
        pixels_per_frame  = (float) modeline.htotal * modeline.vtotal;
        _glfwWin.refreshRate = (int)(pixels_per_second/pixels_per_frame+0.5);
    }
#endif

    XFree( fbconfig );
}


//========================================================================
// Poll for new window and input events
//========================================================================

void _glfwPlatformPollEvents( void )
{
    GLboolean closeRequested = GL_FALSE;

    // Flag that the cursor has not moved
    _glfwInput.MouseMoved = GL_FALSE;

    // Process all pending events
    while( XPending( _glfwLibrary.display ) )
    {
        if( processSingleEvent() )
        {
            closeRequested = GL_TRUE;
        }
    }

    // Did we get mouse movement in fully enabled hidden cursor mode?
    if( _glfwInput.MouseMoved && _glfwWin.pointerHidden )
    {
        _glfwPlatformSetMouseCursorPos( _glfwWin.width/2,
                                        _glfwWin.height/2 );
    }

    if( closeRequested && _glfwWin.windowCloseCallback )
    {
        closeRequested = _glfwWin.windowCloseCallback();
    }
    if( closeRequested )
    {
        glfwCloseWindow();
    }
}


//========================================================================
// Wait for new window and input events
//========================================================================

void _glfwPlatformWaitEvents( void )
{
    XEvent event;

    // Block waiting for an event to arrive
    XNextEvent( _glfwLibrary.display, &event );
    XPutBackEvent( _glfwLibrary.display, &event );

    _glfwPlatformPollEvents();
}


//========================================================================
// Hide mouse cursor (lock it)
//========================================================================

void _glfwPlatformHideMouseCursor( void )
{
    // Hide cursor
    if( !_glfwWin.pointerHidden )
    {
        XDefineCursor( _glfwLibrary.display, _glfwWin.window, _glfwWin.cursor );
        _glfwWin.pointerHidden = GL_TRUE;
    }

    // Grab cursor to user window
    if( !_glfwWin.pointerGrabbed )
    {
        if( XGrabPointer( _glfwLibrary.display, _glfwWin.window, True,
                          ButtonPressMask | ButtonReleaseMask |
                          PointerMotionMask, GrabModeAsync, GrabModeAsync,
                          _glfwWin.window, None, CurrentTime ) ==
            GrabSuccess )
        {
            _glfwWin.pointerGrabbed = GL_TRUE;
        }
    }
}


//========================================================================
// Show mouse cursor (unlock it)
//========================================================================

void _glfwPlatformShowMouseCursor( void )
{
    // Un-grab cursor (only in windowed mode: in fullscreen mode we still
    // want the mouse grabbed in order to confine the cursor to the window
    // area)
    if( _glfwWin.pointerGrabbed )
    {
        XUngrabPointer( _glfwLibrary.display, CurrentTime );
        _glfwWin.pointerGrabbed = GL_FALSE;
    }

    // Show cursor
    if( _glfwWin.pointerHidden )
    {
        XUndefineCursor( _glfwLibrary.display, _glfwWin.window );
        _glfwWin.pointerHidden = GL_FALSE;
    }
}


//========================================================================
// Set physical mouse cursor position
//========================================================================

void _glfwPlatformSetMouseCursorPos( int x, int y )
{
    // Store the new position so we can recognise it later
    _glfwInput.CursorPosX = x;
    _glfwInput.CursorPosY = y;

    XWarpPointer( _glfwLibrary.display, None, _glfwWin.window, 0,0,0,0, x, y );
}

