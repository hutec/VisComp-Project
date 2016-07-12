#ifndef GL_COMMON_H
#define GL_COMMON_H

#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <string>
#include <vector>
#include "OGLTexture.h"

class VCModel;

struct EnvVar {
    glm::vec3 camPos;
    OGLTexture envMap;
    glm::mat4 viewMat;
    glm::mat4 projMat;
    std::vector<VCModel *> scene;
};

extern EnvVar ENV_VAR;

void printMat4(glm::mat4 m, std::string matName = "");
void printVec3(glm::vec3 v, std::string vecName = "");

//OpenGL utility functions
bool CreateShaderFromFile(const char* Path, GLhandleARB shader);

//utility function which attempts to compile a GLSL shader, then prints the error messages on failure
bool CompileGLSLShader(GLhandleARB shader);
bool LinkGLSLProgram(GLhandleARB program);

// Utility functions for memory managment
#define SAFE_RELEASE_GL_BUFFER(obj)  do {if(obj){ glDeleteBuffers(1, &obj); obj = 0; }} while(0)
#define SAFE_RELEASE_GL_SHADER(obj)  do {if(obj){ glDeleteShader(obj); obj = 0; }} while(0)
#define SAFE_RELEASE_GL_PROGRAM(obj) do {if(obj){ glDeleteProgram(obj); obj = 0; }} while(0)
#define SAFE_DELETE(obj)			 do {if(obj){ delete obj; obj = NULL; }} while (0)

#define CHECK_FOR_OGL_ERROR()                                  \
   do {                                                        \
     GLenum err;                                               \
     err = glGetError();                                       \
     if (err != GL_NO_ERROR)                                   \
     {                                                         \
       if (err == GL_INVALID_FRAMEBUFFER_OPERATION_EXT)        \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: Invalid Framebuffer Operation\n",\
                 __FILE__, __LINE__);                          \
       }                                                       \
       else                                                    \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: %s\n",               \
                 __FILE__, __LINE__, gluErrorString(err));     \
       }                                                       \
     }                                                         \
   } while(0)

#define V_RETURN_OGL_ERROR()                            \
	do {                                                        \
     GLenum err;                                               \
     err = glGetError();                                       \
	if (err != GL_NO_ERROR)                                   \
     {                                                         \
       if (err == GL_INVALID_FRAMEBUFFER_OPERATION_EXT)        \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: Invalid Framebuffer Operation\n",\
                 __FILE__, __LINE__);                          \
       }                                                       \
       else                                                    \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: %s\n",               \
                 __FILE__, __LINE__, gluErrorString(err));     \
       }                                                       \
       return false;                                           \
     }                                                         \
   } while(0)


#endif // GL_COMMON_H
