#version 120
varying vec4 color;
varying vec3 normal;
varying vec4 position;
varying float depth;
varying float material;
varying float clipPos;

attribute float materialData;
uniform vec4 clipPlane;

void main(void)
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
    color = gl_Color;
    material = materialData;
    
    //in eye space
    normal = normalize(gl_NormalMatrix * gl_Normal);
    position = gl_ModelViewMatrix * gl_Vertex;
    depth = -position.z;
    
    //clipping
    clipPos = dot(position.xyz, clipPlane.xyz) + clipPlane.w;
    
    gl_Position = ftransform();
}
