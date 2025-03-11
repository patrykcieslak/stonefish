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

layout(location = 0,index=0) out vec4 fragColor[3];
in vec2 texcoord;

uniform sampler2D texSpectrum12;
uniform sampler2D texSpectrum34;
uniform vec4 inverseGridSizes;
uniform float fftSize;
uniform float t;

// h(k,t), complex number
vec2 getSpectrum(float k, vec2 s0, vec2 s0c) 
{
	float w = sqrt(9.81 * k * (1.0 + k * k / (370.0 * 370.0)));
	float c = cos(w * t);
	float s = sin(w * t);
	return vec2((s0.x + s0c.x) * c - (s0.y + s0c.y) * s, (s0.x - s0c.x) * s + (s0.y - s0c.y) * c);
}

vec2 i(vec2 z) 
{
	return vec2(-z.y, z.x); // returns i times z (complex number)
}

void main() 
{
	vec2 st = floor(texcoord * fftSize)/fftSize; // in [-N/2,N/2[
	float x = texcoord.x > 0.5 ? st.x - 1.0 : st.x;
	float y = texcoord.y > 0.5 ? st.y - 1.0 : st.y;

	// h0(k)
	vec4 s12 = textureLod(texSpectrum12, texcoord, 0.0)*1.414213562;
	vec4 s34 = textureLod(texSpectrum34, texcoord, 0.0)*1.414213562;
	// conjugate (h0(k))
	vec4 s12c = textureLod(texSpectrum12, vec2(1.0 + 0.5/fftSize) - st, 0.0)*1.414213562;
	vec4 s34c = textureLod(texSpectrum34, vec2(1.0 + 0.5/fftSize) - st, 0.0)*1.414213562;

	// k
	vec2 k1 = vec2(x, y) * inverseGridSizes.x;
	vec2 k2 = vec2(x, y) * inverseGridSizes.y;
	vec2 k3 = vec2(x, y) * inverseGridSizes.z;
	vec2 k4 = vec2(x, y) * inverseGridSizes.w;

	// k magnitude
	float K1 = length(k1);
	float K2 = length(k2);
	float K3 = length(k3);
	float K4 = length(k4);

	// h(k,t)
	vec2 h1 = getSpectrum(K1, s12.xy, s12c.xy);
	vec2 h2 = getSpectrum(K2, s12.zw, s12c.zw);
	vec2 h3 = getSpectrum(K3, s34.xy, s34c.xy);
	vec2 h4 = getSpectrum(K4, s34.zw, s34c.zw);

	// h(K,t) for 4 Grids (with different Lx Lz)
	fragColor[0] = vec4(h1 + i(h2), h3 + i(h4)); // Tes01 eq19

	// slopes (for normal computation)
	fragColor[1] = vec4(i(k1.x * h1) - (k1.y * h1), i(k2.x * h2) - (k2.y * h2)); // Tes01 eq20
	fragColor[2] = vec4(i(k3.x * h3) - (k3.y * h3), i(k4.x * h4) - (k4.y * h4)); // Tes01 eq20
}
