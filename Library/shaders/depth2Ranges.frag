#version 330

in vec2 texcoord;
out float range;

uniform vec4 clipInfo;
uniform vec4 projInfo;
uniform vec2 rangeInfo;
uniform sampler2D texDepth;

//Camera space Z (linear depth)
float linearizeDepth(float depth)
{
    if(clipInfo[3] != 0)
        return (clipInfo[0]/(clipInfo[1] * depth + clipInfo[2]));
    else
        return (clipInfo[1]+clipInfo[2] - depth * clipInfo[1]);
}

//Position from UV and linear depth
vec3 viewPositionFromDepth(vec2 uv, float linearDepth)
{
    return vec3((uv * projInfo.xy + projInfo.zw) * linearDepth, linearDepth);
}

void main()
{
    vec3 position = viewPositionFromDepth(texcoord, linearizeDepth(texture(texDepth, texcoord).r));
    range = clamp(length(position), rangeInfo.x, rangeInfo.y);
}
