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

const vec3 eyeDir = vec3(0.0,0.0,1.0);

//Generate random number
float random(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
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
    
    float nominalRadius = SHADOWMAP_SIZE/400.0;
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
    float spotEffect = dot(normalize(lightDirection), -lightDir);
    
    if(spotEffect > lightAngle)
    {
        spotEffect = pow(spotEffect, 100.0);
        float attenuation = spotEffect/(distance + distance*distance);
        float NdotL = dot(normal, lightDir);
        float NdotV = dot(normal, eyeDir);
        
        if(NdotL > 0.0)
        {
            float bias = 0.0005 * tan(acos(NdotL));
            bias = clamp(bias, 0, 0.005);
            float lightness = calculateShadowCoef(vec4(position,1.0), bias);

            if(lightness > 0.0)
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
                    
                    finalColor = vec4(lightColor.rgb * color * L1 * attenuation * lightness, 1.0);
                }
                else if(mat_type == 1) //Blinn-Phong
                {
                    vec3 H = normalize(lightDir + eyeDir);
                    float specular = pow(max(dot(normal, H), 0.0), factor1 * factor1 * 512.0 + 8.0);
                    
                    finalColor = vec4(lightColor.rgb * color * NdotL * attenuation * lightness
                                      + lightColor.rgb * specular * attenuation * lightness, 1.0);
                }
                else if(mat_type == 2) //Reflective
                {
                    finalColor = vec4(0.0,0.0,0.0,1.0);
                }
            }
        }
        
        //Rim lighting
        if(mat_type == 1 && factor2 > 0.0)
        {
            float rim = 0.90 - NdotV;
            rim = smoothstep(0.0, 1.0, rim);
            rim = pow(rim, (1.0 - factor2) * 9.0 + 1.0);
            finalColor += vec4(lightColor.rgb * color * attenuation * rim, 1.0);
        }
    }
    
    gl_FragColor = finalColor;
}
