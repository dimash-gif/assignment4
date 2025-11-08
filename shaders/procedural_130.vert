#version 130

attribute vec3 aPos;
attribute vec3 aNormal;

varying vec3 FragPos_World;
varying vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos_World = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(model) * aNormal;
    gl_Position = projection * view * vec4(FragPos_World, 1.0);
}
