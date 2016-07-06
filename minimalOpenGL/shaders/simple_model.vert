#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

uniform mat4 MVP;

out vec2 texcoords;

void main () {
	texcoords = uv;
	
	gl_Position = MVP * vec4 (position, 1.0);
}