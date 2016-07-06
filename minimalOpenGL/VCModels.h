/*
*  VCModel class wraps shader, shader program and transformation 
*  VCWVObjModel class is derived from VCModel. 
*  It reads Wavefront .obj file and create VAO, VBOs and textures for OpenGL
*
*  Written by Chao Jia, 2016.
*/

#pragma once
#include "helper/wavefront_obj_loader/GLMmodel.h"
#include "helper/OGLTexture.h"
#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include <iostream>
#include <map>
#include <vector>
#include "helper/GLCommon.h"
#include <algorithm>
#include <memory>

class VCModel {
public:
    // shaderPaths format: <path, shader type>
    // names of all uniforms used in all shaders should be in uniformNames
    VCModel(const std::map<std::string, GLenum> &shaderPaths, 
            const std::vector<std::string> &uniformNames);
    virtual ~VCModel();
    bool initShaderProg();
    glm::mat4 modelMat() const;
    glm::mat3 normalMat() const;
    void setTranslation(const glm::vec3 &_translation) { m_translation = _translation; }
    void translate(const glm::vec3 &deltaT);
    void setRotation(const glm::quat &_rotation) { m_rotation = _rotation; }
    // angle in radians
    void rotate(float angle, const glm::vec3 &axis);
    void setScaleFactor(const glm::vec3 &_factor) { m_scaleFactor = _factor; }
    void scale(const glm::vec3 &deltaFactor);
    void resetTransform();

protected:
    GLuint m_shaderProg;
    glm::quat m_rotation;
    glm::vec3 m_translation;
    glm::vec3 m_scaleFactor;
    std::map<std::string, GLenum> m_shaderPaths;
    std::map<std::string, GLuint> m_uniformLocs;
};

/////////////////////////////////////////////////////////////////////////////////////////
// specify what kinds of data will be read from .obj/.mtl file 
// for all classes derived from VCWVObjModel.
// uniform names "diffuse", "specular", "shininess", "emmissive", "ambient" in shader
// are used exclusively for data read from .mtl file if corresponding options 
// are turned on. 
//
// position, vertex attribute
const GLuint VC_POS = 0x0001 << 0; 
// normal, vertex attribute
const GLuint VC_NORM = 0x0001 << 1; 
// tex coordinates, vertex attribute
const GLuint VC_TEX = 0x0001 << 2;

// diffuse, a uniform named "diffuse" is expected in Shader
const GLuint VC_KD = 0x0001 << 3; 
// diffuse texture, if on, binding location is 0
const GLuint VC_KD_MAP = 0x0001 << 4; 
// specular, a uniform named "specular" is expected in Shader
const GLuint VC_KS = 0x0001 << 5; 
// specular texture, if on and VC_KD_MAP is also on, binding loc is 1, otherwise binding loc is 0
const GLuint VC_KS_MAP = 0x0001 << 6; 
// shininess exponent, a uniform named "shininess" is expected in Shader
const GLuint VC_NS = 0x0001 << 7; 
// emmissive, a uniform named "emmissive" is expected in Shader
const GLuint VC_KE = 0x0001 << 8; 
// ambient, a uniform named "ambient" is expected in Shader
const GLuint VC_KA = 0x0001 << 9; 

/////////////////////////////////////////////////////////////////////////////////////////
class VCMtlGroup {
private:
    std::vector<std::vector<GLfloat>> m_data; // 0: position, 1: normal, 2: texCoord
public:
    std::string m_mtlName;
    GLuint m_option;
    GLuint m_numVert;
    GLuint m_vao;
    std::vector<GLuint> m_BOs;

    GLfloat m_diffuse[4]; 
    GLfloat m_ambient[4];
    GLfloat m_specular[4];
    GLfloat m_emmissive[4];
    GLfloat m_shininess;
    
    VCMtlGroup(GLMmaterial* pMtl, GLuint _option);
    ~VCMtlGroup();
    void addTriangle(GLMtriangle* tri, GLMmodel *model);
    void initVao();

};

/////////////////////////////////////////////////////////////////////////////////////////
class VCWVObjGroup {
public:
    std::string m_name;
    std::vector<VCMtlGroup *> m_mtlGroups;
    VCWVObjGroup(GLMgroup *glmGrp, GLMmodel *model, GLuint _option);
    size_t idxMtlGroup(GLMmaterial* pMtl, GLuint _option);
    ~VCWVObjGroup();
};

/////////////////////////////////////////////////////////////////////////////////////////
class VCWVObjModel : public VCModel {
public:
    // if _texSuffix is not null, texture of name "mtlName._texSuffix" is created 
    VCWVObjModel(const std::string &_objPath,
        const std::map<std::string, GLenum> &_shaderPaths,
        const std::vector<std::string> &_uniformNames,
        GLuint _option, 
        const std::string& _texSuffix = std::string("")); 
    ~VCWVObjModel();
protected:
    std::vector<VCWVObjGroup *> m_groups;
    std::map<std::string, std::vector<std::unique_ptr<OGLTexture>>> m_texes; // <mtl_name, texture(s)>
    GLuint m_option;

    VCWVObjGroup* getGroup(const std::string &_groupName);

    // diffuse, specular... and texture, 
    // the uniform names used in shader are fixed, i.e. must be "diffuse", "specular", "ambient" ...
    void setupMtlUniforms(VCMtlGroup* _mtlGrp); 

    // all mtls use the same texture
    void setupTexForAllMtls(const std::string& _texName); 

    // should be called in the constructor of derived class. store textures and mtl name in m_texes
    // if textures for both kd and ks are used, kd tex binding loc = 0, ks tex binding loc = 1
    virtual void setupTexForMtls() {}

private:
    void autoSetupTexForMtls(const std::string &_objPath, const std::string& _texSuffix);
};


/////////////////////////////////////////////////////////////////////////////////////////
const GLuint TEXT2D_OPTION = VC_POS | VC_TEX | VC_KD_MAP;
class VCText2D : public VCWVObjModel {
public:
    VCText2D(const std::string &_objPath,
        const std::map<std::string, GLenum> &_shaderPaths,
        const std::vector<std::string> &_uniformNames,
        const std::string& _texName, GLuint _option = TEXT2D_OPTION); // all mtls use the same texture
    ~VCText2D() {}
    void update(float elapsedTime);
    void alignToCamera(glm::vec3 viewDir, glm::vec3 worldUp);
    void setLeapPosition(glm::vec3 pos);
    void draw(const glm::mat4 &projMat, glm::mat4 &viewMat);
private:
    glm::vec4 m_leapPos;

    
};

/////////////////////////////////////////////////////////////////////////////////////////
const GLuint VCCH3D_OPTION = VC_POS | VC_NORM | VC_KD | VC_KS | VC_NS;

class VCCh3D : public VCWVObjModel {
public:
    VCCh3D(const std::string &_objPath,
        const std::map<std::string, GLenum> &_shaderPaths,
        const std::vector<std::string> &_uniformNames, GLuint _option = VCCH3D_OPTION);
    ~VCCh3D() {}
    void update(float elapsedTime);
    void draw(const glm::mat4 &projMat, glm::mat4 &viewMat);
};


/////////////////////////////////////////////////////////////////////////////////////////
const GLuint VCPSMODEL_OPTION = VC_POS | VC_NORM | VC_TEX | VC_KD_MAP;

// PhotoScan model
class VCPSModel : public VCWVObjModel {
public:
    VCPSModel(const std::string &_objPath,
        const std::map<std::string, GLenum> &_shaderPaths,
        const std::vector<std::string> &_uniformNames,
        const std::string& _texSuffix, GLuint _option = VCPSMODEL_OPTION);
    ~VCPSModel() {}
    void update(float elapsedTime);
    void draw(const glm::mat4 &projMat, glm::mat4 &viewMat);
};

/////////////////////////////////////////////////////////////////////////////////////////
class Sky : public VCModel {
public:
    Sky(const std::map<std::string, GLenum> &_shaderPaths,
        const std::vector<std::string> &_uniformNames);
    ~Sky() { glDeleteVertexArrays(1, &m_vao); }
    void draw(int windowWidth, int windowHeight, const float* cameraToWorldMatrix, 
        const float* projectionMatrixInverse, const float* light);
private:
    GLuint m_vao;
};