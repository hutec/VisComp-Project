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

	void init();
	void update(float elapsedTime);
	void render(glm::mat4 view_projection);
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