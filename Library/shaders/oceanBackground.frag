/*    
    Copyright (c) 2019 Patryk Cieslak. All rights reserved.

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

in vec4 fragPos;
in float logz;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec4 fragNormal;

uniform vec3 eyePos;
uniform float FC;

#inject "lightingDef.glsl"

vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);
vec3 InScatteringSun(vec3 L, vec3 D, vec3 V, float z, float d);
vec3 InScatteringPointLight(vec3 O, vec3 L, vec3 X, vec3 V, vec3 P, float d, float dw);
vec3 InScatteringSpotLight(vec3 O, vec3 D, float w, float fn, vec3 L, vec3 X, vec3 V, vec3 P, float d, float dw);
vec3 RefractToWater(vec3 I, vec3 N);

const float d = 1000000.0;
const vec3 waterSurfaceN = vec3(0.0, 0.0, -1.0);

void main() 
{
    //Logarithmic z-buffer correction
	gl_FragDepth = log2(logz) * FC;

    vec3 P = fragPos.xyz/fragPos.w * d;
	vec3 V = normalize(eyePos - P);
    vec3 center = vec3(0,0, planetRadiusInUnits);
    fragColor = vec3(0.0);
	
    //1. In-scattering from Sun/Sky
    vec3 R = RefractToWater(-sunDirection, waterSurfaceN);
    if(R.z > 0.0 && sunDirection.z < 0.0)
    {
	    vec3 skyIlluminance;
	    vec3 sunIlluminance = GetSunAndSkyIlluminance(-center, waterSurfaceN, sunDirection, skyIlluminance);
	    vec3 L = sunIlluminance/whitePoint;
	    fragColor = InScatteringSun(L, R, -V, max(eyePos.z, 0.0), d);
    }

    //2. In-scattering from point lights
	for(int i=0; i<numPointLights; ++i)
	{
		if(pointLights[i].position.z > 0.0)
			fragColor.rgb += InScatteringPointLight(pointLights[i].position, 
												pointLights[i].color,
												eyePos, -V, P, d, d);
	}

	//3. In-scattering from spot lights
	for(int i=0; i<numSpotLights; ++i)
	{
		if(spotLights[i].position.z > 0.0)
			fragColor.rgb += InScatteringSpotLight(spotLights[i].position,
											   spotLights[i].direction,
											   spotLights[i].cone,
											   spotLights[i].frustumNear,
											   spotLights[i].color,
											   eyePos, -V, P, d, d);
	}
	
    fragNormal = vec4(vec3(0.0,0.0,-1.0) * 0.5 + 0.5, 0.0);
}
