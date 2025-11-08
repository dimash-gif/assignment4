#version 130

varying vec3 FragPos_World;
varying vec3 Normal;

uniform vec3 viewPos;
uniform vec3 lightPos;

// Simple procedural noise for distortion
float noise(vec3 p) {
    return fract(sin(dot(p, vec3(12.9898, 78.233, 151.7182))) * 43758.5453);
}

// "Interesting" 3D Wood Grain procedural texture function
vec3 getProceduralColor(vec3 pos) {
    vec3 lightWood = vec3(0.8, 0.65, 0.4);
    vec3 darkWood  = vec3(0.45, 0.25, 0.1);
    
    float ring = fract(sqrt(pos.x * pos.x + pos.z * pos.z) * 12.0);
    float distortion = 0.2 * noise(pos * vec3(1.0, 0.2, 1.0));
    float pattern = smoothstep(0.4, 0.6, ring + distortion);
    
    return mix(darkWood, lightWood, pattern);
}

void main()
{
    vec3 objectColor = getProceduralColor(FragPos_World);

    // Standard Phong Lighting
    vec3 ambient = 0.2 * vec3(1.0);
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos_World);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);
    
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos_World);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * vec3(1.0);
    
    vec3 result = (ambient + diffuse) * objectColor + specular;
    gl_FragColor = vec4(result, 1.0);
}
