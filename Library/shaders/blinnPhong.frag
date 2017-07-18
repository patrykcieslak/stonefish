/*
 * Implementation of a Blinn-Phong shading model
 * 
 * Author: Patryk Cieslak
 * Copyright (c) 2017
*/

#version 330 core

in vec3 normal;
in vec2 texCoord;
in vec3 fragPos;
in vec3 eyeSpaceNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;

uniform vec3 eyePos;
uniform vec3 viewDir;
uniform vec4 color;
uniform sampler2D tex;

//Blinn-Phong model
uniform float shininess;
uniform float specularStrength;

vec3 shadingModel(vec3 N, vec3 toEye, vec3 toLight, vec3 albedo)
{
	vec3 halfway = normalize(toEye + toLight);
	float diffuse = max(dot(N, toLight), 0.0);
	float specular = pow(max(dot(N, halfway), 0.0), shininess) * specularStrength;
	return (diffuse+specular)*albedo;
}

#inject "commonLights.frag"

void main()
{	
	//Common
	vec3 N = normalize(normal);
	vec3 toEye = normalize(eyePos - fragPos);
	
	vec3 albedo = color.rgb;
	if(color.a > 0.0)
	{
		vec4 texColor = texture(tex, texCoord);
		albedo = mix(color.rgb, texColor.rgb, color.a*texColor.a);
	}
	
	//Ambient
	fragColor = texture(texSkyDiffuse, vec3(N.x, N.z, -N.y)).rgb * albedo;
	//Sun
	fragColor += calcSunContribution(N, toEye, albedo);
	//Point lights
	for(int i=0; i<numPointLights; ++i)
		fragColor += calcPointLightContribution(i, N, toEye, albedo);
	//Spot lights
	for(int i=0; i<numSpotLights; ++i)
		fragColor += calcSpotLightContribution(i, N, toEye, albedo);
		
	//Normal
	fragNormal = normalize(eyeSpaceNormal) * 0.5 + 0.5;
}