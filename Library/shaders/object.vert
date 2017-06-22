#version 330 core

layout(location = 0) in vec3 vt;
layout(location = 1) in vec3 n;
layout(location = 2) in vec2 uv;
out vec3 normal;
out vec2 texCoord;
out vec3 fragPos;

uniform mat4 MVP;
uniform mat4 M;
uniform mat3 N;

void main()
{
	normal = normalize(N * n);
	texCoord = uv;
	fragPos = vec3(M * vec4(vt, 1.0));
	gl_Position = MVP * vec4(vt, 1.0); 
}
