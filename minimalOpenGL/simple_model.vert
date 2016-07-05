#version 400

in vec3 position;
in vec2 uv;

uniform mat4 matVP;

out vec2 texcoords;

void main () {
	texcoords = uv;
	
	gl_Position = matVP * vec4 (position, 1.0);
}