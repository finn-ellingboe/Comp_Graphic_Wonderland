#version 330 core

in vec2 uv;
in vec3 worldPosition;
in vec3 worldNormal;
in vec4 LightSpacePos;

uniform sampler2D textureSampler;
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform sampler2D shadowMap;
uniform float farPlane;

out vec3 finalColor;

float ShadowCalculation() {
    vec3 projCoords = LightSpacePos.xyz / LightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0){
        return 0.0; 
    }

    float closestDistance = texture(shadowMap, projCoords.xy).r * farPlane; 

    float currentDistance = length(worldPosition - lightPosition);

    float bias = 0.005; // arbtitrarily chose value, can be changed to what fits best
    
    float shadow = currentDistance - bias > closestDistance ? 1.0 : 0.0;

    return shadow;
}

void main()
{
	vec2 repeatUV = uv * 10.0;
	vec3 textureColor = texture(textureSampler, repeatUV).rgb;

    float shadowFactor = ShadowCalculation();
    
    vec3 ambient = 0.1 * textureColor; 

    vec3 N = normalize(worldNormal);
    vec3 L = normalize(lightPosition - worldPosition); 
    float r = distance(worldPosition, lightPosition);

    float cosBeta = max(dot(N, L), 0.0);
    vec3 totalReflectedRadiance = ( (0.78 / 3.14) * (cosBeta) * (lightIntensity / (4.0 * 3.14 * pow(r, 2))) );

    vec3 shadowedRadiance = totalReflectedRadiance * (1.0 - shadowFactor);

    const float gamma = 2.2;
    vec3 hdrColor = ambient + shadowedRadiance;

    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0)); // Reinhard tone mapping

    // gamma correction 
    mapped = pow(mapped, vec3(gamma)); // Gamma correction
 
    finalColor = mapped;
}