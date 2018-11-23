#version 330

in vec2 texcoord;
out vec4 fragColor;
uniform sampler3D tex;
uniform int level;

void main(void) 
{
    fragColor = texture(tex, vec3(texcoord, float(level)));
}
