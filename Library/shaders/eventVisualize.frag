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

#version 430

in vec2 texcoord;
layout(location = 0) out vec3 fragColor;

uniform sampler2D texCamera;
uniform isampler2D texEventTimes;

void main() 
{
    vec3 color = texture(texCamera, texcoord).rgb;
    int eventTime = texture(texEventTimes, texcoord).x;
    
    if(eventTime > 0)
        fragColor = vec3(0.0, 0.0, 1.0);
    else if(eventTime < 0) 
        fragColor = vec3(1.0, 0.0, 0.0);
    else
        fragColor = color;
}