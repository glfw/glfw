//========================================================================
// This is a small test application for GLFW.
// The program lists all available fullscreen video modes.
//========================================================================

#include <stdio.h>
#include <GL/glfw3.h>

// Maximum number of modes that we want to list
#define MAX_NUM_MODES 400


//========================================================================
// main()
//========================================================================

int main( void )
{
    GLFWvidmode dtmode, modes[ MAX_NUM_MODES ];
    int     modecount, i;

    // Initialize GLFW
    if( !glfwInit() )
    {
        return 0;
    }

    // Show desktop video mode
    glfwGetDesktopMode( &dtmode );
    printf( "Desktop mode: %d x %d x %d\n\n",
            dtmode.width, dtmode.height, dtmode.redBits +
            dtmode.greenBits + dtmode.blueBits );

    // List available video modes
    modecount = glfwGetVideoModes( modes, MAX_NUM_MODES );
    printf( "Available modes:\n" );
    for( i = 0; i < modecount; i ++ )
    {
        printf( "%3d: %d x %d x %d\n", i,
                modes[i].width, modes[i].height, modes[i].redBits +
                modes[i].greenBits + modes[i].blueBits );
    }

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
