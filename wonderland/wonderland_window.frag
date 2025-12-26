#version 330 core

in vec3 color;			// The current colour of this spot of the vertex
in vec3 worldPosition;	// This equals the vertex position
in vec3 worldNormal;	// This equals the vertex's normal

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;

void main()
{
	// finalColor = color;

	// TODO: tone mapping, gamma correction

	// reflected radiance = (Pd / 3.14) (cosBeta) (Omega / 4 x 3.14 x r^2)
	// cosBeta = Normal dotproduct LightSource
	// beta = angle between normal and light source
	// Pd = 0.78
	// Omega = total energy    -  this is lightintensity?
	// r = distance from lightsource


	vec3 L = lightPosition - worldPosition; // this is the normalized vector of light to vertex

	float dotProduct = max(dot(worldNormal, L), 0.0);   // world Normal is N;    We also have to max so values dont become negative

	vec3 reflectedRadiance = ( (0.78 / 3.14) * (dotProduct) * (lightIntensity / (4 * 3.14 * pow(distance(worldPosition, lightPosition), 2))) );


	//finalColor = color + reflectedRadiance;



	const float gamma = 2.2;
	vec3 hdrColor = color + reflectedRadiance;
  
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(gamma));
 
	finalColor = mapped;

	



}
