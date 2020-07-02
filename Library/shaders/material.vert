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

layout(location = 0) in vec3 vt;
layout(location = 1) in vec3 n;

out vec3 normal;
out vec4 fragPos;
out vec3 eyeSpaceNormal;
out float logz;

uniform mat4 MVP;
uniform mat4 M;
uniform mat3 N;
uniform mat3 MV;
uniform float FC;

void main()
{
	normal = normalize(N * n);
	eyeSpaceNormal = normalize(MV * n);
	fragPos = M * vec4(vt, 1.0);
	gl_Position = MVP * vec4(vt, 1.0); 
    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * 2.0 * FC - 1.0;
    logz = 1.0 + gl_Position.w;
}
