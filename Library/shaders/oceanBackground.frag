#version 330

#define MEAN_SUN_ILLUMINANCE 107527.0

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec4 fragNormal;

uniform vec3 eyePos;
uniform vec3 sunDirection;
uniform float planetRadius;
uniform vec3 whitePoint;
uniform vec3 lightAbsorption;
uniform float turbidity;

vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);

const float d = 100000.0;
const vec3 rayleigh = vec3(0.15023, 0.405565, 1.0);

void main() 
{
    vec3 center = vec3(0,0,-planetRadius);
	
    //Water properties
    vec3 b = turbidity * rayleigh; //Scattering coefficient
    vec3 c = lightAbsorption + b * 0.1; //Full attenuation coefficient
    
    //Inscattering
    vec3 skyIlluminance;
    vec3 sunIlluminance = GetSunAndSkyIlluminance(-center, vec3(0,0,1.0), sunDirection, skyIlluminance);
    vec3 inFactor = exp(-c * max(-eyePos.z,0.0)) * b / c;
    fragColor = (sunIlluminance + skyIlluminance)/whitePoint/MEAN_SUN_ILLUMINANCE * inFactor * 0.01;
    
    //Normal
    fragNormal = vec4(vec3(0.0,0.0,1.0) * 0.5 + 0.5, 0.0);
}
