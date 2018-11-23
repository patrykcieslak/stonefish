#version 330

in vec2 texcoord;
out vec4 fragColor;
uniform sampler2D tex;
uniform vec4 color;

void main(void) 
{
    fragColor =  texture(tex, texcoord) * color;
}
