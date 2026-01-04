#version 330 core

void main()
{
    // We dont need anything here
}

/*
#version 330 core

in vec3 WorldPos; 

uniform vec3 lightPosition;
uniform float farPlane;

void main() {
    float distance = length(WorldPos - lightPosition);

    gl_FragDepth = distance / farPlane;
}
*/