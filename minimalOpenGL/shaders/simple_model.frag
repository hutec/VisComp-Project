#version 430

layout(binding = 0) uniform sampler2D diffuse;
layout(binding = 3) uniform sampler2D texture_enhanced;


in vec2 texcoords;
uniform vec4 leappos;

// uniform samplerCube cube_texture;
out vec4 frag_colour;

void main () {
  if (texture(texture_enhanced, texcoords)[3] == 1.0) {
	frag_colour = leappos;
  } else {
	frag_colour = texture(texture_enhanced, texcoords) * leappos;
  }
}