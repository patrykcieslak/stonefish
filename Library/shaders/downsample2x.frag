/*
    Downsample x4 by using only one pass achieved by making use of the built-in bilinear filtering
 */

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

out vec4 fragcolor;
uniform sampler2D source;
uniform vec2 srcViewport;

vec4 sourceSample(vec2 offset)
{
    ivec2 dstCoord = ivec2(gl_FragCoord.xy);
    vec2 srcCoord = 4.0 * dstCoord + offset;
    vec2 srcTexCoord = (2.0 * srcCoord + vec2(1.0))/(2.0 * srcViewport);
    return texture(source, srcTexCoord);
}

void main(void)
{
    fragcolor = (sourceSample(vec2(0.5, 0.5)) +
                                sourceSample(vec2(2.5, 0.5)) +
								sourceSample(vec2(0.5, 2.5)) +
								sourceSample(vec2(2.5, 2.5))) * 0.25;
	fragcolor = mix(fragcolor, vec4(1.0), 0.33);
}
