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

/*
    Based on "Real-time Animation and Rendering of Ocean Whitecaps"
    by Jonathan Dupuy and Eric Bruneton.
    https://github.com/jdupuy/whitecaps
*/

#version 330

uniform sampler2D texSpectrum12;
uniform sampler2D texSpectrum34;
uniform vec4 invGridSizes;
uniform float fftSize;
uniform float zoom;
uniform float linear;

in vec2 texcoord;
out vec4 fragColor;

void main() 
{
    vec2 st = texcoord - vec2(0.5);
    float r = length(st);
    float k = pow(10.0, -3.0 + 12.0 * r);

    vec2 kxy = st * pow(10.0, zoom * 3.0);

    float S = 0.0;
    if (abs(kxy.x) < invGridSizes.x && abs(kxy.y) < invGridSizes.x) 
	{
        st = 0.5 * kxy / invGridSizes.x + 0.5 / fftSize;
        S += length(texture(texSpectrum12, st).xy) * fftSize / invGridSizes.x;
    }
    if (abs(kxy.x) < invGridSizes.y && abs(kxy.y) < invGridSizes.y) 
	{
        st = 0.5 * kxy / invGridSizes.y + 0.5 / fftSize;
        S += length(texture(texSpectrum12, st).zw) * fftSize / invGridSizes.y;
    }
    if (abs(kxy.x) < invGridSizes.z && abs(kxy.y) < invGridSizes.z) 
	{
        st = 0.5 * kxy / invGridSizes.z + 0.5 / fftSize;
        S += length(texture(texSpectrum34, st).xy) * fftSize / invGridSizes.z;
    }
    if (abs(kxy.x) < invGridSizes.w && abs(kxy.y) < invGridSizes.w) 
	{
        st = 0.5 * kxy / invGridSizes.w + 0.5 / fftSize;
        S += length(texture(texSpectrum34, st).zw) * fftSize / invGridSizes.w;
    }
    S = S * S * 0.5;

    float s;
    if (linear > 0.0) 
	{
        s = S * 100.0; // linear scale in intensity
    } 
	else 
	{
        s = (log(S) / log(10.0) + 15.0) / 18.0; // logarithmic scale in intensity
    }

    fragColor = vec4(s);
}
