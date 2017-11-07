#version 430 core

out vec3 fragColor;
in vec4 fragPos;

uniform sampler2D texReflection; 
uniform vec2 viewport;
uniform vec3 eyePos;
uniform float R0;
uniform float time;
uniform vec3 sunDirection;
uniform float planetRadius;
uniform vec3 whitePoint;
uniform float cosSunSize;
uniform vec3  lightAbsorption;// = vec3(0.3,0.08,0.07);

const float w2a = 1.33/1.0;

//Atmosphere
vec3 GetSolarLuminance();
vec3 GetSkyLuminance(vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSkyLuminanceToPoint(vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance);
vec3 GetSunAndSkyIlluminance(vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance);

float random (in vec2 st) 
{ 
    return fract(sin(dot(st, vec2(12.9898,78.233)))* 43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) 
{
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
	
    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

#define NUM_OCTAVES 8

float fbm(in vec2 st) 
{
    float v = 0.0;
    float a = 0.5;
    vec2 shift = vec2(100.0);
    
	//Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
	
	for(int i = 0; i < NUM_OCTAVES; ++i) 
	{
        v += a * noise(st);
        st = rot * st * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}

float fresnel(float cosTheta)
{
	return R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
	vec3 pos = fragPos.xyz/fragPos.w;
	vec3 toEye = normalize(eyePos - pos);
	vec3 center = vec3(0, 0, -planetRadius);
	vec3 P = -center;
	vec3 normal = vec3(0,0,-1.0);
	float distance = length(eyePos - pos);
	
	vec2 st = pos.xy * 5.0;
	vec2 q = vec2(0.);
	q.x = fbm(st);
	q.y = fbm(st + vec2(1.0));
	
	vec2 r = vec2(0.);
	r.x = fbm(st + q + vec2(1.7,9.2)+ 5.1 * time);
    r.y = fbm(st + q + vec2(5.3,2.8)+ 2.3 * time);
	
	float f = fbm(st+r);

    vec2 dn = mix( vec2(-0.34,0.54), vec2(0.67,0.14), clamp((f*f)*4.0,0.0,1.0) );
	dn *= (f*f*f+0.6*f*f+0.5*f);
	
	normal.xy -= dn*0.1;
	normal = normalize(normal);
	
	//Sky refraction
	vec3 Lsky = vec3(0.);
	vec3 ray = refract(-toEye, normal, w2a);
	if(ray.z > 0.0)
	{
		vec3 trans;
		Lsky = GetSkyLuminance(P, ray, 0.0, sunDirection, trans);
		Lsky += smoothstep(cosSunSize*0.99999, cosSunSize, dot(ray, sunDirection)) * trans * GetSolarLuminance()/1000.0;
	}
	
	//Reflected objects
	vec4 reflectedColor = texture(texReflection, vec2(gl_FragCoord.x/viewport.x, gl_FragCoord.y/viewport.y) + dn*0.01);
	
	//Reflection/Refraction ratio
	float fresn = fresnel(dot(toEye,normal));

	//Final color
	fragColor = fresn * reflectedColor.rgb + (1.0-fresn) * Lsky/whitePoint/10000.0;
	
	//Absorption
	fragColor *= exp(-lightAbsorption*distance);
	
	//Inscatter
	vec3 Isky;
	vec3 Isun = GetSunAndSkyIlluminance(P, vec3(0,0,1.0), sunDirection, Isky);
    //distance *= 10.0;
	fragColor += Isky/whitePoint/1000000.0 * exp(-lightAbsorption * -eyePos.z) * ( exp((-toEye.z - 1.0)*lightAbsorption*distance)-1.0 )/( (-toEye.z - 1.0)*lightAbsorption );
}