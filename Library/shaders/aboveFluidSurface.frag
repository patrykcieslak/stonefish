#version 120
uniform sampler2D texPosition;
uniform sampler2D texReflection;
uniform sampler2D texRefraction;
uniform sampler2D texWaveNormal;
uniform float R0;
uniform vec2 viewport;
uniform mat4 invProj;
uniform float visibility;
uniform vec3 eyeSurfaceNormal;
uniform vec3 eyeSurfacePosition;
uniform float time;

const float fadeSpeed = 0.15;
const vec3 depthColor = vec3(0.0078, 0.5176, 0.7)/10.0;
const vec3 bigDepthColor = vec3(0.0039, 0.00196, 0.145);
const vec3 extinction = vec3(7.0, 30.0, 40.0);

vec3 getEyeNormal(vec2 texCoord)
{
    vec2 fragCoord = (texCoord-0.5)*2.0;
    vec4 deviceNormal = vec4(fragCoord, 0.0, 1.0);
    vec3 eyeNormal = normalize((invProj * deviceNormal).xyz);
    return eyeNormal;
}

vec3 getFluidNormal(vec2 coord)
{
    vec3 waveNormal = vec3(0.0,0.0,-1.0);//texture2D(texWaveNormal, 10.0 * coord + 0.05 * time).rgb;
    //waveNormal.z *= sin(time) * sin(time * 0.3);
    vec3 normal = 2.0 * waveNormal - vec3(1.0);
    normal = normalize(reflect(eyeSurfaceNormal, normal));
    return normal;
}

void main(void)
{
    vec2 texCoord = gl_FragCoord.xy/viewport;
    vec4 position_mat = texture2D(texPosition, texCoord);
    vec3 position = position_mat.xyz;
    
    vec3 eyeNormal = getEyeNormal(texCoord);
    vec3 modEyeSurfaceNormal = getFluidNormal(texCoord);

    //vec3 reflection = texture2D(texReflection, vec2(texCoord.x, 1.0-texCoord.y) + modEyeSurfaceNormal.xy * 0.01).rgb;
    //vec3 refraction = texture2D(texRefraction, texCoord + modEyeSurfaceNormal.xy * 0.01).rgb;
    vec3 reflection = texture2D(texReflection, vec2(texCoord.x, 1.0-texCoord.y)).rgb;
    vec3 refraction = texture2D(texRefraction, texCoord).rgb;
    
    vec4 plane = vec4(eyeSurfaceNormal,-dot(eyeSurfaceNormal, eyeSurfacePosition));
    
    float t = 0.0;
    float den = dot(plane.xyz, eyeNormal);
    if(den != 0.0)
        t = -plane.w/den;
        
    float waterDepth = dot(eyeSurfacePosition-position, eyeSurfaceNormal);
    float eyeWaterDepth = (eyeNormal*t).z - position.z;
    
    if(eyeWaterDepth < 0.0)
        refraction = depthColor;
    else
    {
        eyeWaterDepth = eyeWaterDepth+0.3;
        refraction = mix(refraction, depthColor, clamp(eyeWaterDepth/extinction, 0.0, 1.0));
    }
    
    //    refraction = mix(mix(refraction, depthColor, clamp(eyeWaterDepth*eyeWaterDepth/10.0, 0.0, 1.0)),
    //                     bigDepthColor,
    //                     clamp(waterDepth/5.0, 0.0, 1.0));
    
    float fresnel = R0 + (1.0-R0)*pow(1.0-dot(-eyeNormal, eyeSurfaceNormal), 5.0);
    
    //gl_FragColor = vec4(refraction, 1.0);
    gl_FragColor = vec4(mix(refraction, reflection, fresnel), 1.0);
}
