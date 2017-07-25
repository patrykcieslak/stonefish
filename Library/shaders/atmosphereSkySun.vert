#version 430 core    

//out vec3 viewRay;

void main() 
{
	uint idx = gl_VertexID % 3; 
	vec4 pos =  vec4((float(idx&1U))*4.0-1.0, (float((idx>>1U)&1U))*4.0-1.0, 0, 1.0);
	gl_Position = pos;
	//viewRay = (invView * vec4((invClip * pos).xyz, 0.0)).xyz;
}