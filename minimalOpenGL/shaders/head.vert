#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 MVP;
uniform mat4 modelMat;
uniform mat3 normalMat;

out vec3 vsWorldPos;
out vec3 vsNormal;
out vec2 vsTexCoord;

void main () {
	vsWorldPos = vec3(modelMat * vec4(position, 1.0));
	vsNormal = normalMat * normal;
    vsTexCoord = texCoord;
	gl_Position = MVP * vec4 (position, 1.0);
}