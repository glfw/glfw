#pragma once
#include <windows.h>
#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>

/**
*	With this class we wrap the ability of OpenGL to render into a Depth Texture. We need this to create the shadow map.
*/
class DepthRenderTexture
{
public:
	// Ctors/Dtors
	DepthRenderTexture(int width, int height);
	~DepthRenderTexture();

	// Methods
	void	start();
	void	stop();
	void	showTexture(float fWindowsWidth, float fWindowsHeight, float fSizeX = 400, float fSizeY = 400, float x = 0, float y = 0) const;

	GLuint	getTexture() const { return m_depthTexture; }

private:

	// Variables
	GLuint			m_fbo; // The FBO ID
    GLuint			m_depthBufferRT; // Depth buffer handle
	GLuint			m_depthTexture; // Texture

	unsigned int	m_width; // FBO width
	unsigned int	m_height; // FBO height
};
