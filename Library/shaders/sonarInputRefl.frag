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

// Created 2026 by Vasutorn S.: added reflectivity-texture sonar input
// (derived from sonarInputUv.frag).

#version 330

in mat3 TBN;
in vec2 texCoord;
in vec3 fragPos;
layout(location = 0) out vec2 rangeIntensity;

uniform vec3 eyePos;
uniform sampler2D texReflectivity;

float rgbToFloat32(vec3 rgbNormalized)
{
    uvec3 rgb = uvec3(round(rgbNormalized * 255.0));

    uint top24 = (rgb.r << 16u) |
                 (rgb.g << 8u)  |
                  rgb.b;

    return uintBitsToFloat(top24 << 8u);
}

void main()
{
    vec3 N = normalize(TBN[2]);              // geometric normal
    vec3 toEye = eyePos - fragPos;
    float len = length(toEye);
    toEye /= len;
    float refl_val = rgbToFloat32(texture(texReflectivity, texCoord).rgb);
    rangeIntensity.x = len;                  // slant range
    rangeIntensity.y = clamp(dot(N, toEye), 0.0, 1.0) * refl_val;
}