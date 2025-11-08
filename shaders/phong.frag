#version 130
varying vec3 FragPos;
varying vec3 Normal;

uniform vec3 lightPos;
uniform vec3 lightColor;

void main() {
    // Ambient light component
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
  	
    // Diffuse light component
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Final color combines lighting with a base color for the patch
    vec3 patchColor = vec3(0.6, 0.8, 0.7); // A mint-green color
    vec3 result = (ambient + diffuse) * patchColor;
    gl_FragColor = vec4(result, 1.0);
}
