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

#define MAX_OCEAN_CURRENTS  64

struct VelocityField
{
    vec4 posR;
    vec4 dirV;
    vec3 params;
    uint type;
};

layout(std140) uniform OceanCurrents
{
    VelocityField currents[MAX_OCEAN_CURRENTS];
    vec3 gravity;
    uint numCurrents;
};

//Velocity of fluid for jet current
vec3 jet(vec3 p, vec3 c, float r, vec3 n, float vout)
{
    vec3 cp = p-c;
    
    //Calculate distance from outlet
    float t = dot(cp, n);
    if(t < 0.0) 
        return vec3(0.0);
    
    //Calculate distance to axis
    float d = length(cross(cp, n));

    //Calculate radius at point
    float r_ = 1.0/5.0 * (t + 5.0*r); //Jet angle is around 24 deg independent of conditions!
    if(d >= r_) 
        return vec3(0.0);
    
    //Calculate central velocity
    vec3 vmax = 10.0*r/(t + 5.0*r) * vout * n;  
    
    //Calculate fraction of central velocity
    float f = exp(-50.0*d*d/(t*t));
    
    return f*vmax;
}

//Velocity of fluid for pipe current
vec3 pipe(vec3 p, vec3 p1, float r1, float r2, float l, vec3 n, float vin, float gamma)
{
    vec3 p1p = p-p1;
    
    //Calculate closest point on line section between P1 and P2
    float t = dot(p1p,n);
    if(t < 0.0 || t > l) 
        return vec3(0.0);
    
    //Calculate distance to line
    float d = length(cross(p1p,n));

    //Calculate radius at point
    float r = r1 + (r2-r1) * t/l;
    if(d >= r) 
        return vec3(0.0);
    
    //Calculate central velocity
    vec3 v = r1/r * vin * n;
    
    //Calculate fraction of central velocity
    float f = pow(1.0-d/r, gamma);
    
    return f*v;
}

//Velocity of water for thruster
vec3 thruster(vec3 p, vec3 c, float r, vec3 n, float vout)
{
    vec3 cp = p-c;
    
    //Calculate distance from duct back
    float t = dot(cp, n);
    if(t < 0.0) //Front of thruster?
    { 
        n = -n;
        t = -t;
        vout = -vout;
    }
    
    //Calculate distance to axis
    float d = length(cross(cp, n));

    //Calculate radius at point
    float r_ = 1.0/5.0 * (t + 5.0*r); //Jet angle is around 24 deg independent of conditions!
    if(d >= r_) 
        return vec3(0.0);
    
    //Calculate central velocity
    vec3 vmax = 10.0*r/(t + 5.0*r) * vout * n;  
    
    //Calculate fraction of central velocity
    float f = exp(-50.0*d*d/(t*t));
    
    return f*vmax;
}