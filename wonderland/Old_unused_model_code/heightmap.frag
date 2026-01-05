#version 330 core

in vec2 UV;
out vec4 color;

uniform sampler2D terrainTextureSampler;

void main()
{
    color = texture(terrainTextureSampler, UV);
}