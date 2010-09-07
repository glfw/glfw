//========================================================================
// GLFW - An OpenGL framework
// Platform:    Win32/WGL
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


//************************************************************************
//****                  GLFW internal functions                       ****
//************************************************************************

//========================================================================
// Convert BPP to RGB bits based on "best guess"
//========================================================================

static void bpp2rgb( int bpp, int *r, int *g, int *b )
{
    int delta;

    // We assume that by 32 they really meant 24
    if( bpp == 32 )
    {
        bpp = 24;
    }

    // Convert "bits per pixel" to red, green & blue sizes

    *r = *g = *b = bpp / 3;
    delta = bpp - (*r * 3);
    if( delta >= 1 )
    {
        *g = *g + 1;
    }
    if( delta == 2 )
    {
        *r = *r + 1;
    }
}


//========================================================================
// Return closest video mode by dimensions, refresh rate and bits per pixel
//========================================================================

int _glfwGetClosestVideoModeBPP( int *w, int *h, int *bpp, int *refresh )
{
    int     mode, bestmode, match, bestmatch, rr, bestrr, success;
    DEVMODE dm;

    // Find best match
    bestmatch = 0x7fffffff;
    bestrr    = 0x7fffffff;
    mode = bestmode = 0;
    do
    {
        dm.dmSize = sizeof( DEVMODE );
        success = EnumDisplaySettings( NULL, mode, &dm );
        if( success )
        {
            match = dm.dmBitsPerPel - *bpp;
            if( match < 0 ) match = -match;
            match = ( match << 25 ) |
                    ( (dm.dmPelsWidth - *w) *
                      (dm.dmPelsWidth - *w) +
                      (dm.dmPelsHeight - *h) *
                      (dm.dmPelsHeight - *h) );
            if( match < bestmatch )
            {
                bestmatch = match;
                bestmode  = mode;
                bestrr = (dm.dmDisplayFrequency - *refresh) *
                         (dm.dmDisplayFrequency - *refresh);
            }
            else if( match == bestmatch && *refresh > 0 )
            {
                rr = (dm.dmDisplayFrequency - *refresh) *
                     (dm.dmDisplayFrequency - *refresh);
                if( rr < bestrr )
                {
                    bestmatch = match;
                    bestmode  = mode;
                    bestrr    = rr;
                }
            }
        }
        mode ++;
    }
    while( success );

    // Get the parameters for the best matching display mode
    dm.dmSize = sizeof( DEVMODE );
    (void) EnumDisplaySettings( NULL, bestmode, &dm );

    // Fill out actual width and height
    *w = dm.dmPelsWidth;
    *h = dm.dmPelsHeight;

    // Return bits per pixel
    *bpp = dm.dmBitsPerPel;

    // Return vertical refresh rate
    *refresh = dm.dmDisplayFrequency;

    return bestmode;
}


//========================================================================
// Return closest video mode by dimensions, refresh rate and channel sizes
//========================================================================

static int getClosestVideoMode( int *w, int *h,
                                int *r, int *g, int *b,
                                int *refresh )
{
    int bpp, bestmode;

    // Colorbits = sum of red/green/blue bits
    bpp = *r + *g + *b;

    // If colorbits < 15 (e.g. 0) or >= 24, default to 32 bpp
    if( bpp < 15 || bpp >= 24 )
    {
        bpp = 32;
    }

    // Find best match
    bestmode = _glfwGetClosestVideoModeBPP( w, h, &bpp, refresh );

    // Convert "bits per pixel" to red, green & blue sizes
    bpp2rgb( bpp, r, g, b );

    return bestmode;
}


//========================================================================
// Change the current video mode
//========================================================================

void _glfwSetVideoModeMODE( int mode )
{
    DEVMODE dm;
    int success;

    // Get the parameters for the best matching display mode
    dm.dmSize = sizeof( DEVMODE );
    (void) EnumDisplaySettings( NULL, mode, &dm );

    // Set which fields we want to specify
    dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

    // Do we have a prefered refresh rate?
    if( _glfwWin.desiredRefreshRate > 0 )
    {
        dm.dmFields = dm.dmFields | DM_DISPLAYFREQUENCY;
        dm.dmDisplayFrequency = _glfwWin.desiredRefreshRate;
    }

    // Change display setting
    dm.dmSize = sizeof( DEVMODE );
    success = ChangeDisplaySettings( &dm, CDS_FULLSCREEN );

    // If the mode change was not possible, query the current display
    // settings (we'll use the desktop resolution for fullscreen mode)
    if( success == DISP_CHANGE_SUCCESSFUL )
    {
        _glfwWin.modeID = mode;
    }
    else
    {
        _glfwWin.modeID = ENUM_REGISTRY_SETTINGS;
        EnumDisplaySettings( NULL, ENUM_REGISTRY_SETTINGS, &dm );
    }

    // Set the window size to that of the display mode
    _glfwWin.width  = dm.dmPelsWidth;
    _glfwWin.height = dm.dmPelsHeight;
}


//========================================================================
// _glfwSetVideoMode() - Change the current video mode
//========================================================================

void _glfwSetVideoMode( int *w, int *h, int r, int g, int b, int refresh )
{
    int     bestmode;

    // Find a best match mode
    bestmode = getClosestVideoMode( w, h, &r, &g, &b, &refresh );

    // Change mode
    _glfwSetVideoModeMODE( bestmode );
}


//************************************************************************
//****                    GLFW user functions                         ****
//************************************************************************

//========================================================================
// _glfwPlatformGetVideoModes() - Get a list of available video modes
//========================================================================

int _glfwPlatformGetVideoModes( GLFWvidmode *list, int maxcount )
{
    int count, success, mode, i, j;
    int m1, m2, bpp, r, g, b;
    DEVMODE dm;

    // Loop through all video modes and extract all the UNIQUE modes
    count = 0;
    mode  = 0;
    do
    {
        // Get video mode properties
        dm.dmSize = sizeof( DEVMODE );
        success = EnumDisplaySettings( NULL, mode, &dm );

        // Is it a valid mode? (only list depths >= 15 bpp)
        if( success && dm.dmBitsPerPel >= 15 )
        {
            // Convert to RGB, and back to bpp ("mask out" alpha bits etc)
            bpp2rgb( dm.dmBitsPerPel, &r, &g, &b );
            bpp = r + g + b;

            // Mode "code" for this mode
            m1 = (bpp << 25) | (dm.dmPelsWidth * dm.dmPelsHeight);

            // Insert mode in list (sorted), and avoid duplicates
            for( i = 0; i < count; i ++ )
            {
                // Mode "code" for already listed mode
                bpp = list[i].RedBits + list[i].GreenBits +
                      list[i].BlueBits;
                m2 = (bpp << 25) | (list[i].Width * list[i].Height);
                if( m1 <= m2 )
                {
                    break;
                }
            }

            // New entry at the end of the list?
            if( i >= count )
            {
                list[count].Width     = dm.dmPelsWidth;
                list[count].Height    = dm.dmPelsHeight;
                list[count].RedBits   = r;
                list[count].GreenBits = g;
                list[count].BlueBits  = b;
                count ++;
            }
            // Insert new entry in the list?
            else if( m1 < m2 )
            {
                for( j = count; j > i; j -- )
                {
                    list[j] = list[j-1];
                }
                list[i].Width     = dm.dmPelsWidth;
                list[i].Height    = dm.dmPelsHeight;
                list[i].RedBits   = r;
                list[i].GreenBits = g;
                list[i].BlueBits  = b;
                count ++;
            }
        }
        mode ++;
    }
    while( success && (count < maxcount) );

    return count;
}


//========================================================================
// Get the desktop video mode
//========================================================================

void _glfwPlatformGetDesktopMode( GLFWvidmode *mode )
{
    DEVMODE dm;

    // Get desktop display mode
    dm.dmSize = sizeof( DEVMODE );
    (void) EnumDisplaySettings( NULL, ENUM_REGISTRY_SETTINGS, &dm );

    // Return desktop mode parameters
    mode->Width  = dm.dmPelsWidth;
    mode->Height = dm.dmPelsHeight;
    bpp2rgb( dm.dmBitsPerPel, &mode->RedBits, &mode->GreenBits, &mode->BlueBits );
}

