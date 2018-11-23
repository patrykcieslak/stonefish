#version 330

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec3 uv;
out vec3 texcoord;

void main(void)
{
	texcoord = uv;
	gl_Position = vec4(vertex, 0.0, 1.0);
}
