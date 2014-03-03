#version 120
uniform sampler2D texPosition;
uniform sampler2D texScene;
uniform sampler2D texReflection;
uniform float R0;
uniform vec2 viewport;
uniform mat4 invProj;
uniform float visibility;
uniform vec3 eyeSurfaceNormal;
uniform vec3 eyeSurfacePosition;

varying vec3 fluidPosition;
//varying vec3 fluidNormal;

const float fadeSpeed = 0.15;
const vec3 depthColor = vec3(0.0078, 0.5176, 0.7);
const vec3 bigDepthColor = vec3(0.0039, 0.00196, 0.145);
const vec3 extinction = vec3(7.0, 30.0, 40.0);

vec3 getEyeNormal(vec2 texCoord)
{
    vec2 fragCoord = (texCoord-0.5)*2.0;
    vec4 deviceNormal = vec4(fragCoord, 0.0, 1.0);
    vec3 eyeNormal = normalize((invProj * deviceNormal).xyz);
    return eyeNormal;
}

void main(void)
{
    vec2 texCoord = gl_FragCoord.xy/viewport;
    vec4 position_mat = texture2D(texPosition, texCoord);
    vec3 position = position_mat.xyz;
    
    if(fluidPosition.z < position.z) //depth testing
        discard;
    
    vec3 eyeNormal = getEyeNormal(texCoord);
    vec3 reflection = texture2D(texReflection, texCoord).rgb;
    vec3 refraction = texture2D(texScene, texCoord).rgb;
    float waterDepth = dot(eyeSurfacePosition-position, eyeSurfaceNormal)/100.0;
    float eyeWaterDepth = eyeSurfacePosition.z - position.z;
    float depthN = eyeWaterDepth/100.0 * fadeSpeed;
            
    refraction = mix(mix(refraction, depthColor, clamp(depthN/visibility, 0.0, 1.0)),
                    bigDepthColor,
                    clamp(waterDepth/extinction, 0.0, 1.0));

    float fresnel = R0 + (1.0-R0)*pow(1.0-dot(-eyeNormal, eyeSurfaceNormal), 5.0);
    gl_FragColor = vec4(mix(refraction, reflection, fresnel), 1.0);
}