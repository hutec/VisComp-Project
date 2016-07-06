#version 430

layout(binding = 0) uniform sampler2D diffuseTex;

uniform vec3 camPos;
uniform vec3 lightPos;

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
    
    vec3 texColor = texture(diffuseTex, vsTexCoord).rgb;
    vec3 diffuseLight = max(dot(N, L), 0.0) * texColor;
    return diffuseLight;
}

void main () {
    color = vec4 (lighting(), 1.0);
}