#version 120
varying vec4 color;
varying vec4 position;
varying vec3 normal;
varying float depth;

void main(void)
{
    //texture and material
	gl_TexCoord[0] = gl_MultiTexCoord0;
    color = gl_Color;
   
    //in eye space
    normal = normalize(gl_NormalMatrix * gl_Normal);
    position = gl_ModelViewMatrix * gl_Vertex;
    depth = -position.z;

    //clipping plane in eye space
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    
    //transform
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; //ftransform();
}
