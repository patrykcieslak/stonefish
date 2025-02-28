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

//Cook-Torrance model
uniform float metallic;
uniform float roughness;
const float PI = 3.14159265359;

//Schlick's approximation to Fresnel function (assuming wavelength dependent IOR)
// R0 = ((n1-n2)/(n1+n2))^2
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 ShadingModel(vec3 N, vec3 V, vec3 L, vec3 Lcolor, vec3 albedo)
{
    vec3 H = normalize(V+L); //Half-way vector (bisection vector)
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	
	float NDF = DistributionGGX(N, H, roughness);        
	float G = GeometrySmith(N, V, L, roughness);      
	vec3 F = fresnelSchlick(max(min(dot(H, V), 1.0), 0.0), F0);       
	
	vec3 nominator = NDF * G * F;
	float NdotL = max(dot(N, L), 0.0);                
	float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001; 
	vec3 specular = nominator/denominator;
    
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;	  
    
    return Lcolor * (kD * albedo / PI + specular) * NdotL;
}
