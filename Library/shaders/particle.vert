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

layout(std140) buffer Positions
{
    vec4 posSize[];
};

void main() 
{
    //Expand points to quads without using GS
    int particleID = gl_VertexID >> 2; // 4 vertices per particle
    vec4 particlePosSize = posSize[particleID];

    //Map vertex ID to quad vertex
    vec2 quadPos = vec2( ((gl_VertexID - 1) & 2) >> 1, (gl_VertexID & 2) >> 1);
    vec4 particlePosEye = MV * vec4(particlePosSize.xyz, 1.0);
    vec4 vertexPosEye = particlePosEye + vec4((quadPos*2.0-1.0) * particlePosSize.w, 0, 0);
    
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