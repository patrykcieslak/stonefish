#version 330

out vec2 texcoord;

void main()
{
	int idx = gl_VertexID % 3;
	vec4 pos =  vec4((float(idx&1))*4.0-1.0, (float((idx>>1)&1))*4.0-1.0, 1.0, 1.0);
	gl_Position = pos;
	texcoord = pos.xy * 0.5 + 0.5;
}
