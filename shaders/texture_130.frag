#version 130

// Use 'varying' to receive data from the vertex shader
varying vec3 FragPos;
varying vec3 Normal;
varying vec2 TexCoords;

// Uniforms for lighting and material
uniform vec3 viewPos;
uniform vec3 light_position; // Renamed to avoid GLSL keywords
uniform sampler2D diffuseMap;
uniform float material_shininess; // Renamed

void main()
{
    // Ambient light
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

    // Diffuse light
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light_position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    
    // Specular light
    float specularStrength = 0.8;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material_shininess);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0); 

    // Get color from the texture. Use texture2D for GLSL 1.30
    vec3 objectColor = texture2D(diffuseMap, TexCoords).rgb;

    // Combine lighting with the texture color
    vec3 result = (ambient + diffuse) * objectColor + specular;
    
    // Use gl_FragColor as the output
    gl_FragColor = vec4(result, 1.0);
}
