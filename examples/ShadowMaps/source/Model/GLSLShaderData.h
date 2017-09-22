#pragma once
#include <string>

/**
*	GLSLShaderData encapsulate all the GLSL data.<br>
*	Constructor initialize data from files.
*/
class GLSLShaderData
{
public:	
	// Functions
	//------------------------------------------------------------
	GLSLShaderData(const std::string& sVSFileName, const std::string& sFSFileName);

	// Variables
	//------------------------------------------------------------
	GLhandleARB			m_vertexShader; // Vertex shader handle
	GLhandleARB			m_fragmentShader; // Fragment shader handle
	GLhandleARB			m_programHandler; // Shader handle
	std::string			m_VSFileName; // Vertex shader filename
	std::string			m_FSFileName; // Fragment shader filename
};