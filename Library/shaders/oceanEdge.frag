#version 330

in vec2 texCoord;
out vec4 fragColor;

void main(void)
{
    fragColor = vec4(0.0, 0.0, 0.0, smoothstep(0.8, 0.0, abs(texCoord.y)));
}
