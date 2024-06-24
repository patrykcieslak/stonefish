/*   
    Copyright (c) 2024 Patryk Cieslak. All rights reserved.

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

#version 330

in vec4 fragPos;
in float logz;

layout(location = 0) out vec2 fragColor;

uniform float FC;

//Camera properties
uniform mat3 VR; // View rotation matrix
uniform vec3 d; // View vector
uniform vec2 c; // Center of image in pixels
uniform float f; // Focal length in pixels

//All quantities in world frame
uniform vec3 P_b; // Position of body (center of gravity)
uniform vec3 v_b; // Linear velocity of body
uniform vec3 w_b; // Angular velocity of body
uniform vec3 P_c; // Position of camera
uniform vec3 v_c; // Linear velocity of camera
uniform vec3 w_c; // Angular velocity of camera

void main()
{	
	//Logarithmic z-buffer correction
	gl_FragDepth = log2(logz) * FC;
    
	//Compute velocity in camera frame (m/s)
	vec3 P_f = fragPos.xyz/fragPos.w; // Position of fragment (world frame)
    vec3 R_bf = P_f - P_b;
    vec3 v_f1 = v_b + cross(w_b, R_bf); // Velocity coming from body motion
    vec3 R_cf = P_f - P_c;
    vec3 v_f2 = v_c + cross(w_c, R_cf); // Velocity coming from camera motion
    vec3 v = VR * (v_f1 - v_f2); // Velocity of fragment in camera frame

    // Compute velicity in image plane (px/s)
    float depth = dot(d, R_cf);
    vec2 p = gl_FragCoord.xy - c;
    fragColor = vec2(f*v.x + p.x*v.z, f*v.y + p.y*v.z)/depth;
}
