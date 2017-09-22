#pragma once
#include <windows.h>
#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "../Model/GLSLShaderData.h"
#include "FBORenderTexture.h"
#include "DepthRenderTexture.h"

/**
*	This object is used to render a big screen sized quad with the deferred rendering shader applied on it.
*/
class DeferredRendering
{
public:
	// Ctors/Dtors
	DeferredRendering(int width, int height);

    // Methods
    void    setLightMatrices(float worldToLightViewMatrix[16], float lightViewToProjectionMatrix[16], float worldToCameraViewMatrix[16]);
    void    startRenderToFBO();
    void    stopRenderToFBO();
    void    startRenderToShadowMap();
    void    stopRenderToShadowMap();
    void	showTexture(unsigned int i, float fWindowsWidth, float fWindowsHeight, float fSizeX = 400, float fSizeY = 400, float x = 0, float y = 0) const;
    void	showShadowMap(float fWindowsWidth, float fWindowsHeight, float fSizeX = 400, float fSizeY = 400, float x = 0, float y = 0) const;
	void	render();

private:
	// Variables
	GLSLShaderData 		m_shader; // Deferred rendering shader
	FBORenderTexture	m_fboRenderTexture; // A pointer to the FBO render texture that contains diffuse, normals and positions
    DepthRenderTexture  m_shadowMap; // A pointer to the FBO that renders the depth into the shadow map  

	unsigned int		m_width; // width
	unsigned int		m_height; // height

    float               m_worldToLightViewMatrix[16]; // Matrix that takes a vector from World Space into Light View Space
    float               m_lightViewToProjectionMatrix[16]; // Matrix that takes a vector from View Space into Projection Space (Clip Space)
    float               m_worldToCameraViewMatrix[16]; // Matrix that takes a vector from World Space into Camera View Space

	GLuint				m_diffuseID; // Diffuse texture handle for the shader
	GLuint				m_positionID; // Position texture handle for the shader
    GLuint				m_normalsID; // Normals texture handle for the shader
	GLuint				m_shadowMapID; // Shadow Map texture handle for the shader

    GLuint              m_worldToLightViewMatrix_shaderID; // Shader ID for the specified matrix
    GLuint              m_lightViewToProjectionMatrix_shaderID; // Shader ID for the specified matrix
    GLuint              m_worldToCameraViewMatrix_shaderID; // Shader ID for the specified matrix
};
