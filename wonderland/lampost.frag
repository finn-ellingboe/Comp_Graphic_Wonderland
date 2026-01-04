#version 330 core

in vec3 vNormal;

out vec4 FragColor;

uniform vec3 objectColor;

void main()
{
    vec3 lightDir = normalize(vec3(0.4, 1.0, 0.2));
    float diffuse = max(dot(normalize(vNormal), lightDir), 0.0);

    vec3 color = objectColor * (0.3 + 0.7 * diffuse);
    FragColor = vec4(color, 1.0);
}