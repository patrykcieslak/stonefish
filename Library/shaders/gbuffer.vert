#version 330 core

out vec4 position;
out vec3 normal;
out vec2 texcoord;
out float depth;

uniform mat4 MV;
uniform mat4 MVP;

layout(location = 0) in vec4 vertex;
layout(location = 2) in vec3 n;
layout(location = 8) in vec2 uv;

void main()
{
    //texture and material
	texcoord = uv;
   
    //in eye space
    normal =   normalize(MV*vec4(n, 0.0)).xyz;
    position = MV * vertex;
    depth = -position.z;

    //clipping plane in eye space
    //gl_ClipVertex = MV * vertex;
    
    //transform
    gl_Position = MVP * vertex; 
}
