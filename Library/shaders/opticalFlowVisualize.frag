/*    
    Copyright (c) 2024 Patryk Cieslak. All rights reserved.

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
out vec4 fragColor;
uniform sampler2D texFlow;
uniform float maxVel;

const float PI = 3.14159265359;

vec3 hsvToRgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec2 vel = texture(texFlow, texcoord).xy;
    float angle = 0.0;
    if(abs(vel.x) > 1e-6)
        angle = clamp((atan(vel.y, vel.x)/PI+1.0)/2.0, 0.0, 1.0); // Angle 0.0 - 1.0 (0.0 - 360.0 deg)
    float mag = clamp(length(vel)/maxVel, 0.0, 1.0); // Normalized velocity magnitude
    fragColor = vec4(hsvToRgb(vec3(angle, 1.0, mag)), 1.0);
}
