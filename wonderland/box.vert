#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;

// Output data, to be interpolated for each fragment
out vec3 color;
out vec3 worldPosition;
out vec3 worldNormal;
out vec4 LightSpacePos;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 lightSpaceMatrix;

void main() {
    // Transform vertex
    gl_Position =  MVP * vec4(vertexPosition, 1);

    
    // Pass vertex color to the fragment shader
    color = vertexColor;

    // World-space geometry 
    vec4 worldPos_4 = M * vec4(vertexPosition, 1.0);
    worldPosition = worldPos_4.xyz;
    worldNormal = vertexNormal;

    LightSpacePos = lightSpaceMatrix * worldPos_4;
}