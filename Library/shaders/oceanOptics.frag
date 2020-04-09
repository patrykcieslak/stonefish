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

uniform vec3 cWater;
uniform vec3 bWater;

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

float MieLorenzg(float cosTheta, float g) //For large particles
{
	return 1.0/(4.0*PI)*(1.0/2.0+g/2.0*pow((1.0+cosTheta)/2.0, g-1.0));
}

float HenyeyGreenstein(float cosTheta, float g)
{
	float f = 1.0+g*g-2.0*g*cosTheta;
	return 1.0/(4.0*PI)*(1.0-g*g)/sqrt(f*f*f);
}

float FournierForand(float cosTheta, float n, float mu)
{
    float nu = (3.0 - mu)/2.0;
    float theta = acos(cosTheta);
    float sinTheta2 = pow(sin(theta/2.0),2.0);
    float delta180 = 4.0/(3.0*pow(n-1.0, 2.0));
    float delta = delta180 * sinTheta2;
    float deltaNu = pow(delta, nu);
    float delta180Nu = pow(delta180, nu);
    float c1 = 1.0/(4.0*PI*pow(1.0-delta, 2.0)*deltaNu) * (nu*(1.0-delta)-(1.0-deltaNu)+(delta*(1.0-deltaNu)-nu*(1.0-delta))/(sinTheta2));
    float c2 = (1.0-delta180Nu)/(16.0*PI*(delta180-1.0)*delta180Nu) * (3.0*pow(cosTheta, 2.0)-1.0);
    return c1 + c2;
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
	return exp(-cWater*d);
}

vec3 InScattering(vec3 L, vec3 D, vec3 V, float z, float d)
{   
    float phase = HenyeyGreenstein(dot(D,V), 0.924);
	//float phase = MieLorenzg(dot(D,V), g*30.0+1.0); 
    //float phase = FournierForand(dot(D,V), 1.08, 3.483);
	return 10.0*L*phase*bWater/(cWater*(-V.z/D.z+1.0)) * exp(-cWater*z/D.z) * (1.0 - exp(-cWater*d*(-V.z/D.z+1.0)));
}