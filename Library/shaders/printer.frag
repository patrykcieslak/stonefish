#version 330

in vec2 texcoord;
out vec4 fragColor;

uniform sampler2D tex;
uniform vec4 color;

void main() 
{
    fragColor = vec4(1.0, 1.0, 1.0, texture(tex, texcoord).r) * color;
}
