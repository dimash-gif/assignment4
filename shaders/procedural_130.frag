#version 130

varying vec3 FragPos_World;
varying vec3 Normal;

uniform vec3 viewPos;
uniform vec3 lightPos;

// "Interesting" 3D Wood Grain procedural texture function
vec3 getProceduralColor(vec3 pos) {
    vec3 lightWood = vec3(0.8, 0.65, 0.4);
    vec3 darkWood  = vec3(0.45, 0.25, 0.1);
    
    // Create concentric rings based on distance from the Y-axis.
    // The frequency (12.0) controls the density of the rings.
    float ring = fract(sqrt(pos.x * pos.x + pos.z * pos.z) * 12.0);
    
    // Add 3D noise-based distortion to make rings less perfect
    float distortion = 0.2 * fract(sin(dot(pos, vec3(12.98, 78.23, 151.71))) * 43758.5);

    // Create a smooth transition for the grain
    float pattern = smoothstep(0.4, 0.6, ring + distortion);
    
    return mix(darkWood, lightWood, pattern);
}

void main()
{
    // Diffuse color is determined by the procedural function
    vec3 objectColor = getProceduralColor(FragPos_World);

    // Standard Phong Lighting
    vec3 ambient = 0.2 * objectColor; // Ambient light reflects the object's color
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos_World);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * objectColor; // Diffuse light reflects the object's color
    
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos_World);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // 32 is shininess
    vec3 specular = specularStrength * spec * vec3(1.0); // Specular highlights are typically white
    
    vec3 result = ambient + diffuse + specular;
    gl_FragColor = vec4(result, 1.0);
}
