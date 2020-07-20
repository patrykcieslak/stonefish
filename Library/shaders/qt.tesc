/*    
    Copyright (c) 2020 Patryk Cieslak. All rights reserved.

    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
    Based on "Quadtrees on the GPU" by Jonathan Dupuy. 
*/

#version 430

layout(vertices = 4) out;

uniform vec3 eyePos;
uniform float u_scene_size;
uniform float u_gpu_tess_factor;

layout(std140, binding = 0) uniform ViewFrustumPlanes
{
	vec4 planes[6]; // frustum planes of the camera
};

#define SQRT_2 1.414213561

//Linear quad tree API
#inject "ltree.glsl"

//Implementation dependent heightfield function
float displace(vec2 p);

void main() 
{
	// get edge data
	vec4 edge = 0.5 * gl_in[gl_InvocationID].gl_Position
	          + 0.5 * gl_in[(gl_InvocationID+1)%4].gl_Position;
	vec3 p  = vec3(edge.xy, displace(eyePos.xy)).xyz;
	float s = 2.0 * distance(eyePos, p);
	float tess_level = edge.w * 8.0 * SQRT_2 / s;

	// set tess levels
	gl_TessLevelInner[gl_InvocationID%2] = tess_level - 1.0 + pow(2.0, u_gpu_tess_factor);
	gl_TessLevelOuter[gl_InvocationID] = pow(2.0, u_gpu_tess_factor + ceil(log2(tess_level)));

	// send data
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}