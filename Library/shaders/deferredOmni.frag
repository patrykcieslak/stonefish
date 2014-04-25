uniform sampler2D texDiffuse;
uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform vec3 lightPosition;
uniform vec4 lightColor;

const vec3 eyeDir = vec3(0.0,0.0,1.0);

void main(void)
{
    vec4 finalColor = vec4(0.0,0.0,0.0,0.0);
    
    vec4 color_mat = texture2D(texDiffuse, gl_TexCoord[0].xy);
    vec3 color = color_mat.rgb;
    float factor1 = color_mat.a;
    
    vec4 position_mat = texture2D(texPosition, gl_TexCoord[0].xy);
    int mat_type = int(floor(position_mat.a/10.0));
    float factor2 = position_mat.a-float(mat_type)*10.0;
    vec3 position = position_mat.xyz;
    
    vec4 normal_depth = texture2D(texNormal, gl_TexCoord[0].xy);
    vec3 normal = normalize(normal_depth.xyz);
    
    vec3 lightDir = lightPosition - position;
    float distance = length(lightDir);
    lightDir = lightDir/distance;

    float attenuation = 1.0/(distance + distance*distance);
    float NdotL = dot(normal, lightDir);
    float NdotV = dot(normal, eyeDir);
    
    if(NdotL > 0.0)
    {
        if(mat_type == 0) //Oren-Nayar
        {
            float angleVN = acos(NdotV);
            float angleLN = acos(NdotL);
            
            float alpha = max(angleVN, angleLN);
            float beta = min(angleVN, angleLN);
            float gamma = dot(eyeDir - normal * NdotV, lightDir - normal * NdotL);
            factor1 *= 100.0;
            float roughnessSquared = factor1*factor1;
            float A = 1.0 - 0.5 * (roughnessSquared / (roughnessSquared + 0.57));
            float B = 0.45 * (roughnessSquared / (roughnessSquared + 0.09));
            float C = sin(alpha) * tan(beta);
            float L1 = max(0.0, NdotL) * (A + B * max(0.0, gamma) * C);
            
            finalColor = vec4(lightColor.rgb * color * L1 * attenuation, 1.0);
        }
        else if(mat_type == 1) //Blinn-Phong
        {
            vec3 H = normalize(lightDir + eyeDir);
            float specular = pow(max(dot(normal, H), 0.0), factor1 * factor1 * 512.0 + 8.0);
            
            finalColor = vec4(lightColor.rgb * color * NdotL * attenuation
                              + lightColor.rgb * specular * attenuation, 1.0);
        }
        else if(mat_type == 2) //Reflective
        {
            finalColor = vec4(0.0,0.0,0.0,1.0);
        }
    }
    
    //Rim lighting
    if(mat_type == 1 && factor2 > 0.0)
    {
        float rim = 0.90 - NdotV;
        rim = smoothstep(0.0, 1.0, rim);
        rim = pow(rim, (1.0 - factor2) * 9.0 + 1.0);
        finalColor += vec4(lightColor.rgb * color *  attenuation * rim, 1.0);
    }

    gl_FragColor = finalColor;
}
