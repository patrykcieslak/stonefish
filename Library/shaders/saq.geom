#version 330

layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;

in vec2 texcoord[];
out vec2 gTexcoord;

void main()
{
	for(int i = 0; i < 3; ++i)
	{
		gTexcoord = texcoord[i];
		gl_Layer = gl_PrimitiveIDIn;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
}
