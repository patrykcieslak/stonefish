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
out float range;

uniform vec4 rangeInfo;
uniform vec4 projInfo;
uniform sampler2D texDepth;

float linearDepth(float d)
{
    return rangeInfo.z/(rangeInfo.y + d * rangeInfo.w);
}

//Position from UV and linear depth
vec3 viewPositionFromDepth(vec2 uv, float d)
{
    return vec3((uv * projInfo.xy + projInfo.zw) * d, d);
}

void main()
{
    vec2 uv = vec2(texcoord.x, 1.0-texcoord.y); //Vertical flip 
    float depth = linearDepth(texture(texDepth, uv).r);
    vec3 position = viewPositionFromDepth(uv, depth);
    range = clamp(length(position), rangeInfo.x, rangeInfo.y);
}
