#version 330 core
//430
in vec2 texcoord;
in vec2 blurtexcoord[4];
out vec4 fragcolor;
uniform sampler2D source;

void main()
{
    fragcolor = vec4(0.0);
    fragcolor += texture(source, blurtexcoord[0]) * 0.0702702703;
    fragcolor += texture(source, blurtexcoord[1]) * 0.3162162162;
    fragcolor += texture(source, texcoord) * 0.2270270270;
    fragcolor += texture(source, blurtexcoord[2]) * 0.3162162162;
    fragcolor += texture(source, blurtexcoord[3]) * 0.0702702703;
}
