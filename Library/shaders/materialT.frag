/*   
    Copyright (c) 2024 Patryk Cieslak. All rights reserved.

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

layout(location = 0) out float fragColor;

uniform vec3 eyePos;
uniform vec3 viewDir;
uniform float FC;
uniform vec4 color;
uniform float reflectivity;
uniform float temperature;

#inject "lightingDef.glsl"

//---------------Functions-------------------
vec3 GetSolarLuminance();
vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSkyLuminanceToPoint(vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);
vec3 SunContribution(vec3 P, vec3 N, vec3 toEye, vec3 albedo, vec3 illuminance);

void main()
{	
	//Logarithmic z-buffer correction
	gl_FragDepth = log2(logz) * FC;

	//Vectors
	vec3 P = fragPos.xyz/fragPos.w;
	vec3 N = normalize(normal);
	vec3 toEye = normalize(eyePos - P);
	vec4 absorption = vec4(clamp(1.0 - color.rgb, 1.0, 0.001), 1.0);
	
	//Ambient
	vec3 center = vec3(0.0, 0.0, planetRadiusInUnits);
	vec3 posSky = vec3(P.xy/atmLengthUnitInMeters, clamp(P.z/atmLengthUnitInMeters, -100000.0/atmLengthUnitInMeters, -0.5/atmLengthUnitInMeters));
	vec3 skyIlluminance;
    vec3 sunIlluminance = GetSunAndSkyIlluminance(posSky - center, N, sunDirection, skyIlluminance);
    fragColor = temperature + length(absorption.rgb * skyIlluminance / whitePoint) * 0.0001;
	
	//Sun
	fragColor += length(SunContribution(P, N, toEye, absorption.rgb, sunIlluminance) / whitePoint) * 0.0001;
}
