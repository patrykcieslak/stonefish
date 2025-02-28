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

layout(location = 0) out float fragColor;

uniform sampler2D texLogDepth;
uniform float FC;

float linearDepth(float logd)
{
    return pow(2, logd/FC) - 1.0;
}

void main() 
{
    float depth = texelFetch(texLogDepth, ivec2(gl_FragCoord.xy), 0).x;
	  fragColor = linearDepth(depth);
}


