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

out vec2 texcoord;
uniform vec4 rect;

void main(void)
{
    vec2 pos = vec2(float(((uint(gl_VertexID) + 2u) / 3u)%2u),
                    float(((uint(gl_VertexID) + 1u) / 3u)%2u)); 
    texcoord = pos.xy;
    gl_Position = vec4((rect.xy + pos.xy * rect.zw) * 2.0 - 1.0, 0.0, 1.0);
}
