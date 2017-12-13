#pragma once
#include <windows.h>
#include "DeferredRendering/DeferredRendering.h"
#include "DeferredRendering/FBORenderTexture.h"

//Fwd
class IModel;

/**
*	This class contains all the system stuff that we need to render with OpenGL
*/
class GLApplication {
public:
	// Methods
	bool	initialize(HWND hwnd, int w, int h);
	void	setSize(int w, int h);
	void	update();
	void	render();
	void	release();

	void	showDeferredRendering(){ m_state = 0; }
	void	showRenderTargets(){ m_state = 1; }

private:
	// Methods
	void	loadAssets();
	void	releaseAssets();

	// Static consts
	static const int	c_modelsCount = 5;

	// Fields
	IModel*				m_models[c_modelsCount];
    DeferredRendering*	m_deferredRendering;

	int					m_windowWidth;
	int					m_windowHeight;

	HGLRC				m_hrc;	// Rendering's context
	HDC					m_hdc;	// Device's context
	HWND				m_hWnd; // Window's handle

	unsigned int		m_lastTick;
	unsigned char		m_state; // 0 - Normal render, 1 - Show render targets

    float               m_lightRotation;
};