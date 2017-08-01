/*
 * Implementation of a Cook-Torrance shading model
 * 
 * Author: Patryk Cieslak
 * Copyright (c) 2017
*/

#version 330 core

//Cook-Torrance model
uniform float metallic;
uniform float roughness;
const float PI = 3.14159265359;

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

vec3 ShadingModel(vec3 N, vec3 toEye, vec3 toLight, vec3 albedo)
{
	vec3 halfway = normalize(toEye + toLight);
	
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	
	float NDF = DistributionGGX(N, halfway, roughness);        
	float G = GeometrySmith(N, toEye, toLight, roughness);      
	vec3 F = fresnelSchlick(max(dot(halfway, toEye), 0.0), F0);       
	
	vec3 nominator = NDF * G * F;
	float NdotL = max(dot(N, toLight), 0.0);                
	float denominator = 4.0 * max(dot(N, toEye), 0.0) * NdotL + 0.001; 
	vec3 specular = nominator/denominator;
    
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;	  
        
    return (kD * albedo / PI + specular) * NdotL;
}