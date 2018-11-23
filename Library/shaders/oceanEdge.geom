#version 330

layout(lines) in;
layout(triangle_strip, max_vertices=6) out;

//Computation grid params
uniform mat4 MVP;
//Wave height params
uniform sampler2DArray texWaveFFT;
uniform vec4 gridSizes;

out vec2 texCoord;

const float edgeThickness = 0.03;

vec3 getWaveSurfaceVertex(in vec3 P, in vec2 dx, in vec2 dy)
{
    float dz = 0.0;
    
    //Basic wave height (layer 0)
    dz += textureGrad(texWaveFFT, vec3(P.xy/gridSizes.x, 0.0), dx/gridSizes.x, dy/gridSizes.x).x;
    dz += textureGrad(texWaveFFT, vec3(P.xy/gridSizes.y, 0.0), dx/gridSizes.y, dy/gridSizes.y).y;
    dz += textureGrad(texWaveFFT, vec3(P.xy/gridSizes.z, 0.0), dx/gridSizes.z, dy/gridSizes.z).z;
    dz += textureGrad(texWaveFFT, vec3(P.xy/gridSizes.w, 0.0), dx/gridSizes.w, dy/gridSizes.w).w;
    
    return vec3(P.xy, dz); //Position of deformed vertex in world space
}

vec3 intersectLines(in vec3 A, in vec3 B, in vec3 C, in vec3 D)
{
    vec3 v1 = B-A;
    vec3 v2 = D-C;
    
    float denom = v1.x * (-v2.y) - (v1.y * (-v2.x));
    float nom = (C.x - A.x) * (-v2.y) - ((C.y - A.y) * (-v2.x));
    
    return A + nom/denom * v1;
}

void main()
{
    mat4 iMVP = inverse(MVP);
    
    //Get frustum edge coordinate in world frame
    vec4 end1t = iMVP * vec4(gl_in[0].gl_Position.x,  1.0, -1.0, 1.0);
    vec4 end1b = iMVP * vec4(gl_in[0].gl_Position.x, -1.0, -1.0, 1.0);
    vec4 end2t = iMVP * vec4(gl_in[1].gl_Position.x,  1.0, -1.0, 1.0);
    vec4 end2b = iMVP * vec4(gl_in[1].gl_Position.x, -1.0, -1.0, 1.0);
    vec3 end1t_cam = end1t.xyz / end1t.w;
    vec3 end1b_cam = end1b.xyz / end1b.w;
    vec3 end2t_cam = end2t.xyz / end2t.w;
    vec3 end2b_cam = end2b.xyz / end2b.w;
    
    //Get world pos on ocean surface
    vec2 dx = vec2(0.0);//end2t_cam.xy - end1t_cam.xy;
    vec2 dy = vec2(0.0);//end1t_cam.xy - end1b_cam.xy;
    vec3 end1t_water = getWaveSurfaceVertex(end1t_cam, dx, dy);
    vec3 end1b_water = getWaveSurfaceVertex(end1b_cam, dx, dy);
    vec3 end2t_water = getWaveSurfaceVertex(end2t_cam, dx, dy);
    vec3 end2b_water = getWaveSurfaceVertex(end2b_cam, dx, dy);
    
    //Find intersection between line connecting water points and frustum plane
    vec3 i1 = (end1t_water + end1b_water)/2.0; //intersectLines(end1t_cam, end1b_cam, end1t_water, end1b_water);
    vec3 i2 = (end2t_water + end2b_water)/2.0; //intersectLines(end2t_cam, end2b_cam, end2t_water, end2b_water);
    
    //Project world pos back to camera frame
    vec4 end1 = MVP * vec4(i1, 1.0);
    vec4 end2 = MVP * vec4(i2, 1.0);
    end1.xyz /= end1.w;
    end2.xyz /= end2.w;
    
    //Draw triangles
    gl_Position = vec4(end1.x, end1.y + edgeThickness, -1.0, 1.0);
    texCoord = vec2(0.0, -1.0);
    EmitVertex();
    
    gl_Position = vec4(end2.x, end2.y + edgeThickness, -1.0, 1.0);
    texCoord = vec2(1.0, -1.0);
    EmitVertex();
            
    gl_Position = vec4(end2.x, end2.y - edgeThickness, -1.0, 1.0);
    texCoord = vec2(1.0, 1.0);
    EmitVertex();
            
    gl_Position = vec4(end1.x, end1.y + edgeThickness, -1.0, 1.0);
    texCoord = vec2(0.0, -1.0);
    EmitVertex();

    gl_Position = vec4(end2.x, end2.y - edgeThickness, -1.0, 1.0);
    texCoord = vec2(1.0, 1.0);
    EmitVertex();
            
    gl_Position = vec4(end1.x, end1.y - edgeThickness, -1.0, 1.0);
    texCoord = vec2(0.0, 1.0);
    EmitVertex();
    
    EndPrimitive();
}
