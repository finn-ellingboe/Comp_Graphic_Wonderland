#version 330 core

// Vertex attributes
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexUV;

// Uniforms
uniform mat4 MVP;

// Outputs to fragment shader
out vec2 uv;

void main()
{
    uv = vertexUV;
    gl_Position = MVP * vec4(vertexPosition, 1.0);
}