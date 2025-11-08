#version 130
attribute vec3 aPos;
attribute vec3 aNormal;

uniform mat4 view;
uniform mat4 projection;

varying vec3 FragPos;
varying vec3 Normal;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    FragPos = aPos;
    Normal = aNormal;
}
