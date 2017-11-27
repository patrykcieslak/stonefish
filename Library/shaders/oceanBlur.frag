#version 330 core
in vec2 texcoord;
out vec4 fragColor;
uniform sampler2DArray texScene;
uniform sampler2D texLinearDepth;

void main(void) 
{
	float depth = texture(texLinearDepth, texcoord).r;
	vec3 blur = texture(texScene, vec3(texcoord, int(clamp(depth/5.0,1.0,5.0)))).rgb;
	fragColor = vec4(blur, clamp(depth/5.0, 0.0, 1.0));
}