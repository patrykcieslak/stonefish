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

//Gaussian blur equivalent to 9 samples achieved by making use of the built-in bilinear filtering
//http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
//+ texture coordinates precomputed in vertex shader.

#version 330

in vec2 texcoord;
in vec2 blurtexcoord[4];
out vec4 fragcolor;
uniform sampler2D source;

void main()
{
    fragcolor = vec4(0.0);
    fragcolor += texture(source, blurtexcoord[0]) * 0.0702702703;
    fragcolor += texture(source, blurtexcoord[1]) * 0.3162162162;
    fragcolor += texture(source, texcoord) * 0.2270270270;
    fragcolor += texture(source, blurtexcoord[2]) * 0.3162162162;
    fragcolor += texture(source, blurtexcoord[3]) * 0.0702702703;
}
