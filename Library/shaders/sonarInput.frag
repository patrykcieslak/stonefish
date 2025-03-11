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

#version 330

in vec3 normal;
in vec3 fragPos;
layout(location = 0) out vec2 rangeIntensity;

uniform vec3 eyePos;
uniform float restitution;

void main()
{
    vec3 N = normalize(normal);
    vec3 toEye = eyePos-fragPos;
    float len = length(toEye);
    toEye /= len;
    rangeIntensity.x = len;
    rangeIntensity.y = clamp(dot(N, toEye), 0.0, 1.0) * restitution;
}