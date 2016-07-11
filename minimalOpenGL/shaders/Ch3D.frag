#version 430

uniform vec3 camPos;
uniform vec3 lightPos;

in vec3 vsWorldPos;
in vec3 vsNormal;
uniform vec4 diffuse;
uniform vec4 specular;
uniform float shininess;

out vec4 color;

vec3 lighting()
{
    vec3 N = normalize(vsNormal);
    vec3 L = normalize(lightPos - vsWorldPos);
    vec3 V = normalize(camPos - vsWorldPos);
    vec3 R = reflect(-L, N);
    
    vec3 diffuseLight = max(dot(N, L), 0.0) * diffuse.rgb;
    vec3 specularLight = pow(max(dot(R, V), 0.0), shininess) * specular.rgb;
    return diffuseLight + specularLight;
}

void main () {
    color = vec4 (0.7 * lighting(), 1.0);
    // color = vec4(1.0);
}