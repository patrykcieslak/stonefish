#version 430

out vec3 normal;
out mat3 TBN; //Not used
out vec2 texCoord;
out vec4 fragPos;
out vec3 eyeSpaceNormal;
out float logz;

uniform mat4 MV;
uniform mat4 iMV;
uniform mat4 P;
uniform float FC;

struct Pose
{
    vec4 posScaleX; // xyz - position, w - scale X
    vec4 ori;       // xyz - orientation axis, w - angle
};

layout(std140) buffer Poses
{
    Pose pose[];
};

void main() 
{
    //Fill outputs
    normal = vec3(0.0,0.0,-1.0);
    texCoord = quadPos;
    fragPos = iMV * vertexPosEye; 
    eyeSpaceNormal = vec3(0.0,0.0,1.0);
    
    //Compute projected position
    gl_Position = P * vertexPosEye;
    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * 2.0 * FC - 1.0;
    logz = 1.0 + gl_Position.w;   
}