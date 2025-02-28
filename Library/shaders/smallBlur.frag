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

in vec2 texcoord;
out vec4 fragColor;
uniform sampler2D tex;
uniform vec2 invTexSize;

const float gaussian[9] = float[](
0.077847, 0.123317, 0.077847,
0.123317, 0.195346, 0.123317,
0.077847, 0.123317, 0.077847
);

void main(void) 
{
    fragColor = vec4(0.0);
    for(int i=-1; i<=1; ++i)
        for(int h=-1; h<=1; ++h)
            fragColor += texture(tex, texcoord + vec2(invTexSize.x*i, invTexSize.y*h)) * gaussian[(i+1)*3+(h+1)];
}
