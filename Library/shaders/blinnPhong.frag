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
layout(location = 1) out vec3 viewNormal;

uniform vec3 eyePos;
uniform vec3 viewDir;
uniform vec4 color;
uniform sampler2D tex;

//Blinn-Phong model
uniform float shininess;
uniform float specularStrength;

vec3 shadingModel(vec3 toEye, vec3 toLight, vec3 albedo)
{
	vec3 halfway = normalize(toEye + toLight);
	float diffuse = max(dot(normal, toLight), 0.0);
	float specular = pow(max(dot(normal, halfway), 0.0), shininess) * specularStrength;
	return (diffuse+specular)*albedo;
}

#inject "commonLights.frag"

void main()
{	
	//Common
	vec3 toEye = normalize(eyePos - fragPos);
	
	vec3 albedo = color.rgb;
	if(color.a > 0.0)
	{
		vec4 texColor = texture(tex, texCoord);
		albedo = mix(color.rgb, texColor.rgb, color.a*texColor.a);
	}
	
	//Ambient
	fragColor = texture(texSkyDiffuse, vec3(normal.x, normal.z, -normal.y)).rgb * albedo;
	//Sun
	fragColor += calcSunContribution(toEye, albedo);
	//Point lights
	for(int i=0; i<numPointLights; ++i)
		fragColor += calcPointLightContribution(i, toEye, albedo);
	//Spot lights
	for(int i=0; i<numSpotLights; ++i)
		fragColor += calcSpotLightContribution(i, toEye, albedo);
		
	//Normal
	viewNormal = eyeSpaceNormal * 0.5 + 0.5;
}