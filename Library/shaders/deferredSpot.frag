#version 120

#define SHADOWMAP_SIZE (2048.0)
#define NUM_SAMPLES (32)
#define INV_NUM_SAMPLES (1.0/32.0)
#define NUM_SPIRAL_TURNS (7)

uniform sampler2D texDiffuse;
uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2DShadow texShadow;
uniform vec3 lightPosition;
uniform vec3 lightDirection;
uniform float lightAngle;
uniform vec4 lightColor;
uniform mat4 lightClipSpace;

const vec3 eyeDir = vec3(0.0, 0.0, 1.0);

//Generate random number
float random(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_product) * 43758.5453);
}

//Generate sample location based on a spiral curve
vec2 tapLocation(int sampleNumber, float spinAngle, out float radius)
{
    float alpha = (float(sampleNumber) + 0.5) * INV_NUM_SAMPLES;
    float angle = alpha * (float(NUM_SPIRAL_TURNS) * 6.28) + spinAngle;
    
    radius = alpha;
    return vec2(cos(angle), sin(angle));
}

//Calculate shadow contribution
float calculateShadowCoef(vec4 eyePosition, float bias)
{
	//Transform this fragment's position from view space to scaled light clip space such that the xy coordinates are in [0;1]
	vec4 shadowCoord = lightClipSpace * eyePosition;
    float fragmentDepth = (shadowCoord.z + bias)/shadowCoord.w;
    
    float nominalRadius = SHADOWMAP_SIZE / 400.0;
    float rnd = random(eyePosition.xyz, 0) * 10.0;
    
    //Cheap shadow testing for all pixels (5 samples)
    int inShadow = 0;
    inShadow += int(1.0 - shadow2D(texShadow, vec3(shadowCoord.xy/shadowCoord.w, fragmentDepth)).r);
    
    for(int i = NUM_SAMPLES - 5; i < NUM_SAMPLES - 1; i++)
    {
        float radiusFactor;
        vec2 offset = tapLocation(i, rnd, radiusFactor);
        inShadow += int(1.0 - shadow2D(texShadow, vec3(shadowCoord.xy/shadowCoord.w + (offset * radiusFactor * nominalRadius)/SHADOWMAP_SIZE, fragmentDepth)).r);
    }
    
    if(inShadow == 0) //Fully in light
        return 1.0;
    else if(inShadow == 5) //Fully in shadow
        return 0.0;
    else //Precise sampling for the shadow border pixels
    {
        float lightness = 1.0 - inShadow * INV_NUM_SAMPLES;
        
        for(int i = 0; i < NUM_SAMPLES - 5; i++)
        {
            float radiusFactor;
            vec2 offset = tapLocation(i, rnd, radiusFactor);
            lightness -= INV_NUM_SAMPLES * (1.0 - shadow2D(texShadow, vec3(shadowCoord.xy/shadowCoord.w + (offset * radiusFactor * nominalRadius)/SHADOWMAP_SIZE, fragmentDepth)).r);
        }
        
        return lightness;
    }
}

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
    float spotEffect = dot(normalize(lightDirection), -lightDir);
    
    if(spotEffect > lightAngle)
    {
        float NdotL = dot(normal, lightDir);
        
        if(NdotL > 0.0)
        {
            float bias = 0.0005 * tan(acos(NdotL));
            bias = clamp(bias, 0, 0.005);
            float lightness = calculateShadowCoef(vec4(position, 1.0), bias);

            if(lightness > 0.0)
            {
                //attenuation with distance
                spotEffect = pow(spotEffect, 100.0);
                float attenuation = spotEffect/(distance + distance * distance);
                
                vec3 halfVector = normalize(lightDir + eyeDir);
                float NdotH = max(dot(normal, halfVector), 0.001);
                float NdotV = max(dot(normal, eyeDir), 0.001);
                float VdotH = max(dot(eyeDir, halfVector), 0.001);
                roughness = max(roughness, 0.01);
                
                // geometric attenuation
                float NH2 = 2.0 * NdotH;
                float g1 = (NH2 * NdotV) / VdotH;
                float g2 = (NH2 * NdotL) / VdotH;
                float geoAtt = min(1.0, min(g1, g2));
                
                // roughness (or: microfacet distribution function)
                // beckmann distribution function
                float mSquared = roughness * roughness;
                float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
                float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
                float roughness = r1 * exp(r2);
                
                /*
                 //Pixar
                 float alpha = acos(NdotH);
                 roughness = 100.0 * exp(-(alpha * alpha)/mSquared);
                 */
                
                // fresnel
                // Schlick approximation
                float fresnel = pow(1.0 - VdotH, 5.0);
                fresnel *= (1.0 - F0);
                fresnel += F0;
                
                float specular = (fresnel * geoAtt * roughness) / (NdotV * NdotL * 3.14159);
                finalColor.rgb = lightness * attenuation * lightColor.rgb * NdotL * (diffuseReflectance * color + specular * (1.0 - diffuseReflectance));
            }
        }
    }
    
    gl_FragColor = finalColor;
}
