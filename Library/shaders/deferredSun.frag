#version 120
#extension GL_EXT_texture_array : enable

uniform sampler2D texDiffuse;
uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2DArrayShadow texShadowArray;
uniform vec3 lightDirection;
uniform vec4 lightColor;
uniform vec4 frustumFar;
uniform mat4 lightClipSpace[4];

const vec3 eyeDir = vec3(0.0,0.0,1.0);
const vec2 poissonDisk[16] = vec2[]( vec2( -0.94201624, -0.39906216 ),
                                     vec2( 0.94558609, -0.76890725 ),
                                     vec2( -0.094184101, -0.92938870 ),
                                     vec2( 0.34495938, 0.29387760 ),
                                     vec2( -0.91588581, 0.45771432 ),
                                     vec2( -0.81544232, -0.87912464 ),
                                     vec2( -0.38277543, 0.27676845 ),
                                     vec2( 0.97484398, 0.75648379 ),
                                     vec2( 0.44323325, -0.97511554 ),
                                     vec2( 0.53742981, -0.47373420 ),
                                     vec2( -0.26496911, -0.41893023 ),
                                     vec2( 0.79197514, 0.19090188 ),
                                     vec2( -0.24188840, 0.99706507 ),
                                     vec2( -0.81409955, 0.91437590 ),
                                     vec2( 0.19984126, 0.78641367 ),
                                     vec2( 0.14383161, -0.14100790 ) );

float random(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float calculateShadowCoef(float depth, vec4 eyePosition, float bias)
{
	//Find the appropriate depth map to look up in based on the depth of this fragment
    int index = 3;
	
	if(depth < frustumFar.x)
		index = 0;
	else if(depth < frustumFar.y)
		index = 1;
	else if(depth < frustumFar.z)
		index = 2;
    
	//Transform this fragment's position from view space to scaled light clip space such that the xy coordinates are in [0;1]
	//Note: there is no need to divide by w for othogonal light sources
    vec4 shadowCoord =  lightClipSpace[index] * eyePosition;
    float fragmentDepth = shadowCoord.z - bias;

    //Compare
    float visibility = 1.0;
    for(int i=0; i<4; i++)
    {
        int pdIndex = int(mod(random(floor(eyePosition.xyz * 1000.0), i) * 16.0, 16.0));
        //float shadowDepth = texture2DArray(texShadowArray, vec3(shadowCoord.xy + poissonDisk[pdIndex]/(750.0 + 50.0 * depth), float(index))).x;
        //if(shadowDepth < fragmentDepth)
        //    visibility -= 0.25;
        visibility -= 0.25 * (1.0 - shadow2DArray(texShadowArray, vec4(shadowCoord.xy + poissonDisk[pdIndex]/(750.0 + 50.0 * depth), float(index), fragmentDepth)).x);
    }
    return visibility;
}

void main(void)
{
    vec4 finalColor = vec4(0.0,0.0,0.0,0.0);
    
    vec4 color_mat = texture2D(texDiffuse, gl_TexCoord[0].xy);
    vec3 color = color_mat.rgb;
    float factor1 = color_mat.a;
    
    vec4 position_mat = texture2D(texPosition, gl_TexCoord[0].xy);
    int mat_type = int(floor(position_mat.a/10.0));
    float factor2 = position_mat.a - float(mat_type) * 10.0;
    
    vec4 normal_depth = texture2D(texNormal, gl_TexCoord[0].xy);
    vec3 normal = normalize(normal_depth.xyz);
    float depth = normal_depth.w;
    
    vec3 lightDir = -lightDirection;
    float NdotL = dot(normal, lightDir);
    float NdotV = dot(normal, eyeDir);
    
    if(NdotL > 0.0)
    {
        //Calculate shadow contribution
        vec4 position = vec4(position_mat.xyz,1.0);
        float bias = 0.005 * tan(acos(NdotL));
        bias = clamp(bias, 0, 0.01);
        float shadow = calculateShadowCoef(depth, position, bias);
        
        //Use shading according to material type
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
            
            finalColor = vec4(lightColor.rgb * color * L1 * shadow, 1.0);
        }
        else if(mat_type == 1) //Blinn-Phong
        {
            vec3 H = normalize(lightDir + eyeDir);
            float specular = pow(max(dot(normal, H), 0.0), factor1 * factor1 * 512.0 + 8.0);
            
            finalColor = vec4(lightColor.rgb * color * NdotL * shadow
                              + lightColor.rgb * specular * shadow, 1.0);
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
        finalColor += vec4(lightColor.rgb * color * rim, 1.0);
    }
    
    gl_FragColor = finalColor;
}
