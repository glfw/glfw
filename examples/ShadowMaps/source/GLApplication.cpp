#include <windows.h>
#include "GLApplication.h"
#include "Model/SphereModel.h"
#include "Model/CubeModel.h"
#include "Model/PlaneModel.h"

/**
*	Initialize our GL application
*/
bool GLApplication::initialize(HWND hwnd, int width, int height) 
{
	GLuint pixelFormat;
	m_windowHeight = height;
	m_windowWidth = width;

	m_hWnd = hwnd; 
    m_lightRotation = 0.0f;
	
	static	PIXELFORMATDESCRIPTOR pfd=
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		16,	
		0, 0, 0, 0, 0, 0,
		0,
		0,	
		0,
		0, 0, 0, 0,
		16,	
		0,	
		0,
		PFD_MAIN_PLANE,	
		0,
		0, 0, 0
	};

	if (!(m_hdc=GetDC(m_hWnd)))
		return FALSE;

	if (!(pixelFormat=ChoosePixelFormat(m_hdc,&pfd)))
		return FALSE;

	if(!SetPixelFormat(m_hdc,pixelFormat,&pfd))
		return FALSE;

	if (!(m_hrc=wglCreateContext(m_hdc)))	
		return FALSE;

	if(!wglMakeCurrent(m_hdc,m_hrc))
		return FALSE;

	ShowWindow(m_hWnd,SW_SHOW);	
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);
	setSize(width, height);

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_CULL_FACE);

	GLenum err = glewInit();
	if (GLEW_OK != err)
		return false;
	
	loadAssets();

	return true;
}

/**
*	Set window's size
*/
void GLApplication::setSize(int w, int h) 
{
	m_windowWidth = w;
	m_windowHeight = h;

	glViewport(0,0,m_windowWidth,m_windowHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(40.0f,(GLfloat)m_windowWidth/(GLfloat)m_windowHeight,1.0f,30.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/**
*	Update
*/
void GLApplication::update() 
{
	float time = (GetTickCount() - m_lastTick) * 0.01f;
	m_lastTick = GetTickCount();

    m_models[3]->addRotation( time, time*2, 0 );
    m_models[4]->addRotation( 0.2f + time, 0.1f + time*2, 0 );

    m_lightRotation += time;
    if(m_lightRotation > 360)
        m_lightRotation -= 360;
}

/**
*	Render the scene
*/
void GLApplication::render() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2f, 0.3f, 0.8f, 1.0f);

    // We move the near plane just a bit to make the depth texture a bit more visible. 
    // It also increases the precision.
    glMatrixMode(GL_PROJECTION);

    glPushMatrix();
    glLoadIdentity();
    gluPerspective(20.0f, 1, 40.0f, 70.0f);

    // Set the light position
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(55, 1, 0, 0);
    glRotatef(-45, 0, 1, 0);
    glTranslatef(-25.0f, -50.0f, -25.0f);
    glRotatef(m_lightRotation, 0, 1, 0);
    glFrontFace(GL_CW);

    // Render the shadow map
    m_deferredRendering->startRenderToShadowMap();
    for(int i=0; i<c_modelsCount; ++i)
        m_models[i]->render();
    m_deferredRendering->stopRenderToShadowMap();

    // We then save out the matrices and send them to the deferred rendering, so when it comes to do the deferred pass
    // it can project the pixel it's rendering to the light and see if it's in shadows
    float worldToLightViewMatrix[16];
    float lightViewToProjectionMatrix[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, worldToLightViewMatrix);
    glGetFloatv(GL_PROJECTION_MATRIX, lightViewToProjectionMatrix);

    // Re-set the projection to the default one we have pushed on the stack
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Set the camera position
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(20, 1, 0, 0);
    glTranslatef(0.0f,-6.5f,-11.0f);
    glFrontFace(GL_CCW);

    float worldToCameraViewMatrix[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, worldToCameraViewMatrix);

	// Render our geometry into the FBO
	m_deferredRendering->startRenderToFBO();
	for(int i=0; i<c_modelsCount; ++i)
        m_models[i]->render();
    m_deferredRendering->stopRenderToFBO();

	// Render to the screen
	if(m_state == 0)
    {
		// Render to screen using the deferred rendering shader
        m_deferredRendering->setLightMatrices(worldToLightViewMatrix, lightViewToProjectionMatrix, worldToCameraViewMatrix);
		m_deferredRendering->render();
	}
	else if(m_state == 1)
	{
		m_deferredRendering->showTexture( 0, (float)m_windowWidth, (float)m_windowHeight, 512, 384, 0);
        m_deferredRendering->showTexture( 1, (float)m_windowWidth, (float)m_windowHeight, 512, 384, 512);
        m_deferredRendering->showTexture( 2, (float)m_windowWidth, (float)m_windowHeight, 512, 384, 0, 384);
        m_deferredRendering->showShadowMap(  (float)m_windowWidth, (float)m_windowHeight, 384, 384, 512, 384);
	}

	SwapBuffers(m_hdc);	
}

/**
*	Release all the GL resources we have allocated
*/
void GLApplication::release() 
{
	releaseAssets();

	wglMakeCurrent(m_hdc, 0);
	wglDeleteContext(m_hrc);

	ReleaseDC(m_hWnd, m_hdc);
}

/**
*	Load all the required assets
*/
void GLApplication::loadAssets()
{
	m_state = 0;

	m_deferredRendering = new DeferredRendering(m_windowWidth, m_windowHeight);

    m_models[0] = new PlaneModel("data/deferredShading.vert", "data/deferredShading.frag", 5);
    m_models[0]->loadTexture("data/plane.raw");
    m_models[0]->setPosition(0,0,0);

    m_models[1] = new CubeModel("data/deferredShading.vert", "data/deferredShading.frag", 1);
    m_models[1]->loadTexture("data/box.raw");
    m_models[1]->setPosition(-1.0f,3.0f,-1);

    m_models[2] = new CubeModel("data/deferredShading.vert", "data/deferredShading.frag", 1);
    m_models[2]->loadTexture("data/box.raw");
    m_models[2]->setPosition(-1.0f,1.0f,-1);
    m_models[2]->setRotation(0, 20.0f, 0.0f);

    m_models[3] = new SphereModel("data/deferredShading.vert", "data/deferredShading.frag", 1, 64);
    m_models[3]->loadTexture("data/earth.raw");
    m_models[3]->setPosition(1.5f, 2.0f, 0.5f);

    m_models[4] = new SphereModel("data/deferredShading.vert", "data/deferredShading.frag", 1, 64);
    m_models[4]->loadTexture("data/earth.raw");
    m_models[4]->setPosition(1.5f, 5.0f, 0.5f);
}

/**
*	Release all the assets
*/
void GLApplication::releaseAssets()
{
	delete m_deferredRendering;

	for(int i=0; i<c_modelsCount; ++i)
		delete m_models[i];
}