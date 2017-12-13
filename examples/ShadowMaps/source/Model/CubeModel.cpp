#include "CubeModel.h"
#include <math.h>

/** 
*	Construct the cube
*/
CubeModel::CubeModel(const std::string& sVSFileName, const std::string& sFSFileName, float side)
	: IModel(sVSFileName, sFSFileName)
	, m_side(side)
{	
}

/** 
*	Render
*/
void CubeModel::render() const
{
	glPushMatrix();

	// Save the current world matrix to compensate the normals in the shader
	float worldMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, worldMatrix);

	glScalef(m_side,m_side,m_side);
	glTranslatef(m_posX, m_posY, m_posZ);
	glRotatef(m_rotX, 1, 0, 0);
	glRotatef(m_rotY, 0, 1, 0);
	glRotatef(m_rotZ, 0, 0, 1);

	glUseProgramObjectARB(m_shader.m_programHandler);

	glBindTexture(GL_TEXTURE_2D, m_texture);
	glUniform1iARB ( m_textureID, 0);
	glUniformMatrix4fvARB ( m_worldMatrixID, 1, false, worldMatrix);

	glBegin(GL_QUADS);
	// Front Face
	glTexCoord2f(0.0f, 0.0f); glNormal3f(0.0f, 0.0f,  1.0f); glVertex3f(-1.0f, -1.0f,  1.0f); 
	glTexCoord2f(1.0f, 0.0f); glNormal3f(0.0f, 0.0f,  1.0f); glVertex3f( 1.0f, -1.0f,  1.0f); 
	glTexCoord2f(1.0f, 1.0f); glNormal3f(0.0f, 0.0f,  1.0f); glVertex3f( 1.0f,  1.0f,  1.0f); 
	glTexCoord2f(0.0f, 1.0f); glNormal3f(0.0f, 0.0f,  1.0f); glVertex3f(-1.0f,  1.0f,  1.0f); 
	// Back Face
	glTexCoord2f(1.0f, 0.0f); glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f); glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f); glNormal3f(0.0f, 0.0f, -1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
	// Top Face
	glTexCoord2f(0.0f, 1.0f); glNormal3f(0.0f, 1.0f,  0.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f); glNormal3f(0.0f, 1.0f,  0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
	glTexCoord2f(1.0f, 0.0f); glNormal3f(0.0f, 1.0f,  0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
	glTexCoord2f(1.0f, 1.0f); glNormal3f(0.0f, 1.0f,  0.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
	// Bottom Face
	glTexCoord2f(1.0f, 1.0f); glNormal3f(0.0f, -1.0f,  0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glNormal3f(0.0f, -1.0f,  0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f); glNormal3f(0.0f, -1.0f,  0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
	glTexCoord2f(1.0f, 0.0f); glNormal3f(0.0f, -1.0f,  0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
	// Right face
	glTexCoord2f(1.0f, 0.0f); glNormal3f(1.0f, 0.0f,  0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f); glNormal3f(1.0f, 0.0f,  0.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glNormal3f(1.0f, 0.0f,  0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
	glTexCoord2f(0.0f, 0.0f); glNormal3f(1.0f, 0.0f,  0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
	// Left Face
	glTexCoord2f(0.0f, 0.0f); glNormal3f(-1.0f, 0.0f,  0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f); glNormal3f(-1.0f, 0.0f,  0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
	glTexCoord2f(1.0f, 1.0f); glNormal3f(-1.0f, 0.0f,  0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
	glTexCoord2f(0.0f, 1.0f); glNormal3f(-1.0f, 0.0f,  0.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glEnd();

	glUseProgramObjectARB(0);

	glPopMatrix();
}

