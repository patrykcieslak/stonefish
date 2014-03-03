uniform sampler2D texDiffuse;
uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform vec3 lightPosition;
uniform vec3 lightDirection;
uniform vec4 lightColor;

void main(void)
{
    vec4 normal_light = texture2D(texNormal, gl_TexCoord[0].xy);
    vec4 finalColor = vec4(0.0,0.0,0.0,1.0);
    vec3 cameraDirection = vec3(0.0,0.0,-1.0);
    
    if(normal_light.a > 0.0)
    {
        vec4 color_sh = texture2D(texDiffuse, gl_TexCoord[0].xy);
        vec4 color = vec4(color_sh.rgb, 1.0);
        float shininess = color_sh.a;
        vec4 position = texture2D(texPosition, gl_TexCoord[0].xy);
        vec3 normal = normal_light.xyz;
        
        vec3 lightDir = normalize(-lightDirection);
        float lambertTerm = dot(normal, lightDir);
        
        if(lambertTerm > 0.0)
        {
            finalColor += lightColor * color * lambertTerm;
            vec3 ref = reflect(-lightDir, normal);
            float specular = pow(max(dot(ref, cameraDirection), 0.0), shininess*99.0+1.0);
            finalColor += lightColor * specular;
        }
    }
    
    gl_FragColor = finalColor;
    
}
