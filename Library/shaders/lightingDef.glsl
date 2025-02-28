#define MAX_POINT_LIGHTS 32
#define MAX_SPOT_LIGHTS 32

struct PointLight 
{
	vec3 position;
    float radius;
	vec3 color;
};

struct SpotLight 
{
	mat4 clipSpace;
    vec3 position;
	float frustumNear;
    vec3 direction;
	float frustumFar;
    vec3 color;
	float cone;
    vec3 radius;
};

layout (std140) uniform Lights
{
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];
    int numPointLights;
    int numSpotLights;
};

layout (std140) uniform SunSky
{
    mat4 sunClipSpace[4];
    vec4 sunFrustumNear;
    vec4 sunFrustumFar;
    vec3 sunDirection;
	float planetRadiusInUnits;
	vec3 whitePoint;
    float atmLengthUnitInMeters;
};