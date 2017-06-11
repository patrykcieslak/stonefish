#version 120

void main(void)
{
	gl_Position = vec4(gl_Vertex.xy, 0, 1.0);
}
