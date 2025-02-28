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
out vec4 fragcolor;
uniform sampler2D texHDR;
uniform sampler2D texAverage;

unifrom float fovx;
uniform float aspect;
uniform float aperture;
uniform float shutter;
uniform float iso;
uniform float vignetting;

const float pi4 = 3.14159/4.0;

float photometricExposure()
{
    vec2 camCoord = 2.0 * texcoord - vec2(1.0);
    float angle = sqrt(camCoord.x*camCoord.x + camCoord.y*camCoord.y*aspect*aspect) * fovx / 2.0;
    float q = pi4 * 0.9 * vignetting * angle*angle * pow(cos(angle), 4.0);
    return q*shutter/(aperture*aperture);
}

void main(void)
{
     vec3 rgbColor = photometricExposure() * texture(texHDR, texcoord).rgb;
     fragcolor = vec4(pow(rgbColor, 2.2), 1.0);
}