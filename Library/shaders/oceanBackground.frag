#version 330

out vec4 fragColor;
in vec2 texcoord;

uniform vec3 eyePos;
uniform mat4 invProj;
uniform mat3 invView;
uniform vec3 sunDirection;
uniform float planetRadius;
uniform vec3 whitePoint;
uniform vec3 lightAbsorption;

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
	
	vec3 skyIlluminance;
	vec3 sunIlluminance = GetSunAndSkyIlluminance(-center, vec3(0,0,1.0), sunDirection, skyIlluminance);
    vec3 color = skyIlluminance/whitePoint/30000.0 * exp(-lightAbsorption * -min(-5.0, eyePos.z)) * 0.2;
	fragColor = vec4(color, 1.0);
}
