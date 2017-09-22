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

	gluPerspective(45.0f,(GLfloat)m_windowWidth/(GLfloat)m_windowHeight,0.1f,100.0f);

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

 	m_models[0]->addRotation( time, time*2, 0 );
 	m_models[1]->addRotation( 0, time*2, time );
}

/**
*	Render the scene
*/
void GLApplication::render() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2f, 0.3f, 0.8f, 1.0f);

	glLoadIdentity();
	glRotatef(20, 1, 0, 0);
	glTranslatef(0.0f,-4.6f,-10.0f);

	// Render our geometry into the FBO
	m_multipleRenderTarget->start();
	for(int i=0; i<c_modelsCount; ++i)
		m_models[i]->render();
	m_multipleRenderTarget->stop();

	// Render to the screen
	if(m_state == 0)
	{
		// Render to screen using the deferred rendering shader
		m_deferredRendering->render();
	}
	else if(m_state == 1)
	{
		m_multipleRenderTarget->showTexture( 0, 512, 384, 0);
		m_multipleRenderTarget->showTexture( 1, 512, 384, 512);
		m_multipleRenderTarget->showTexture( 2, 512, 384, 0, 384);
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

	m_multipleRenderTarget = new FBORenderTexture(m_windowWidth, m_windowHeight);
	m_deferredRendering = new DeferredRendering(m_windowWidth, m_windowHeight, m_multipleRenderTarget);

	m_models[0] = new SphereModel("data/deferredShading.vert", "data/deferredShading.frag", 1, 64);
	m_models[0]->loadTexture("data/earth.raw");
	m_models[0]->setPosition(2,2.5f,0);

	m_models[1] = new CubeModel("data/deferredShading.vert", "data/deferredShading.frag", 1);
	m_models[1]->loadTexture("data/box.raw");
	m_models[1]->setPosition(-2,2.5f,0);

	m_models[2] = new PlaneModel("data/deferredShading.vert", "data/deferredShading.frag", 5);
	m_models[2]->loadTexture("data/plane.raw");
	m_models[2]->setPosition(0,0,0);
}

/**
*	Release all the assets
*/
void GLApplication::releaseAssets()
{
	delete m_multipleRenderTarget;
	delete m_deferredRendering;

	for(int i=0; i<c_modelsCount; ++i)
		delete m_models[i];
}