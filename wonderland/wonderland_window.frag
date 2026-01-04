#version 330 core

in vec3 color;			// The current colour of this spot of the vertex
in vec3 worldPosition;	// This equals the vertex position
in vec3 worldNormal;	// This equals the vertex's normal
in vec4 LightSpacePos;

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform sampler2D shadowMap;
uniform float farPlane;

float ShadowCalculation() {
    vec3 projCoords = LightSpacePos.xyz / LightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    // If its too far away then no shadow
    if (projCoords.z > 1.0) {
        return 0.0; 
    }

    float closestDistance = texture(shadowMap, projCoords.xy).r * farPlane; 

    float currentDistance = length(worldPosition - lightPosition);

    float bias = max(0.05 * (1.0 - dot(worldNormal, normalize(lightPosition - worldPosition))), 0.005);
    
    float shadow = currentDistance - bias > closestDistance ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    float shadowFactor = ShadowCalculation();
    
    // but also ambient lighting
    vec3 ambient = 0.1 * color;

    vec3 N = normalize(worldNormal);
	vec3 L = lightPosition - worldPosition; // this is the normalized vector of light to vertex
    float r = distance(worldPosition, lightPosition);

	float dotProduct = max(dot(N, L), 0.0);   // world Normal is N;    We also have to max so values dont become negative

	vec3 reflectedRadiance = ( (0.78 / 3.14) * (dotProduct) * (lightIntensity / (4 * 3.14 * pow(r, 2))) );


    vec3 shadowedRadiance = reflectedRadiance * (1.0 - shadowFactor);


	const float gamma = 2.2;
	vec3 hdrColor = ambient + shadowedRadiance;
  
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(gamma));
 
	finalColor = mapped;
}
