#version 430

layout(binding = 0) uniform sampler2D envMapTex;
in vec2 tesTexCoord;

out vec4 color;

void main () {
    color = vec4(texture(envMapTex, tesTexCoord).rgb, 1.0);
    // color = vec4(1.0);
}