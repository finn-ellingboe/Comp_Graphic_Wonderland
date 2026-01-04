#version 330 core

in vec2 uv;

uniform sampler2D textureSampler;

out vec3 finalColor;

void main()
{
	vec2 repeatUV = uv * 10.0;
	finalColor = texture(textureSampler, repeatUV).rgb;
}