#include "FBORenderTexture.h"
#include "DeferredRendering.h"
#include <exception>

/**
*	Create the deferred rendering object. I have hardcoded the shader's name here.
*/
DeferredRendering::DeferredRendering(int _dWidth, int _dHeight, FBORenderTexture* fboRenderTexture)
	: m_shader("data/deferredRendering.vert", "data/deferredRendering.frag")
	, m_fboRenderTexture(fboRenderTexture)
	, m_width(_dWidth)
	, m_height(_dHeight)
{	
	// Get the handles from the shader
	m_diffuseID = glGetUniformLocationARB(m_shader.m_programHandler,"tDiffuse");
	m_positionID = glGetUniformLocationARB(m_shader.m_programHandler,"tPosition");
	m_normalsID = glGetUniformLocationARB(m_shader.m_programHandler,"tNormals");
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
	glBindTexture(GL_TEXTURE_2D, m_fboRenderTexture->getDiffuseTexture());
	glUniform1iARB ( m_diffuseID, 0 );
	
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_fboRenderTexture->getPositionTexture());
	glUniform1iARB ( m_positionID, 1 );
	
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_fboRenderTexture->getNormalsTexture());
	glUniform1iARB ( m_normalsID, 2 );

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

	glUseProgramObjectARB(0);
	
	//Reset to the matrices	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	
}