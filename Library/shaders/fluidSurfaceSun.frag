#version 120

uniform sampler2D texWaveNormal;
uniform vec3 eyeSurfaceNormal;
uniform vec3 lightDirection;
uniform vec4 lightColor;
uniform float time;
uniform vec2 viewport;

const vec3 eyeDir = vec3(0.0,0.0,1.0);

vec3 getFluidNormal(vec2 coord)
{
    vec3 waveNormal = texture2D(texWaveNormal, 10.0 * coord + 0.05 * time).rgb;
    waveNormal.z *= sin(time) * sin(time * 0.3);
    vec3 normal = 2.0 * waveNormal - vec3(1.0);
    normal = normalize(reflect(eyeSurfaceNormal, normal));
    return normal;
}

void main(void)
{
    vec2 texCoord = gl_FragCoord.xy/viewport;
    
    vec3 normal = getFluidNormal(texCoord);
    
    vec3 lightDir = -lightDirection;
    vec3 H = normalize(lightDir + eyeDir);
    float specular = pow(max(dot(normal, H), 0.0), 10.0);
    
    gl_FragColor = vec4(lightColor.rgb * specular * 0.1, 1.0);
}