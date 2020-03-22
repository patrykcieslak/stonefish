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
in vec3 fragPos;
in vec3 eyeSpaceNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec4 fragNormal;

uniform vec3 eyePos;
uniform vec3 viewDir;
uniform vec4 color;
uniform float reflectivity;
uniform sampler2D tex;

uniform vec3 sunDirection;
uniform float planetRadius;
uniform vec3 whitePoint;

uniform int numPointLights;
uniform int numSpotLights;

//---------------Functions-------------------
vec3 GetSolarLuminance();
vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSkyLuminanceToPoint(vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);
float SpotShadow(int id);
float SunShadow();
vec3 PointLightContribution(int id, vec3 N, vec3 toEye, vec3 albedo);
vec3 SpotLightContribution(int id, vec3 N, vec3 toEye, vec3 albedo);
vec3 SunContribution(vec3 N, vec3 toEye, vec3 albedo, vec3 illuminance);

void main()
{	
	//Common
	vec3 N = normalize(normal);
	vec3 toEye = normalize(eyePos - fragPos);
	vec3 center = vec3(0,0,-planetRadius);
	vec3 albedo = color.rgb;
	
	if(color.a > 0.0)
	{
		vec4 texColor = texture(tex, texCoord);
		albedo = mix(color.rgb, texColor.rgb, color.a*texColor.a);
	}
	
	//Ambient
	vec3 skyIlluminance;
    vec3 sunIlluminance = GetSunAndSkyIlluminance(fragPos - center, N, sunDirection, skyIlluminance);
    fragColor = albedo * skyIlluminance/whitePoint/MEAN_SUN_ILLUMINANCE;
	
	//Sun
	fragColor += SunContribution(N, toEye, albedo, sunIlluminance/whitePoint/MEAN_SUN_ILLUMINANCE);
	//Point lights
	for(int i=0; i<numPointLights; ++i)
		fragColor += PointLightContribution(i, N, toEye, albedo);
	//Spot lights
	for(int i=0; i<numSpotLights; ++i)
		fragColor += SpotLightContribution(i, N, toEye, albedo);
		
	//Normal
	fragNormal = vec4(normalize(eyeSpaceNormal) * 0.5 + 0.5, reflectivity);
}
