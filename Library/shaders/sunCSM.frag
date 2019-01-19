#version 330

in vec2 texcoord;
out vec4 fragcolor;

uniform sampler2DArray shadowmapArray;
uniform float shadowmapLayer;

void main()
{
    float depth = texture(shadowmapArray, vec3(texcoord, shadowmapLayer)).r;
    fragcolor = vec4(vec3(depth), 1.0);
}
