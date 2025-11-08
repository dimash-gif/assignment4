// --- START OF MODIFIED FILE shaders/procedural_patch.frag ---

#version 130

varying vec3 FragPos;
varying vec3 Normal;
varying vec2 TexCoords;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform float shininess;

void main()
{
    // --- Procedural Color Generation ---
    float dist = distance(TexCoords, vec2(0.5, 0.5));
    
    float frequency = 40.0;
    float r = sin(dist * frequency + 0.0) * 0.5 + 0.5;
    float g = sin(dist * frequency + 2.094) * 0.5 + 0.5;
    float b = sin(dist * frequency + 4.188) * 0.5 + 0.5;
    
    vec3 objectColor = vec3(r, g, b);
    
    // --- MODIFIED: Make the procedural color darker ---
    objectColor *= 0.7; // Multiply by a factor less than 1 to darken
    
    // --- Phong Lighting Calculation ---
    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    
    float specularStrength = 1.0;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);
    
    vec3 result = (ambient + diffuse) * objectColor + specular;
    
    gl_FragColor = vec4(result, 1.0);
}

// --- END OF MODIFIED FILE ---
