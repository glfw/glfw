#include "FBORenderTexture.h"
#include "DeferredRendering.h"
#include <exception>

/**
*	Create the deferred rendering object. I have hardcoded the shader's name here.
*/
DeferredRendering::DeferredRendering(int _dWidth, int _dHeight)
	: m_shader("data/deferredRendering.vert", "data/deferredRendering.frag")
    , m_fboRenderTexture(_dWidth, _dHeight)
	, m_shadowMap(1024, 1024)
	, m_width(_dWidth)
	, m_height(_dHeight)
{	
	// Get the handles from the shader
	m_diffuseID = glGetUniformLocationARB(m_shader.m_programHandler,"tDiffuse");
	m_positionID = glGetUniformLocationARB(m_shader.m_programHandler,"tPosition");
    m_normalsID = glGetUniformLocationARB(m_shader.m_programHandler,"tNormals");
	m_shadowMapID = glGetUniformLocationARB(m_shader.m_programHandler,"tShadowMap");

    m_worldToLightViewMatrix_shaderID = glGetUniformLocationARB(m_shader.m_programHandler,"worldToLightViewMatrix");
    m_lightViewToProjectionMatrix_shaderID = glGetUniformLocationARB(m_shader.m_programHandler,"lightViewToProjectionMatrix");
    m_worldToCameraViewMatrix_shaderID = glGetUniformLocationARB(m_shader.m_programHandler,"worldToCameraViewMatrix");
}

/**
*	Acquire the light matrices
*/
void DeferredRendering::setLightMatrices(float worldToLightViewMatrix[16], float lightViewToProjectionMatrix[16], float worldToCameraViewMatrix[16])
{
    memcpy(m_worldToLightViewMatrix, worldToLightViewMatrix, sizeof(float) * 16);
    memcpy(m_lightViewToProjectionMatrix, lightViewToProjectionMatrix, sizeof(float) * 16);
    memcpy(m_worldToCameraViewMatrix, worldToCameraViewMatrix, sizeof(float) * 16);
}

/**
*	Start rendering to the FBO
*/
void DeferredRendering::startRenderToFBO()
{
    m_fboRenderTexture.start();
}

/**
*	Stop rendering to the FBO
*/
void DeferredRendering::stopRenderToFBO()
{
    m_fboRenderTexture.stop();
}

/**
*	Render the debug texture
*/
void DeferredRendering::showTexture(unsigned int i, float fWindowsWidth, float fWindowsHeight, float fSizeX, float fSizeY, float x, float y) const
{
    m_fboRenderTexture.showTexture(i, fWindowsWidth, fWindowsHeight, fSizeX, fSizeY, x, y);
}

/**
*	Render the depth into the shadow map
*/
void DeferredRendering::startRenderToShadowMap()
{
    m_shadowMap.start();
}

/**
*	Stop to render the depth into the shadow map
*/
void DeferredRendering::stopRenderToShadowMap()
{
    m_shadowMap.stop();
}

/**
*	Render the debug texture for the shadow map
*/
void DeferredRendering::showShadowMap( float fWindowsWidth, float fWindowsHeight, float fSizeX, float fSizeY, float x, float y) const
{
    m_shadowMap.showTexture(fWindowsWidth, fWindowsHeight, fSizeX, fSizeY, x, y);
}

/**
*	Render the big quad with the deferred rendering shader on it
*/
void DeferredRendering::render()
{
	//Projection setup
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0,m_width,0,m_height,0.1f,2);	
	
	//Model setup
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	glUseProgramObjectARB(m_shader.m_programHandler);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_fboRenderTexture.getDiffuseTexture());
	glUniform1iARB ( m_diffuseID, 0 );
	
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_fboRenderTexture.getPositionTexture());
    glUniform1iARB ( m_positionID, 1 );

    glActiveTextureARB(GL_TEXTURE2_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_fboRenderTexture.getNormalsTexture());
    glUniform1iARB ( m_normalsID, 2 );

    glActiveTextureARB(GL_TEXTURE3_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap.getTexture());
    glUniform1iARB ( m_shadowMapID, 3 );

    glUniformMatrix4fv ( m_worldToCameraViewMatrix_shaderID, 1, GL_FALSE, m_worldToCameraViewMatrix );
    glUniformMatrix4fv ( m_lightViewToProjectionMatrix_shaderID, 1, GL_FALSE, m_lightViewToProjectionMatrix );
    glUniformMatrix4fv ( m_worldToLightViewMatrix_shaderID, 1, GL_FALSE, m_worldToLightViewMatrix );

	// Render the quad
	glLoadIdentity();
	glColor3f(1,1,1);
	glTranslatef(0,0,-1.0);
	
	glBegin(GL_QUADS);
	glTexCoord2f( 0, 0 );
	glVertex3f(    0.0f, 0.0f, 0.0f);
	glTexCoord2f( 1, 0 );
	glVertex3f(   (float) m_width, 0.0f, 0.0f);
	glTexCoord2f( 1, 1 );
	glVertex3f(   (float) m_width, (float) m_height, 0.0f);
	glTexCoord2f( 0, 1 );
	glVertex3f(    0.0f,  (float) m_height, 0.0f);
	glEnd();
	
	// Reset OpenGL state
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTextureARB(GL_TEXTURE2_ARB);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTextureARB(GL_TEXTURE3_ARB);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgramObjectARB(0);
	
	//Reset to the matrices	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	
}