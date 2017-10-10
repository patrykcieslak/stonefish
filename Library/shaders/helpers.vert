#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 vcolor;
uniform mat4 MVP;
uniform vec3 scale;
out vec4 color;

void main()
{
    color = vcolor;
	gl_Position = MVP * vec4(vertex * scale, 1.0);
}