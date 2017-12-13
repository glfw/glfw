///////////////////////////////////////////////////////////////////////////////
// main.cpp
// ========
// testing Frame Buffer Object (FBO) for "Render To Texture" (stencil buffer)
// OpenGL draws the scene directly to a texture object.
//
// GL_EXT_framebuffer_object extension is promoted to a core feature of OpenGL
// version 3.0 (GL_ARB_framebuffer_object)
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2008-05-16
// UPDATED: 2016-11-14
///////////////////////////////////////////////////////////////////////////////

// in order to get function prototypes from glext.h, define GL_GLEXT_PROTOTYPES before including glext.h
#define GL_GLEXT_PROTOTYPES

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include "glext.h"                              // OpenGL extensions
#include "glInfo.h"                             // glInfo struct
#include "Timer.h"
#include "teapot.h"

using std::stringstream;
using std::string;
using std::cout;
using std::endl;
using std::ends;


// GLUT CALLBACK functions ////////////////////////////////////////////////////
void displayCB();
void reshapeCB(int w, int h);
void timerCB(int millisec);
void idleCB();
void keyboardCB(unsigned char key, int x, int y);
void mouseCB(int button, int stat, int x, int y);
void mouseMotionCB(int x, int y);

// CALLBACK function when exit() called ///////////////////////////////////////
void exitCB();

// function declearations /////////////////////////////////////////////////////
void initGL();
int  initGLUT(int argc, char **argv);
bool initSharedMem();
void clearSharedMem();
void initLights();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void drawString(const char *str, int x, int y, float color[4], void *font);
void drawString3D(const char *str, float pos[3], float color[4], void *font);
void showInfo();
void showFPS();
void toOrtho();
void toPerspective();
void draw();

// FBO utils
bool checkFramebufferStatus(GLuint fbo);
void printFramebufferInfo(GLuint fbo);
std::string convertInternalFormatToString(GLenum format);
std::string getTextureParameters(GLuint id);
std::string getRenderbufferParameters(GLuint id);


// constants
const int   SCREEN_WIDTH    = 400;
const int   SCREEN_HEIGHT   = 300;
const float CAMERA_DISTANCE = 6.0f;
const int   TEXT_WIDTH      = 8;
const int   TEXT_HEIGHT     = 13;
const int   TEXTURE_WIDTH   = 256;  // NOTE: texture size cannot be larger than
const int   TEXTURE_HEIGHT  = 256;  // the rendering window size in non-FBO mode

// global variables
void *font = GLUT_BITMAP_8_BY_13;
GLuint fboId;                       // ID of FBO
GLuint textureId;                   // ID of texture
GLuint rboId;                       // ID of Renderbuffer object for depth & stencil
int screenWidth;
int screenHeight;
bool mouseLeftDown;
bool mouseRightDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
bool fboSupported;
bool fboUsed;
int drawMode;
Timer timer, t1;
float playTime;                     // to compute rotation angle
float renderToTextureTime;          // elapsed time for render-to-texture


// function pointers for FBO extension
// Windows needs to get function pointers from ICD OpenGL drivers,
// because opengl32.dll does not support extensions higher than v1.1.
#ifdef _WIN32
// ARB Framebuffer object
PFNGLGENFRAMEBUFFERSPROC                     pglGenFramebuffers = 0;                      // FBO name generation procedure
PFNGLDELETEFRAMEBUFFERSPROC                  pglDeleteFramebuffers = 0;                   // FBO deletion procedure
PFNGLBINDFRAMEBUFFERPROC                     pglBindFramebuffer = 0;                      // FBO bind procedure
PFNGLCHECKFRAMEBUFFERSTATUSPROC              pglCheckFramebufferStatus = 0;               // FBO completeness test procedure
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC pglGetFramebufferAttachmentParameteriv = 0;  // return various FBO parameters
PFNGLGENERATEMIPMAPPROC                      pglGenerateMipmap = 0;                       // FBO automatic mipmap generation procedure
PFNGLFRAMEBUFFERTEXTURE1DPROC                pglFramebufferTexture1D = 0;                 // FBO texdture attachement procedure
PFNGLFRAMEBUFFERTEXTURE2DPROC                pglFramebufferTexture2D = 0;                 // FBO texdture attachement procedure
PFNGLFRAMEBUFFERTEXTURE3DPROC                pglFramebufferTexture3D = 0;                 // FBO texdture attachement procedure
PFNGLFRAMEBUFFERTEXTURELAYERPROC             pglFramebufferTextureLayer = 0;              // FBO texdture attachement procedure
PFNGLFRAMEBUFFERRENDERBUFFERPROC             pglFramebufferRenderbuffer = 0;              // FBO renderbuffer attachement procedure
PFNGLISFRAMEBUFFERPROC                       pglIsFramebuffer = 0;                        // FBO state = true/false
PFNGLBLITFRAMEBUFFERPROC                     pglBlitFramebuffer = 0;                      // FBO copy
// Renderbuffer object
PFNGLGENRENDERBUFFERSPROC                    pglGenRenderbuffers = 0;                     // renderbuffer generation procedure
PFNGLDELETERENDERBUFFERSPROC                 pglDeleteRenderbuffers = 0;                  // renderbuffer deletion procedure
PFNGLBINDRENDERBUFFERPROC                    pglBindRenderbuffer = 0;                     // renderbuffer bind procedure
PFNGLRENDERBUFFERSTORAGEPROC                 pglRenderbufferStorage = 0;                  // renderbuffer memory allocation procedure
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC      pglRenderbufferStorageMultisample = 0;       // renderbuffer memory allocation with multisample
PFNGLGETRENDERBUFFERPARAMETERIVPROC          pglGetRenderbufferParameteriv = 0;           // return various renderbuffer parameters
PFNGLISRENDERBUFFERPROC                      pglIsRenderbuffer = 0;                       // determine renderbuffer object type

#define glGenFramebuffers                        pglGenFramebuffers
#define glDeleteFramebuffers                     pglDeleteFramebuffers
#define glBindFramebuffer                        pglBindFramebuffer
#define glCheckFramebufferStatus                 pglCheckFramebufferStatus
#define glGetFramebufferAttachmentParameteriv    pglGetFramebufferAttachmentParameteriv
#define glGenerateMipmap                         pglGenerateMipmap
#define glFramebufferTexture1D                   pglFramebufferTexture1D
#define glFramebufferTexture2D                   pglFramebufferTexture2D
#define glFramebufferTexture3D                   pglFramebufferTexture3D
#define glFramebufferTextureLayer                pglFramebufferTextureLayer
#define glFramebufferRenderbuffer                pglFramebufferRenderbuffer
#define glIsFramebuffer                          pglIsFramebuffer
#define glBlitFramebuffer                        pglBlitFramebuffer

#define glGenRenderbuffers                       pglGenRenderbuffers
#define glDeleteRenderbuffers                    pglDeleteRenderbuffers
#define glBindRenderbuffer                       pglBindRenderbuffer
#define glRenderbufferStorage                    pglRenderbufferStorage
#define glRenderbufferStorageMultisample         pglRenderbufferStorageMultisample
#define glGetRenderbufferParameteriv             pglGetRenderbufferParameteriv
#define glIsRenderbuffer                         pglIsRenderbuffer

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
// draw a textured cube
///////////////////////////////////////////////////////////////////////////////
void draw()
{
    glBindTexture(GL_TEXTURE_2D, textureId);

    glColor4f(1, 1, 1, 1);
    glBegin(GL_TRIANGLES);
        // front faces
        glNormal3f(0,0,1);
        // face v0-v1-v2
        glTexCoord2f(1,1);  glVertex3f(1,1,1);
        glTexCoord2f(0,1);  glVertex3f(-1,1,1);
        glTexCoord2f(0,0);  glVertex3f(-1,-1,1);
        // face v2-v3-v0
        glTexCoord2f(0,0);  glVertex3f(-1,-1,1);
        glTexCoord2f(1,0);  glVertex3f(1,-1,1);
        glTexCoord2f(1,1);  glVertex3f(1,1,1);

        // right faces
        glNormal3f(1,0,0);
        // face v0-v3-v4
        glTexCoord2f(0,1);  glVertex3f(1,1,1);
        glTexCoord2f(0,0);  glVertex3f(1,-1,1);
        glTexCoord2f(1,0);  glVertex3f(1,-1,-1);
        // face v4-v5-v0
        glTexCoord2f(1,0);  glVertex3f(1,-1,-1);
        glTexCoord2f(1,1);  glVertex3f(1,1,-1);
        glTexCoord2f(0,1);  glVertex3f(1,1,1);

        // top faces
        glNormal3f(0,1,0);
        // face v0-v5-v6
        glTexCoord2f(1,0);  glVertex3f(1,1,1);
        glTexCoord2f(1,1);  glVertex3f(1,1,-1);
        glTexCoord2f(0,1);  glVertex3f(-1,1,-1);
        // face v6-v1-v0
        glTexCoord2f(0,1);  glVertex3f(-1,1,-1);
        glTexCoord2f(0,0);  glVertex3f(-1,1,1);
        glTexCoord2f(1,0);  glVertex3f(1,1,1);

        // left faces
        glNormal3f(-1,0,0);
        // face  v1-v6-v7
        glTexCoord2f(1,1);  glVertex3f(-1,1,1);
        glTexCoord2f(0,1);  glVertex3f(-1,1,-1);
        glTexCoord2f(0,0);  glVertex3f(-1,-1,-1);
        // face v7-v2-v1
        glTexCoord2f(0,0);  glVertex3f(-1,-1,-1);
        glTexCoord2f(1,0);  glVertex3f(-1,-1,1);
        glTexCoord2f(1,1);  glVertex3f(-1,1,1);

        // bottom faces
        glNormal3f(0,-1,0);
        // face v7-v4-v3
        glTexCoord2f(0,0);  glVertex3f(-1,-1,-1);
        glTexCoord2f(1,0);  glVertex3f(1,-1,-1);
        glTexCoord2f(1,1);  glVertex3f(1,-1,1);
        // face v3-v2-v7
        glTexCoord2f(1,1);  glVertex3f(1,-1,1);
        glTexCoord2f(0,1);  glVertex3f(-1,-1,1);
        glTexCoord2f(0,0);  glVertex3f(-1,-1,-1);

        // back faces
        glNormal3f(0,0,-1);
        // face v4-v7-v6
        glTexCoord2f(0,0);  glVertex3f(1,-1,-1);
        glTexCoord2f(1,0);  glVertex3f(-1,-1,-1);
        glTexCoord2f(1,1);  glVertex3f(-1,1,-1);
        // face v6-v5-v4
        glTexCoord2f(1,1);  glVertex3f(-1,1,-1);
        glTexCoord2f(0,1);  glVertex3f(1,1,-1);
        glTexCoord2f(0,0);  glVertex3f(1,-1,-1);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}



///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // init global vars
    initSharedMem();

    // register exit callback
    atexit(exitCB);

    // init GLUT and GL
    initGLUT(argc, argv);
    initGL();

    // create a texture object
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap generation included in OpenGL v1.4
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // get OpenGL info
    glInfo glInfo;
    glInfo.getInfo();
    glInfo.printSelf();

#ifdef _WIN32
    // check if FBO is supported by your video card
    if(glInfo.isExtensionSupported("GL_ARB_framebuffer_object"))
    {
        // get pointers to GL functions
        glGenFramebuffers                     = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
        glDeleteFramebuffers                  = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
        glBindFramebuffer                     = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
        glCheckFramebufferStatus              = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
        glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
        glGenerateMipmap                      = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
        glFramebufferTexture1D                = (PFNGLFRAMEBUFFERTEXTURE1DPROC)wglGetProcAddress("glFramebufferTexture1D");
        glFramebufferTexture2D                = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
        glFramebufferTexture3D                = (PFNGLFRAMEBUFFERTEXTURE3DPROC)wglGetProcAddress("glFramebufferTexture3D");
        glFramebufferTextureLayer             = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)wglGetProcAddress("glFramebufferTextureLayer");
        glFramebufferRenderbuffer             = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
        glIsFramebuffer                       = (PFNGLISFRAMEBUFFERPROC)wglGetProcAddress("glIsFramebuffer");
        glBlitFramebuffer                     = (PFNGLBLITFRAMEBUFFERPROC)wglGetProcAddress("glBlitFramebuffer");
        glGenRenderbuffers                    = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
        glDeleteRenderbuffers                 = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
        glBindRenderbuffer                    = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
        glRenderbufferStorage                 = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
        glRenderbufferStorageMultisample      = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)wglGetProcAddress("glRenderbufferStorageMultisample");
        glGetRenderbufferParameteriv          = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetRenderbufferParameteriv");
        glIsRenderbuffer                      = (PFNGLISRENDERBUFFERPROC)wglGetProcAddress("glIsRenderbuffer");

        // check once again FBO extension
        if(glGenFramebuffers && glDeleteFramebuffers && glBindFramebuffer && glCheckFramebufferStatus &&
           glGetFramebufferAttachmentParameteriv && glGenerateMipmap && glFramebufferTexture1D && glFramebufferTexture2D && glFramebufferTexture3D &&
           glFramebufferTextureLayer && glFramebufferRenderbuffer && glIsFramebuffer && glBlitFramebuffer &&
           glGenRenderbuffers && glDeleteRenderbuffers && glBindRenderbuffer && glRenderbufferStorage &&
           glRenderbufferStorageMultisample && glGetRenderbufferParameteriv && glIsRenderbuffer)
        {
            fboSupported = fboUsed = true;
            std::cout << "Video card supports GL_ARB_framebuffer_object." << std::endl;
        }
        else
        {
            fboSupported = fboUsed = false;
            std::cout << "Video card does NOT support GL_ARB_framebuffer_object." << std::endl;
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
    if(glInfo.isExtensionSupported("GL_ARB_framebuffer_object"))
    {
        fboSupported = fboUsed = true;
        std::cout << "Video card supports GL_ARB_framebuffer_object." << std::endl;
    }
    else
    {
        fboSupported = fboUsed = false;
        std::cout << "Video card does NOT support GL_ARB_framebuffer_object." << std::endl;
    }
#endif

    if(fboSupported)
    {
        // create a framebuffer object, you need to delete them when program exits.
        glGenFramebuffers(1, &fboId);
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);

        // create a renderbuffer object to store depth & stencil info with GL_DEPTH_STENCIL
        // Check "GL_EXT_packed_depth_stencil" extension spec for more details.
        // NOTE: A depth renderable image should be attached the FBO for depth test.
        // If we don't attach a depth renderable image to the FBO, then
        // the rendering output will be corrupted because of missing depth test.
        // If you also need stencil test for your rendering, then you must
        // attach additional image to the stencil attachement point, too.
        glGenRenderbuffers(1, &rboId);
        glBindRenderbuffer(GL_RENDERBUFFER, rboId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, TEXTURE_WIDTH, TEXTURE_HEIGHT);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // attach a texture to FBO color attachement point
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

        // attach a renderbuffer to depth attachment point
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboId);

        // attach a renderbuffer to stencil attachment point
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboId);

        //@@ disable color buffer if you don't attach any color buffer image,
        //@@ for example, rendering depth buffer only to a texture.
        //@@ Otherwise, glCheckFramebufferStatusEXT will not be complete.
        //glDrawBuffer(GL_NONE);
        //glReadBuffer(GL_NONE);

        // check FBO status
        printFramebufferInfo(fboId);
        bool status = checkFramebufferStatus(fboId);
        if(!status)
            fboUsed = false;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // start timer, the elapsed time will be used for rotating the teapot
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
    // It must be called before any other GLUT routine.
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);   // display mode

    glutInitWindowSize(screenWidth, screenHeight);  // window size

    glutInitWindowPosition(100, 100);               // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // It returns a unique ID.
    int handle = glutCreateWindow(argv[0]);         // param is the title of window

    // register GLUT callback functions
    glutDisplayFunc(displayCB);
    //glutTimerFunc(33, timerCB, 33);                 // redraw only every given millisec
    glutIdleFunc(idleCB);                           // redraw whenever system is idle
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
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
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

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initSharedMem()
{
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = false;
    mouseX = mouseY = 0;

    cameraAngleX = cameraAngleY = 45;
    cameraDistance = 4;
    drawMode = 0; // 0:fill, 1: wireframe, 2:points

    fboId = rboId = textureId = 0;
    fboSupported = fboUsed = false;

    playTime = renderToTextureTime = 0;
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// clean up global vars
///////////////////////////////////////////////////////////////////////////////
void clearSharedMem()
{
    glDeleteTextures(1, &textureId);
    textureId = 0;

    // clean up FBO, RBO
    if(fboSupported)
    {
        glDeleteFramebuffers(1, &fboId);
        fboId = 0;
        glDeleteRenderbuffers(1, &rboId);
        rboId = 0;
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
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    gluOrtho2D(0, screenWidth, 0, screenHeight);  // set to orthogonal projection

    float color[4] = {1, 1, 1, 1};

    stringstream ss;
    ss << "FBO: ";
    if(fboUsed)
        ss << "on" << ends;
    else
        ss << "off" << ends;

    drawString(ss.str().c_str(), 1, screenHeight-TEXT_HEIGHT, color, font);
    ss.str(""); // clear buffer

    ss << std::fixed << std::setprecision(3);
    ss << "Render-To-Texture Time: " << renderToTextureTime << " ms" << ends;
    drawString(ss.str().c_str(), 1, screenHeight-(2*TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Press SPACE to toggle FBO." << ends;
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
// display frame rates
///////////////////////////////////////////////////////////////////////////////
void showFPS()
{
    static Timer timer;
    static int count = 0;
    static std::string fps = "0.0 FPS";
    double elapsedTime = 0.0;;

    ++count;

    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    gluOrtho2D(0, screenWidth, 0, screenHeight); // set to orthogonal projection

    float color[4] = {1, 1, 0, 1};

    // update fps every second
    elapsedTime = timer.getElapsedTime();
    if(elapsedTime > 1.0)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1);
        ss << (count / elapsedTime) << " FPS" << std::ends; // update fps string
        ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
        fps = ss.str();
        count = 0;                      // reset counter
        timer.start();                  // restart timer
    }
    int textWidth = (int)fps.size() * TEXT_WIDTH;
    drawString(fps.c_str(), screenWidth-textWidth, screenHeight-TEXT_HEIGHT, color, font);

    // restore projection matrix
    glPopMatrix();                      // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);         // switch to modelview matrix
    glPopMatrix();                      // restore to previous modelview matrix
}



///////////////////////////////////////////////////////////////////////////////
// check FBO completeness
///////////////////////////////////////////////////////////////////////////////
bool checkFramebufferStatus(GLuint fbo)
{
    // check FBO status
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); // bind
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status)
    {
    case GL_FRAMEBUFFER_COMPLETE:
        std::cout << "Framebuffer complete." << std::endl;
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
        return false;
/*
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
        std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
        return false;
*/
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        std::cout << "[ERROR] Framebuffer incomplete: Multisample." << std::endl;
        return false;

    case GL_FRAMEBUFFER_UNSUPPORTED:
        std::cout << "[ERROR] Framebuffer incomplete: Unsupported by FBO implementation." << std::endl;
        return false;

    default:
        std::cout << "[ERROR] Framebuffer incomplete: Unknown error." << std::endl;
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // unbind
}



///////////////////////////////////////////////////////////////////////////////
// print out the FBO infos
///////////////////////////////////////////////////////////////////////////////
void printFramebufferInfo(GLuint fbo)
{
    // bind fbo
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    std::cout << "\n===== FBO STATUS =====\n";

    // print max # of colorbuffers supported by FBO
    int colorBufferCount = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &colorBufferCount);
    std::cout << "Max Number of Color Buffer Attachment Points: " << colorBufferCount << std::endl;

    // get max # of multi samples
    int multiSampleCount = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &multiSampleCount);
    std::cout << "Max Number of Samples for MSAA: " << multiSampleCount << std::endl;

    int objectType;
    int objectId;

    // print info of the colorbuffer attachable image
    for(int i = 0; i < colorBufferCount; ++i)
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_COLOR_ATTACHMENT0+i,
                                              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                              &objectType);
        if(objectType != GL_NONE)
        {
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                                  GL_COLOR_ATTACHMENT0+i,
                                                  GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                                  &objectId);

            std::string formatName;

            std::cout << "Color Attachment " << i << ": ";
            if(objectType == GL_TEXTURE)
            {
                std::cout << "GL_TEXTURE, " << getTextureParameters(objectId) << std::endl;
            }
            else if(objectType == GL_RENDERBUFFER)
            {
                std::cout << "GL_RENDERBUFFER, " << getRenderbufferParameters(objectId) << std::endl;
            }
        }
    }

    // print info of the depthbuffer attachable image
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                          GL_DEPTH_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                          &objectType);
    if(objectType != GL_NONE)
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_DEPTH_ATTACHMENT,
                                              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                              &objectId);

        std::cout << "Depth Attachment: ";
        switch(objectType)
        {
        case GL_TEXTURE:
            std::cout << "GL_TEXTURE, " << getTextureParameters(objectId) << std::endl;
            break;
        case GL_RENDERBUFFER:
            std::cout << "GL_RENDERBUFFER, " << getRenderbufferParameters(objectId) << std::endl;
            break;
        }
    }

    // print info of the stencilbuffer attachable image
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                          GL_STENCIL_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                          &objectType);
    if(objectType != GL_NONE)
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_STENCIL_ATTACHMENT,
                                              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                              &objectId);

        std::cout << "Stencil Attachment: ";
        switch(objectType)
        {
        case GL_TEXTURE:
            std::cout << "GL_TEXTURE, " << getTextureParameters(objectId) << std::endl;
            break;
        case GL_RENDERBUFFER:
            std::cout << "GL_RENDERBUFFER, " << getRenderbufferParameters(objectId) << std::endl;
            break;
        }
    }

    std::cout << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



///////////////////////////////////////////////////////////////////////////////
// return texture parameters as string using glGetTexLevelParameteriv()
///////////////////////////////////////////////////////////////////////////////
std::string getTextureParameters(GLuint id)
{
    if(glIsTexture(id) == GL_FALSE)
        return "Not texture object";

    int width, height, format;
    std::string formatName;
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);            // get texture width
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);          // get texture height
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format); // get texture internal format
    glBindTexture(GL_TEXTURE_2D, 0);

    formatName = convertInternalFormatToString(format);

    std::stringstream ss;
    ss << width << "x" << height << ", " << formatName;
    return ss.str();
}



///////////////////////////////////////////////////////////////////////////////
// return renderbuffer parameters as string using glGetRenderbufferParameteriv
///////////////////////////////////////////////////////////////////////////////
std::string getRenderbufferParameters(GLuint id)
{
    if(glIsRenderbuffer(id) == GL_FALSE)
        return "Not Renderbuffer object";

    int width, height, format, samples;
    std::string formatName;
    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);       // get renderbuffer width
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);     // get renderbuffer height
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format); // get renderbuffer internal format
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);   // get multisample count
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    formatName = convertInternalFormatToString(format);

    std::stringstream ss;
    ss << width << "x" << height << ", " << formatName << ", MSAA(" << samples << ")";
    return ss.str();
}



///////////////////////////////////////////////////////////////////////////////
// convert OpenGL internal format enum to string
///////////////////////////////////////////////////////////////////////////////
std::string convertInternalFormatToString(GLenum format)
{
    std::string formatName;

    switch(format)
    {
    case GL_STENCIL_INDEX:      // 0x1901
        formatName = "GL_STENCIL_INDEX";
        break;
    case GL_DEPTH_COMPONENT:    // 0x1902
        formatName = "GL_DEPTH_COMPONENT";
        break;
    case GL_ALPHA:              // 0x1906
        formatName = "GL_ALPHA";
        break;
    case GL_RGB:                // 0x1907
        formatName = "GL_RGB";
        break;
    case GL_RGBA:               // 0x1908
        formatName = "GL_RGBA";
        break;
    case GL_LUMINANCE:          // 0x1909
        formatName = "GL_LUMINANCE";
        break;
    case GL_LUMINANCE_ALPHA:    // 0x190A
        formatName = "GL_LUMINANCE_ALPHA";
        break;
    case GL_R3_G3_B2:           // 0x2A10
        formatName = "GL_R3_G3_B2";
        break;
    case GL_ALPHA4:             // 0x803B
        formatName = "GL_ALPHA4";
        break;
    case GL_ALPHA8:             // 0x803C
        formatName = "GL_ALPHA8";
        break;
    case GL_ALPHA12:            // 0x803D
        formatName = "GL_ALPHA12";
        break;
    case GL_ALPHA16:            // 0x803E
        formatName = "GL_ALPHA16";
        break;
    case GL_LUMINANCE4:         // 0x803F
        formatName = "GL_LUMINANCE4";
        break;
    case GL_LUMINANCE8:         // 0x8040
        formatName = "GL_LUMINANCE8";
        break;
    case GL_LUMINANCE12:        // 0x8041
        formatName = "GL_LUMINANCE12";
        break;
    case GL_LUMINANCE16:        // 0x8042
        formatName = "GL_LUMINANCE16";
        break;
    case GL_LUMINANCE4_ALPHA4:  // 0x8043
        formatName = "GL_LUMINANCE4_ALPHA4";
        break;
    case GL_LUMINANCE6_ALPHA2:  // 0x8044
        formatName = "GL_LUMINANCE6_ALPHA2";
        break;
    case GL_LUMINANCE8_ALPHA8:  // 0x8045
        formatName = "GL_LUMINANCE8_ALPHA8";
        break;
    case GL_LUMINANCE12_ALPHA4: // 0x8046
        formatName = "GL_LUMINANCE12_ALPHA4";
        break;
    case GL_LUMINANCE12_ALPHA12:// 0x8047
        formatName = "GL_LUMINANCE12_ALPHA12";
        break;
    case GL_LUMINANCE16_ALPHA16:// 0x8048
        formatName = "GL_LUMINANCE16_ALPHA16";
        break;
    case GL_INTENSITY:          // 0x8049
        formatName = "GL_INTENSITY";
        break;
    case GL_INTENSITY4:         // 0x804A
        formatName = "GL_INTENSITY4";
        break;
    case GL_INTENSITY8:         // 0x804B
        formatName = "GL_INTENSITY8";
        break;
    case GL_INTENSITY12:        // 0x804C
        formatName = "GL_INTENSITY12";
        break;
    case GL_INTENSITY16:        // 0x804D
        formatName = "GL_INTENSITY16";
        break;
    case GL_RGB4:               // 0x804F
        formatName = "GL_RGB4";
        break;
    case GL_RGB5:               // 0x8050
        formatName = "GL_RGB5";
        break;
    case GL_RGB8:               // 0x8051
        formatName = "GL_RGB8";
        break;
    case GL_RGB10:              // 0x8052
        formatName = "GL_RGB10";
        break;
    case GL_RGB12:              // 0x8053
        formatName = "GL_RGB12";
        break;
    case GL_RGB16:              // 0x8054
        formatName = "GL_RGB16";
        break;
    case GL_RGBA2:              // 0x8055
        formatName = "GL_RGBA2";
        break;
    case GL_RGBA4:              // 0x8056
        formatName = "GL_RGBA4";
        break;
    case GL_RGB5_A1:            // 0x8057
        formatName = "GL_RGB5_A1";
        break;
    case GL_RGBA8:              // 0x8058
        formatName = "GL_RGBA8";
        break;
    case GL_RGB10_A2:           // 0x8059
        formatName = "GL_RGB10_A2";
        break;
    case GL_RGBA12:             // 0x805A
        formatName = "GL_RGBA12";
        break;
    case GL_RGBA16:             // 0x805B
        formatName = "GL_RGBA16";
        break;
    case GL_DEPTH_COMPONENT16:  // 0x81A5
        formatName = "GL_DEPTH_COMPONENT16";
        break;
    case GL_DEPTH_COMPONENT24:  // 0x81A6
        formatName = "GL_DEPTH_COMPONENT24";
        break;
    case GL_DEPTH_COMPONENT32:  // 0x81A7
        formatName = "GL_DEPTH_COMPONENT32";
        break;
    case GL_DEPTH_STENCIL:      // 0x84F9
        formatName = "GL_DEPTH_STENCIL";
        break;
    case GL_RGBA32F:            // 0x8814
        formatName = "GL_RGBA32F";
        break;
    case GL_RGB32F:             // 0x8815
        formatName = "GL_RGB32F";
        break;
    case GL_RGBA16F:            // 0x881A
        formatName = "GL_RGBA16F";
        break;
    case GL_RGB16F:             // 0x881B
        formatName = "GL_RGB16F";
        break;
    case GL_DEPTH24_STENCIL8:   // 0x88F0
        formatName = "GL_DEPTH24_STENCIL8";
        break;
    default:
        std::stringstream ss;
        ss << "Unknown Format(0x" << std::hex << format << ")" << std::ends;
        formatName = ss.str();
    }

    return formatName;
}



///////////////////////////////////////////////////////////////////////////////
// set projection matrix as orthogonal
///////////////////////////////////////////////////////////////////////////////
void toOrtho()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);

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
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(screenWidth)/screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}








//=============================================================================
// CALLBACKS
//=============================================================================

void displayCB()
{
    // get the total elapsed time
    playTime = (float)timer.getElapsedTime();

    // compute rotation angle
    const float ANGLE_SPEED = 90;   // degree/s
    float angle = ANGLE_SPEED * playTime;

    // render to texture //////////////////////////////////////////////////////
    t1.start();

    // adjust viewport and projection matrix to texture dimension
    glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(TEXTURE_WIDTH)/TEXTURE_HEIGHT, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    // camera transform
    glLoadIdentity();
    glTranslatef(0, 0, -CAMERA_DISTANCE);

    // with FBO
    // render directly to a texture
    if(fboUsed)
    {
        // set the rendering destination to FBO
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);

        // clear buffer
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // enable stencil test
        glEnable(GL_STENCIL_TEST);

        // draw a rotating teapot
        glPushMatrix();
        glRotatef(angle*0.5f, 1, 0, 0);
        glRotatef(angle, 0, 1, 0);
        glRotatef(angle*0.7f, 0, 0, 1);
        glTranslatef(0, -1.575f, 0);

        // PASS1: draw teapot on both color and stencil buffer
        // The reference value will be written to the stencil buffer if the test is passed.
        glStencilFunc(GL_ALWAYS, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glColor3f(0.929524f, 0.796542f, 0.178823f);
        drawTeapot();

        // PASS 2: draw the wireframe teapot only where the stencil value is NOT set to 1.
        glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDisable(GL_LIGHTING);
        glColor3f(0,1,0); // outline color
        glLineWidth(5.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
        drawTeapot();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // return to fill mode
        glEnable(GL_LIGHTING);

        glPopMatrix();
        glDisable(GL_STENCIL_TEST);

        // back to normal window-system-provided framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

        // trigger mipmaps generation explicitly
        // NOTE: If GL_GENERATE_MIPMAP is set to GL_TRUE, then glCopyTexSubImage2D()
        // triggers mipmap generation automatically. However, the texture attached
        // onto a FBO should generate mipmaps manually via glGenerateMipmapEXT().
        glBindTexture(GL_TEXTURE_2D, textureId);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // without FBO
    // render to the backbuffer and copy the backbuffer to a texture
    else
    {
        // clear buffer
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT| GL_STENCIL_BUFFER_BIT);

        glPushAttrib(GL_COLOR_BUFFER_BIT | GL_PIXEL_MODE_BIT); // for GL_DRAW_BUFFER and GL_READ_BUFFER
        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_BACK);

        // enable stencil test
        glEnable(GL_STENCIL_TEST);

        // draw a rotating teapot
        glPushMatrix();
        glRotatef(angle*0.5f, 1, 0, 0);
        glRotatef(angle, 0, 1, 0);
        glRotatef(angle*0.7f, 0, 0, 1);
        glTranslatef(0, -1.575f, 0);

        // PASS1: draw teapot on both color and stencil buffer
        // The reference value will be written to the stencil buffer if the test is passed.
        glStencilFunc(GL_ALWAYS, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glColor3f(0.929524f, 0.796542f, 0.178823f);
        drawTeapot();

        // PASS 2: draw the wireframe teapot only where the stencil value is NOT set to 1.
        glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDisable(GL_LIGHTING);
        glColor3f(0,1,0); // outline color
        glLineWidth(5.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
        drawTeapot();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // return to fill mode
        glEnable(GL_LIGHTING);

        glPopMatrix();
        glDisable(GL_STENCIL_TEST);

        // copy the framebuffer pixels to a texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
        glBindTexture(GL_TEXTURE_2D, 0);

        glPopAttrib(); // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
    }

    // measure the elapsed time of render-to-texture
    t1.stop();
    renderToTextureTime = t1.getElapsedTimeInMilliSec();
    ///////////////////////////////////////////////////////////////////////////


    // rendering as normal ////////////////////////////////////////////////////

    // back to normal viewport and projection matrix
    glViewport(0, 0, screenWidth, screenHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(screenWidth)/screenHeight, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // tramsform camera
    glTranslatef(0, 0, -cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0);   // pitch
    glRotatef(cameraAngleY, 0, 1, 0);   // heading

    // clear framebuffer
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glPushMatrix();

    // draw a cube with the dynamic texture
    draw();

    glPopMatrix();

    // draw info messages
    showInfo();
    showFPS();
    glutSwapBuffers();
}


void reshapeCB(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
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
        if(fboSupported)
            fboUsed = !fboUsed;
        cout << "FBO mode: " << (fboUsed ? "on" : "off") << endl;
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
        mouseY = y;
    }
}


void exitCB()
{
    clearSharedMem();
}
