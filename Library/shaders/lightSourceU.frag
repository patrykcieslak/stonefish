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

const vec3 waterSurfaceN = vec3(0.0, 0.0, -1.0);

//---------------Functions-------------------
vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSkyLuminanceToPoint(vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);
float SpotShadow(int id);
float SunShadow();
vec4 PointLightContribution(int id, vec3 P, vec3 N, vec3 toEye, vec3 albedo);
vec4 SpotLightContribution(int id, vec3 P, vec3 N, vec3 toEye, vec3 albedo);
vec3 SunContribution(vec3 P, vec3 N, vec3 toEye, vec3 albedo, vec3 illuminance);
vec3 RefractToWater(vec3 I, vec3 N);
vec3 RefractToAir(vec3 I, vec3 N);
vec3 BeerLambert(float d);
vec3 InScatteringSun(vec3 L, vec3 D, vec3 V, float z, float d);
vec3 InScatteringPointLight(vec3 O, vec3 L, vec3 X, vec3 V, vec3 P, float d, float dw);
vec3 InScatteringSpotLight(vec3 O, vec3 D, float w, float fn, vec3 L, vec3 X, vec3 V, vec3 P, float d, float dw);

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

	if(eyePos.z < 0.0)
		dw = max(P.z, 0.0)/dot(V, waterSurfaceN);
	
	//1. Direct lighting + out-scattering
	//Ambient
	vec3 center = vec3(0.0, 0.0, planetRadiusInUnits);
	vec3 posSky = vec3(P.xy/atmLengthUnitInMeters,-0.5/atmLengthUnitInMeters);
	vec3 skyIlluminance;
	vec3 sunIlluminance = GetSunAndSkyIlluminance(posSky - center, N, sunDirection, skyIlluminance);
	fragColor = color * skyIlluminance * BeerLambert(dw+P.z);
	
	//Sun
	if(S.z > 0.0)
		fragColor += SunContribution(P, N, V, color, sunIlluminance) * BeerLambert(dw+P.z/S.z);
	
	fragColor = fragColor/whitePoint; //Color correction and normalization
	
	//Point lights
    if(lightId.x == 0)
    {
        for(int i=0; i<numPointLights; ++i)
        {
            if(lightId.y == i) continue;
            vec4 Ld = PointLightContribution(i, P, N, V, color);
            fragColor += Ld.rgb * BeerLambert(dw + Ld.a);
        }
    }
    else
    {
        for(int i=0; i<numPointLights; ++i)
        {
            vec4 Ld = PointLightContribution(i, P, N, V, color);
            fragColor += Ld.rgb * BeerLambert(dw + Ld.a);
        }
    }

    //Spot lights
	if(lightId.x == 1)
    {
        for(int i=0; i<numSpotLights; ++i)
        {
            if(lightId.y == i) continue;
            vec4 Ld = SpotLightContribution(i, P, N, V, color);
            fragColor += Ld.rgb * BeerLambert(dw + Ld.a);
        }
    }
    else
    {
        for(int i=0; i<numSpotLights; ++i)
        {
            vec4 Ld = SpotLightContribution(i, P, N, V, color);
            fragColor += Ld.rgb * BeerLambert(dw + Ld.a);
        }
    }

	//2. In-scattering from Sun/Sky
    vec3 R = RefractToWater(-sunDirection, waterSurfaceN);
    if(R.z > 0.0 && sunDirection.z < 0.0)
    {
	    vec3 skyIlluminance;
	    vec3 sunIlluminance = GetSunAndSkyIlluminance(posSky - center, waterSurfaceN, sunDirection, skyIlluminance);
	    vec3 L = sunIlluminance/whitePoint;
	    fragColor += InScatteringSun(L, R, -V, max(eyePos.z, 0.0), dw);
    }

	//3. In-scattering from point lights
	for(int i=0; i<numPointLights; ++i)
	{
        if(pointLights[i].position.z > 0.0)
		    fragColor += InScatteringPointLight(pointLights[i].position, 
			    								pointLights[i].color,
				    							eyePos, -V, P, d, dw);
	}

	//4. In-scattering from spot lights
	for(int i=0; i<numSpotLights; ++i)
	{
        if(spotLights[i].position.z > 0.0)
		    fragColor += InScatteringSpotLight(spotLights[i].position,
			    							   spotLights[i].direction,
				    						   spotLights[i].cone,
					    					   spotLights[i].frustumNear,
						    				   spotLights[i].color,
							    			   eyePos, -V, P, d, dw);
	}

	//5. Light itself
    fragColor += color * BeerLambert(dw);

    //Normal
	fragNormal = vec4(normalize(eyeSpaceNormal) * 0.5 + 0.5, 0.0);
}
    
