#version 120
#extension GL_EXT_texture_array : enable

uniform sampler2D texDiffuse;
uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2DArray texShadowArray;
uniform vec3 lightDirection;
uniform vec4 lightColor;
uniform vec4 frustumFar;
uniform mat4 lightClipSpace[4];

const vec3 eyeDir = vec3(0.0,0.0,-1.0);

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
    vec4 shadowCoord =  lightClipSpace[index] * eyePosition; //lightClipSpace[index] * eyePosition;
    float fragmentDepth = (shadowCoord.z - bias)/shadowCoord.w;
	float shadowDepth = texture2DArray(texShadowArray, vec3(shadowCoord.xy, float(index))).x;
	
    //Compare
    float visibility = 1.0;
    if(shadowDepth < fragmentDepth)
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
    vec3 normal = normalize(normal_depth.xyz);
    
    vec4 finalColor = vec4(0.0,0.0,0.0,0.0);
    vec3 lightDir = -lightDirection;
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    if(NdotL > 0.0)
    {
        float bias = 0.0025 * tan(acos(NdotL));
        bias = clamp(bias, 0, 0.01);
        
        vec4 position = vec4(position_mat.xyz,1.0);
        
        float shadowCoef = calculateShadowCoef(normal_depth.w, position, bias);
        
        finalColor = vec4(lightColor.rgb * color * NdotL * shadowCoef, 1.0);
    }
    
    gl_FragColor = finalColor;
}
