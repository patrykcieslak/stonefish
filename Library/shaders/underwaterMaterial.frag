/*    
    Copyright (c) 2020 Patryk Cieslak. All rights reserved.

    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

in vec3 normal;
in vec2 texCoord;
in vec4 fragPos;
in vec3 eyeSpaceNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec4 fragNormal;

uniform vec3 eyePos;
uniform vec3 viewDir;
uniform vec4 color;
uniform float reflectivity;
uniform sampler2D tex;

layout (std140) uniform SunSky
{
    mat4 sunClipSpace[4];
    vec4 sunFrustumNear;
    vec4 sunFrustumFar;
    vec3 sunDirection;
	float planetRadiusInUnits;
	vec3 whitePoint;
    float atmLengthUnitInMeters;
};

struct PointLight 
{
	vec3 position;
	vec3 color;
};

struct SpotLight 
{
	mat4 clipSpace;
    vec3 position;
	float frustumNear;
    vec3 direction;
	float frustumFar;
    vec3 color;
	float cone;
	vec2 radius;
};

layout (std140) uniform Lights
{
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];
    int numPointLights;
    int numSpotLights;
};

const vec3 waterSurfaceN = vec3(0.0, 0.0, -1.0);

//---------------Functions-------------------
vec3 GetSolarLuminance();
vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSkyLuminanceToPoint(vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);
vec3 ShadingModel(vec3 N, vec3 toEye, vec3 toLight, vec3 albedo);
float SpotShadow(int id);
float SunShadow();
vec4 PointLightContribution(int id, vec3 P, vec3 N, vec3 toEye, vec3 albedo);
vec4 SpotLightContribution(int id, vec3 P, vec3 N, vec3 toEye, vec3 albedo);
vec3 SunContribution(vec3 P, vec3 N, vec3 toEye, vec3 albedo, vec3 illuminance);
vec3 RefractToWater(vec3 I, vec3 N);
vec3 RefractToAir(vec3 I, vec3 N);
vec3 BeerLambert(float d);
vec3 InScattering(vec3 L, vec3 D, vec3 V, float z, float d);

void main()
{	
	//Vectors
	vec3 P = fragPos.xyz/fragPos.w;
	vec3 N = normalize(normal);
	vec3 toEye = eyePos - P;
	float d = length(toEye);
	vec3 V = toEye/d;
	vec3 R = RefractToAir(N, -waterSurfaceN);
	vec3 S = RefractToWater(sunDirection, waterSurfaceN);
	
	if(eyePos.z < 0.0)
		d = P.z/dot(V, waterSurfaceN);
	
	//Diffuse color
	vec3 albedo = color.rgb;
	if(color.a > 0.0)
	{
		vec4 texColor = texture(tex, texCoord);
		albedo = mix(color.rgb, texColor.rgb, color.a*texColor.a);
	}
	
	//1. Direct lighting + out-scattering
	//Ambient
	vec3 center = vec3(0.0, 0.0, planetRadiusInUnits);
	vec3 posSky = vec3(P.xy/atmLengthUnitInMeters,-0.5/atmLengthUnitInMeters);
	vec3 skyIlluminance;
	vec3 sunIlluminance = GetSunAndSkyIlluminance(posSky - center, N, sunDirection, skyIlluminance);
	fragColor = albedo * skyIlluminance * BeerLambert(d+P.z);
	
	//Sun
	if(S.z > 0.0)
		fragColor += SunContribution(P, N, V, albedo, sunIlluminance) * BeerLambert(d+P.z/S.z);
	
	fragColor = fragColor/whitePoint/MEAN_SUN_ILLUMINANCE; //Color correction and normalization
	
	//Point lights
	for(int i=0; i<numPointLights; ++i)
	{
		vec4 Ld = PointLightContribution(i, P, N, V, albedo);
		fragColor += Ld.rgb * BeerLambert(d + Ld.a);
	}
	//Spot lights
	for(int i=0; i<numSpotLights; ++i)
	{
		vec4 Ld = SpotLightContribution(i, P, N, V, albedo);
		fragColor += Ld.rgb * BeerLambert(d + Ld.a);
	}
	
	//2. In-scattering
	sunIlluminance = GetSunAndSkyIlluminance(posSky - center, waterSurfaceN, sunDirection, skyIlluminance);
	vec3 L = (sunIlluminance + skyIlluminance)/whitePoint/MEAN_SUN_ILLUMINANCE;
	fragColor += InScattering(L, -waterSurfaceN, V, max(eyePos.z, 0.0), d);
	
    //Normal
	fragNormal = vec4(normalize(eyeSpaceNormal) * 0.5 + 0.5, reflectivity);
}
    
