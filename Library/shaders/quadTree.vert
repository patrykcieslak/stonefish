#version 430 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in float edge;
out float edgeDiv;

void main()
{
	edgeDiv = edge;
    gl_Position = vec4(vertex, 1.f);
}