#version 330 core

in vec3 normal;
in vec2 texCoord;
in vec3 fragPos;
out vec4 fragColor;

uniform vec3 eyePos;
uniform vec4 color;
uniform sampler2D tex;
uniform float shininess;
uniform float specularStrength;

float ModelDiffuse(vec3 lightDir, vec3 halfwayDir)
{
	return max(dot(normal, -lightDir), 0.0);
}

float ModelSpecular(vec3 halfwayDir)
{
	return pow(max(dot(normal, -halfwayDir), 0.0), shininess*100.0) * specularStrength;
}

//============================COMMON===========================================
#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 8

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
	sampler2DShadow shadow;
};

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform samplerCube texSkyDiffuse;
uniform vec3 sunDirection;
uniform vec4 sunColor;

vec3 CalcPointLightContribution(int id, vec3 eyeDir)
{
	vec3 lightDir = normalize(pointLights[id].position - fragPos);
	vec3 halfwayDir = normalize(eyeDir + lightDir);
	float diffuse = ModelDiffuse(lightDir, halfwayDir);
	float specular = ModelSpecular(halfwayDir);
	return (diffuse + specular)*pointLights[id].color;
}

vec3 CalcSpotLightContribution(int id, vec3 eyeDir)
{
	vec3 lightDir = normalize(spotLights[id].position - fragPos);
	float spotEffect = dot(spotLights[id].direction, -lightDir);
	
	if(spotEffect > spotLights[id].angle)
	{
		vec3 halfwayDir = normalize(eyeDir + lightDir);
		float diffuse = ModelDiffuse(lightDir, halfwayDir);
		float specular = ModelSpecular(halfwayDir);
		return (diffuse + specular)*spotLights[id].color;
	}
	else
		return vec3(0.0);
}

vec3 CalcSunContribution(vec3 eyeDir)
{
	vec3 halfwayDir = normalize(eyeDir + sunDirection);
	float diffuse = ModelDiffuse(sunDirection, halfwayDir);
	float specular = ModelSpecular(halfwayDir);
	return (diffuse + specular)*sunColor.rgb;
}
//===========================================================================

void main()
{	
	//Common
	vec3 eyeDir = normalize(eyePos.xyz - fragPos);
	//Ambient
	vec3 irradiance = texture(texSkyDiffuse, vec3(normal.x, normal.z, -normal.y)).rgb;
	//Sun
	irradiance += CalcSunContribution(eyeDir);
	
	//Point lights
	//for(int i=0; i<MAX_POINT_LIGHTS; ++i)
	//	irradiance += CalcPointLightContribution(i, eyeDir);
	//Spot lights
	//for(int i=0; i<MAX_SPOT_LIGHTS; ++i)
	//	irradiance += CalcSpotLightContribution(i, eyeDir);
	
	irradiance *= color.rgb;
	fragColor = vec4(irradiance, 1.0);
}
