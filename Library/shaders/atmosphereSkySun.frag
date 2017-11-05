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
const float kGroundAlbedo = 0.5;
const float PI = 3.14159265358979323846;

vec3 GetSolarLuminance();
vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSkyLuminanceToPoint(vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance); 

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
	vec3 P = eyePos - center;
	
	float PdotV = dot(P, viewDir);
	float PdotP = dot(P, P);
	float rayEarthCenterSquaredDistance = PdotP - PdotV * PdotV;
	float distanceToIntersection = -PdotV - sqrt(center.z * center.z - rayEarthCenterSquaredDistance);
	
	//Compute the radiance reflected by the ground, if the ray intersects it.
	float groundAlpha = 0.0;
	vec3 groundLuminance = vec3(0.0);
	
	if(distanceToIntersection > 0.0) 
	{
		vec3 point = eyePos + viewDir * distanceToIntersection;
		vec3 normal = normalize(point - center);

		//Compute the radiance reflected by the ground.
		vec3 skyIlluminance;
		vec3 sunIlluminance = GetSunAndSkyIlluminance(point - center, normal, sunDir, skyIlluminance);
		groundLuminance = kGroundAlbedo * (1.0 / PI) * (sunIlluminance + skyIlluminance);

		vec3 transmittance;
		vec3 inScatter = GetSkyLuminanceToPoint(P, point - center, 0.0, sunDir, transmittance);
		groundLuminance = groundLuminance * transmittance + inScatter;
		groundAlpha = 1.0;
	}

	//Sky
	vec3 transmittance;
	vec3 luminance = GetSkyLuminance(P, viewDir, 0.0, sunDir, transmittance);

	//Sun
	luminance += smoothstep(cosSunSize*0.99999, cosSunSize, dot(viewDir, sunDir)) * transmittance * GetSolarLuminance()/100000.0;

	//Mix
	luminance = mix(luminance, groundLuminance, groundAlpha);

	//Color correction
	fragColor = luminance/whitePoint/10000.0;
}