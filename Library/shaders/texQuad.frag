#version 330 core

in vec2 texcoord;
out vec4 fragcolor;
uniform sampler2D tex;
uniform vec4 color;

void main(void) 
{
    fragcolor =  texture2D(tex, texcoord) * color;
}