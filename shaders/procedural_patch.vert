#version 130

attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTexCoords;

varying vec3 FragPos;
varying vec3 Normal;
varying vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(model) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
