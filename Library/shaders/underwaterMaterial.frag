#version 330

//---------------Definitions--------------------
#define MEAN_SUN_ILLUMINANCE 107527.0
#define MAX_POINT_LIGHTS 	32
#define MAX_SPOT_LIGHTS 	32
#define NUM_SAMPLES 		32
#define INV_NUM_SAMPLES 	(1.0/32.0)
#define NUM_SPIRAL_TURNS 	7

const vec2 Poisson32[32] = vec2[](
    vec2(-0.975402, -0.0711386),
    vec2(-0.920347, -0.41142),
    vec2(-0.883908, 0.217872),
    vec2(-0.884518, 0.568041),
    vec2(-0.811945, 0.90521),
    vec2(-0.792474, -0.779962),
    vec2(-0.614856, 0.386578),
    vec2(-0.580859, -0.208777),
    vec2(-0.53795, 0.716666),
    vec2(-0.515427, 0.0899991),
    vec2(-0.454634, -0.707938),
    vec2(-0.420942, 0.991272),
    vec2(-0.261147, 0.588488),
    vec2(-0.211219, 0.114841),
    vec2(-0.146336, -0.259194),
    vec2(-0.139439, -0.888668),
    vec2(0.0116886, 0.326395),
    vec2(0.0380566, 0.625477),
    vec2(0.0625935, -0.50853),
    vec2(0.125584, 0.0469069),
    vec2(0.169469, -0.997253),
    vec2(0.320597, 0.291055),
    vec2(0.359172, -0.633717),
    vec2(0.435713, -0.250832),
    vec2(0.507797, -0.916562),
    vec2(0.545763, 0.730216),
    vec2(0.56859, 0.11655),
    vec2(0.743156, -0.505173),
    vec2(0.736442, -0.189734),
    vec2(0.843562, 0.357036),
    vec2(0.865413, 0.763726),
    vec2(0.872005, -0.927)
);

const vec2 Poisson64[64] = vec2[](
    vec2(-0.934812, 0.366741),
    vec2(-0.918943, -0.0941496),
    vec2(-0.873226, 0.62389),
    vec2(-0.8352, 0.937803),
    vec2(-0.822138, -0.281655),
    vec2(-0.812983, 0.10416),
    vec2(-0.786126, -0.767632),
    vec2(-0.739494, -0.535813),
    vec2(-0.681692, 0.284707),
    vec2(-0.61742, -0.234535),
    vec2(-0.601184, 0.562426),
    vec2(-0.607105, 0.847591),
    vec2(-0.581835, -0.00485244),
    vec2(-0.554247, -0.771111),
    vec2(-0.483383, -0.976928),
    vec2(-0.476669, -0.395672),
    vec2(-0.439802, 0.362407),
    vec2(-0.409772, -0.175695),
    vec2(-0.367534, 0.102451),
    vec2(-0.35313, 0.58153),
    vec2(-0.341594, -0.737541),
    vec2(-0.275979, 0.981567),
    vec2(-0.230811, 0.305094),
    vec2(-0.221656, 0.751152),
    vec2(-0.214393, -0.0592364),
    vec2(-0.204932, -0.483566),
    vec2(-0.183569, -0.266274),
    vec2(-0.123936, -0.754448),
    vec2(-0.0859096, 0.118625),
    vec2(-0.0610675, 0.460555),
    vec2(-0.0234687, -0.962523),
    vec2(-0.00485244, -0.373394),
    vec2(0.0213324, 0.760247),
    vec2(0.0359813, -0.0834071),
    vec2(0.0877407, -0.730766),
    vec2(0.14597, 0.281045),
    vec2(0.18186, -0.529649),
    vec2(0.188208, -0.289529),
    vec2(0.212928, 0.063509),
    vec2(0.23661, 0.566027),
    vec2(0.266579, 0.867061),
    vec2(0.320597, -0.883358),
    vec2(0.353557, 0.322733),
    vec2(0.404157, -0.651479),
    vec2(0.410443, -0.413068),
    vec2(0.413556, 0.123325),
    vec2(0.46556, -0.176183),
    vec2(0.49266, 0.55388),
    vec2(0.506333, 0.876888),
    vec2(0.535875, -0.885556),
    vec2(0.615894, 0.0703452),
    vec2(0.637135, -0.637623),
    vec2(0.677236, -0.174291),
    vec2(0.67626, 0.7116),
    vec2(0.686331, -0.389935),
    vec2(0.691031, 0.330729),
    vec2(0.715629, 0.999939),
    vec2(0.8493, -0.0485549),
    vec2(0.863582, -0.85229),
    vec2(0.890622, 0.850581),
    vec2(0.898068, 0.633778),
    vec2(0.92053, -0.355693),
    vec2(0.933348, -0.62981),
    vec2(0.95294, 0.156896)
);

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
	float zNear;
	float zFar;
	vec2 radius;
};

in vec3 normal;
in vec2 texCoord;
in vec3 fragPos;
in vec3 eyeSpaceNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec4 fragNormal;

uniform vec3 eyePos;
uniform vec3 viewDir;
uniform vec4 color;
uniform float reflectivity;
uniform sampler2D tex;

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform int numPointLights;
uniform int numSpotLights;
uniform sampler2DArray spotLightsDepthMap;
uniform sampler2DArrayShadow spotLightsShadowMap;

uniform vec3 sunDirection;
uniform mat4 sunClipSpace[4];
uniform float sunFrustumNear[4];
uniform float sunFrustumFar[4];
uniform sampler2DArray sunDepthMap;
uniform sampler2DArrayShadow sunShadowMap;
uniform float planetRadius;
uniform vec3 whitePoint;
uniform vec3 lightAbsorption;
uniform float turbidity;

const float water2Air = 1.33/1.0;
const vec3 rayleigh = vec3(0.15023, 0.405565, 1.0);

//---------------Functions-------------------
vec3 GetSolarLuminance();
vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSkyLuminanceToPoint(vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);
vec3 ShadingModel(vec3 N, vec3 toEye, vec3 toLight, vec3 albedo);

// Derivatives of light-space depth with respect to texture2D coordinates
vec2 depthGradient(vec2 uv, float z)
{
    vec2 dz_duv = vec2(0.0, 0.0);

    vec3 duvdist_dx = dFdx(vec3(uv,z));
    vec3 duvdist_dy = dFdy(vec3(uv,z));

    dz_duv.x = duvdist_dy.y * duvdist_dx.z;
    dz_duv.x -= duvdist_dx.y * duvdist_dy.z;

    dz_duv.y = duvdist_dx.x * duvdist_dy.z;
    dz_duv.y -= duvdist_dy.x * duvdist_dx.z;

    float det = (duvdist_dx.x * duvdist_dy.y) - (duvdist_dx.y * duvdist_dy.x);
    dz_duv /= det;

    return dz_duv;
}

float biasedZ(float z0, vec2 dz_duv, vec2 offset)
{
    return z0 + dot(dz_duv, offset);
}

//Sample depth from multilayer depth texture (works for sun and spot lights)
float borderDepthTexture(int layer, sampler2DArray tex, vec2 uv)
{
	return ((uv.x <= 1.0) && (uv.y <= 1.0) && (uv.x >= 0.0) && (uv.y >= 0.0)) ? textureLod(tex, vec3(uv, float(layer)), 0.0).z : 1.0;
}

//Sample shadow from multilayer shadow texture (works for sun and spot lights)
float borderPCFTexture(int layer, sampler2DArrayShadow tex, vec3 uvz)
{
	return ((uvz.x <= 1.0) && (uvz.y <= 1.0) && (uvz.x >= 0.0) && (uvz.y >= 0.0)) ? texture(tex, vec4(uvz.xy, float(layer), uvz.z)) : ((uvz.z <= 1.0) ? 1.0 : 0.0);
}

// Returns average blocker depth in the search region, as well as the number of found blockers.
// Blockers are defined as shadow-map samples between the surface point and the light.
void findBlocker(int layer, sampler2DArray tex, out float accumBlockerDepth, out float numBlockers, out float maxBlockers,
					vec2 uv, float z0, vec2 dz_duv, vec2 searchRegionRadiusUV)
{
    accumBlockerDepth = 0.0;
    numBlockers = 0.0;
	maxBlockers = 32.0;
	
    for (int i = 0; i < 32; ++i)
    {
		vec2 offset = Poisson32[i] * searchRegionRadiusUV;
        float shadowMapDepth = borderDepthTexture(layer, tex, uv + offset);
        float z = biasedZ(z0, dz_duv, offset);
        
		if (shadowMapDepth < z)
        {
			accumBlockerDepth += shadowMapDepth;
            numBlockers++;
		}
	}
}

// Performs PCF filtering on the shadow map using multiple taps in the filter region.
float pcfFilter(int layer, sampler2DArrayShadow tex, vec2 uv, float z0, vec2 dz_duv, vec2 filterRadiusUV)
{
	float sum = 0.0;

	for (int i = 0; i < 64; ++i)
	{
		vec2 offset = Poisson64[i] * filterRadiusUV;
        float z = biasedZ(z0, dz_duv, offset);
        sum += borderPCFTexture(layer, tex, vec3(uv + offset, z));
	}
    return sum / 64.0;
}

//Calculate in-shadow coefficient by sampling shadow edges
float calcSpotShadow(int id)
{
	vec4 fragPosLight = spotLights[id].clipSpace * vec4(fragPos, 1.0);
	vec3 shadowCoord = fragPosLight.xyz/fragPosLight.w;
	vec2 dz_duv = depthGradient(shadowCoord.xy, shadowCoord.z);
	
    // STEP 1: blocker search
    float accumBlockerDepth, numBlockers, maxBlockers;
    vec2 searchRegionRadiusUV = spotLights[id].radius * (shadowCoord.z - spotLights[id].zNear) / shadowCoord.z;
    findBlocker(id, spotLightsDepthMap, accumBlockerDepth, numBlockers, maxBlockers, shadowCoord.xy, shadowCoord.z, dz_duv, searchRegionRadiusUV);

    // Early out if not in the penumbra
    if (numBlockers == 0.0)
        return 1.0;

    // STEP 2: penumbra size
    float avgBlockerDepth = accumBlockerDepth / numBlockers;
    float avgBlockerDepthWorld = spotLights[id].zFar * spotLights[id].zNear / (spotLights[id].zFar - avgBlockerDepth * (spotLights[id].zFar - spotLights[id].zNear));
    vec2 penumbraRadius = spotLights[id].radius * (shadowCoord.z - avgBlockerDepthWorld) / avgBlockerDepthWorld;
    vec2 filterRadius = penumbraRadius * spotLights[id].zNear / shadowCoord.z;

    // STEP 3: filtering
    return pcfFilter(id, spotLightsShadowMap, shadowCoord.xy, shadowCoord.z, dz_duv, filterRadius);
}

//Calculate in-shadow coefficient by sampling shadow edges
float calcSunShadow(float waterDepth)
{
	float depth = dot(fragPos-eyePos, viewDir); 
	
	//Find the appropriate depth map to look up in based on the depth of this fragment
    int index = 3;
	if(depth < sunFrustumFar[0])
		index = 0;
	else if(depth < sunFrustumFar[1])
    	index = 1;
	else if(depth < sunFrustumFar[2])
		index = 2;
	
	vec4 fragPosLight = sunClipSpace[index] * vec4(fragPos, 1.0);
	vec3 shadowCoord = fragPosLight.xyz; //Orthographic projection doesn't need division by w
	shadowCoord.z += 0.001; //Bias
	vec2 dz_duv = depthGradient(shadowCoord.xy, shadowCoord.z);
	
	vec2 radiusUV = vec2(0.001) * (sunFrustumFar[0]-sunFrustumNear[0])/(sunFrustumFar[index]-sunFrustumNear[index]);
    radiusUV *= exp(waterDepth/2.0) * (1.0-exp(-turbidity/2.0)); //Underwater shadow blur
	
	// STEP 1: blocker search
    float accumBlockerDepth, numBlockers, maxBlockers;
    vec2 searchRegionRadiusUV = radiusUV * (shadowCoord.z - sunFrustumNear[index]) / shadowCoord.z;
    findBlocker(index, sunDepthMap, accumBlockerDepth, numBlockers, maxBlockers, shadowCoord.xy, shadowCoord.z, dz_duv, searchRegionRadiusUV);

    // Early out if not in the penumbra
    if (numBlockers == 0.0)
        return 1.0;

	//Constant penumbra for now!!!!
    //STEP: penumbra size
    //float avgBlockerDepth = accumBlockerDepth / numBlockers;
    //float avgBlockerDepthWorld = sunFrustumFar[index] * sunFrustumNear[index] / (sunFrustumFar[index] - avgBlockerDepth * (sunFrustumFar[index] - sunFrustumNear[index]));
    //vec2 penumbraRadius = radiusUV * (shadowCoord.z - avgBlockerDepthWorld) / avgBlockerDepthWorld;
    //vec2 filterRadius = penumbraRadius * sunFrustumNear[index] / shadowCoord.z;

    // STEP 2: filtering
    return pcfFilter(index, sunShadowMap, shadowCoord.xy, shadowCoord.z, dz_duv, radiusUV);
}

//Calculate contribution of different light types
vec3 calcPointLightContribution(int id, vec3 N, vec3 toEye, vec3 albedo)
{
	vec3 toLight = pointLights[id].position - fragPos;
	float distance = length(toLight);
	toLight /= distance;
	
	float attenuation = 1.0/(distance*distance);
	return ShadingModel(N, toEye, toLight, albedo) * pointLights[id].color * attenuation;
}

vec3 calcSpotLightContribution(int id, vec3 N, vec3 toEye, vec3 albedo)
{	
	vec3 toLight = spotLights[id].position - fragPos;
	float distance = length(toLight);
	toLight /= distance;
	
	float spotEffect = dot(spotLights[id].direction, -toLight); //Angle between spot direction and point-light vector
    float NdotL = dot(N, toLight);
        
	if(spotEffect > spotLights[id].angle && NdotL > 0.0)
	{
		float attenuation = 1.0/(distance*distance);
        float edge = smoothstep(1, 1.05, spotEffect/spotLights[id].angle);
		return ShadingModel(N, toEye, toLight, albedo) * spotLights[id].color * calcSpotShadow(id) * edge * attenuation;
	}
	else
		return vec3(0.0);
}

vec3 calcSunContribution(vec3 N, vec3 toEye, float waterDepth, vec3 albedo, vec3 illuminance)
{
	float NdotL = dot(N, sunDirection);
	
	if(NdotL > 0.0)
	{	
		return ShadingModel(N, toEye, sunDirection, albedo) * illuminance * calcSunShadow(waterDepth);
	}
	else
		return vec3(0.0);
}

void main()
{	
	//Common
    fragColor = vec3(0.);
	vec3 N = normalize(normal);
	vec3 toEye = normalize(eyePos - fragPos);
	vec3 center = vec3(0,0,-planetRadius);
	vec3 albedo = color.rgb;
	
	if(color.a > 0.0)
	{
		vec4 texColor = texture(tex, texCoord);
		albedo = mix(color.rgb, texColor.rgb, color.a*texColor.a);
	}
    
    //Water properties
    vec3 b = turbidity * rayleigh * 0.5; //Scattering coefficient
    vec3 c = lightAbsorption + b * 0.1; //Full attenuation coefficient
    
	//Water is assumed to be a flat plane so... 
	float depth = -min(0.0,fragPos.z);
    vec3 toSky = normalize(refract(N, vec3(0.0,0.0,-1.0), water2Air));
	vec3 skyIlluminance;
	vec3 sunIlluminance;
    
    //1. Direct lighting
	if(N.z > 0.0)// && toSky.z > 0.0)
	{
		//Ambient
		sunIlluminance = GetSunAndSkyIlluminance(vec3(fragPos.xy, 0.0) - center, N, sunDirection, skyIlluminance);
		fragColor += albedo * skyIlluminance/whitePoint/MEAN_SUN_ILLUMINANCE;
	
		//Sun
		fragColor += calcSunContribution(N, toEye, depth, albedo, sunIlluminance/whitePoint/MEAN_SUN_ILLUMINANCE);
		
		//Absorption
        float lSurface = depth/N.z;
        fragColor *= exp(-c * lSurface) * smoothstep(0.0, 0.5, N.z);
    }
    
    //2. Inscatter
    sunIlluminance = GetSunAndSkyIlluminance(-center, vec3(0,0,1.0), sunDirection, skyIlluminance);
    vec3 inFactor = exp(-c * depth) * (N.z + 1.0)/2.0 * b;
    fragColor += albedo * (sunIlluminance + skyIlluminance)/whitePoint/MEAN_SUN_ILLUMINANCE * inFactor * 0.1;
    
    //vec3 inFactor = (exp(-lightAbsorption * (depth - lSurface * N.z + lSurface)) - exp(-lightAbsorption * depth))/(lightAbsorption * (N.z-1.0));
    //fragColor += albedo * skyIlluminance/whitePoint/MEAN_SUN_ILLUMINANCE * inFactor * (N.z + 1.0)/2.0;
    //vec3 inFactor = exp(-lightAbsorption * depth) * (exp((N.z - 1.0)*lightAbsorption*lSurface) - 1.0)/((N.z - 1.0) * lightAbsorption);
    //fragColor += albedo * skyIlluminance/whitePoint/MEAN_SUN_ILLUMINANCE * inFactor;
    
	//--> Point lights
	for(int i=0; i<numPointLights; ++i)
		fragColor += calcPointLightContribution(i, N, toEye, albedo);
	//--> Spot lights
	for(int i=0; i<numSpotLights; ++i)
		fragColor += calcSpotLightContribution(i, N, toEye, albedo);
    
    //3. To camera
    //Absorption
    float d = 0.0;
    
    if(eyePos.z > 0.0 && toEye.z > 0.0)
        d = depth/toEye.z;
    else
        d = length(eyePos - fragPos);
    
    vec3 aFactor = exp(-c * d);
    fragColor *= aFactor;
     
    //Inscattering
    inFactor = exp(-c * max(-eyePos.z,0.0)) * (exp((-toEye.z - 1.0)* c * d) - 1.0)/((-toEye.z - 1.0) * c) * b;
    fragColor += (sunIlluminance + skyIlluminance)/whitePoint/MEAN_SUN_ILLUMINANCE * inFactor * 0.01;
    
    //Normal
	fragNormal = vec4(normalize(eyeSpaceNormal) * 0.5 + 0.5, reflectivity);
}
    
