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

uniform vec3 tau; //Beer-Lambert optical thickness

const float water2air = 1.33/1.0;
const float air2water = 1.0/1.33;
const float PI = 3.14159265358979323846;
const vec3 waterRayleigh = vec3(0.15023, 0.405565, 1.0);

//Phase functions
float Rayleigh(float cosTheta) //For small particles
{
	return 3.0/(16.0*PI)*(1.0+cosTheta*cosTheta);
}

float MieLorenz(float cosTheta) //For large particles, thick media 
{
	return 1.0/(4.0*PI)*(1.0/2.0+33.0/2.0*pow((1.0+cosTheta)/2.0, 32.0));
}

float HenyeyGreenstein(float cosTheta, float g)
{
	float f = 1.0+g*g-2.0*g*cosTheta;
	return 1.0/(4.0*PI)*(1.0-g*g)/sqrt(f*f*f);
}

//Optical effects
vec3 RefractToWater(vec3 I, vec3 N)
{
	return normalize(refract(I, N, air2water));
}

vec3 RefractToAir(vec3 I, vec3 N)
{
	return normalize(refract(I, N, water2air));
}

vec3 BeerLambert(float d)
{
	return exp(-tau*d);
}

vec3 InScattering(vec3 L, vec3 D, vec3 V, float z, float d)
{
	float phase = MieLorenz(dot(D,V)); //HenyeyGreenstein(dot(D,V), 0.924);
	return L*phase/(tau*(-V.z+1.0)) * exp(-tau*z) * (1.0 - exp(-tau*(-V.z*d+d)));
}