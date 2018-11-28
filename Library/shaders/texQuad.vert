#version 330

layout(location = 0) in vec4 vertex;
out vec2 texcoord;
uniform vec4 rect;

void main(void)
{
	texcoord = vertex.zw;
	gl_Position = vec4( (vertex.xy + vec2(1.0))*rect.zw - vec2(1.0) + 2.0*rect.xy,    0, 1.0);
}
