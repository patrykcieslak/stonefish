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

//Frostbite standard model
uniform float metallic;
uniform float roughness;
const float PI = 3.14159265359;

vec3 F_Schlick(vec3 f0, float f90, float u)
{
    return f0 + (f90 - f0) * pow (1.0-u, 5.0);
}

float V_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
    //Original formulation
    // lambda_v = ( -1 + sqrt ( alphaG2 * (1 - NdotL2 ) / NdotL2 + 1) ) * 0.5 f ;
    // lambda_l = ( -1 + sqrt ( alphaG2 * (1 - NdotV2 ) / NdotV2 + 1) ) * 0.5 f ;
    // G = 1 / (1 + lambda_v + lambda_l ) ;
    // V = G / (4.0 f * NdotL * NdotV ) ;

    // This is the optimized version
    float alphaG2 = alphaG * alphaG;
    // Caution : the " NdotL *" and " NdotV *" are explicitely inversed , this is not a mistake .
    float Lambda_GGXV = NdotL * sqrt(( - NdotV * alphaG2 + NdotV ) * NdotV + alphaG2 );
    float Lambda_GGXL = NdotV * sqrt(( - NdotL * alphaG2 + NdotL ) * NdotL + alphaG2 );
    return 0.5/(Lambda_GGXV + Lambda_GGXL); 
}

float D_GGX(float NdotH, float m)
{
    //Divide by PI is applied later
    float m2 = m * m;
    float f = ( NdotH * m2 - NdotH ) * NdotH + 1.0;
    return m2/(f*f);
}

float Fr_DisneyDiffuse(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
    float energyBias = mix(0, 0.5, linearRoughness);
    float energyFactor = mix(1.0, 1.0/1.51, linearRoughness);
    float fd90 = energyBias + 2.0 * LdotH * LdotH * linearRoughness;
    vec3 f0 = vec3(1.0);
    float lightScatter = F_Schlick(f0, fd90, NdotL).r;
    float viewScatter = F_Schlick(f0, fd90, NdotV).r;
    return lightScatter * viewScatter * energyFactor;
}

vec3 ShadingModel(vec3 N, vec3 V, vec3 L, vec3 Lcolor, vec3 albedo)
{
	float NdotV = abs(dot(N, V)) + 1e-5;
    vec3 H = normalize(V + L);
    float LdotH = clamp(dot(L, H), 0.0, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float NdotL = clamp(dot(N, L), 0.0, 1.0);
    float linearRoughness = roughness * roughness;

    vec3 f0 = vec3(0.16*metallic*metallic);
    float f90 = 1.0;
    vec3 F = F_Schlick(f0, f90, LdotH);
    float Vis = V_SmithGGXCorrelated(NdotV, NdotL, roughness);
    float D = D_GGX(NdotH, roughness);
    float Fr = D * F.r * Vis / PI;
    float Fd = Fr_DisneyDiffuse(NdotV, NdotL, LdotH, linearRoughness) / PI;
    
    return Lcolor * (albedo * Fd +  Fr);
}
