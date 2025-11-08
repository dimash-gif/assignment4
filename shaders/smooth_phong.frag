#version 130
varying vec3 FragPos;
varying vec3 Normal;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor; // Per-object diffuse color
void main() {
float ambientStrength = 0.2;
vec3 ambient = ambientStrength * lightColor;
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(lightPos - FragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * lightColor;

vec3 result = (ambient + diffuse) * objectColor;
gl_FragColor = vec4(result, 1.0);
}
