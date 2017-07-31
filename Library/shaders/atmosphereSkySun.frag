#version 330 core
uniform vec3 eyePos;
uniform vec3 sunDir;
uniform mat4 invProj;
uniform mat3 invView;
uniform vec2 viewport;
uniform vec3 whitePoint;
uniform float cosSunSize;

out vec3 fragColor;

const vec3 center = vec3(0, 0, -6360000.0);

vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSolarLuminance();

vec3 getWorldNormal()
{
    vec2 fragPos = gl_FragCoord.xy/viewport;
    fragPos = (fragPos-0.5)*2.0;
    vec4 deviceNormal = vec4(fragPos, 0.0, 1.0);
    vec3 eyeNormal = normalize((invProj * deviceNormal).xyz);
    vec3 worldNormal = normalize(invView * eyeNormal);
    return worldNormal;
}

void main()
{
	//Fragment ray
	vec3 viewDir = getWorldNormal();

	//Sky
	vec3 transmittance;
	vec3 luminance = GetSkyLuminance(eyePos - center, viewDir, 0.f, sunDir, transmittance);

	//Sun
	if(dot(viewDir, sunDir) > cosSunSize) 
		luminance += transmittance * GetSolarLuminance();

	//Color correction
	fragColor = luminance/whitePoint/10000.0;
}