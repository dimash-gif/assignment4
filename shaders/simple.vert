#version 130
attribute vec3 aPos;
attribute vec3 aColor; // For axes

uniform mat4 view;
uniform mat4 projection;

varying vec3 vColor;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    vColor = aColor;
}
