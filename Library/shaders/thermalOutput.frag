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
out float fragColor;
uniform sampler2D texSource;
uniform vec2 temperatureRange;
uniform vec3 noiseSeed;
uniform float noiseStddev;

#inject "gaussianNoise.glsl"

void main(void) 
{
    //Read perfect temperature
    float temperature = texture(texSource, vec2(texcoord.x, 1.0-texcoord.y)).r;

    //Add noise
    temperature += gaussian(texcoord, noiseSeed, noiseStddev, 0.0);

    //Limit temperature to range
    fragColor = clamp(temperature, temperatureRange.x, temperatureRange.y);
}