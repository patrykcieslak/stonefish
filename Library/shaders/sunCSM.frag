#version 330

in vec2 texcoord;
out vec4 fragcolor;

uniform sampler2DArray shadowmapArray;
uniform float shadowmapLayer;

void main()
{
    fragcolor = texture(shadowmapArray, vec3(texcoord, shadowmapLayer));
}
