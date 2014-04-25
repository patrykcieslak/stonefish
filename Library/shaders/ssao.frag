#version 120
/*
    :copyright: 2011 by Florian Boesch <pyalot@gmail.com>.
    :license: GNU AGPL3, see LICENSE for more details.
*/
#define sample_count 8
#define pattern_size 5

uniform sampler2D texNormal, texRandom;
uniform vec2 viewport;
uniform float radius, epsilon, full_occlusion_treshold, no_occlusion_treshold, occlusion_power, random_size;
uniform mat4 proj, inv_proj;
uniform mat3 inv_rot;

vec3 getSample(int i)
{
    vec2 mod_coord = mod(floor(gl_FragCoord.xy), pattern_size);
    float x = (float(i)+0.5)/float(sample_count);
    float y = ((mod_coord.x + mod_coord.y*pattern_size)+0.5)/(pattern_size * pattern_size);
    return (texture2D(texRandom, vec2(x, y)).xyz-0.5)*2.0;
}

vec3 getEyeNormal()
{
    vec2 frag_coord = gl_FragCoord.xy/viewport;
    frag_coord = (frag_coord-0.5)*2.0;
    vec4 device_normal = vec4(frag_coord, 0.0, 1.0);
    return normalize((inv_proj * device_normal).xyz);
}

float occlusionFunction(float dist)
{
    if(dist > epsilon)
    {
        if(dist < full_occlusion_treshold)
        {
            return 1.0;
        }
        else
        {
            float range = no_occlusion_treshold - full_occlusion_treshold;
            return max(1.0 - pow((dist - full_occlusion_treshold)/range, occlusion_power), 0.0);
        }
    }
    else
    {
        return 0.0;
    }
}

float testOcclusion(vec3 eye_normal, vec3 eye_pos, vec3 sample_offset)
{
    sample_offset *= inv_rot;
    sample_offset *= sign(dot(eye_normal, sample_offset));
    vec3 sample_pos = eye_pos + sample_offset;
    vec4 device = proj * vec4(sample_pos, 1.0);
    vec4 device_norm = device/device.w;
    vec2 screen_coord = (device_norm.xy+1.0)*0.5;

    vec4 sample_data = texture2D(texNormal, screen_coord);
    float sample_depth = sample_data.w;
    float dist = length(sample_pos) - sample_depth;
    return occlusionFunction(dist)*dot(normalize(sample_offset), eye_normal);
}

void main(void)
{
    vec3 eye_ray = getEyeNormal();
    vec4 eye_data = texture2D(texNormal, gl_TexCoord[0].xy);
    vec3 eye_normal = eye_data.xyz;
    float eye_depth = eye_data.w;
    vec3 eye_pos = eye_depth * eye_ray;
    float result = 0.0;
    
    for(int i=0; i<sample_count; i++)
    {
        vec3 sample_offset = getSample(i) * radius;
        result += testOcclusion(eye_normal, eye_pos, sample_offset);
    }
    result = result/float(sample_count);
    gl_FragColor = vec4(max(1.0 - 25.0*result, 0.01)); //vec4(pow(1.0-result, 5.0));
}

