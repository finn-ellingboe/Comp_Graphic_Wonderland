#version 330 core

// inputs
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexUV;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 lightSpaceMatrix;

// Outputs
out vec2 uv;
out vec3 worldPosition;
out vec3 worldNormal;
out vec4 LightSpacePos;

void main()
{
    gl_Position = MVP * vec4(vertexPosition, 1.0);

    vec4 worldPos_4 = M * vec4(vertexPosition, 1.0);
    worldPosition = worldPos_4.xyz;
    
    LightSpacePos = lightSpaceMatrix * worldPos_4; 
    
    worldNormal = vec3(0.0, 1.0, 0.0);

    uv = vertexUV;
}