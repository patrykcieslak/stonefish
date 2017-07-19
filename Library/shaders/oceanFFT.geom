#version 430 core

layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;

uniform int nLayers;
uniform int sLayer;

in vec2 uvIn[];
out vec2 uv;

void main() 
{
	for (int i = sLayer; i < nLayers + sLayer; ++i)
    {
        gl_Layer = i;
        gl_PrimitiveID = i;
        gl_Position = gl_in[0].gl_Position;
        uv = uvIn[0];
        EmitVertex();
        gl_Position = gl_in[1].gl_Position;
        uv = uvIn[1];
        EmitVertex();
        gl_Position = gl_in[2].gl_Position;
        uv = uvIn[2];
        EmitVertex();
        EndPrimitive();
    }
}