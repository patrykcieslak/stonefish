/*
    Gaussian blur equivalent to 9 samples achieved by making use of the built-in bilinear filtering
    http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
    + texture coordinates precomputed in vertex shader.
 */
#version 400
//430
out vec2 texcoord;
out vec2 blurtexcoord[4];
uniform vec2 texelOffset;

void main()
{
	//SAQ
	uint idx = gl_VertexID % 3; 
	vec4 pos =  vec4((float(idx&1U))*4.0-1.0, (float((idx>>1U)&1U))*4.0-1.0, 0, 1.0);
	gl_Position = pos;
	texcoord = pos.xy * 0.5 + 0.5;
	
	//Precomputed texture coordinates
	blurtexcoord[0] = texcoord + texelOffset * 3.2307692308;
    blurtexcoord[1] = texcoord + texelOffset * 1.3846153846;
    blurtexcoord[2] = texcoord - texelOffset * 1.3846153846;
    blurtexcoord[3] = texcoord - texelOffset * 3.2307692308;
}
