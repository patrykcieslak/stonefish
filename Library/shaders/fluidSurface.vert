#version 120

//varying vec3 fluidPosition;
//varying vec3 fluidNormal;

void main(void)
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
    
    //in eye space
    //fluidNormal = normalize(gl_NormalMatrix * gl_Normal);
    //fluidPosition = (gl_ModelViewMatrix * gl_Vertex).xyz;
    
    gl_Position = ftransform();
}
