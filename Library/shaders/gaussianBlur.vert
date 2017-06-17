/*
    Gaussian blur equivalent to 9 samples achieved by making use of the built-in bilinear filtering
    http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
    + texture coordinates precomputed in vertex shader.
 */

#version 330 core

layout(location = 0) in vec4 vertex;
out vec2 texcoord;
out vec2 blurtexcoord[4];
uniform vec2 texelOffset;

void main()
{
	texcoord = vertex.zw;
    blurtexcoord[0] = texcoord + texelOffset * 3.2307692308;
    blurtexcoord[1] = texcoord + texelOffset * 1.3846153846;
    blurtexcoord[2] = texcoord - texelOffset * 1.3846153846;
    blurtexcoord[3] = texcoord - texelOffset * 3.2307692308;
	gl_Position = vec4(vertex.xy,0,1.0);
}
