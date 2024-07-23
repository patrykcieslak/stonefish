#version 460

layout(location = 0) in vec3 vt;
layout(location = 1) in vec3 n;

out vec3 normal;
out vec4 fragPos;
out vec3 eyeSpaceNormal;
out float logz;

uniform mat4 V;
uniform mat4 VP;
uniform float FC;

struct Pose
{
    vec4 posScaleX; // xyz - position, w - scale X
    vec4 ori;       // quaternion
};

struct Twist
{
    vec4 velScaleY; // xyz - velocity, w - scale Y
    vec4 avelScaleZ; // xyz - angular velocity, w - scale Z
};

layout(std140) buffer Poses
{
    Pose pose[];
};

layout(std140) buffer Twists
{
    Twist twist[];
};

mat3 quat2mat3(vec4 q)
{
    float x = q.x;
    float y = q.y;
    float z = q.z;
    float w = q.w;
    float x2 = x + x;
    float y2 = y + y;
    float z2 = z + z;
    float xx = x * x2;
    float xy = x * y2;
    float xz = x * z2;
    float yy = y * y2;
    float yz = y * z2;
    float zz = z * z2;
    float wx = w * x2;
    float wy = w * y2;
    float wz = w * z2;
    return mat3(1.0 - (yy + zz), xy + wz, xz - wy,
                xy - wz, 1.0 - (xx + zz), yz + wx,
                xz + wy, yz - wx, 1.0 - (xx + yy));
}

void main() 
{
    Pose particlePose = pose[gl_DrawID];
    Twist particleTwist = twist[gl_DrawID];
    mat3 R = quat2mat3(particlePose.ori);
    mat4 M = mat4(vec4(R[0], particlePose.posScaleX.w),
                  vec4(R[1], particleTwist.velScaleY.w),
                  vec4(R[2], particleTwist.avelScaleZ.w),
                  vec4(particlePose.posScaleX.xyz, 1.0));
    mat3 N = mat3(transpose(inverse(M)));
    mat4 MV = V * M;
    mat4 MVP = VP * M;

    normal = normalize(N * n);
	eyeSpaceNormal = normalize(mat3(MV) * n);
	fragPos = M * vec4(vt, 1.0);
	gl_Position = MVP * vec4(vt, 1.0); 
    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * 2.0 * FC - 1.0;
    logz = 1.0 + gl_Position.w;
}