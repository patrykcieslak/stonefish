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

#version 330

layout(location = 0) in vec3 billboardCorner;
layout(location = 1) in vec4 billboardCenterSize;

out vec3 normal;
out vec2 texCoord;
out vec4 fragPos;
out vec3 eyeSpaceNormal;
out float logz;

uniform mat4 MVP;
uniform vec3 camRight;
uniform vec3 camUp;
uniform float FC;

void main()
{
	vec3 vertex = 
		billboardCenterSize.xyz
		+ camRight * billboardCorner.x * billboardCenterSize.w
		+ camUp * billboardCorner.y * billboardCenterSize.w;
    normal = vec3(0.0,0.0,-1.0);
    eyeSpaceNormal = vec3(0.0,0.0,1.0);
    texCoord = billboardCorner.xy + vec2(0.5, 0.5);
    fragPos = vec4(vertex, 1.0);
    gl_Position = MVP * vec4(vertex, 1.0);
    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * 2.0 * FC - 1.0;
    logz = 1.0 + gl_Position.w;
}
