/*    
    Copyright (c) 2019 Patryk Cieslak. All rights reserved.

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
uniform mat4 MVP;
uniform vec3 camRight;
uniform vec3 camUp;

out vec2 texcoord;
out vec3 fragPos;

void main()
{
	vec3 vertex = 
		billboardCenterSize.xyz
		+ camRight * billboardCorner.x * billboardCenterSize.w
		+ camUp * billboardCorner.y * billboardCenterSize.w;

	gl_Position = MVP * vec4(vertex, 1.0);
    texcoord = billboardCorner.xy + vec2(0.5, 0.5);
    fragPos = vertex;
}
