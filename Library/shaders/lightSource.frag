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
in vec2 texCoord;
in vec4 fragPos;
in vec3 eyeSpaceNormal;
in float logz;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec4 fragNormal;

uniform vec3 eyePos;
uniform vec3 viewDir;
uniform float FC;
uniform vec3 color;
uniform ivec2 lightId;

#inject "lightingDef.glsl"

//---------------Functions-------------------
vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSkyLuminanceToPoint(vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);
float SpotShadow(int id);
float SunShadow();
vec4 PointLightContribution(int id, vec3 P, vec3 N, vec3 toEye, vec3 albedo);
vec4 SpotLightContribution(int id, vec3 P, vec3 N, vec3 toEye, vec3 albedo);
vec3 SunContribution(vec3 P, vec3 N, vec3 toEye, vec3 albedo, vec3 illuminance);

void main()
{	
	//Logarithmic z-buffer correction
	gl_FragDepth = log2(logz) * FC;

	//Vectors
	vec3 P = fragPos.xyz/fragPos.w;
	vec3 N = normalize(normal);
	vec3 toEye = normalize(eyePos - P);
	
	//Ambient
	vec3 center = vec3(0.0, 0.0, planetRadiusInUnits);
	vec3 posSky = vec3(P.xy/atmLengthUnitInMeters, clamp(P.z/atmLengthUnitInMeters, -100000.0/atmLengthUnitInMeters, -0.5/atmLengthUnitInMeters));
	vec3 skyIlluminance;
    vec3 sunIlluminance = GetSunAndSkyIlluminance(posSky - center, N, sunDirection, skyIlluminance);
    fragColor = color * skyIlluminance;
	
	//Sun
	fragColor += SunContribution(P, N, toEye, color, sunIlluminance);
	
	fragColor = fragColor/whitePoint; //Color correction and normalization
	
	//Point lights
    if(lightId.x == 0)
	{
        for(int i=0; i<numPointLights; ++i)
        {
            if(lightId.y == i) continue;
		    fragColor += PointLightContribution(i, P, N, toEye, color).rgb;
        }
    }
    else
    {
        for(int i=0; i<numPointLights; ++i)
		    fragColor += PointLightContribution(i, P, N, toEye, color).rgb;
    }

    //Spot lights
    if(lightId.x == 1)
    {
        for(int i=0; i<numSpotLights; ++i)
        {
            if(lightId.y == i) continue;
            fragColor += SpotLightContribution(i, P, N, toEye, color).rgb;
        }
    }
    else
    {
        for(int i=0; i<numSpotLights; ++i)
            fragColor += SpotLightContribution(i, P, N, toEye, color).rgb;
    }

    //Light itself
    fragColor += color;

	//Normal
	fragNormal = vec4(normalize(eyeSpaceNormal) * 0.5 + 0.5, 0.0);
}