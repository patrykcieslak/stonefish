#version 330 core

layout(location = 0) in vec4 vertex;
out vec4 fragPos;
uniform mat4 MVP;

void main()
{
	fragPos = vertex;
	gl_Position = MVP * vertex;
}
