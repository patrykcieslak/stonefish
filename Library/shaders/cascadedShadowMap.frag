#version 330 core
#extension GL_EXT_texture_array : enable

in vec2 texcoord;
out vec4 fragcolor;

uniform sampler2DArray shadowmapArray;
uniform float shadowmapLayer;

void main()
{
    vec3 texcoord3D = vec3(texcoord, shadowmapLayer);
    fragcolor = texture2DArray(shadowmapArray, texcoord3D);
}