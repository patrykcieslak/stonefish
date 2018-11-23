#version 330

in vec3 texcoord;
out vec4 fragcolor;
uniform samplerCube tex;
    
void main(void)
{
    vec4 color = texture(tex, texcoord);
    fragcolor = vec4(color.rgb, 1.0);
}
