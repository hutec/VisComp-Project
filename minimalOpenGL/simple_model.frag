#version 400

uniform sampler2D diffuse;
in vec2 texcoords;
uniform vec4 leappos;

uniform samplerCube cube_texture;
out vec4 frag_colour;

void main () {
  frag_colour = texture(diffuse, texcoords) * leappos;
}