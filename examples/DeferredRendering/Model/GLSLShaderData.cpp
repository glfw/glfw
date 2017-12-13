#include <windows.h>
#include "gl/glew.h"
#include <gl/gl.h>
#include <gl/glu.h>
#include "GLSLShaderData.h"

/**
*	Constructor
*/
GLSLShaderData::GLSLShaderData(const std::string& _sVSFileName, const std::string& _sFSFileName) 
	: m_VSFileName(_sVSFileName)
	, m_FSFileName(_sFSFileName)
{
	// Create OGL resources
	m_vertexShader		= glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	m_fragmentShader	= glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	//Read out the shader data from the given files
	char *sVSData, *sFSData;

	//Read out the vertex shader data
	FILE *file;
	fopen_s(&file, m_VSFileName.c_str(),"r");
	if(!file)
		throw new std::exception( std::string("Can't load GLSL Vertex Shader from file: " + m_VSFileName).c_str() );

	fseek(file, 0, SEEK_END);
    long count = ftell(file);
    rewind(file);

	sVSData = new char[count+1];
	memset(sVSData,0,count+1);
	fread(sVSData, 1, count, file);
	fclose(file);

	file = NULL;

	//Read out the fragment shader data
	fopen_s(&file, m_FSFileName.c_str(),"r");
	if(!file)
		throw new std::exception( std::string("Can't load GLSL Fragment Shader from file: "+m_FSFileName).c_str() );
	
	fseek(file, 0, SEEK_END);
    count = ftell(file);
    rewind(file);

	sFSData = new char[count+1];
	memset(sFSData,0,count+1);
	fread(sFSData, 1, count, file);
	fclose(file);

	// Now that we have the two shaders in memory we can compile them
	const char * pVS = sVSData;
	const char * pFS = sFSData;
	int			bCompiled = false;
	
	glShaderSourceARB(m_vertexShader, 1, &pVS, NULL);
	glCompileShaderARB(m_vertexShader);

    glGetObjectParameterivARB( m_vertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled );
    if( bCompiled  == false )
		throw new std::exception("Vertex shader compilation failed.");

	glShaderSourceARB(m_fragmentShader, 1, &pFS, NULL);
	glCompileShaderARB(m_fragmentShader);
	
    glGetObjectParameterivARB( m_fragmentShader, GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled );
    if( bCompiled  == false )
		throw new std::exception("Fragment shader compilation failed.");
	
	// Once compiled we can bind everything together for OpenGL to use
	m_programHandler = glCreateProgramObjectARB();
	glAttachObjectARB(m_programHandler,m_vertexShader);
	glAttachObjectARB(m_programHandler,m_fragmentShader);
	
	glLinkProgramARB(m_programHandler);

	// We release the shader data read from file since we now have everything compiled in memory
	delete [] sFSData;
	delete [] sVSData;
}

