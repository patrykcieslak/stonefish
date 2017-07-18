#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in float edge;
uniform mat4 MVP;
out float edgeDiv;

void main()
{
	edgeDiv = edge;
    gl_Position = MVP * vec4(vertex, 1.f);
}