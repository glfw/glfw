#include "IModel.h"
#include <math.h>

/** 
*	This method assumes that the file passed as parameter is a raw block of data made of RGB components (one byte per channel)
*	It is *NOT* a good way to load/store textures, but for sake of simplicity I've decided to use it for this tutorial.
*/
bool IModel::loadTexture(const std::string& textureName)
{
	byte* data = NULL;

	FILE* f;
	fopen_s(&f, textureName.c_str(), "r");
	if(f != NULL)
	{
		fseek (f, 0, SEEK_END);
		unsigned int size = ftell (f);
		fseek (f, 0, SEEK_SET);

		data = new byte[size];
		fread( data, sizeof(byte), size, f);
		fclose(f);

		// Assuming that the raw image is square and RGB; don't fancy doing anything more complicated since the tutorial is not focused on textures loading
		GLuint side = (GLuint)sqrt(size/3.0f);

		// Generate the texture
		if(m_texture != 0)
			glDeleteTextures(1, &m_texture);
		glGenTextures(1, &m_texture);

		glBindTexture(GL_TEXTURE_2D, m_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);	

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glTexImage2D(GL_TEXTURE_2D, 0, 3, side, side, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else
		return false;

	return true;
}

/** 
*	Set position
*/
void IModel::setPosition(float x, float y, float z)
{
	m_posX = x;
	m_posY = y;
	m_posZ = z;
}

/** 
*	Set rotation
*/
void IModel::setRotation(float x, float y, float z)
{
	m_rotX = x;
	m_rotY = y;
	m_rotZ = z;
}

/** 
*	Add a delta to the current rotation
*/
void IModel::addRotation(float x, float y, float z)
{
	m_rotX += x;
	m_rotY += y;
	m_rotZ += z;
}

