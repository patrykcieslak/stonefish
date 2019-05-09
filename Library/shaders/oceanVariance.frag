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
uniform float varianceSize;
uniform int fftSize;
uniform vec4 gridSizes;
uniform float slopeVarianceDelta;
uniform float c;

in vec2 texcoord;
out vec2 fragColor;

#define M_PI 3.14159265

vec2 getSlopeVariances(vec2 k, float A, float B, float C, vec2 spectrumSample) 
{
    float w = 1.0 - exp(A * k.x * k.x + B * k.x * k.y + C * k.y * k.y);
    vec2 kw = k * w;
    return kw * kw * dot(spectrumSample, spectrumSample) * 2.0;
}

void main() 
{
    const float SCALE = 10.0;
    float a = floor(texcoord.x * varianceSize);
    float b = floor(texcoord.y * varianceSize);
    float A = pow(a / (varianceSize - 1.0), 4.0) * SCALE;
    float C = pow(c / (varianceSize - 1.0), 4.0) * SCALE;
    float B = (2.0 * b / (varianceSize - 1.0) - 1.0) * sqrt(A * C);
    A = -0.5 * A;
    B = - B;
    C = -0.5 * C;

    vec2 slopeVariances = vec2(slopeVarianceDelta);
    for (int y = 0; y < fftSize; ++y) 
	{
        for (int x = 0; x < fftSize; ++x) 
		{
            int i = x >= fftSize / 2 ? x - fftSize : x;
            int j = y >= fftSize / 2 ? y - fftSize : y;
            vec2 k = 2.0 * M_PI * vec2(i, j);

            vec4 spectrum12 = texture(texSpectrum12, vec2(float(x) + 0.5, float(y) + 0.5) / float(fftSize));
            vec4 spectrum34 = texture(texSpectrum34, vec2(float(x) + 0.5, float(y) + 0.5) / float(fftSize));

            slopeVariances += getSlopeVariances(k / gridSizes.x, A, B, C, spectrum12.xy);
            slopeVariances += getSlopeVariances(k / gridSizes.y, A, B, C, spectrum12.zw);
            slopeVariances += getSlopeVariances(k / gridSizes.z, A, B, C, spectrum34.xy);
            slopeVariances += getSlopeVariances(k / gridSizes.w, A, B, C, spectrum34.zw);
        }
    }
    fragColor = slopeVariances;
}
