//---------------Definitions--------------------
#define MAX_POINT_LIGHTS 	8
#define MAX_SPOT_LIGHTS 	8
#define SHADOWMAP_SIZE 		2048.0
#define SUN_SHADOWMAP_SIZE	4096.0
#define NUM_SAMPLES 		32
#define INV_NUM_SAMPLES 	(1.0/32.0)
#define NUM_SPIRAL_TURNS 	7

//---------------Data structures-----------------
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
	sampler2DShadow shadowMap;
};

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform int numPointLights;
uniform int numSpotLights;

uniform vec3 sunDirection;
uniform vec4 sunColor;
uniform mat4 sunClipSpace[4];
uniform vec4 sunFrustumFar;
uniform sampler2DArrayShadow sunShadowMap;

uniform samplerCube texSkyDiffuse;

//---------------Functions-------------------
vec3 GetSolarLuminance();
vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSkyLuminanceToPoint(vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);

vec3 getWorldNormal(mat4 invProj, mat3 invViewRot, vec2 viewportSize)
{
    vec2 fragPos = gl_FragCoord.xy/viewportSize;
    fragPos = (fragPos-0.5)*2.0;
    vec4 deviceNormal = vec4(fragPos, 0.0, 1.0);
    vec3 eyeNormal = normalize((invProj * deviceNormal).xyz);
    vec3 worldNormal = normalize(invViewRot * eyeNormal);
    return worldNormal;
}

//Generate random numbers
float random(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_product) * 43758.5453);
}

//Generate sample location based on a spiral curve
vec2 tapLocation(int sampleNumber, float spinAngle, out float radius)
{
    float alpha = (float(sampleNumber) + 0.5) * INV_NUM_SAMPLES;
    float angle = alpha * (float(NUM_SPIRAL_TURNS) * 6.28) + spinAngle;
    radius = alpha;
    return vec2(cos(angle), sin(angle));
}

//Calculate in-shadow coefficient by sampling shadow edges
float calcSpotShadow(int id, float bias)
{
	vec4 fragPosLight = spotLights[id].clipSpace * vec4(fragPos, 1.0);
	vec3 shadowCoord = fragPosLight.xyz/fragPosLight.w;
	float fragDepth = shadowCoord.z + bias/fragPosLight.w;
	
	float nominalRadius = SHADOWMAP_SIZE/400.0;
    float rnd = random(fragPos, 0) * 10.0;
	
    //Cheap shadow testing of outward pixels (5 samples)
    float inShadow = texture(spotLights[id].shadowMap, vec3(shadowCoord.xy, fragDepth));
		
    for(int i = NUM_SAMPLES - 5; i < NUM_SAMPLES - 1; ++i)
    {
        float radiusFactor;
        vec2 offset = tapLocation(i, rnd, radiusFactor);
        inShadow += texture(spotLights[id].shadowMap, vec3(shadowCoord.xy + (offset*radiusFactor*nominalRadius)/SHADOWMAP_SIZE, fragDepth));
	}
    
    if(inShadow > 4.0) //Fully in light 5*0.75
        return 1.0;
    else if(inShadow < 0.75) //Fully in shadow
        return 0.0;
    else //Precise sampling for the shadow edges
    {
        inShadow = inShadow * INV_NUM_SAMPLES;
        for(int i = 0; i < NUM_SAMPLES - 5; ++i)
        {
            float radiusFactor;
            vec2 offset = tapLocation(i, rnd, radiusFactor);
            inShadow += INV_NUM_SAMPLES * texture(spotLights[id].shadowMap, vec3(shadowCoord.xy + (offset*radiusFactor*nominalRadius)/SHADOWMAP_SIZE, fragDepth));
        }
        
        return inShadow;
    }
}

//Calculate in-shadow coefficient by sampling shadow edges
float calcSunShadow(float bias)
{
	float depth = dot(fragPos-eyePos, viewDir); 
	
	//Find the appropriate depth map to look up in based on the depth of this fragment
    int index = 3;
	float ff = sunFrustumFar.w;
    
	if(depth < sunFrustumFar.x)
    {
		index = 0;
        ff = sunFrustumFar.x;
    }
	else if(depth < sunFrustumFar.y)
    {
		index = 1;
        ff = sunFrustumFar.y;
    }
	else if(depth < sunFrustumFar.z)
    {
		index = 2;
        ff = sunFrustumFar.z;
    }
    
	//Transform this fragment's position from view space to scaled light clip space such that the xy coordinates are in [0;1]
	//Note: there is no need to divide by w for othogonal light sources
    vec4 shadowCoord =  sunClipSpace[index] * vec4(fragPos, 1.0);
    float fragDepth = shadowCoord.z + bias;
    
	float nominalRadius = SUN_SHADOWMAP_SIZE/400.0 * sunFrustumFar.x/ff;
    float rnd = random(fragPos, 0);
    
    //Cheap shadow testing for all pixels (5 samples)
	float inShadow = texture(sunShadowMap, vec4(shadowCoord.xy, float(index), fragDepth));
    
	for(int i = NUM_SAMPLES - 5; i < NUM_SAMPLES - 1; ++i)
    {
        float radiusFactor;
		vec2 offset = tapLocation(i, rnd, radiusFactor);
        inShadow += texture(sunShadowMap, vec4(shadowCoord.xy + (offset*radiusFactor*nominalRadius)/SUN_SHADOWMAP_SIZE, float(index), fragDepth));
    }
    
	if(inShadow == 5.0) //Fully in light
        return 1.0;
    else if(inShadow == 0.0) //Fully in shadow
        return 0.0;
    else //Precise sampling for the shadow border pixels
    {
        inShadow *= INV_NUM_SAMPLES;
        for(int i = 0; i < NUM_SAMPLES - 5; ++i)
        {
            float radiusFactor;
			vec2 offset = tapLocation(i, rnd, radiusFactor);
            inShadow += INV_NUM_SAMPLES * texture(sunShadowMap, vec4(shadowCoord.xy + (offset*radiusFactor*nominalRadius)/SUN_SHADOWMAP_SIZE, float(index), fragDepth));
        }
        
        return inShadow;
    }
}

//Calculate contribution of different light types
vec3 calcPointLightContribution(int id, vec3 N, vec3 toEye, vec3 albedo)
{
	vec3 toLight = pointLights[id].position - fragPos;
	float distance = length(toLight);
	toLight /= distance;
	
	float attenuation = 1.0/(distance*distance);
	return shadingModel(N, toEye, toLight, albedo) * pointLights[id].color * attenuation;
}

vec3 calcSpotLightContribution(int id, vec3 N, vec3 toEye, vec3 albedo)
{	
	vec3 toLight = spotLights[id].position - fragPos;
	float distance = length(toLight);
	toLight /= distance;
	
	float spotEffect = dot(spotLights[id].direction, -toLight);
	float NdotL = dot(N, -spotLights[id].direction);
        
	if(spotEffect > spotLights[id].angle && NdotL > 0.0)
	{
		float attenuation = 1.0/(distance*distance);
		float bias = 0.0005 * tan(acos(NdotL));
		bias = clamp(bias, 0, 0.005);
		return shadingModel(N, toEye, toLight, albedo) * spotLights[id].color * calcSpotShadow(id, bias) * pow(spotEffect, 10.0) * attenuation;
	}
	else
		return vec3(0.0);
}

vec3 calcSunContribution(vec3 N, vec3 toEye, vec3 albedo)
{
	float NdotL = dot(N, -sunDirection);
	
	if(NdotL > 0.0)
	{	
		//float bias = 0.001 * tan(acos(NdotL));
		//bias = clamp(bias, 0, 0.01);
		float bias = 0.001;
		return shadingModel(N, toEye, -sunDirection, albedo) * sunColor.rgb * calcSunShadow(bias);
	}
	else
		return vec3(0.0);
}