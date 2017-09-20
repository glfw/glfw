///////////////////////////////////////////////////////////////////////////////
// main.cpp
// ========
// testing Pixel Buffer Object for packing (read-back) pixel data from
// framebuffer to a PBO using GL_ARB_pixel_buffer_object extension
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2007-11-30
// UPDATED: 2014-05-13
///////////////////////////////////////////////////////////////////////////////

// in order to get function prototypes from glext.h, define GL_GLEXT_PROTOTYPES before including glext.h
#define GL_GLEXT_PROTOTYPES

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "glext.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include "glInfo.h"                             // glInfo struct
#include "Timer.h"



// GLUT CALLBACK functions ////////////////////////////////////////////////////
void displayCB();
void displayFirstCB();
void reshapeCB(int w, int h);
void timerCB(int millisec);
void idleCB();
void keyboardCB(unsigned char key, int x, int y);
void mouseCB(int button, int stat, int x, int y);
void mouseMotionCB(int x, int y);

// CALLBACK function when exit() called ///////////////////////////////////////
void exitCB();

void initGL();
int  initGLUT(int argc, char **argv);
bool initSharedMem();
void clearSharedMem();
void initLights();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void drawString(const char *str, int x, int y, float color[4], void *font);
void drawString3D(const char *str, float pos[3], float color[4], void *font);
void showInfo();
void showTransferRate();
void printTransferRate();
void draw();
void add(unsigned char* src, int width, int height, int shift, unsigned char* dst);
void toOrtho();
void toPerspective();


// constants
const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 256;
const float CAMERA_DISTANCE = 5.0f;
const int CHANNEL_COUNT = 4;
const int DATA_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * CHANNEL_COUNT;
const GLenum PIXEL_FORMAT = GL_BGRA;
const int PBO_COUNT = 2;

// global variables
void *font = GLUT_BITMAP_8_BY_13;
GLuint pboIds[PBO_COUNT];           // IDs of PBOs
bool mouseLeftDown;
bool mouseRightDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
bool pboSupported;
bool pboUsed;
int drawMode = 0;
Timer timer, t1;
float readTime, processTime;
GLubyte* colorBuffer = 0;


// function pointers for PBO Extension
// Windows needs to get function pointers from ICD OpenGL drivers,
// because opengl32.dll does not support extensions higher than v1.1.
#ifdef _WIN32
PFNGLGENBUFFERSARBPROC pglGenBuffersARB = 0;                     // VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC pglBindBufferARB = 0;                     // VBO Bind Procedure
PFNGLBUFFERDATAARBPROC pglBufferDataARB = 0;                     // VBO Data Loading Procedure
PFNGLBUFFERSUBDATAARBPROC pglBufferSubDataARB = 0;               // VBO Sub Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC pglDeleteBuffersARB = 0;               // VBO Deletion Procedure
PFNGLGETBUFFERPARAMETERIVARBPROC pglGetBufferParameterivARB = 0; // return various parameters of VBO
PFNGLMAPBUFFERARBPROC pglMapBufferARB = 0;                       // map VBO procedure
PFNGLUNMAPBUFFERARBPROC pglUnmapBufferARB = 0;                   // unmap VBO procedure
#define glGenBuffersARB           pglGenBuffersARB
#define glBindBufferARB           pglBindBufferARB
#define glBufferDataARB           pglBufferDataARB
#define glBufferSubDataARB        pglBufferSubDataARB
#define glDeleteBuffersARB        pglDeleteBuffersARB
#define glGetBufferParameterivARB pglGetBufferParameterivARB
#define glMapBufferARB            pglMapBufferARB
#define glUnmapBufferARB          pglUnmapBufferARB
#endif


// function pointers for WGL_EXT_swap_control
#ifdef _WIN32
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef int (WINAPI * PFNWGLGETSWAPINTERVALEXTPROC) (void);
PFNWGLSWAPINTERVALEXTPROC pwglSwapIntervalEXT = 0;
PFNWGLGETSWAPINTERVALEXTPROC pwglGetSwapIntervalEXT = 0;
#define wglSwapIntervalEXT      pwglSwapIntervalEXT
#define wglGetSwapIntervalEXT   pwglGetSwapIntervalEXT
#endif


///////////////////////////////////////////////////////////////////////////////
// draw a cube with immediate mode
// 54 calls = 24 glVertex*() calls + 24 glColor*() calls + 6 glNormal*() calls
///////////////////////////////////////////////////////////////////////////////
void draw()
{
    glBegin(GL_QUADS);
        // face v0-v1-v2-v3
        glNormal3f(0,0,1);
        glColor3f(1,1,1);
        glVertex3f(1,1,1);
        glColor3f(1,1,0);
        glVertex3f(-1,1,1);
        glColor3f(1,0,0);
        glVertex3f(-1,-1,1);
        glColor3f(1,0,1);
        glVertex3f(1,-1,1);

        // face v0-v3-v4-v6
        glNormal3f(1,0,0);
        glColor3f(1,1,1);
        glVertex3f(1,1,1);
        glColor3f(1,0,1);
        glVertex3f(1,-1,1);
        glColor3f(0,0,1);
        glVertex3f(1,-1,-1);
        glColor3f(0,1,1);
        glVertex3f(1,1,-1);

        // face v0-v5-v6-v1
        glNormal3f(0,1,0);
        glColor3f(1,1,1);
        glVertex3f(1,1,1);
        glColor3f(0,1,1);
        glVertex3f(1,1,-1);
        glColor3f(0,1,0);
        glVertex3f(-1,1,-1);
        glColor3f(1,1,0);
        glVertex3f(-1,1,1);

        // face  v1-v6-v7-v2
        glNormal3f(-1,0,0);
        glColor3f(1,1,0);
        glVertex3f(-1,1,1);
        glColor3f(0,1,0);
        glVertex3f(-1,1,-1);
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);
        glColor3f(1,0,0);
        glVertex3f(-1,-1,1);

        // face v7-v4-v3-v2
        glNormal3f(0,-1,0);
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);
        glColor3f(0,0,1);
        glVertex3f(1,-1,-1);
        glColor3f(1,0,1);
        glVertex3f(1,-1,1);
        glColor3f(1,0,0);
        glVertex3f(-1,-1,1);

        // face v4-v7-v6-v5
        glNormal3f(0,0,-1);
        glColor3f(0,0,1);
        glVertex3f(1,-1,-1);
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);
        glColor3f(0,1,0);
        glVertex3f(-1,1,-1);
        glColor3f(0,1,1);
        glVertex3f(1,1,-1);
    glEnd();
}



///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    initSharedMem();

    // register exit callback
    atexit(exitCB);

    // init GLUT and GL
    initGLUT(argc, argv);
    initGL();

    // get OpenGL info
    glInfo glInfo;
    glInfo.getInfo();
    glInfo.printSelf();

#ifdef _WIN32
    // check PBO is supported by your video card
    if(glInfo.isExtensionSupported("GL_ARB_pixel_buffer_object"))
    {
        // get pointers to GL functions
        glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
        glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
        glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
        glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");
        glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
        glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)wglGetProcAddress("glGetBufferParameterivARB");
        glMapBufferARB = (PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
        glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");

        // check once again PBO extension
        if(glGenBuffersARB && glBindBufferARB && glBufferDataARB && glBufferSubDataARB &&
           glMapBufferARB && glUnmapBufferARB && glDeleteBuffersARB && glGetBufferParameterivARB)
        {
            pboSupported = pboUsed = true;
            std::cout << "Video card supports GL_ARB_pixel_buffer_object." << std::endl;
        }
        else
        {
            pboSupported = pboUsed = false;
            std::cout << "Video card does NOT support GL_ARB_pixel_buffer_object." << std::endl;
        }
    }

    // check EXT_swap_control is supported
    if(glInfo.isExtensionSupported("WGL_EXT_swap_control"))
    {
        // get pointers to WGL functions
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
        if(wglSwapIntervalEXT && wglGetSwapIntervalEXT)
        {
            // disable v-sync
            wglSwapIntervalEXT(0);
            std::cout << "Video card supports WGL_EXT_swap_control." << std::endl;
        }
    }

#else // for linux, do not need to get function pointers, it is up-to-date
    if(glInfo.isExtensionSupported("GL_ARB_pixel_buffer_object"))
    {
        pboSupported = pboUsed = true;
        std::cout << "Video card supports GL_ARB_pixel_buffer_object." << std::endl;
    }
    else
    {
        pboSupported = pboUsed = false;
        std::cout << "Video card does NOT support GL_ARB_pixel_buffer_object." << std::endl;
    }
#endif

    if(pboSupported)
    {
        // create 2 pixel buffer objects, you need to delete them when program exits.
        // glBufferDataARB with NULL pointer reserves only memory space.
        glGenBuffersARB(PBO_COUNT, pboIds);
        glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[0]);
        glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, DATA_SIZE, 0, GL_STREAM_READ_ARB);
        glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[1]);
        glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, DATA_SIZE, 0, GL_STREAM_READ_ARB);

        glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
    }

    // start timer, the elapsed time will be used for updateVertices()
    timer.start();

    // the last GLUT call (LOOP)
    // window will be shown and display callback is triggered by events
    // NOTE: this call never return main().
    glutMainLoop(); /* Start GLUT event-processing loop */

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// initialize GLUT for windowing
///////////////////////////////////////////////////////////////////////////////
int initGLUT(int argc, char **argv)
{
    // GLUT stuff for windowing
    // initialization openGL window.
    // it is called before any other GLUT routine
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_ALPHA); // display mode

    glutInitWindowSize(SCREEN_WIDTH*2, SCREEN_HEIGHT);    // window size

    glutInitWindowPosition(100, 100);           // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow(argv[0]);     // param is the title of window

    // register GLUT callback functions
    glutDisplayFunc(displayFirstCB);            // we will render normal drawing first
                                                // so, the first frame has a valid content
    //glutTimerFunc(33, timerCB, 33);             // redraw only every given millisec
    glutIdleFunc(idleCB);                       // redraw only every given millisec
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);

    return handle;
}



///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void initGL()
{
    glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();
    //setCamera(0, 0, 8, 0, 0, 0);
}



///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// draw a string in 3D space
///////////////////////////////////////////////////////////////////////////////
void drawString3D(const char *str, float pos[3], float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos3fv(pos);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initSharedMem()
{
    cameraAngleX = cameraAngleY = 0.0f;
    cameraDistance = CAMERA_DISTANCE;

    mouseX = mouseY = 0;
    mouseLeftDown = mouseRightDown = false;

    drawMode = 0; // 0:fill, 1:wireframe, 2:point

    // allocate buffers to store frames
    colorBuffer = new GLubyte[DATA_SIZE];
    memset(colorBuffer, 255, DATA_SIZE);

    return true;
}



///////////////////////////////////////////////////////////////////////////////
// clean up shared memory
///////////////////////////////////////////////////////////////////////////////
void clearSharedMem()
{
    // deallocate frame buffer
    delete [] colorBuffer;
    colorBuffer = 0;

    // clean up PBOs
    if(pboSupported)
    {
        glDeleteBuffersARB(PBO_COUNT, pboIds);
    }
}



///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0, 0, 20, 1}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}



///////////////////////////////////////////////////////////////////////////////
// display info messages
///////////////////////////////////////////////////////////////////////////////
void showInfo()
{
    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);     // switch to projection matrix
    glPushMatrix();                  // save current projection matrix
    glLoadIdentity();                // reset projection matrix
    gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);  // set to orthogonal projection

    const int FONT_HEIGHT = 14;
    float color[4] = {1, 1, 1, 1};

    std::stringstream ss;
    ss << "PBO: ";
    if(pboUsed)
        ss << "on" << std::ends;
    else
        ss << "off" << std::ends;

    drawString(ss.str().c_str(), 1, SCREEN_HEIGHT-FONT_HEIGHT, color, font);
    ss.str(""); // clear buffer

    ss << "Read Time: " << readTime << " ms" << std::ends;
    drawString(ss.str().c_str(), 1, SCREEN_HEIGHT-(2*FONT_HEIGHT), color, font);
    ss.str("");

    ss << std::fixed << std::setprecision(3);
    ss << "Process Time: " << processTime << " ms" << std::ends;
    drawString(ss.str().c_str(), 1, SCREEN_HEIGHT-(3*FONT_HEIGHT), color, font);
    ss.str("");

    ss << "Press SPACE to toggle PBO." << std::ends;
    drawString(ss.str().c_str(), 1, 1, color, font);

    // unset floating format
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // restore projection matrix
    glPopMatrix();                   // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
    glPopMatrix();                   // restore to previous modelview matrix
}



///////////////////////////////////////////////////////////////////////////////
// display transfer rates
///////////////////////////////////////////////////////////////////////////////
void showTransferRate()
{
    static Timer timer;
    static int count = 0;
    static std::stringstream ss;

    // update fps every second
    ++count;
    double elapsedTime = timer.getElapsedTime();
    if(elapsedTime > 1.0)
    {
        ss.str("");
        ss << std::fixed << std::setprecision(1);
        ss << "Transfer Rate: " << (count / elapsedTime) * (SCREEN_WIDTH * SCREEN_HEIGHT) / (1024 * 1024) << " Mp" << std::ends; // update fps string
        ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
        count = 0;                      // reset counter
        timer.start();                  // restart timer
    }

    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT); // set to orthogonal projection

    float color[4] = {1, 1, 0, 1};
    drawString(ss.str().c_str(), 200, 286, color, font);

    // restore projection matrix
    glPopMatrix();                      // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);         // switch to modelview matrix
    glPopMatrix();                      // restore to previous modelview matrix
}



///////////////////////////////////////////////////////////////////////////////
// print transfer rates
///////////////////////////////////////////////////////////////////////////////
void printTransferRate()
{
    const double INV_MEGA = 1.0 / (1024 * 1024);
    static Timer timer;
    static int count = 0;
    static std::stringstream ss;
    double elapsedTime;

    // loop until 1 sec passed
    elapsedTime = timer.getElapsedTime();
    if(elapsedTime < 1.0)
    {
        ++count;
    }
    else
    {
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "Transfer Rate: " << (count / elapsedTime) * (SCREEN_WIDTH * SCREEN_HEIGHT) * INV_MEGA << " Mpixels/s. (" << count / elapsedTime << " FPS)\n";
        std::cout << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
        count = 0;                      // reset counter
        timer.start();                  // restart timer
    }
}


///////////////////////////////////////////////////////////////////////////////
// change the brightness
///////////////////////////////////////////////////////////////////////////////
void add(unsigned char* src, int width, int height, int shift, unsigned char* dst)
{
    if(!src || !dst)
        return;

    int value;
    for(int i = 0; i < height; ++i)
    {
        for(int j = 0; j < width; ++j)
        {
            value = *src + shift;
            if(value > 255) *dst = (unsigned char)255;
            else            *dst = (unsigned char)value;
            ++src;
            ++dst;

            value = *src + shift;
            if(value > 255) *dst = (unsigned char)255;
            else            *dst = (unsigned char)value;
            ++src;
            ++dst;

            value = *src + shift;
            if(value > 255) *dst = (unsigned char)255;
            else            *dst = (unsigned char)value;
            ++src;
            ++dst;

            ++src;    // skip alpha
            ++dst;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// set projection matrix as orthogonal
///////////////////////////////////////////////////////////////////////////////
void toOrtho()
{
    // set viewport to be the entire window
    glViewport((GLsizei)SCREEN_WIDTH, 0, (GLsizei)SCREEN_WIDTH, (GLsizei)SCREEN_HEIGHT);

    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, -1, 1);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



///////////////////////////////////////////////////////////////////////////////
// set the projection matrix as perspective
///////////////////////////////////////////////////////////////////////////////
void toPerspective()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)SCREEN_WIDTH, (GLsizei)SCREEN_HEIGHT);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(SCREEN_WIDTH)/SCREEN_HEIGHT, 0.1f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}






//=============================================================================
// CALLBACKS
//=============================================================================

void displayCB()
{
    static int shift = 0;
    static int index = 0;
    int nextIndex = 0;                  // pbo index used for next frame

    // brightness shift amount
    shift = ++shift % 200;

    // increment current index first then get the next index
    // "index" is used to read pixels from a framebuffer to a PBO
    // "nextIndex" is used to process pixels in the other PBO
    index = (index + 1) % 2;
    nextIndex = (index + 1) % 2;

    // set the framebuffer to read
    glReadBuffer(GL_FRONT);

    if(pboUsed) // with PBO
    {
        // read framebuffer ///////////////////////////////
        t1.start();

        // copy pixels from framebuffer to PBO
        // Use offset instead of ponter.
        // OpenGL should perform asynch DMA transfer, so glReadPixels() will return immediately.
        glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[index]);
        glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, 0);

        // measure the time reading framebuffer
        t1.stop();
        readTime = t1.getElapsedTimeInMilliSec();
        ///////////////////////////////////////////////////

        // process pixel data /////////////////////////////
        t1.start();

        // map the PBO that contain framebuffer pixels before processing it
        glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[nextIndex]);
        GLubyte* src = (GLubyte*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);
        if(src)
        {
            // change brightness
            add(src, SCREEN_WIDTH, SCREEN_HEIGHT, shift, colorBuffer);
            glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);     // release pointer to the mapped buffer
        }

        // measure the time reading framebuffer
        t1.stop();
        processTime = t1.getElapsedTimeInMilliSec();
        ///////////////////////////////////////////////////

        glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
    }

    else        // without PBO
    {
        // read framebuffer ///////////////////////////////
        t1.start();

        glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, colorBuffer);

        // measure the time reading framebuffer
        t1.stop();
        readTime = t1.getElapsedTimeInMilliSec();
        ///////////////////////////////////////////////////

        // covert to greyscale ////////////////////////////
        t1.start();

        // change brightness
        add(colorBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, shift, colorBuffer);

        // measure the time reading framebuffer
        t1.stop();
        processTime = t1.getElapsedTimeInMilliSec();
        ///////////////////////////////////////////////////
    }

    // render to the framebuffer //////////////////////////
    glDrawBuffer(GL_BACK);
    toPerspective(); // set to perspective on the left side of the window

    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // tramsform camera
    glTranslatef(0, 0, -cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0);   // pitch
    glRotatef(cameraAngleY, 0, 1, 0);   // heading

    // draw a cube
    glPushMatrix();
    draw();
    glPopMatrix();

    // draw the read color buffer to the right side of the window
    toOrtho();      // set to orthographic on the right side of the window
    glRasterPos2i(0, 0);
    glDrawPixels(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, colorBuffer);

    // draw info messages
    showInfo();
    printTransferRate();

    glutSwapBuffers();
}


void displayFirstCB()
{
    // normal drawing to the framebuffer, so, the first call of glReadPixels()
    // in displayCB() will get valid content

    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // tramsform camera
    glTranslatef(0, 0, -cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0);   // pitch
    glRotatef(cameraAngleY, 0, 1, 0);   // heading

    // draw a cube
    glPushMatrix();
    draw();
    glPopMatrix();

    // draw the read color buffer to the right side of the window
    toOrtho();      // set to orthographic on the right side of the window
    glRasterPos2i(0, 0);
    glDrawPixels(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, colorBuffer);

    // draw info messages
    showInfo();
    printTransferRate();

    glutSwapBuffers();

    // switch to read-back drawing callback function
    glutDisplayFunc(displayCB);
}


void reshapeCB(int w, int h)
{
    toPerspective();
}


void timerCB(int millisec)
{
    glutTimerFunc(millisec, timerCB, millisec);
    glutPostRedisplay();
}


void idleCB()
{
    glutPostRedisplay();
}


void keyboardCB(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27: // ESCAPE
        exit(0);
        break;

    case ' ':
        if(pboSupported)
            pboUsed = !pboUsed;
        std::cout << "PBO mode: " << (pboUsed ? "on" : "off") << std::endl;
         break;

    case 'd': // switch rendering modes (fill -> wire -> point)
    case 'D':
        drawMode = ++drawMode % 3;
        if(drawMode == 0)        // fill mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
        else if(drawMode == 1)  // wireframe mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        else                    // point mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        break;

    default:
        ;
    }
}


void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if(button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if(state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if(button == GLUT_RIGHT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if(state == GLUT_UP)
            mouseRightDown = false;
    }
}


void mouseMotionCB(int x, int y)
{
    if(mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown)
    {
        cameraDistance -= (y - mouseY) * 0.2f;
        if(cameraDistance < 2.0f)
            cameraDistance = 2.0f;

        mouseY = y;
    }
}



void exitCB()
{
    clearSharedMem();
}
