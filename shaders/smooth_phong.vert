#version 130
attribute vec3 aPos;
attribute vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

varying vec3 FragPos;
varying vec3 Normal;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPos;
    FragPos = vec3(worldPos);

    // --- FIXED LINE ---
    // Simpler, more compatible normal transformation that avoids inverse()
    Normal = mat3(model) * aNormal;
}
