#version 120
uniform sampler2D texDiffuse;
uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texSSAO;
uniform samplerCube texSkyDiff;
uniform samplerCube texSkyReflect;
uniform mat3 inv_view_rot;
uniform mat4 inv_proj;
uniform vec2 viewport;

vec3 get_world_normal()
{
    vec2 frag_coord = gl_FragCoord.xy/viewport;
    frag_coord = (frag_coord-0.5)*2.0;
    vec4 device_normal = vec4(frag_coord, 0.0, 1.0);
    vec3 eye_normal = normalize((inv_proj * device_normal).xyz);
    //vec3 world_normal = normalize(inv_view_rot*eye_normal);
    return eye_normal;//world_normal;
}

vec3 sky(samplerCube tex, vec3 srcNormal)
{
    vec3 worldNormal = normalize(inv_view_rot*srcNormal);
    return textureCube(tex, worldNormal).rgb;
}

void main()
{
    vec4 color_mat = texture2D(texDiffuse, gl_TexCoord[0].xy);
    vec4 position_mat = texture2D(texPosition, gl_TexCoord[0].xy);
    vec4 normal_depth = texture2D(texNormal, gl_TexCoord[0].xy);
    
    vec3 color = color_mat.rgb;
    vec3 normal = normal_depth.xyz;
    
    float occlusion = texture2D(texSSAO, gl_TexCoord[0].xy).r;
    float factor1 = color_mat.a;
    int mat_type = int(floor(position_mat.a/10.0));
    float factor2 = position_mat.a-float(mat_type)*10.0;
    vec3 diffuse_color = 1.5 * sky(texSkyDiff, normal);
    vec3 result = diffuse_color.rgb * color;
        
    /*if(mat_type == 1 && factor1 > 0.0)
    {
        vec3 eye_normal = get_world_normal();
        vec3 specular_normal = reflect(eye_normal, normal);
        float reflection = smoothstep(0.0, 1.0, pow(dot(-eye_normal, normal), (1.0-factor1)*(1.0-factor1)*10.0));
        vec3 specular_color = sky(texSkyReflect, specular_normal) * reflection;
        result += specular_color.rgb * color;
    }*/
        
    if(occlusion > 0.0)
        result *= occlusion;
        
    gl_FragColor = vec4(result, 1.0); //vec4(pow(result, vec3(1.0/1.0)), 1.0);
}
