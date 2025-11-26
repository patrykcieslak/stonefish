#version 330

layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;

uniform int nLayers;

in vec2 texcoord[];
out vec2 gTexcoord;

void main()
{
	for(int i = 0; i < nLayers; ++i)
	{
		gl_Layer = i;
        gl_PrimitiveID = i;

		gTexcoord = texcoord[0];
        gl_Position = gl_in[0].gl_Position;
		EmitVertex();
        gTexcoord = texcoord[1];
        gl_Position = gl_in[1].gl_Position;
		EmitVertex();
        gTexcoord = texcoord[2];
        gl_Position = gl_in[2].gl_Position;
		EmitVertex();
	}
}
