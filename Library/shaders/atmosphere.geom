#version 430 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
uniform int layer;
    
void main() 
{
	gl_Position = gl_in[0].gl_Position;
	gl_Layer = layer;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	gl_Layer = layer;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	gl_Layer = layer;
	gl_PrimitiveID = gl_PrimitiveIDIn;
	EmitVertex();
	EndPrimitive();
}