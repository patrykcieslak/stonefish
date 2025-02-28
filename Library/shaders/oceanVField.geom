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

#version 430

layout (points) in;
layout (line_strip, max_vertices = 2) out;

uniform mat4 VP;
uniform float vectorSize;
uniform float velocityMax;
uniform vec3 eyePos;

#inject "velocityField.glsl"

out GS_OUT
{
    vec4 color;
} gs_out;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 colormap(float value, float limit)
{
    vec3 c;
    float data = value/limit;
    c.r = clamp((data-0.375)*4.0, 0.0, 1.0) - clamp((data-0.875)*4.0, 0.0, 0.5);
    c.g = clamp((data-0.125)*4.0, 0.0, 1.0) - clamp((data-0.625)*4.0, 0.0, 1.0);
    c.b = 0.5 + clamp(data*4.0, 0.0, 0.5) - clamp((data-0.375)*4.0, 0.0, 1.0);
    return c;
}

void main()
{
    //Calculate velocity at point
    vec3 p = gl_in[0].gl_Position.xyz;
    p.x += rand(vec2(p.x, p.y))*0.5;
    p.y += rand(vec2(p.y*p.x, p.z))*0.5;
    p.z += rand(vec2(p.z, p.x))*0.5;

    float d = length(eyePos - p);
    if(d > 10.0)
        return;

    vec3 velocity = vec3(0.0);
    
    for(uint i=0; i<numCurrents; ++i)
    {
        switch(currents[i].type)
        {
            case 0: //Uniform
                velocity += currents[i].dirV.xyz * currents[i].dirV.w;
                break;

            case 1: //Jet
                velocity += jet(p, currents[i].posR.xyz, currents[i].posR.w, 
                                   currents[i].dirV.xyz, currents[i].dirV.w);
                break;

            case 2: //Pipe
                velocity += pipe(p, currents[i].posR.xyz, currents[i].posR.w,
                                    currents[i].params.y, currents[i].params.x,
                                    currents[i].dirV.xyz, currents[i].dirV.w,
                                    currents[i].params.z);
                break;

            /*case 10: //Thruster
                velocity += thruster(p, currents[i].posR.xyz, currents[i].posR.w, 
                                        currents[i].dirV.xyz, currents[i].dirV.w);
                break;*/
                
            default:
                break;
        }
    }

    //Generate vector
    float vmag = length(velocity);
    if(vmag > 0.01)
    {
        float opacity = exp(-0.1*d);
        gl_Position = VP * vec4(p, 1.0);
        gs_out.color = vec4(0.0);
        EmitVertex();
        gl_Position = VP * vec4(p + velocity/vmag * vectorSize * vmag, 1.0);
        gs_out.color = vec4(colormap(vmag, velocityMax), opacity);
        EmitVertex();
        EndPrimitive();
    }
}