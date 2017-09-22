#pragma once
#include <windows.h>
#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "../Model/GLSLShaderData.h"
#include "FBORenderTexture.h"

/**
*	This object is used to render a big screen sized quad with the deferred rendering shader applied on it.
*/
class DeferredRendering
{
public:
	// Ctors/Dtors
	DeferredRendering(int width, int height, FBORenderTexture* fboRenderTexture);

	// Methods
	void	render();

private:
	// Variables
	GLSLShaderData 		m_shader; // Deferred rendering shader
	FBORenderTexture*	m_fboRenderTexture; // A pointer to the FBO render texture that contains diffuse, normals and positions

	unsigned int		m_width; // width
	unsigned int		m_height; // height

	GLuint				m_diffuseID; // Diffuse texture handle for the shader
	GLuint				m_positionID; // Position texture handle for the shader
	GLuint				m_normalsID; // Normals texture handle for the shader
};
