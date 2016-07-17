#version 430

layout(binding = 0) uniform sampler2D diffuseTex;
layout(binding = 3) uniform sampler2D enhancedTex;

uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec4 leapPos;


in vec3 vsWorldPos;
in vec3 vsNormal;
in vec2 vsTexCoord;


out vec4 color;

vec3 lighting()
{
    vec3 N = normalize(vsNormal);
    vec3 L = normalize(lightPos - vsWorldPos);
    vec3 V = normalize(camPos - vsWorldPos);
    vec3 R = reflect(-L, N);
    
	vec3 texColor;
	vec3 leap = vec3(leapPos.x, leapPos.y, leapPos.z);

	if (distance(leap, vsWorldPos) < .2f) {
	//if (abs(leapPos.x - vsWorldPos.x) < .5f && abs(leapPos.y - vsWorldPos.y) < .5f) {
		texColor = texture(enhancedTex, vsTexCoord).rgb;
	} else {
		texColor = texture(diffuseTex, vsTexCoord).rgb;
	}
    vec3 diffuseLight = max(dot(N, L), 0.0) * texColor;
    return diffuseLight;
}

void main () {
    color = vec4 (lighting(), 1);
}