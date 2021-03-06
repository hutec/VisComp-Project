////////////////////////////////////////////////////////////
// 
//  Class for holding the resources needed to draw 
//  a wavefront .obj mesh. 
//  Author: Kai Braun, 2016
// 

#pragma once
#include "helper/wavefront_obj_loader/GLMmodel.h"
#include "helper/OGLTexture.h"
#include <glm.hpp>
#include <gtx/transform.hpp>
#include <iostream>
#include "helper/GLCommon.h"
#include <algorithm>

class CMeshComponent
{
public:
	CMeshComponent();
	~CMeshComponent();

	/* Loads the model, shader and textures */
	void init(char* obj_path, char* diffuse_path, char* frag_shader_path, char* vert_shader_path);

	/* Updates the model transformation matrix */
	void update(float elapsedTime);

	/* Rotate for given angles*/
	void rotate(glm::vec3 rotation);

	/* Rotate the object to be parallel to the image plane */
	void alignToCamera(glm::vec3 viewDir, glm::vec3 cameraUp);

	/* Set Leap position */
	void setLeapPosition(glm::vec3 pos);

	/* Move to point */
	void moveTo(glm::vec3 pos);

	/* Renders the object */
	void render(glm::mat4 view_projection);

	/* Releases the allocated buffers */
	void cleanup();


private:
	bool initVertexBuffer();
	bool loadShader(char* frag_shader_path, char* vert_shader_path);
	glm::mat4 billboard(glm::vec3 viewDir, glm::vec3 cameraUp);

private:
	GLMmodel	*model;
	char		*filename;
	glm::mat4	model_transform;

	bool isFullyInitialized;

	OGLTexture			tDiffuse;
	OGLTexture			tBumpmap;

	GLhandleARB model_vert_shader;
	GLhandleARB model_frag_shader;
	GLhandleARB	modelProgram;

	GLuint matVP_ID;

	GLuint vao, vbo[2];
	GLfloat* vertex_data_position;
	GLfloat* vertex_data_uv;
	GLuint numVertices;

	GLuint diffuseID;

	GLuint leapPos_ID;
	glm::vec4 leap_pos;

};