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

out vec2 luminance;

uniform sampler2D texHDR;
uniform ivec2 samples;

const float pi2 = 3.14159/2.0;
const float delta = 1e-3;
const vec3 rgbToLum = vec3(0.212671, 0.715160, 0.072169); // vec3(0.299, 0.587, 0.114)

void main(void)
{
    float Lsum = 0.0;
    float Lmax = 0.0;
    
    for(int a = 0; a < samples.x; ++a) //angle
	{
		float angle = (a+0.5)/float(samples.x)*pi2;
		
        for(int r = 1; r <= samples.y; ++r) //radius
        {
            vec2 texCoord = vec2(0.5) + (gl_FragCoord.xy-vec2(1.0)) * float(r)/float(samples.y) * vec2(cos(angle), sin(angle));
            float L = dot(texture(texHDR, texCoord).rgb, rgbToLum);
            Lsum += log(delta + L);
            Lmax = max(Lmax, L);
        }
    }
	
	luminance.r = exp(Lsum/float(samples.x * samples.y));
    luminance.g = Lmax;
}
