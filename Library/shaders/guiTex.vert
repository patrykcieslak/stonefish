#version 330 core

layout(location = 0) in vec4 vertex;
out vec2 texCoord;
out vec2 backTexCoord;

void main()
{
	texCoord = vertex.zw;
	backTexCoord = (vertex.xy+vec2(1.0))/2.0;
	gl_Position = vec4(vertex.xy, 0, 1.0);
}
