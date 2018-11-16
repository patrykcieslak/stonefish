#version 330 core
//430
layout(points) in;
layout(triangle_strip, max_vertices=6) out;

//Computation grid params
uniform vec2 axis;
uniform vec2 cellSize;
uniform mat4 MVP;
//Wave height params
uniform sampler2DArray texWaveFFT;
uniform vec4 gridSizes;

vec3 getWaveSurfaceVertex(in vec3 P)
{
	float dz = 0.0;
	
	//Basic wave height (layer 0)
    dz += texture(texWaveFFT, vec3(P.xy/gridSizes.x, 0.0)).x;
    dz += texture(texWaveFFT, vec3(P.xy/gridSizes.y, 0.0)).y;
    dz += texture(texWaveFFT, vec3(P.xy/gridSizes.z, 0.0)).z;
    dz += texture(texWaveFFT, vec3(P.xy/gridSizes.w, 0.0)).w;
  
	return vec3(P.xy, dz-0.001); //Position of deformed vertex in world space
}

void main()
{
	vec3 basePos = gl_in[0].gl_Position.xyz;
	
	vec3 ws01 = getWaveSurfaceVertex(basePos + vec3(-axis.y,axis.x, 0.0) * cellSize.y);
	gl_Position = MVP * vec4(ws01.xy, ws01.z - 0.005, 1.0);
	EmitVertex();
	
	vec3 ws02 = getWaveSurfaceVertex(basePos + vec3(axis, 0.0) * cellSize.x + vec3(-axis.y,axis.x, 0.0) * cellSize.y);
	gl_Position = MVP * vec4(ws02.xy, ws02.z - 0.005, 1.0);
	EmitVertex();
	
	vec3 ws1 = getWaveSurfaceVertex(basePos);
	gl_Position = MVP * vec4(ws1.xy, ws1.z - 0.005, 1.0);
	EmitVertex();
	
	vec3 ws2 = getWaveSurfaceVertex(basePos + vec3(axis, 0.0) * cellSize.x);
	gl_Position = MVP * vec4(ws2.xy, ws2.z - 0.005, 1.0);
	EmitVertex();
	
	gl_Position = MVP * vec4(ws1.xy, ws1.z - 100.0, 1.0);
	EmitVertex();
	
	gl_Position = MVP * vec4(ws2.xy, ws2.z - 100.0, 1.0);
	EmitVertex();
	
	EndPrimitive();
}
