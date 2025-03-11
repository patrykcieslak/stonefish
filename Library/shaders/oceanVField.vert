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

uniform ivec3 gridOrigin;
uniform ivec3 gridSize;
uniform float gridScale;

void main()
{
    ivec3 point;

    if(gl_VertexID == 0)
    {
        point.x = 0;
        point.y = 0;
        point.z = 0;
    }
    else
    {
        point.x = gl_VertexID/(gridSize.y*gridSize.z); 
        point.y = gl_VertexID/gridSize.z - point.x*gridSize.y;
        point.z = gl_VertexID - (point.x*gridSize.y + point.y)*gridSize.z;        
    }

    gl_Position = vec4((point + gridOrigin) * gridScale, 1.f); //world position
}