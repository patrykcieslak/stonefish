#version 120
uniform sampler2D texDiffuse;
uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texShadow;
uniform vec3 lightPosition;
uniform vec3 lightDirection;
uniform float lightAngle;
uniform vec4 lightColor;
uniform mat4 lightClipSpace;

const vec3 eyeDir = vec3(0.0,0.0,-1.0);

float shadowCoeff(vec4 eyePosition, float bias)
{
    vec4 shadowCoord = lightClipSpace * eyePosition;
    float shadowDepth = texture2D(texShadow, shadowCoord.xy/shadowCoord.w).x;
	
    float visibility = 1.0;
    if(shadowDepth < (shadowCoord.z-bias)/shadowCoord.w)
        visibility = 0.0;
    
	return visibility;
}

void main(void)
{
    vec4 color_mat = texture2D(texDiffuse, gl_TexCoord[0].xy);
    vec4 position_mat = texture2D(texPosition, gl_TexCoord[0].xy);
    vec4 normal_depth = texture2D(texNormal, gl_TexCoord[0].xy);
 
    vec3 color = color_mat.rgb;
    float factor1 = color_mat.a;
    int mat_type = int(floor(position_mat.a/10.0));
    float factor2 = position_mat.a-float(mat_type)*10.0;
    vec3 position = position_mat.xyz;
    vec3 normal = normalize(normal_depth.xyz);

    vec4 finalColor = vec4(0.0,0.0,0.0,0.0);
    vec3 lightDir = lightPosition - position;
    float distance = length(lightDir);
    lightDir = lightDir/distance;
    float spotEffect = dot(normalize(lightDirection), -lightDir);
    
    if(spotEffect > lightAngle)
    {
        float NdotL = max(dot(normal, lightDir), 0.0);
        
        if(NdotL > 0.0)
        {
            spotEffect = pow(spotEffect, 100.0);
            float attenuation = spotEffect/(distance + distance*distance);
            float NdotV = dot(normal, eyeDir);
            
            float bias = 0.0025 * tan(acos(NdotL));
            bias = clamp(bias, 0, 0.01);
            
            float shadow = shadowCoeff(vec4(position, 1.0), bias);
            
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
                
                finalColor = vec4(lightColor.rgb * color * L1 * attenuation * shadow, 1.0);
            }
            else if(mat_type == 1) //Phong
            {
                vec3 R = reflect(lightDir, normal);
                float specular = pow(max(dot(R, eyeDir), 0.0), factor1*10.0+1.0);
                
                finalColor = vec4(lightColor.rgb * color * NdotL * attenuation * shadow + lightColor.rgb * specular * attenuation * shadow, 1.0);
            }
            else if(mat_type == 2) //Metallic
            {
                
                
                
                
                
                finalColor = vec4(0.0,0.0,0.0,1.0);
            }
        }
    }
    
    gl_FragColor = finalColor;
}
