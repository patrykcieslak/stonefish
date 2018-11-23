#version 330

layout(location = 0) in vec2 vertex;

void main(void)
{
	gl_Position = vec4(vertex.xy, 0, 1.0);
}
