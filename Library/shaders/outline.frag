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
uniform sampler2D texStencil;
uniform vec4 color;
uniform vec2 uvOffset;

void main(void) 
{
    float stencil = 0.0;
    stencil += texture(texStencil, texcoord + vec2(-uvOffset.x, 0.0)).a;
    stencil += texture(texStencil, texcoord + vec2(uvOffset.x, 0.0)).a;
    stencil += texture(texStencil, texcoord + vec2(0.0, -uvOffset.y)).a;
    stencil += texture(texStencil, texcoord + vec2(0.0, uvOffset.y)).a;
    fragColor = vec4(color.rgb, clamp(stencil, 0.0, 1.0));
}
