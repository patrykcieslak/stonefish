#version 430 core

layout(points) in;
layout(triangle_strip,max_vertices=4) out;

//Computation grid params
uniform vec2 axis;
uniform vec2 cellSize;
uniform mat4 MVP;
//Wave height params
uniform sampler2DArray texWaveFFT;
uniform vec4 gridSizes;
uniform vec4 choppyFactor;

vec3 getWaveSurfaceVertex(in vec3 P)
{
	vec3 dP = vec3(0.0);
	
	//Basic wave height (layer 0)
    dP.z += texture(texWaveFFT, vec3(P.xy/gridSizes.x, 0.0)).x;
    dP.z += texture(texWaveFFT, vec3(P.xy/gridSizes.y, 0.0)).y;
    dP.z += texture(texWaveFFT, vec3(P.xy/gridSizes.z, 0.0)).z;
    dP.z += texture(texWaveFFT, vec3(P.xy/gridSizes.w, 0.0)).w;
    //Choppy waves (layers 3,4)
	dP.xy += choppyFactor.x*texture(texWaveFFT, vec3(P.xy/gridSizes.x, 3.0)).xy;
	dP.xy += choppyFactor.y*texture(texWaveFFT, vec3(P.xy/gridSizes.y, 3.0)).zw;
	dP.xy += choppyFactor.z*texture(texWaveFFT, vec3(P.xy/gridSizes.z, 4.0)).xy;
	dP.xy += choppyFactor.w*texture(texWaveFFT, vec3(P.xy/gridSizes.w, 4.0)).zw;
    
	return vec3(P.xy + dP.xy, dP.z-0.001); //Position of deformed vertex in world space
}

void main()
{
	vec3 basePos = gl_in[0].gl_Position.xyz;
	
	//First wave surface vertex
	vec3 ws1 = getWaveSurfaceVertex(basePos);
	gl_Position = MVP * vec4(ws1, 1.0);
	EmitVertex();
	
	//Second wave surface vertex
	vec3 ws2 = getWaveSurfaceVertex(basePos + vec3(axis * cellSize.x, 0.0));
	gl_Position = MVP * vec4(ws2, 1.0);
	EmitVertex();
	
	//Two vertices extending deep down
	gl_Position = MVP * vec4(ws1.xy, -100.0, 1.0);
	EmitVertex();
	gl_Position = MVP * vec4(ws2.xy, -100.0, 1.0);
	EmitVertex();
}