#version 330 core
/*
    :copyright: 2011 by Florian Boesch <pyalot@gmail.com>.
    :license: GNU AGPL3, see LICENSE for more details.
*/

in vec2 texcoord;
out vec4 fragcolor;

uniform vec2 viewport;
uniform mat4 inv_proj;
uniform mat3 inv_view_rot;
uniform samplerCube source;
    
vec3 get_world_normal()
{
    vec2 frag_coord =  gl_FragCoord.xy/viewport;
    frag_coord = (frag_coord-0.5)*2.0;
    vec4 device_normal = vec4(frag_coord, 0.0, 1.0);
    vec3 eye_normal = normalize((inv_proj * device_normal).xyz);
    vec3 world_normal = normalize(inv_view_rot * eye_normal);
    return world_normal;
}

void main(void)
{
    vec3 normal = get_world_normal();
    vec4 color = texture(source, normal);
    fragcolor = vec4(color.rgb, 1.0);
}
