#pragma once
#include <windows.h>
#include "gl/glew.h"
#include <gl/gl.h>
#include <gl/glu.h>
#include <string>
#include "GLSLShaderData.h"

/** 
*	Every renderable object inherits from this interface for simplicity
*/
class IModel
{
public:
	// Methods
	IModel(const std::string& sVSFileName, const std::string& sFSFileName)
		: m_shader(sVSFileName, sFSFileName)
		, m_texture(0)
	{
		m_rotX = m_rotY = m_rotZ = 0;
		m_posX = m_posY = m_posZ = 0;

		m_worldMatrixID = glGetUniformLocationARB(m_shader.m_programHandler,"WorldMatrix");
		m_textureID = glGetUniformLocationARB(m_shader.m_programHandler,"tDiffuse");
	}
	virtual ~IModel(){}

	bool			loadTexture(const std::string& textureName);
	void			setPosition(float x, float y, float z);
	void			setRotation(float x, float y, float z);
	void			addRotation(float x, float y, float z);
	virtual void	render() const = 0;

protected:
	// Fields
	GLSLShaderData 	m_shader; // Every model must have a shader associated (both vertex and fragment)

	GLuint			m_worldMatrixID; // This ID is used to pass the world matrix into the shader
	float			m_rotX, m_rotY, m_rotZ; // Rotations
	float			m_posX, m_posY, m_posZ; // Positions
	
	GLuint			m_textureID; // Texture ID used to pass the texture into the shader
	GLuint			m_texture; // OpenGL texture ID
};

