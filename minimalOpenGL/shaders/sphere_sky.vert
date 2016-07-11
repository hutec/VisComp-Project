#version 430

layout (location = 0) in vec3 position;

uniform mat4 MVP;

out vec2 vsTexCoord;

void main () {
    float M_PI = atan(1.0) * 4.0;
	gl_Position = MVP * vec4 (position, 1.0);
    vec3 dir = normalize(position);
    float theta = acos(dir.y);
    float phi = 2 * atan(dir.z, dir.x) + M_PI;
    vsTexCoord = vec2(1 - phi / (2 * M_PI),1 - theta / M_PI);
}