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

/**
 * Copyright (c) 2017 Eric Bruneton
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#version 330

uniform vec3 eyePos;
uniform vec3 sunDir;
uniform vec3 whitePoint;
uniform float cosSunSize;
uniform float bottomRadius;
uniform float groundAlbedo;

out vec3 fragColor;
in vec3 viewRay;

const float PI = 3.14159265358979323846;

vec3 GetSolarLuminance();
vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSkyLuminanceToPoint(vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance); 

void main()
{
	vec3 viewDir = normalize(viewRay);
	vec3 center = vec3(0.0, 0.0, bottomRadius);
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
		groundLuminance = groundAlbedo * (1.0 / PI) * (sunIlluminance + skyIlluminance);
        
		vec3 transmittance;
		vec3 inScatter = GetSkyLuminanceToPoint(P, point - center, 0.0, sunDir, transmittance);
		groundLuminance = groundLuminance * transmittance + inScatter;
		groundAlpha = 1.0;
	}
	
	//Sky
	vec3 transmittance;
    vec3 luminance = GetSkyLuminance(P, viewDir, 0.0, sunDir, transmittance);

	//Sun
	if(dot(viewDir, sunDir) > cosSunSize)
		luminance += transmittance * GetSolarLuminance();
	
	//Mix
	luminance = mix(luminance, groundLuminance, groundAlpha);
	
	//Color correction
    fragColor = luminance/whitePoint;
}
