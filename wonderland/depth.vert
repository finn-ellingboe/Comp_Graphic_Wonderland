#version 330 core

layout (location = 0) in vec3 vertexPosition;

uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * vec4(vertexPosition, 1.0);
}





/*
#version 330 core
layout (location = 0) in vec3 vertexPosition;

uniform mat4 lightSpaceMatrix;
uniform mat4 M;
uniform vec3 lightPosition;
uniform float farPlane;

out vec3 WorldPos; 

void main() {
    WorldPos = vec3(M * vec4(vertexPosition, 1.0));
    
    gl_Position = lightSpaceMatrix * vec4(vertexPosition, 1.0);
}
*/