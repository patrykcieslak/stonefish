/*
 * Implementation of a Cook-Torrance shading model
 * 
 * Author: Patryk Cieslak
 * Copyright (c) 2017
*/

#version 330 core

in vec3 normal;
in vec2 texCoord;
in vec3 fragPos;
out vec3 fragColor;

uniform vec3 eyePos;
uniform vec3 viewDir;
uniform vec4 color;
uniform sampler2D tex;
uniform float shininess;
uniform float specularStrength;

float modelDiffuse(vec3 toLight)
{
	return max(dot(normal, toLight), 0.0);
}

float modelSpecular(vec3 halfwayDir)
{
	return pow(max(dot(normal, halfwayDir), 0.0), shininess+1.0) * specularStrength;
}

#inject "commonLights.frag"

void main()
{	
	//Common
	vec3 toEye = normalize(eyePos - fragPos);
	//Ambient
	vec3 irradiance = texture(texSkyDiffuse, vec3(normal.x, normal.z, -normal.y)).rgb;
	//Sun
	irradiance += calcSunContribution(toEye);
	//Point lights
	for(int i=0; i<numPointLights; ++i)
		irradiance += calcPointLightContribution(i, toEye);
	//Spot lights
	for(int i=0; i<numSpotLights; ++i)
		irradiance += calcSpotLightContribution(i, toEye);
	//Final composition	
	if(color.a > 0.0)
	{
		vec4 texColor = texture(tex, texCoord);
		fragColor = irradiance * mix(color.rgb, texColor.rgb, color.a*texColor.a);
	}
	else
		fragColor = irradiance * color.rgb;
}
