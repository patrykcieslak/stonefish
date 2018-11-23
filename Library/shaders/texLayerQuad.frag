#version 330

in vec2 texcoord;
out vec4 fragColor;
uniform sampler2DArray tex;
uniform int layer;

void main(void) 
{
    fragColor = texture(tex, vec3(texcoord, float(layer)));
}
