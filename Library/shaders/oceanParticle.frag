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

#define MEAN_SUN_ILLUMINANCE 107527.0
#define MAX_POINT_LIGHTS     32
#define MAX_SPOT_LIGHTS     32

struct PointLight
{
    vec3 position;
    vec3 color;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    vec3 color;
    float angle;
    mat4 clipSpace;
    float zNear;
    float zFar;
    vec2 radius;
};

in vec2 texcoord;
in vec3 fragPos;
out vec4 fragColor;

uniform sampler2D tex;
uniform vec4 color;
uniform vec3 eyePos;
uniform vec3 lookingDir;
uniform vec3 sunDirection;
uniform float planetRadius;
uniform vec3 whitePoint;
uniform vec3 lightAbsorption;
uniform float turbidity;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform int numPointLights;
uniform int numSpotLights;

const vec3 rayleigh = vec3(0.15023, 0.405565, 1.0);

vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);

vec3 calcPointLightContribution(int id)
{
    vec3 toLight = pointLights[id].position - fragPos;
    float distance = length(toLight);
    toLight /= distance;
    float attenuation = 1.0/(distance*distance);
    return pointLights[id].color * attenuation * (0.5 + 0.5 * abs(dot(-toLight, lookingDir)));
}

vec3 calcSpotLightContribution(int id)
{
    vec3 toLight = spotLights[id].position - fragPos;
    float distance = length(toLight);
    toLight /= distance;
    
    float spotEffect = dot(spotLights[id].direction, -toLight); //Angle between spot direction and point-light vector
    
    if(spotEffect > spotLights[id].angle)
    {
        float attenuation = 1.0/(distance*distance);
        float edge = smoothstep(1, 1.05, spotEffect/spotLights[id].angle);
        return spotLights[id].color * edge * attenuation * (0.5 + 0.5 * abs(dot(-toLight, lookingDir)));
    }
    else
        return vec3(0.0);
}

void main(void)
{
    vec4 albedo = texture(tex, texcoord).rrrr * color;
    vec3 center = vec3(0,0,-planetRadius);
    
    //Water properties
    vec3 b = turbidity * rayleigh * 0.5; //Scattering coefficient
    vec3 c = lightAbsorption + b * 0.1; //Full attenuation coefficient
    float depth = max(0.0, fragPos.z);
    
    //Sky illumination
    vec3 skyIlluminance;
    vec3 sunIlluminance = GetSunAndSkyIlluminance(-center, vec3(0,0,-1.0), sunDirection, skyIlluminance);
    fragColor = vec4((sunIlluminance + skyIlluminance)/whitePoint/MEAN_SUN_ILLUMINANCE, 1.0);
    fragColor.rgb *= exp(-c * depth) * (0.5 + 0.5 * abs(dot(sunDirection, lookingDir)));
    
    //Artificial ligths illumination
    //--> Point lights
    for(int i=0; i<numPointLights; ++i)
        fragColor.rgb += calcPointLightContribution(i);
    //--> Spot lights
    for(int i=0; i<numSpotLights; ++i)
        fragColor.rgb += calcSpotLightContribution(i);
    
    //Material
    fragColor *= albedo;
    
    //Absorption from particle to camera
    float d = length(eyePos - fragPos);
    vec3 aFactor = exp(-c * d);
    fragColor.rgb *= aFactor;
}
