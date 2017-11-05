#version 330 core

out vec4 fragColor;
in vec2 texcoord;

uniform vec3 eyePos;
uniform mat4 invProj;
uniform mat3 invView;
uniform vec3 sunDirection;
uniform float planetRadius;
uniform vec3 whitePoint;
const vec3 waterAbsorption = vec3(0.3,0.08,0.07);

vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);

vec3 getWorldNormal()
{
    vec2 fragPos = texcoord;
    fragPos = (fragPos-0.5)*2.0;
    vec4 deviceNormal = vec4(fragPos, 0.0, 1.0);
    vec3 eyeNormal = normalize((invProj * deviceNormal).xyz);
    vec3 worldNormal = normalize(invView * eyeNormal);
    return worldNormal;
}

void main() 
{
	vec3 toEye = -getWorldNormal();
	vec3 center = vec3(0,0,-planetRadius);
	float distance = 1000000.0;
	
	vec3 skyIlluminance;
	vec3 sunIlluminance = GetSunAndSkyIlluminance(-center, vec3(0,0,1.0), sunDirection, skyIlluminance);
    vec3 color = skyIlluminance/whitePoint/1000000.0 * exp(-waterAbsorption * -eyePos.z) * ( exp((-toEye.z - 1.0)*waterAbsorption*distance)-1.0 )/( (-toEye.z - 1.0)*waterAbsorption );
	fragColor = vec4(color, 1.0);
}