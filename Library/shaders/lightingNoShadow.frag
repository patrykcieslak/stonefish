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

//Constants
const float sunLightRadius = 0.03;

//Inputs
uniform vec3 eyePos;
uniform vec3 viewDir;

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

#inject "lightingDef.glsl"

vec3 ShadingModel(vec3 N, vec3 V, vec3 L, vec3 Lcolor, vec3 albedo);

//Calculate contribution of different light types
vec4 PointLightContribution(int id, vec3 P, vec3 N, vec3 toEye, vec3 albedo)
{
	vec3 toLight = pointLights[id].position - P;
	float distance = length(toLight);
	toLight /= distance;
	
	float attenuation = 1.0/(max(0.01*0.01, distance*distance));
	return vec4(ShadingModel(N, toEye, toLight, pointLights[id].color * attenuation, albedo), distance);
}

vec4 SpotLightContribution(int id, vec3 P, vec3 N, vec3 toEye, vec3 albedo)
{	
	vec3 toLight = spotLights[id].position - P;
	float distance = length(toLight);
	toLight /= distance;
	
	float spotEffect = dot(spotLights[id].direction, -toLight); //Angle between spot direction and point-light vector
    float NdotL = dot(N, toLight);
        
	if(spotEffect > spotLights[id].cone && NdotL > 0.0)
	{
		float attenuation = 1.0/(max(0.01*0.01, distance*distance));
        float edge = smoothstep(1, 1.05, spotEffect/spotLights[id].cone);
		return vec4(ShadingModel(N, toEye, toLight, spotLights[id].color * edge * attenuation, albedo), distance);
	}
	else
		return vec4(0.0);
}

vec3 SunContribution(vec3 P, vec3 N, vec3 toEye, vec3 albedo, vec3 illuminance)
{
	float NdotL = dot(N, sunDirection);
	
	if(NdotL > 0.0)
	{	
		return ShadingModel(N, toEye, sunDirection, illuminance, albedo);
	}
	else
		return vec3(0.0);
}