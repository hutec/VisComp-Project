#include "GLCommon.h"
#include <iostream>

using namespace std;

glm::vec3 camPos;

void
printMat4(glm::mat4 m, std::string matName)
{
    std::cout << matName << std::endl;
    for (int i = 0; i < 4; ++i) {
        std::cout << "col " << i << ": ( " << std::flush;
        for (int j = 0; j < 4; ++j) {
            std::cout << m[i][j] << " " << std::flush;
        }
        std::cout << ")" << std::endl;
    }
    std::cout << std::endl;
}

void 
printVec3(glm::vec3 v, std::string vecName)
{
    std::cout << vecName << ": ( " << std::flush;
    for (int j = 0; j < 2; ++j) {
        std::cout << v[j] << ", " << std::flush;
    }
    std::cout << v[2] << ")" << std::endl;
    std::cout << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//OpenGL utility functions

void LoadProgram(const char* Path, char** pSource, size_t* SourceSize)
{
	FILE* pFileStream = NULL;

	// open the OpenCL source code file
    #ifdef _WIN32   // Windows version
        if(fopen_s(&pFileStream, Path, "rb") != 0) 
        {       
            cout<<"File not found: "<<Path;
			return;
        }
    #else           // Linux version
        pFileStream = fopen(Path, "rb");
        if(pFileStream == 0) 
        {       
            cout<<"File not found: "<<Path;
			return;
        }
    #endif

	//get the length of the source code
	fseek(pFileStream, 0, SEEK_END);
	*SourceSize = ftell(pFileStream);
	fseek(pFileStream, 0, SEEK_SET);

	*pSource = new char[*SourceSize + 1];
	fread(*pSource, *SourceSize, 1, pFileStream);
	fclose(pFileStream);
	(*pSource)[*SourceSize] = '\0';
}

bool CreateShaderFromFile(const char* Path, GLhandleARB shader)
{
	char* sourceCode = NULL;

	size_t sourceLength;

	LoadProgram(Path, &sourceCode, &sourceLength);

	if(sourceCode == NULL)
		return false;

	glShaderSourceARB(shader, 1, (const char**)&sourceCode, NULL);


	if(!CompileGLSLShader(shader))
	{
        GLint gl_LogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &gl_LogLength);
        GLchar* gl_InfoLogBuffer = new GLchar[gl_LogLength];
        glGetShaderInfoLog(shader, gl_LogLength, NULL, gl_InfoLogBuffer);
        std::string error_msg = std::string(gl_InfoLogBuffer);
        delete gl_InfoLogBuffer;

		delete [] sourceCode;
		cout<<"Failed to compile shader: "<<Path<<endl;
		return false;
	}

	delete [] sourceCode;
	return true;
}

bool CompileGLSLShader(GLhandleARB obj)
{

	GLint success;
	glCompileShaderARB(obj);	
	glGetShaderiv(obj, GL_COMPILE_STATUS, &success);

	if(success == GL_FALSE)
	{
		int infoLogLength = 0;
		char infoLog[1024];
		cout<<"There were compile errors:"<<endl;
		glGetShaderInfoLog(obj, 1024, &infoLogLength, infoLog);
		if(infoLogLength > 0)
			cout<<infoLog<<endl;

		return false;
	}

	return true;
}

bool LinkGLSLProgram(GLhandleARB program)
{
	GLint success;

	glLinkProgramARB(program);

    CHECK_FOR_OGL_ERROR();
	glGetProgramiv(program, GL_LINK_STATUS, &success);
    CHECK_FOR_OGL_ERROR();

	if(success == GL_FALSE)
	{
		int infoLogLength = 0;
		char infoLog[1024];
		cout<<"There were link errors:"<<endl;
		glGetProgramInfoLog(program, 1024, &infoLogLength, infoLog);
		if(infoLogLength > 0)
			cout<<infoLog<<endl;

		return false;
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
