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

#version 330

in vec3 normal;
in vec4 fragPos;
in vec3 eyeSpaceNormal;
in float logz;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragNormal;

uniform vec3 eyePos;
uniform vec3 viewDir;
uniform float FC;
uniform vec4 color;
uniform float reflectivity;

#inject "lightingDef.glsl"

const vec3 waterSurfaceN = vec3(0.0, 0.0, -1.0);

//---------------Functions-------------------
vec3 GetSolarLuminance();
vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSkyLuminanceToPoint(vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);
vec4 PointLightContribution(int id, vec3 P, vec3 N, vec3 toEye, vec3 albedo);
vec4 SpotLightContribution(int id, vec3 P, vec3 N, vec3 toEye, vec3 albedo);
vec3 SunContribution(vec3 P, vec3 N, vec3 toEye, vec3 albedo, vec3 illuminance);
vec3 RefractToWater(vec3 I, vec3 N);
vec3 RefractToAir(vec3 I, vec3 N);
vec3 BeerLambert(float d);
vec3 InScatteringSun(vec3 L, vec3 D, vec3 V, float z, float d);
vec3 InScatteringPointLight(vec3 O, vec3 L, vec3 X, vec3 V, vec3 P, float d, float dw);
vec3 InScatteringSpotLight(vec3 O, vec3 D, float w, float fn, vec3 L, vec3 X, vec3 V, vec3 P, float d, float dw);
float displace(vec2 p);

void main()
{	
	//Logarithmic z-buffer correction
	gl_FragDepth = log2(logz) * FC;

	//Vectors
	vec3 P = fragPos.xyz/fragPos.w;
	vec3 N = normalize(normal);
	vec3 toEye = eyePos - P;
	float d = length(toEye);
	vec3 V = toEye/d;
	vec3 S = RefractToWater(-sunDirection, waterSurfaceN);
	float dw = d;
	float waterLevel = displace(P.xy);
	float depth = P.z - waterLevel;
	float VdotNs = dot(V, waterSurfaceN);
	if(eyePos.z < waterLevel)
	{
		if(VdotNs > 0.0)
			dw = max(P.z - waterLevel, 0.0)/VdotNs;
		else
			dw = 0.0;
	}
	
	//Diffuse color
	vec4 albedo = vec4(color.rgb, 1.0);
	
	//1. Direct lighting + out-scattering
	//Ambient
	vec3 center = vec3(0.0, 0.0, planetRadiusInUnits);
	vec3 posSky = vec3(P.xy/atmLengthUnitInMeters,-0.5/atmLengthUnitInMeters);
	vec3 skyIlluminance;
	vec3 sunIlluminance = GetSunAndSkyIlluminance(posSky - center, N, sunDirection, skyIlluminance);
	fragColor.rgb = albedo.rgb * skyIlluminance * BeerLambert(dw + depth);
	
	//Sun
	if(S.z > 0.0)
		fragColor.rgb += SunContribution(P, N, V, albedo.rgb, sunIlluminance) * BeerLambert(dw + depth/S.z);
	
	fragColor.rgb = fragColor.rgb/whitePoint; //Color correction and normalization
	
	//Point lights
	for(int i=0; i<numPointLights; ++i)
	{
		vec4 Ld = PointLightContribution(i, P, N, V, albedo.rgb);
		fragColor.rgb += Ld.rgb * BeerLambert(dw + Ld.a);
	}

	//Spot lights
	for(int i=0; i<numSpotLights; ++i)
	{
		vec4 Ld = SpotLightContribution(i, P, N, V, albedo.rgb);
		fragColor.rgb += Ld.rgb * BeerLambert(dw + Ld.a);
	}

	//2. In-scattering from Sun/Sky
    vec3 R = RefractToWater(-sunDirection, waterSurfaceN);
    if(R.z > 0.0 && sunDirection.z < 0.0)
    {
	    vec3 skyIlluminance;
	    vec3 sunIlluminance = GetSunAndSkyIlluminance(posSky - center, waterSurfaceN, sunDirection, skyIlluminance);
	    vec3 L = sunIlluminance/whitePoint;
	    fragColor.rgb += InScatteringSun(L, R, -V, max(eyePos.z, 0.0), dw);
    }

	//3. In-scattering from point lights
	for(int i=0; i<numPointLights; ++i)
	{
		if(pointLights[i].position.z > 0.0)
			fragColor.rgb += InScatteringPointLight(pointLights[i].position, 
												pointLights[i].color,
												eyePos, -V, P, d, dw);
	}

	//4. In-scattering from spot lights
	for(int i=0; i<numSpotLights; ++i)
	{
		if(spotLights[i].position.z > 0.0)
			fragColor.rgb += InScatteringSpotLight(spotLights[i].position,
											   spotLights[i].direction,
											   spotLights[i].cone,
											   spotLights[i].frustumNear,
											   spotLights[i].color,
											   eyePos, -V, P, d, dw);
	}

	//5. Transparency
	fragColor.rgb *= albedo.a;
	fragColor.a = 1.0;

    //Normal
	fragNormal = vec4(normalize(eyeSpaceNormal) * 0.5 + 0.5, reflectivity);
}
    
