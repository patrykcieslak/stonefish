/*
    Gaussian blur equivalent to 9 samples achieved by making use of the built-in bilinear filtering
    http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
    + texture coordinates precomputed in vertex shader.
 */

#version 120

uniform vec2 texelOffset;
varying vec2 texCoord;
varying vec2 blurTexCoord[4];

void main(void)
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
    texCoord = gl_TexCoord[0].st;
    blurTexCoord[0] = texCoord + texelOffset * 3.2307692308;
    blurTexCoord[1] = texCoord + texelOffset * 1.3846153846;
    blurTexCoord[2] = texCoord - texelOffset * 1.3846153846;
    blurTexCoord[3] = texCoord - texelOffset * 3.2307692308;
}
