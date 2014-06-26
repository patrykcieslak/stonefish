#version 120
uniform sampler2D texDiffuse;
uniform sampler2D texNormal;
uniform sampler2D texSSAO;
uniform samplerCube texSkyDiff;
uniform samplerCube texSkyReflect;
uniform mat3 inv_view_rot;
uniform mat4 inv_proj;
uniform vec2 viewport;

vec3 sky(samplerCube tex, vec3 srcNormal)
{
    vec3 worldNormal = normalize(inv_view_rot*srcNormal);
    return textureCube(tex, worldNormal).rgb;
}

void main()
{
    vec3 color = texture2D(texDiffuse, gl_TexCoord[0].xy).rgb;
    vec4 normal_depth = texture2D(texNormal, gl_TexCoord[0].xy);
    vec3 normal = normal_depth.xyz;
    
    float occlusion = texture2D(texSSAO, gl_TexCoord[0].xy).r;
    vec3 diffuse_color = sky(texSkyDiff, normal);
    vec3 result = diffuse_color.rgb * color * (1.0 - occlusion);
    
    gl_FragColor = vec4(result, 1.0);
}
