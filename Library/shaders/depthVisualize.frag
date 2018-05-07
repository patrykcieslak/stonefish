#version 330 core

in vec2 texcoord;
out vec4 fragcolor;
uniform sampler2D texLinearDepth;
uniform vec2 range;

void main()
{
    fragcolor = (texture2D(texLinearDepth, texcoord).rrrr-range.xxxx)/(range.y-range.x);
}