#version 330

in vec2 texcoord;
out vec4 fragColor;
uniform sampler2DMS tex;
uniform ivec2 texSize;

void main(void) 
{
	ivec2 uv = ivec2(texSize * texcoord);
    fragColor =  texelFetch(tex, uv, 0);
}
