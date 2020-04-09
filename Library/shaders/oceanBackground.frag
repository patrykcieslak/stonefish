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

in vec4 fragPos;
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec4 fragNormal;

uniform vec3 eyePos;

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

vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);
vec3 InScattering(vec3 L, vec3 D, vec3 V, float z, float d);

const float d = 1000000.0;
const vec3 waterSurfaceN = vec3(0.0, 0.0, -1.0);

void main() 
{
    vec3 P = fragPos.xyz * d;
	vec3 V = normalize(eyePos - P);
    vec3 center = vec3(0,0,planetRadiusInUnits);
	
    //Inscattering
    vec3 skyIlluminance;
	vec3 sunIlluminance = GetSunAndSkyIlluminance(-center, waterSurfaceN, sunDirection, skyIlluminance);
	vec3 L = (sunIlluminance + skyIlluminance)/whitePoint/MEAN_SUN_ILLUMINANCE;
	fragColor = InScattering(L, -waterSurfaceN, V, max(eyePos.z, 0.0), d);
    
    fragNormal = vec4(vec3(0.0,0.0,-1.0) * 0.5 + 0.5, 0.0);
}
