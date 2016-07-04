#version 400

uniform sampler2D diffuse;
in vec2 texcoords;
uniform vec4 leappos;

out vec4 frag_colour;

void main () {
  if (texture(diffuse, texcoords)[3] == 1.0) {
	frag_colour = leappos;
  } else {
	frag_colour = texture(diffuse, texcoords) * leappos;
  }
}