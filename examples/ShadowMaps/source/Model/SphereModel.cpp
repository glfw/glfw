#include "SphereModel.h"
#include <math.h>

/** 
*	Construct the sphere
*/
SphereModel::SphereModel(const std::string& sVSFileName, const std::string& sFSFileName, float radius, unsigned int meshPrecision)
	: IModel(sVSFileName, sFSFileName)
{	
	m_radius = radius;
	m_meshPrecision = meshPrecision;
}

/** 
*	Render
*/
void SphereModel::render() const
{
	glPushMatrix();

	// Save the current world matrix to compensate the normals in the shader
	float worldMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, worldMatrix);

	glTranslatef(m_posX, m_posY, m_posZ);
	glRotatef(m_rotX, 1, 0, 0);
	glRotatef(m_rotY, 0, 1, 0);
	glRotatef(m_rotZ, 0, 0, 1);

	glUseProgramObjectARB(m_shader.m_programHandler);

	glBindTexture(GL_TEXTURE_2D, m_texture);
	glUniform1iARB ( m_textureID, 0);
	glUniformMatrix4fvARB ( m_worldMatrixID, 1, false, worldMatrix);

 	GLUquadricObj *sphere = gluNewQuadric();
	gluQuadricTexture(sphere, true);
	gluSphere(sphere, m_radius, m_meshPrecision, m_meshPrecision);
	gluDeleteQuadric(sphere);

	glUseProgramObjectARB(0);
	
	glPopMatrix();
}

