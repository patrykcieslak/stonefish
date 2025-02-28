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

in vec2 texcoord;
layout(location = 0) out float fragColor;

uniform sampler2D texDepth;
uniform vec4 rangeInfo; //zNear, zFar, zNear*zFar, zNear-zFar
uniform vec3 noiseSeed;
uniform float noiseStddev;

#inject "gaussianNoise.glsl"

float linearDepth(float d)
{
    return rangeInfo.z/(rangeInfo.y + d * rangeInfo.w);
}

void main() 
{
    float depth = texture(texDepth, vec2(texcoord.x, 1.0-texcoord.y)).r; //Vertical flip
    if(depth == 1.0) //Check if out of range
        fragColor = 0.0;
    else
    {
        depth = linearDepth(depth);
	    fragColor = depth + gaussian(texcoord, noiseSeed, depth*depth*noiseStddev, 0.0);
    }
}