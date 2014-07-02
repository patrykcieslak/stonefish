uniform sampler2D texDiffuse;
uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform vec3 lightPosition;
uniform vec4 lightColor;

const vec3 eyeDir = vec3(0.0, 0.0, 1.0);

void unpackMaterialData(float data, out float diffuse, out float roughness, out float f0, out float reflection)
{
    //decode
    reflection = floor(data / (256.0 * 256.0 * 256.0));
    f0 = floor((data - reflection * 256.0 * 256.0 * 256.0) / (256.0 * 256.0));
    roughness = floor((data - reflection * 256.0 * 256.0 * 256.0 - f0 * 256.0 * 256.0) / 256.0);
    diffuse = floor(data - reflection * 256.0 * 256.0 * 256.0 - f0 * 256.0 * 256.0 - roughness * 256.0);
    
    //normalize
    reflection /= 256.0;
    f0 /= 256.0;
    roughness /= 256.0;
    diffuse /= 256.0;
}

void main(void)
{
    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    
    //Color texture
    vec3 color = texture2D(texDiffuse, gl_TexCoord[0].xy).rgb;

    //Position texture
    vec4 position_material = texture2D(texPosition, gl_TexCoord[0].xy);
    vec3 position = position_material.xyz;
    float diffuseReflectance;
    float roughness;
    float F0;
    float reflectionCoeff;
    unpackMaterialData(position_material.w, diffuseReflectance, roughness, F0, reflectionCoeff);
    
    //Normal texture
    vec4 normal_depth = texture2D(texNormal, gl_TexCoord[0].xy);
    vec3 normal = normalize(normal_depth.xyz);
    
    vec3 lightDir = lightPosition - position;
    float distance = length(lightDir);
    lightDir = lightDir/distance;

    float NdotL = dot(normal, lightDir);
    
    if(NdotL > 0.0)
    {
        //distance attenuation
        float attenuation = 1.0/(distance * distance);
        
        vec3 halfVector = normalize(lightDir + eyeDir);
        float NdotH = max(dot(normal, halfVector), 0.001);
        float NdotV = max(dot(normal, eyeDir), 0.001);
        float VdotH = max(dot(eyeDir, halfVector), 0.001);
        roughness = max(roughness, 0.01);
        
        //geometric attenuation
        float NH2 = 2.0 * NdotH;
        float g1 = (NH2 * NdotV) / VdotH;
        float g2 = (NH2 * NdotL) / VdotH;
        float geoAtt = min(1.0, min(g1, g2));
        
        //roughness (or: microfacet distribution function)
        //beckmann distribution function
        float mSquared = roughness * roughness;
        float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
        float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
        float roughness = r1 * exp(r2);
        
        /*
         //Pixar
         float alpha = acos(NdotH);
         roughness = 100.0 * exp(-(alpha * alpha)/mSquared);
         */
        
        //fresnel
        //Schlick approximation
        float fresnel = pow(1.0 - VdotH, 5.0);
        fresnel *= (1.0 - F0);
        fresnel += F0;
        
        float specular = (fresnel * geoAtt * roughness) / (NdotV * NdotL * 3.14159);
        finalColor.rgb = attenuation * lightColor.rgb * NdotL * (diffuseReflectance * color + specular * (1.0 - diffuseReflectance));
    }
    
    gl_FragColor = finalColor;
}
