/*
 * Implementation of a Blinn-Phong shading model
 * 
 * Author: Patryk Cieslak
 * Copyright (c) 2017
*/

#version 330 core

//Blinn-Phong model
uniform float shininess;
uniform float specularStrength;
const float PI = 3.14159265359;

vec3 ShadingModel(vec3 N, vec3 toEye, vec3 toLight, vec3 albedo)
{
	vec3 halfway = normalize(toEye + toLight);
	float diffuse = max(dot(N, toLight), 0.0);
	float specular = pow(max(dot(N, halfway), 0.0), shininess) * specularStrength;
	return (diffuse+specular)*albedo;
}

