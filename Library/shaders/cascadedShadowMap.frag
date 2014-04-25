#version 120
#extension GL_EXT_texture_array : enable

uniform sampler2DArray shadowmapArray;
uniform float shadowmapLayer;

void main()
{
    vec3 texCoord = vec3(gl_TexCoord[0].xy, shadowmapLayer);
    gl_FragColor = texture2DArray(shadowmapArray, texCoord);
}