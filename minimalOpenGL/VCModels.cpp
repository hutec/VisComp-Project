/*
*  VCModel class wraps shader, shader program and transformation
*  VCWVObjModel class is derived from VCModel.
*  It reads Wavefront .obj file and creates VAO, VBOs and optionally textures for OpenGL
*
*  Written by Chao Jia, 2016.
*/

#include <gtc/matrix_transform.hpp>
//#include "glm/ext.hpp"
#include "VCModels.h"

VCModel::VCModel(const std::map<std::string, GLenum> &shaderPaths,
    const std::vector<std::string> &uniformNames)
{
    m_shaderPaths = shaderPaths;
    for (auto i : uniformNames) {
        m_uniformLocs[i] = 0;
    }
    if (!initShaderProg()) {
        std::cout << "init shader program failed" << std::endl;
    }
    resetTransform();
}


VCModel::~VCModel()
{
    if (glIsProgram(m_shaderProg)) {
        glDeleteProgram(m_shaderProg);
    }
}

// could be used in refreshing shader program
bool
VCModel::initShaderProg()
{
    if (glIsProgram(m_shaderProg)) {
        glDeleteProgram(m_shaderProg);
    }
    m_shaderProg = glCreateProgram();
    for (auto i : m_shaderPaths) {
        GLuint shader = glCreateShader(i.second);
        if (!CreateShaderFromFile(i.first.c_str(), shader)) {
            std::cerr << "Loading shader " << i.first << " failed " << std::endl;
            return false;
        }
        glAttachShader(m_shaderProg, shader);
        glDeleteShader(shader);
    }

    if (!LinkGLSLProgram(m_shaderProg))
    {
        std::cout << "Linking shader failed " << std::endl;
        return false;
    }

    for (auto i : m_uniformLocs) {
        m_uniformLocs[i.first] = glGetUniformLocation(m_shaderProg, i.first.c_str());
    }
    return true;
}

glm::mat4
VCModel::modelMat() const
{
    glm::mat4 mm = glm::mat4(1.0f);
    mm = glm::translate(mm, m_translation);
    mm = mm * glm::mat4_cast(m_rotation);
    mm = glm::scale(mm, m_scaleFactor);
    return mm;
}


glm::mat3
VCModel::normalMat() const
{
    glm::mat4 mm = modelMat();
    return glm::mat3(glm::transpose(glm::inverse(mm)));
}

void
VCModel::translate(const glm::vec3 &deltaT)
{
    m_translation += deltaT;
}

// angle in radians, 
// looks like the glm developers confuse themselves,
// documentation is not consistent with the actual behaviour of glm::rotate(...)
void
VCModel::rotate(float angle, const glm::vec3 &axis)
{
    m_rotation = glm::rotate(m_rotation, angle, axis);
}

void
VCModel::scale(const glm::vec3 &deltaFactor)
{
    m_scaleFactor += deltaFactor;
}

void
VCModel::resetTransform()
{
    m_rotation = glm::quat();
    m_translation = glm::vec3(0.f, 0.f, 0.f);
    m_scaleFactor = glm::vec3(1.f);
}

/////////////////////////////////////////////////////////////////////////////////////////
VCMtlGroup::VCMtlGroup(GLMmaterial* pMtl, GLuint _option)
{
    m_mtlName = std::string(pMtl->name);
    m_option = _option;
    std::memcpy(m_diffuse, pMtl->diffuse, 4 * sizeof(GLfloat));
    std::memcpy(m_specular, pMtl->specular, 4 * sizeof(GLfloat));
    std::memcpy(m_ambient, pMtl->ambient, 4 * sizeof(GLfloat));
    std::memcpy(m_emmissive, pMtl->emmissive, 4 * sizeof(GLfloat));
    m_shininess = pMtl->shininess;
    m_numVert = 0;
    if (m_option & VC_POS) m_data.push_back(std::vector<GLfloat>());
    if (m_option & VC_NORM) m_data.push_back(std::vector<GLfloat>());
    if (m_option & VC_TEX) m_data.push_back(std::vector<GLfloat>());
}

void
VCMtlGroup::addTriangle(GLMtriangle* tri, GLMmodel *model)
{
    for (int i = 0; i < 3; ++i) {
        int dataIdx = 0;
        if (m_option & VC_POS) {
            for (int j = 0; j < 3; ++j) {
                m_data[dataIdx].push_back(model->vertices[(tri->vindices)[i] * 3 + j]);
            }
            ++dataIdx;
        }
        if (m_option & VC_NORM) {
            for (int j = 0; j < 3; ++j) {
                m_data[dataIdx].push_back(model->normals[(tri->nindices)[i] * 3 + j]);
            }
            ++dataIdx;
        }
        if (m_option & VC_TEX) {
            for (int j = 0; j < 2; ++j) {
                m_data[dataIdx].push_back(model->texcoords[(tri->tindices)[i] * 2 + j]);
            }
        }
    }
    m_numVert += 3;
}

void 
VCMtlGroup::initVao()
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    int vAttrLoc = 0;
    if (m_option & VC_POS) {
        GLuint posBO;
        glGenBuffers(1, &posBO);
        m_BOs.push_back(posBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_BOs[vAttrLoc]);
        glBufferData(GL_ARRAY_BUFFER, m_numVert * 3 * sizeof(GLfloat), m_data[vAttrLoc].data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vAttrLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vAttrLoc);
        ++vAttrLoc;
    }
    if (m_option & VC_NORM) {
        GLuint normBO;
        glGenBuffers(1, &normBO);
        m_BOs.push_back(normBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_BOs[vAttrLoc]);
        glBufferData(GL_ARRAY_BUFFER, m_numVert * 3 * sizeof(GLfloat), m_data[vAttrLoc].data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vAttrLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vAttrLoc);
        ++vAttrLoc;
    }
    if (m_option & VC_TEX) {
        GLuint texCoordBO;
        glGenBuffers(1, &texCoordBO);
        m_BOs.push_back(texCoordBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_BOs[vAttrLoc]);
        glBufferData(GL_ARRAY_BUFFER, m_numVert * 2 * sizeof(GLfloat), m_data[vAttrLoc].data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vAttrLoc);
    }
    for (auto i : m_data) {
        i.clear();
    }
    m_data.clear();
}

VCMtlGroup::~VCMtlGroup()
{
    for (int i = 0; i < m_BOs.size(); ++i) {
        GLuint bo = m_BOs[i];
        glDeleteBuffers(1, &bo);
    }
    glDeleteVertexArrays(1, &m_vao);
}

/////////////////////////////////////////////////////////////////////////////////////////
VCWVObjGroup::VCWVObjGroup(GLMgroup *glmGrp, GLMmodel *model, GLuint _option)
{
    m_name = std::string(glmGrp->name);
    for (GLuint i = 0; i < glmGrp->numtriangles; ++i) { // iterate all triangles
        GLuint TriIdx = glmGrp->triangles[i];
        GLMtriangle* pTri = model->triangles + TriIdx;
        GLMmaterial* pMtl = model->materials + pTri->material;
        m_mtlGroups[idxMtlGroup(pMtl, _option)]->addTriangle(pTri, model);
    }
    for (auto i : m_mtlGroups) {
        i->initVao();
    }
}

size_t
VCWVObjGroup::idxMtlGroup(GLMmaterial* pMtl, GLuint _option)
{
    for (int i = 0; i < m_mtlGroups.size(); ++i) {
        if (m_mtlGroups[i]->m_mtlName == std::string(pMtl->name)) return i;
    }
    m_mtlGroups.push_back(new VCMtlGroup(pMtl, _option));
    return m_mtlGroups.size() - 1;
}

VCWVObjGroup::~VCWVObjGroup()
{
    for (auto i : m_mtlGroups) delete i;
}

/////////////////////////////////////////////////////////////////////////////////////////

VCWVObjModel::VCWVObjModel(const std::string &_objPath,
    const std::map<std::string, GLenum> &_shaderPaths,
    const std::vector<std::string> &_uniformNames,
    GLuint _option, 
    const  std::string& _texSuffix) :
    VCModel(_shaderPaths, _uniformNames)
{
    m_option = _option;
    char *path = new char[_objPath.length() + 1];
    strcpy(path, _objPath.c_str());
    GLMmodel *model = glmReadOBJ(path);
    delete[] path;
    glmUnitize(model);

    if (model->numnormals == 0) {
        glmFacetNormals(model);
        glmVertexNormals(model, 90);
    }

    GLMgroup* group = model->groups;
    while (group->numtriangles > 0) {
        assert(glGetError() == GL_NONE);
        VCWVObjGroup *wvobjGrp = new VCWVObjGroup(group, model, m_option);
        assert(glGetError() == GL_NONE);
        m_groups.push_back(wvobjGrp);
        group = group->next;
    }
    SAFE_DELETE(model);
    std::cout << "loaded model " << _objPath << std::endl;
    if (_texSuffix != std::string("")) {
        autoSetupTexForMtls(_objPath, _texSuffix);
    }
}

VCWVObjGroup* 
VCWVObjModel::getGroup(const std::string &_groupName)
{
    for (auto i : m_groups) {
        if (i->m_name == _groupName) return i;
    }
    return nullptr;
}

void
VCWVObjModel::setupMtlUniforms(VCMtlGroup* _mtlGrp)
{
    assert(glGetError() == GL_NONE);
    if (m_option & VC_KD) {
        glUniform4fv(m_uniformLocs["diffuse"], 1, _mtlGrp->m_diffuse);
    }
    if (m_option & VC_KS) {
        glUniform4fv(m_uniformLocs["specular"], 1, _mtlGrp->m_specular);
    }
    if (m_option & VC_KA) {
        glUniform4fv(m_uniformLocs["ambient"], 1, _mtlGrp->m_ambient);
    }
    if (m_option & VC_KE) {
        glUniform4fv(m_uniformLocs["emmissive"], 1, _mtlGrp->m_emmissive);
    }
    if (m_option & VC_NS) {
        glUniform1f(m_uniformLocs["shininess"], _mtlGrp->m_shininess);
    }
    assert(glGetError() == GL_NONE);
    int texBindingLoc = 0;
    if (m_option & VC_KD_MAP) {
        glActiveTexture(GL_TEXTURE0 + texBindingLoc);
        m_texes[_mtlGrp->m_mtlName][texBindingLoc]->bind();
        ++texBindingLoc;
    }
    if (m_option & VC_KS_MAP) {
        glActiveTexture(GL_TEXTURE0 + texBindingLoc);
        m_texes[_mtlGrp->m_mtlName][texBindingLoc]->bind();
		++texBindingLoc;
    }

	// Check if there is a enhanced texture 
	if (texBindingLoc < m_texes[_mtlGrp->m_mtlName].size()) {
		glActiveTexture(GL_TEXTURE3);
		m_texes[_mtlGrp->m_mtlName][texBindingLoc]->bind();
		//glUniform1i(m_uniformLocs["enhanced_texture"], 1);
	}

    assert(glGetError() == GL_NONE);
}

void
VCWVObjModel::setupTexForAllMtls(const std::string& _texName)
{
    for (auto i : m_groups) {
        for (auto j : i->m_mtlGroups) {
            //if (m_texes.find(j->m_mtlName) == m_texes.end()) {
			if (true) {
                std::string texName = _texName;
                std::unique_ptr<OGLTexture> texPtr(new OGLTexture());
                char *texNameCstr = new char[texName.length() + 1];
                strcpy(texNameCstr, texName.c_str());
                m_texes[j->m_mtlName].push_back(std::unique_ptr<OGLTexture>(new OGLTexture()));
                size_t size = m_texes[j->m_mtlName].size();
                if (!m_texes[j->m_mtlName][size-1]->load(texNameCstr)) {
                    std::cerr << "load texture " << texName << " failed" << std::endl;
                }
                delete[] texNameCstr;
                // std::vector<std::unique_ptr<OGLTexture>> texVector = {};
                // m_texes[j->m_mtlName] = texVector;
                
                std::cout << "added texture " << texName << " for " << j->m_mtlName << std::endl;
				std::cout << "Texture size: " << m_texes[j->m_mtlName].size() << std::endl;
            }
        }
    }
}

void 
VCWVObjModel::autoSetupTexForMtls(const std::string &_objPath, const std::string& _texSuffix)
{
    auto const slashPos = _objPath.find_last_of('/');
    std::string parentPath{ "" };
    if (slashPos != std::string::npos) {
        parentPath = _objPath.substr(0, slashPos + 1);
    }
    for (auto i : m_groups) {
        for (auto j : i->m_mtlGroups) {
            if (m_texes.find(j->m_mtlName) == m_texes.end()) {
                std::string texName = parentPath + j->m_mtlName + _texSuffix;
                std::unique_ptr<OGLTexture> texPtr(new OGLTexture());
                char *texNameCstr = new char[texName.length() + 1];
                strcpy(texNameCstr, texName.c_str());
                m_texes[j->m_mtlName].push_back(std::unique_ptr<OGLTexture> (new OGLTexture()));
                size_t size = m_texes[j->m_mtlName].size();
                if (!m_texes[j->m_mtlName][size-1]->load(texNameCstr)) {
                    std::cerr << "load texture " << texName << " failed" << std::endl;
                }
                delete[] texNameCstr;
                // std::vector<std::unique_ptr<OGLTexture>> texVector;
                // texVector.push_back(texPtr);
                // m_texes[j->m_mtlName] = texVector;
                // m_texes[j->m_mtlName].push_back(texPtr);
                std::cout << "added texture " << texName << " for " << j->m_mtlName << std::endl;
            }
        }
    }
}

void
VCWVObjModel::setEnhancedTexture(const std::string& _texName)
{
	setupTexForAllMtls(_texName);
}

void
VCWVObjModel::setLeapPosition(glm::vec3 pos)
{
	//TODO this is far from optimal, try to remap the hand positions to OpenG Space
	/*for (int i = 0; i < 3; ++i) {
		pos[i] = std::max(0.f, std::min(std::abs(pos[i]), 255.f)) / 255.f;
	}*/
	m_leapPos = glm::vec4(pos.x, pos.y, pos.z, 1);
}



VCWVObjModel::~VCWVObjModel()
{
    for (auto i : m_groups) {
        delete i;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

VCText2D::VCText2D(const std::string &_objPath,
    const std::map<std::string, GLenum> &_shaderPaths,
    const std::vector<std::string> &_uniformNames,
    const std::string& _texName, GLuint _option)
    : VCWVObjModel(_objPath, _shaderPaths, _uniformNames, _option)
{
    m_leapPos = glm::vec4(0.f);
    if (_texName.length() != 0) setupTexForAllMtls(_texName);
}

void
VCText2D::update(float elapsedTime)
{
    float rotation_speed = glm::radians(0.1f);
    rotate(rotation_speed * elapsedTime * 1000, glm::vec3(0.0f, 1.0f, 0.0f));
}

void
VCText2D::alignToCamera(glm::vec3 camPos, glm::vec3 worldUp)
{
    glm::vec3 textOrientation(modelMat() * glm::vec4(0.f, 0.f, -1.f, 0.f));
    textOrientation.y = 0;
    glm::vec3 objPos(modelMat() * glm::vec4(0.f, 0.f, 0.f, 1.f));
    glm::vec3 V = objPos - camPos;
    V.y = 0;
    if (glm::length(V) == 0.f) return;

    V = glm::normalize(V);
    textOrientation = glm::normalize(textOrientation);
    glm::vec3 axis;
    float theta;
    float epsilon = 0.00001f;
    if (glm::abs(glm::dot(textOrientation, V) - 1.f) < epsilon) {
        return;
    }
    else if (glm::abs(glm::dot(textOrientation, V) + 1.f) < epsilon) {
        axis = glm::normalize(worldUp);
        theta = 180;
    }
    else {
        axis = glm::normalize(glm::cross(textOrientation, V));
        float cosTheta = glm::dot(textOrientation, V);
        // theta = glm::degrees(glm::acos(cosTheta));
        theta = glm::acos(cosTheta);
    }
    // std::cout << "align rotation: " << theta << ", " << std::flush;
    // printVec3(axis);
    rotate(theta, axis);
}

void
VCText2D::draw()
{
    assert(glGetError() == GL_NONE);

    glm::mat4 mvp = ENV_VAR.projMat * ENV_VAR.viewMat * modelMat();
    // printMat4(mvp, "MVP");
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    // glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glDisable(GL_BLEND);

    assert(glGetError() == GL_NONE);

    glUseProgram(m_shaderProg);

    glUniformMatrix4fv(m_uniformLocs["MVP"], 1, GL_FALSE, &mvp[0][0]);
    glUniform4fv(m_uniformLocs["leappos"], 1, &m_leapPos[0]);

    for (auto grp : m_groups) {
        for (auto mtlGrp : grp->m_mtlGroups) {
            glBindVertexArray(mtlGrp->m_vao);
            setupMtlUniforms(mtlGrp);
			glDrawArrays(GL_TRIANGLES, 0, mtlGrp->m_numVert);
        }
    }
    assert(glGetError() == GL_NONE);
}

/////////////////////////////////////////////////////////////////////////////////////////
VCCh3D::VCCh3D(const std::string &_objPath,
    const std::map<std::string, GLenum> &_shaderPaths,
    const std::vector<std::string> &_uniformNames, GLuint _option) :
    VCWVObjModel(_objPath, _shaderPaths, _uniformNames, _option)
{
    rotate(M_PI / 2.f, glm::vec3(1, 0, 0));
}

void
VCCh3D::update(float elapsedTime)
{
    float rotation_speed = glm::radians(0.1f);
    glm::vec3 axis(glm::inverse(modelMat()) * glm::vec4(0.f, 0.f, 1.f, 0.f));
    rotate(rotation_speed * elapsedTime * 1000, axis);
}

void
VCCh3D::draw()
{
    glm::mat4 mm = modelMat();
    glm::mat4 mvp = ENV_VAR.projMat * ENV_VAR.viewMat *mm;
    glm::mat3 nm = normalMat();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    
	//TODO disable, just on for having sphere indicate finger position
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    assert(glGetError() == GL_NONE);

    glUseProgram(m_shaderProg);
    glUniformMatrix4fv(m_uniformLocs["modelMat"], 1, GL_FALSE, &mm[0][0]);
    glUniformMatrix4fv(m_uniformLocs["MVP"], 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix3fv(m_uniformLocs["normalMat"], 1, GL_FALSE, &nm[0][0]);
    glUniform3fv(m_uniformLocs["camPos"], 1, &ENV_VAR.camPos[0]);
    glm::vec3 lightPos = glm::vec3(0.f, 1.f, 0.f) + ENV_VAR.camPos;
    glUniform3fv(m_uniformLocs["lightPos"], 1, &lightPos[0]);

	glm::vec4 pos = mm * glm::vec4(ENV_VAR.camPos, 1.0);
	//std::cout << "vsWorldPos: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
	//std::cout << "leapPos: " << m_leapPos.x << ", " << m_leapPos.y << ", " << m_leapPos.z << std::endl;


    for (auto grp : m_groups) {
        for (auto mtlGrp : grp->m_mtlGroups) {
            glBindVertexArray(mtlGrp->m_vao);
            setupMtlUniforms(mtlGrp);
            glDrawArrays(GL_TRIANGLES, 0, mtlGrp->m_numVert);
        }
    }

	glDisable(GL_BLEND);

    assert(glGetError() == GL_NONE);
}


/////////////////////////////////////////////////////////////////////////////////////////

VCPSModel::VCPSModel(const std::string &_objPath,
    const std::map<std::string, GLenum> &_shaderPaths,
    const std::vector<std::string> &_uniformNames,
    const std::string &_texSuffix, GLuint _option) :
    VCWVObjModel(_objPath, _shaderPaths, _uniformNames, _option, _texSuffix)
{
    rotate(M_PI / 2.0, glm::vec3(1, 0, 0));
}

void
VCPSModel::update(float elapsedTime)
{
    float rotation_speed = glm::radians(0.1f);
    glm::vec3 axis(glm::inverse(modelMat()) * glm::vec4(0.f, 1.f, 0.f, 0.f));
    rotate(rotation_speed * elapsedTime * 1000, axis);
}

void
VCPSModel::draw()
{
    glm::mat4 mm = modelMat();
    glm::mat4 mvp = ENV_VAR.projMat * ENV_VAR.viewMat *mm;
    glm::mat3 nm = normalMat();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    assert(glGetError() == GL_NONE);

    glUseProgram(m_shaderProg);
    glUniformMatrix4fv(m_uniformLocs["modelMat"], 1, GL_FALSE, &mm[0][0]);
    glUniformMatrix4fv(m_uniformLocs["MVP"], 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix3fv(m_uniformLocs["normalMat"], 1, GL_FALSE, &nm[0][0]);
    glUniform3fv(m_uniformLocs["camPos"], 1, &ENV_VAR.camPos[0]);
    glm::vec3 lightPos = glm::vec3(0.f, 1.f, 0.f) + ENV_VAR.camPos;
    glUniform3fv(m_uniformLocs["lightPos"], 1, &lightPos[0]);

	glUniform4fv(m_uniformLocs["leapPos"], 1, &m_leapPos[0]);


    for (auto grp : m_groups) {
        for (auto mtlGrp : grp->m_mtlGroups) {
            glBindVertexArray(mtlGrp->m_vao);
            setupMtlUniforms(mtlGrp);
            glDrawArrays(GL_TRIANGLES, 0, mtlGrp->m_numVert);
        }
    }

    assert(glGetError() == GL_NONE);
}


/////////////////////////////////////////////////////////////////////////////////////////
Sky::Sky(const std::map<std::string, GLenum> &_shaderPaths,
    const std::vector<std::string> &_uniformNames) :
    VCModel(_shaderPaths, _uniformNames)
{
    glGenVertexArrays(1, &m_vao); 
}


void
Sky::draw(int windowWidth, int windowHeight, const float* cameraToWorldMatrix, 
    const float* projectionMatrixInverse, const float* light)
{
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindVertexArray(m_vao);
    glUseProgram(m_shaderProg);
    glUniform3fv(m_uniformLocs["light"], 1, light);
    glUniform2f(m_uniformLocs["resolution"], float(windowWidth), float(windowHeight));
    glUniformMatrix4fv(m_uniformLocs["cameraToWorldMatrix"], 1, GL_TRUE, cameraToWorldMatrix);
    glUniformMatrix4fv(m_uniformLocs["invProjectionMatrix"], 1, GL_TRUE, projectionMatrixInverse);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

/////////////////////////////////////////////////////////////////////////////////////////
SphereSky::SphereSky(const std::map<std::string, GLenum> &_shaderPaths,
    const std::vector<std::string> &_uniformNames,
    const std::string& _sphereObjPath) :
    VCWVObjModel(_sphereObjPath, _shaderPaths, _uniformNames, SPHERE_SKY_OPTION)
{

}

void
SphereSky::draw()
{
    assert(glGetError() == GL_NONE);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glUseProgram(m_shaderProg);
    glm::mat4 mvp = ENV_VAR.projMat * ENV_VAR.viewMat *modelMat();
    glUniformMatrix4fv(m_uniformLocs["MVP"], 1, GL_FALSE, &mvp[0][0]);

    GLfloat tessLvl = 16;
    GLfloat outerLevel[4] = { tessLvl, tessLvl, tessLvl, tessLvl };
    GLfloat innerLevel[2] = { tessLvl, tessLvl };

    glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, outerLevel);
    glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, innerLevel);
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    for (auto grp : m_groups) {
        for (auto mtlGrp : grp->m_mtlGroups) {
            glBindVertexArray(mtlGrp->m_vao);
            setupMtlUniforms(mtlGrp); // for SPHERE_SKY_OPTION this line is actually not necessary
            glActiveTexture(GL_TEXTURE0);
            ENV_VAR.envMap.bind();
            glDrawArrays(GL_PATCHES, 0, mtlGrp->m_numVert);
        }
    }
    assert(glGetError() == GL_NONE);

}

/////////////////////////////////////////////////////////////////////////////////////////
SkySphere::SkySphere(const std::map<std::string, GLenum> &_shaderPaths,
    const std::vector<std::string> &_uniformNames) :
    VCModel(_shaderPaths, _uniformNames)
{
    float r = 1.f;
    glm::vec3 vertices[] = {
        glm::vec3( 0.f,  1.f,  0.f),
        glm::vec3( 0.f,  0.f,  1.f),
        glm::vec3( 1.f,  0.f,  0.f),
        glm::vec3( 0.f,  0.f, -1.f),
        glm::vec3(-0.999999f,  0.f,  -0.00001f), // break the sphere at(-1, 0, 0)
        glm::vec3(-0.999999f,  0.f,  0.00001f),
        glm::vec3( 0.f,   -r,  0.f)
    };
    GLuint indices[] = {
        0, 1, 0, 2,
        0, 2, 0, 3,
        0, 3, 0, 4,
        0, 5, 0, 1,
        6, 2, 6, 1,
        6, 3, 6, 2,
        6, 4, 6, 3,
        6, 5, 6, 1
    };
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 7 * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * 4 * sizeof(GLuint), indices, GL_STATIC_DRAW);
}

SkySphere::~SkySphere()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    glDeleteVertexArrays(1, &m_vao);
}

void
SkySphere::draw()
{
    assert(glGetError() == GL_NONE);
    glBindVertexArray(m_vao);
    glUseProgram(m_shaderProg);
    glActiveTexture(GL_TEXTURE0);
    ENV_VAR.envMap.bind();
    glm::mat4 mvp = ENV_VAR.projMat * ENV_VAR.viewMat *modelMat();
    glUniformMatrix4fv(m_uniformLocs["MVP"], 1, GL_FALSE, &mvp[0][0]);
    GLfloat tessLvl = 32;
    GLfloat outerLevel[4] = { tessLvl, tessLvl, tessLvl, tessLvl };
    GLfloat innerLevel[2] = { tessLvl, tessLvl };

    glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, outerLevel);
    glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, innerLevel);
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    glDrawElements(GL_PATCHES, 8 * 4, GL_UNSIGNED_INT, 0);
    assert(glGetError() == GL_NONE);
}