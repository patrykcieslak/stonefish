#version 120
uniform sampler2D texPosition;
uniform sampler2D texScene;
uniform vec2 viewport;
uniform float visibility;
uniform vec3 eyeSurfaceNormal;
uniform vec3 eyeSurfacePosition;

varying vec3 fluidPosition;
//varying vec3 fluidNormal;

const float fadeSpeed = 0.15;
const vec3 depthColor = vec3(0.0078, 0.5176, 0.7);
const vec3 bigDepthColor = vec3(0.0039, 0.00196, 0.145);
const vec3 extinction = vec3(7.0, 30.0, 40.0);

void main(void)
{
    vec2 texCoord = gl_FragCoord.xy/viewport;
    vec4 position_mat = texture2D(texPosition, texCoord);
    vec3 position = position_mat.xyz;
    
    if(length(position_mat) == 0.0)
        position = fluidPosition;
    
    vec3 refraction = texture2D(texScene, texCoord).rgb;
    float waterDepth = dot(eyeSurfacePosition-position, eyeSurfaceNormal)/100.0;
    float eyeWaterDepth = clamp(-position.z, 0.0, -fluidPosition.z);
    float depthN = eyeWaterDepth/100.0 * fadeSpeed;
    
    refraction = mix(mix(refraction, depthColor, clamp(depthN/visibility, 0.0, 1.0)),
                     bigDepthColor,
                     clamp(waterDepth/extinction, 0.0, 1.0));
    
    gl_FragColor =  vec4(refraction, 1.0);
}