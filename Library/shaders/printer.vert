#version 120

//attribute vec4 coord;
varying vec2 texcoord;

void main(void) 
{
	texcoord = gl_Vertex.zw;
	gl_Position = vec4(gl_Vertex.xy, 0, 1.0);
}
