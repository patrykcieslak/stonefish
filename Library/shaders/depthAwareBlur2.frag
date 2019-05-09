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
in vec2 blurtexcoord[4];
out vec4 fragColor;
uniform sampler2DArray texSource;
uniform sampler2D texLinearDepth;
uniform int sourceLayer;

const float weights[4] = float[4](0.0702702703, 0.3162162162, 0.3162162162, 0.0702702703);

void main()
{
    float depth = texture(texLinearDepth, texcoord).r;
	float weightSum = 0.2270270270;
	fragColor = vec4(0.0);
    fragColor += texture(texSource, vec3(texcoord, sourceLayer)) * 0.2270270270;
    
	for(int i=0; i<4; ++i)
	{
		//float sampleDepth = texture(texLinearDepth, blurtexcoord[i]).r;
		//if(sampleDepth <= depth)
		{
			fragColor += texture(texSource, vec3(blurtexcoord[i], sourceLayer)) * weights[i];
			weightSum += weights[i];
		}
	}
	fragColor /= weightSum;
}
