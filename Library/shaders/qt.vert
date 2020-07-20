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

layout(std140) buffer QTreeCull
{
    vec4 culled[];
};

layout(location = 0) in vec2 i_grid; // grid data

void main() 
{
	vec4 i_transformation = culled[gl_InstanceID];
	vec2 p = i_grid * i_transformation.w + i_transformation.xy + i_transformation.w * 0.5;
	gl_Position = vec4(p, 0, i_transformation.w);
}