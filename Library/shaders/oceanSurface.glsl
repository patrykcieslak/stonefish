/*    
    Copyright (c) 2020 Patryk Cieslak. All rights reserved.

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

uniform sampler2DArray texWaveFFT;
uniform vec4 gridSizes;

float displace(vec2 p)
{
	float dz = 0.0;
    dz -= texture(texWaveFFT, vec3(p/gridSizes.x, 0.0)).x;
    dz -= texture(texWaveFFT, vec3(p/gridSizes.y, 0.0)).y;
    dz -= texture(texWaveFFT, vec3(p/gridSizes.z, 0.0)).z;
    dz -= texture(texWaveFFT, vec3(p/gridSizes.w, 0.0)).w;
    return dz;
}

float displaceGrad(vec2 p, vec2 dx, vec2 dy)
{
    float dz = 0.0;
    dz -= textureGrad(texWaveFFT, vec3(p/gridSizes.x, 0.0), dx/gridSizes.x, dy/gridSizes.x).x;
    dz -= textureGrad(texWaveFFT, vec3(p/gridSizes.y, 0.0), dx/gridSizes.y, dy/gridSizes.y).y;
    dz -= textureGrad(texWaveFFT, vec3(p/gridSizes.z, 0.0), dx/gridSizes.z, dy/gridSizes.z).z;
    dz -= textureGrad(texWaveFFT, vec3(p/gridSizes.w, 0.0), dx/gridSizes.w, dy/gridSizes.w).w;    
    return dz;
}