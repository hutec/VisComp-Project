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
#include <iostream>
#include "helper/GLCommon.h"

class CMeshComponent
{
public:
	CMeshComponent();
	~CMeshComponent();

	/* Loads the model, shader and textures */
	void init();

	/* Updates the model transformation matrix */
	void update(float elapsedTime);

	/* Renders the object */
	void render(glm::mat4 view_projection);

	/* Releases the allocated buffers */
	void cleanup();

private:
	bool initVertexBuffer();
	bool loadShader();

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

};