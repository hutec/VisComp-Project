#include "MeshComponent.h"
#include <gtx/transform.hpp>
#include <iostream>

#define PRINT_TO_CONSOLE 0

CMeshComponent::CMeshComponent()
	: model(NULL)
	, filename(NULL)
	, isFullyInitialized(false)
	, model_vert_shader(0)
	, model_frag_shader(0)
	, modelProgram(0)
	, matVP_ID(0)
	, vao(0)
	, numVertices(0)
	, diffuseID(0)
	, leapPos_ID(0)
{
	model_transform = glm::mat4(1.0f);
	model_transform = glm::translate(model_transform, glm::vec3(0.0f, 1.0f, 0.0f));

	leap_pos = glm::vec4(0.f);
}

CMeshComponent::~CMeshComponent()
{
	cleanup();
}

void CMeshComponent::init(char* obj_path, char* diffuse_path, char* frag_shader_path, char* vert_shader_path)
{
	isFullyInitialized = loadShader(frag_shader_path, vert_shader_path);

	filename = obj_path;
	//filename = (char*)"assets/quad.obj";
	model = glmReadOBJ(filename);
	
	// Scale object to unit size
	glmUnitize(model);

	isFullyInitialized &= initVertexBuffer();
	isFullyInitialized &= tDiffuse.load(diffuse_path);
	//isFullyInitialized &= tDiffuse.load((char*)"assets/generator_diffuse.jpg");

}

bool CMeshComponent::loadShader(char* frag_shader_path, char* vert_shader_path)
{
	model_vert_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	model_frag_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	if (!CreateShaderFromFile(vert_shader_path, model_vert_shader))
	{
		std::cout << "Loading simple_model.vert failed " << std::endl;
		return false;
	}


	if (!CreateShaderFromFile(frag_shader_path, model_frag_shader))
	{
		std::cout << "Loading simple_model.frag failed " << std::endl;
		return false;
	}

	modelProgram = glCreateProgramObjectARB();
	glAttachObjectARB(modelProgram, model_vert_shader);
	glAttachObjectARB(modelProgram, model_frag_shader);
	if (!LinkGLSLProgram(modelProgram))
	{
		std::cout << "Linking shader failed " << std::endl;
		return false;
	}

	// Get a handle for the model uniform
	matVP_ID = glGetUniformLocationARB(modelProgram, "matVP");
	diffuseID = glGetUniformLocation(modelProgram, "diffuse");
	leapPos_ID = glGetUniformLocation(modelProgram, "leappos");

	return true;
}

void CMeshComponent::cleanup()
{
	SAFE_DELETE(model);

	if (vertex_data_position) {
		delete[] vertex_data_position;
		vertex_data_position = NULL;
	}

	if (vertex_data_uv) {
		delete[] vertex_data_uv;
		vertex_data_uv = NULL;
	}

	glDeleteBuffers(2, vbo);
	glDeleteVertexArrays(1, &vao);

	SAFE_DELETE(model);
	SAFE_RELEASE_GL_SHADER(model_vert_shader);
	SAFE_RELEASE_GL_SHADER(model_frag_shader);
	SAFE_RELEASE_GL_PROGRAM(modelProgram);
}


bool CMeshComponent::initVertexBuffer()
{
	numVertices = model->numtriangles * 3;
	vertex_data_position = new GLfloat[numVertices * 3];
	vertex_data_uv = new GLfloat[numVertices * 2];

	GLMgroup* group;
	group = model->groups;

#if PRINT_TO_CONSOLE
	std::cout << "model numVertices " << model->numvertices << std::endl;
	for (int i = 0; i <= model->numvertices * 3; i += 3)
	{
		std::cout << i / 3 << ": " << model->vertices[i] << " " << model->vertices[i + 1] << " " << model->vertices[i + 2] << " " << std::endl;
	}

	std::cout << "model numTriangles " << model->numtriangles << std::endl;
	for (int i = 0; i < model->numtriangles; i++)
	{
		std::cout << i << ": " << model->triangles[i].vindices[0] << " " << model->triangles[i].vindices[1] << " " << model->triangles[i].vindices[2] << " " << std::endl;
	}
#endif

	while (group)
	{
		// !WARNING not sure if multiple groups are supported
		for (GLuint i = 0; i < group->numtriangles; i++)
		{
			// holds the group index of the current group triangle
			int triangle_index = group->triangles[i];

			// holds the indices of the global model vertices ( 3 vertices per triangle )
			GLuint model_vertex_indices[3] = { 
				model->triangles[triangle_index].vindices[0], 
				model->triangles[triangle_index].vindices[1], 
				model->triangles[triangle_index].vindices[2] 
			};

#if PRINT_TO_CONSOLE
			std::cout << "Triangle index: " << triangle_index << " model_vertex_index: " << model->triangles[triangle_index].vindices[0] << " " << model->triangles[triangle_index].vindices[1] << " " << model->triangles[triangle_index].vindices[2] << std::endl;
#endif
			/*
				vertex_data contains a list of floats.
				We iterate over all group triangles i.
				Each triangle has 3 vertices.
				Each vertice has 3 floats (x,y,z).
				So we get 9 floats per triangle
			*/
			// vertex 1 of triangle i
			vertex_data_position[i * 3 * 3 + 0] = model->vertices[model_vertex_indices[0] * 3 + 0];
			vertex_data_position[i * 3 * 3 + 1] = model->vertices[model_vertex_indices[0] * 3 + 1];
			vertex_data_position[i * 3 * 3 + 2] = model->vertices[model_vertex_indices[0] * 3 + 2];

			// vertex 2 of triangle i
			vertex_data_position[i * 3 * 3 + 3] = model->vertices[model_vertex_indices[1] * 3 + 0];
			vertex_data_position[i * 3 * 3 + 4] = model->vertices[model_vertex_indices[1] * 3 + 1];
			vertex_data_position[i * 3 * 3 + 5] = model->vertices[model_vertex_indices[1] * 3 + 2];

			// vertex 3 of triangle i
			vertex_data_position[i * 3 * 3 + 6] = model->vertices[model_vertex_indices[2] * 3 + 0];
			vertex_data_position[i * 3 * 3 + 7] = model->vertices[model_vertex_indices[2] * 3 + 1];
			vertex_data_position[i * 3 * 3 + 8] = model->vertices[model_vertex_indices[2] * 3 + 2];

			// holds the uv indices (two floats) of each vertex of the triangle
			GLuint model_uv_indices[3] = { 
				model->triangles[triangle_index].tindices[0], 
				model->triangles[triangle_index].tindices[1], 
				model->triangles[triangle_index].tindices[2] 
			};
			
			// uv of vertex 1 of triangle i
			vertex_data_uv[i * 3 * 2 + 0] = model->texcoords[model_uv_indices[0] * 2 + 0];
			vertex_data_uv[i * 3 * 2 + 1] = model->texcoords[model_uv_indices[0] * 2 + 1];

			// uv of vertex 2 of triangle i
			vertex_data_uv[i * 3 * 2 + 2] = model->texcoords[model_uv_indices[1] * 2 + 0];
			vertex_data_uv[i * 3 * 2 + 3] = model->texcoords[model_uv_indices[1] * 2 + 1];

			// uv of vertex 3 of triangle i
			vertex_data_uv[i * 3 * 2 + 4] = model->texcoords[model_uv_indices[2] * 2 + 0];
			vertex_data_uv[i * 3 * 2 + 5] = model->texcoords[model_uv_indices[2] * 2 + 1];

#if PRINT_TO_CONSOLE
			std::cout << vertex_data_position[i * 3 * 3 + 0] << " " << vertex_data_position[i * 3 * 3 + 1] << " " << vertex_data_position[i * 3 * 3 + 2] << ", "
				<< vertex_data_position[i * 3 * 3 + 3] << " " << vertex_data_position[i * 3 * 3 + 4] << " " << vertex_data_position[i * 3 * 3 + 5] << ", "
				<< vertex_data_position[i * 3 * 3 + 6] << " " << vertex_data_position[i * 3 * 3 + 7] << " " << vertex_data_position[i * 3 * 3 + 8] << std::endl;
#endif	

		}
		group = group->next;
	}

	fprintf(stderr, "[mesh]: loaded '%s' with '%i' vertices \n", filename, numVertices);

	/* Allocate and assign a Vertex Array Object to our handle */
	glGenVertexArrays(1, &vao);

	/* Bind our Vertex Array Object as the current used object */
	glBindVertexArray(vao);

	/* Allocate and assign two Vertex Buffer Objects to our handle */
	glGenBuffers(2, vbo);

	/* Bind our first VBO as being the active buffer and storing vertex attributes (position) */
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

	/* Copy the vertex data to our buffer */
	/* numVertices * 3 * sizeof(GLfloat) is the size of the position array, since it contains 3 GLfloat per vertex position */
	glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(GLfloat), &vertex_data_position[0], GL_STATIC_DRAW);

	/* Specify that our position data is going into attribute index 0, and contains two three floats per vertex */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Enable attribute index 0 as being used */
	glEnableVertexAttribArray(0);

	/* Bind our second VBO as being the active buffer and storing vertex attributes (colors) */
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

	/* numVertices * 2 * sizeof(GLfloat) is the size of the colors array, since it contains 2 GLfloat (u,v) per vertex */
	glBufferData(GL_ARRAY_BUFFER, numVertices * 2 * sizeof(GLfloat), &vertex_data_uv[0], GL_STATIC_DRAW);

	/* Specify that our uv data is going into attribute index 1, and contains two floats per vertex */
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	/* Enable attribute index 1 as being used */
	glEnableVertexAttribArray(1);

	return true;
}


void CMeshComponent::update(float elapsedTime)
{
	// rotate the mesh with a speed of rotation_speed° per second
	float rotation_speed = glm::radians(0.0f);
	model_transform *= glm::rotate(rotation_speed * elapsedTime, glm::vec3(0.0f, 1.0f, 0.0f));
}

void CMeshComponent::rotate(glm::vec3 rotation)
{

	rotation = rotation / glm::vec3(100000);
	model_transform *= glm::rotate(rotation.x, glm::vec3(0.0f, 1.0f, 0.0f));
	model_transform *= glm::rotate(rotation.y, glm::vec3(1.0f, 0.0f, 0.0f));
	//model_transform *= glm::rotate(rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
}

void CMeshComponent::setLeapPosition(glm::vec3 pos)
{
	//TODO this is far from optimal, try to remap the hand positions
	for (int i = 0; i < 3; ++i) {
		pos[i] = std::max(0.f, std::min(std::abs(pos[i]), 255.f)) / 255.f;
	}
	leap_pos = glm::vec4(pos.x, pos.y, pos.z, 1);

}


void CMeshComponent::render(glm::mat4 view_projection)
{
	if (!isFullyInitialized)
		return;

	glm::mat4 scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));

	// calculate the modelViewProjection matrix
	glm::mat4 mvp = view_projection * model_transform * scale;


	// Set state machine parameters
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set shader and uniforms
	glUseProgram(modelProgram);
	glUniformMatrix4fv(matVP_ID, 1, GL_FALSE, &mvp[0][0]);
	glBindVertexArray(vao);

	glUniform4f(leapPos_ID, leap_pos[0], leap_pos[1], leap_pos[2], leap_pos[3]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// set the texture
	glActiveTexture(GL_TEXTURE0);
	tDiffuse.bind();
	glUniform1i(diffuseID, 0);


	// draw the triangles
	glDrawArrays(GL_TRIANGLES, 0, numVertices);

	// reset the state machine and shader
	glUseProgram(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisable(GL_BLEND);
}