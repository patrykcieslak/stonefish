#version 330

in vec2 texCoord;
in vec2 backTexCoord;
out vec4 fragcolor;
uniform vec4 color;
uniform sampler2D tex;
uniform sampler2D backTex;

void main()
{
	vec4 background = texture(backTex, backTexCoord);
	vec4 surface = texture(tex, texCoord);
	fragcolor =  surface * background * color;
}
