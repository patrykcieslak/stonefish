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

in vec2 texCoord;
in vec2 backTexCoord;
out vec4 fragcolor;
uniform vec4 color;
uniform sampler2D tex;
uniform sampler2D backTex;

void main()
{
	vec4 background = texture(backTex, backTexCoord);
	vec4 surface = texture(tex, texCoord);
	fragcolor =  surface * background * color;
}
