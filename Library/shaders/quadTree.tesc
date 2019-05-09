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

#version 400

layout(vertices = 4) out;
in float edgeDiv[];
uniform float tessDiv;

void main(void)
{
    gl_TessLevelOuter[0] = tessDiv * edgeDiv[3];
    gl_TessLevelOuter[1] = tessDiv * edgeDiv[0];
    gl_TessLevelOuter[2] = tessDiv * edgeDiv[1];
    gl_TessLevelOuter[3] = tessDiv * edgeDiv[2];
	
	gl_TessLevelInner[0] = tessDiv; //(gl_TessLevelOuter[1] + gl_TessLevelOuter[3])/2.0;
    gl_TessLevelInner[1] = tessDiv; //(gl_TessLevelOuter[0] + gl_TessLevelOuter[2])/2.0;
	
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
