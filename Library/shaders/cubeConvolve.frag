#version 120
/*
    :copyright: 2011 by Florian Boesch <pyalot@gmail.com>.
    :license: GNU AGPL3, see LICENSE for more details.
*/

uniform samplerCube source;
uniform vec2 viewport;
uniform mat4 inv_proj;
uniform mat3 inv_view_rot;
uniform float specularity;
    
const vec3 x = vec3(1.0, 0.0, 0.0);
const vec3 y = vec3(0.0, 1.0, 0.0);
const vec3 z = vec3(0.0, 0.0, 1.0);
    
const mat3 front = mat3(x, y, z);
const mat3 back = mat3(x, y, -z);
const mat3 right = mat3(z, y, x);
const mat3 left = mat3(z, y, -x);
const mat3 top = mat3(x, z, y);
const mat3 bottom = mat3(x, z, -y);

const float size = 16.0;
const float start = ((0.5/size)-0.5)*2.0;
const float end = -start;
const float incr = 2.0/size;
    
vec3 get_eye_normal()
{
    vec2 frag_coord = gl_FragCoord.xy/viewport;
    frag_coord = (frag_coord-0.5)*2.0;
    vec4 device_normal = vec4(frag_coord, 0.0, 1.0);
    vec3 eye_normal = normalize((inv_proj * device_normal).xyz);
    vec3 world_normal = normalize(inv_view_rot*eye_normal);
    return world_normal;
}

vec4 sample(mat3 side, vec3 eyedir, vec3 base_ray)
{
    vec3 ray = side*base_ray;
    float lambert = max(0.0, dot(ray, eyedir));
    float term = pow(lambert, specularity)*base_ray.z;
    return vec4(textureCube(source, ray).rgb*term, term);
}

void main()
{
    vec4 result = vec4(0.0);
    vec3 eyedir = get_eye_normal(), ray;

    for(float xi=start; xi<=end; xi+=incr)
    {
        for(float yi=start; yi<=end; yi+=incr)
        {
            ray = normalize((inv_proj * vec4(xi, yi, 0.0, 1.0)).xyz);
            result += sample(front, eyedir, ray);
            result += sample(back, eyedir, ray);
            result += sample(top, eyedir, ray);
            result += sample(bottom, eyedir, ray);
            result += sample(left, eyedir, ray);
            result += sample(right, eyedir, ray);
        }
    }
    result /= result.w;
    gl_FragColor = vec4(result.rgb, 1.0);
}
