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
const float r2 = 9.0;
const float deltaMin = 0.025;
const int nMax = 64;

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

/*
    \param D light direction
    \param V view ray direction
*/
vec3 VSF(vec3 D, vec3 V)
{
    float phase = HenyeyGreenstein(dot(D,-V), 0.924);
    //float phase = FournierForand(dot(D,-V), 1.08, 3.483);
    return phase * bWater;
}

/*
    \param L sun illuminance
    \param D sun direction
    \param V view ray direction
    \param z depth
    \param d distance to fragment in water
*/
vec3 InScatteringSun(vec3 L, vec3 D, vec3 V, float z, float d)
{   
    return 10.0 * L * VSF(D,V)/(cWater*(V.z/D.z+1.0)) * exp(-cWater*z/D.z) * (1.0 - exp(-cWater*d*(V.z/D.z+1.0)));
}

/*
    \param O position of the point light
    \param L light luminance
    \param X position of the eye
    \param V view ray direction
    \param P position of the fragment
    \param d distance to the fragment
    \param dw distance to the fragment in water
*/
vec3 InScatteringPointLight(vec3 O, vec3 L, vec3 X, vec3 V, vec3 P, float d, float dw)
{
    vec3 Sin = vec3(0.0);

    //Calculate start and end of raymarching along view vector that gives significant contribution
	vec3 rayStart = X;
	vec3 rayEnd = P;

	//Find line-sphere intersection
	vec3 OX = X - O;
	float VdotOX = dot(V, OX);
	float det = VdotOX * VdotOX - dot(OX, OX) + r2;
	if(det > 0.0) //View ray intersects light volume?
	{
		//Compute intersection points
		float root = sqrt(det);
		float t1 = -VdotOX - root;
		float t2 = -VdotOX + root;
		if(t1 > t2) //Sorting
		{
			float tmp = t1;
			t1 = t2;
			t2 = tmp;
		}

		if(t2 > 0.0 && t1 < d) //Is in light volume?
		{
			//Update start and end of ray
			if(t1 > 0.0)
				rayStart = X + V*t1;
			if(t2 < d)
				rayEnd = X + V*t2;
			
			int n = nMax;
			vec3 rayDelta = (rayEnd - rayStart)/n;
			float delta = length(rayDelta);
			if(delta < deltaMin)
			{
				n = int(ceil(delta/deltaMin * n));
				rayDelta = (rayEnd - rayStart)/n;
				delta = length(rayDelta);
			}

			//Raymarch
			vec3 T = rayStart + rayDelta/2.0;
			float t = delta/2.0;
			float t0 = max(length(rayStart - X) - max(d - dw, 0.0), 0.0);

			for(int k=0; k<n-1; ++k)
			{
				vec3 OT = T - O;
				float dist = length(OT);
				OT /= dist;
				float attenuation = 1.0/(1.0+dist*dist);
				Sin += delta * L * attenuation * VSF(OT,V) * BeerLambert(t0 + t + dist);
				T += rayDelta;
				t += delta;
			}
		}
    }

	return Sin * 10.0;
}

/*
    \param O position of the spot light
	\param D direction of the spot light
	\param w cosinus of the acute cone angle
	\param fn distance to the near plane of the light frustum
    \param L light luminance
    \param X position of the eye
    \param V view ray direction
    \param P position of the fragment
    \param d distance to the fragment
    \param dw distance to the fragment in water
*/
vec3 InScatteringSpotLight(vec3 O, vec3 D, float w, float fn, vec3 L, vec3 X, vec3 V, vec3 P, float d, float dw)
{
	vec3 Sin = vec3(0.0);

	//Calculate start and end of raymarching along view vector that gives significant contribution
	vec3 rayStart = X;
	vec3 rayEnd = P;

	//Line-cone intersection
	float m = 1.0/(w*w);
	vec3 OX = X - O;
	float VdotD = dot(V, D);
	float OXdotD = dot(OX, D);
	float a = 1.0 - m*VdotD*VdotD; 
	float b = 2.0 * (dot(V, OX) - m*VdotD*OXdotD);
	float c = dot(OX, OX) - m*OXdotD*OXdotD;
	float det = b*b - 4*a*c;

	if(det > 0.0) //Two solutions
	{
		float root = sqrt(det);
		float t1 = (-b-root)/(2.0*a);
		float t2 = (-b+root)/(2.0*a);
		if(t1 > t2) //Sorting
		{
			float tmp = t1;
			t1 = t2;
			t2 = tmp;
		}
		vec3 P1 = X + V*t1;
		vec3 P2 = X + V*t2;
		float P1test = dot(P1 - O, D);
		float P2test = dot(P2 - O, D);
		float c1 = fn; 		 //Near cap distance
		float c2 = sqrt(r2); //Far cap distance
			
		if((P1test <= 0.0 && P2test <= 0.0)   //Both points lie on negative cone
			|| (P1test >= c2 && P2test >= c2)) //Both points lie farther than far cap
			return Sin;

		//Line-cap (plane) intersection
		if(P1test > c2)
		{
			t1 = dot(O + D * c2 - X, D)/VdotD;
			P1 = X + V*t1;
		}
		else if(P1test < c1 && P1test > 0.0)
		{
			t1 = dot(O + D * c1 - X, D)/VdotD;
			P1 = X + V*t1;
		}

		if(P2test > c2)
		{
			t2 = dot(O + D * c2 - X, D)/VdotD;
			P2 = X + V*t2;
		}
		else if(P2test < c1 && P2test > 0.0)
		{
			t2 = dot(O + D * c1 - X, D)/VdotD;
			P2 = X + V*t2;
		}

		//Intersection cases
		if(P1test > 0.0)
		{
			if(P2test > 0.0)
			{
				if(t2 > 0.0 && t1 < d)
				{
					if(t1 > 0.0)
						rayStart = P1;
					if(t2 < d)
						rayEnd = P2;
					//compute
				}
				else
					return Sin;
			}
			else
			{
				if(t1 > 0.0 && t1 < d)
				{
					rayEnd = P1;
					float tc2 = dot(O + D * c2 - X, D)/VdotD;
					if(tc2 > 0.0)
						rayStart = X + V*tc2;
					//compute
				}
				else
					return Sin;
			}
		}
		else if(P2test > 0.0 && P2test < c2)
		{
			if(t2 < d)
			{
				if(t2 > 0.0)
					rayStart = P2;
				float tc2 = dot(O + D * c2 - X, D)/VdotD;
				if(tc2 < d)
					rayEnd = X + V * tc2;
				//compute
			}
			else 
				return Sin;
		}
		else
			return Sin;

		//Compute number of steps and step size
		int n = nMax;
		vec3 rayDelta = (rayEnd - rayStart)/n;
		float delta = length(rayDelta);

		if(delta < deltaMin)
		{
			n = int(ceil(delta/deltaMin * n));
			rayDelta = (rayEnd - rayStart)/n;
			delta = length(rayDelta);
		}

		//Raymarch
		vec3 T = rayStart + rayDelta/2.0;
		float t = delta/2.0;
		float t0 = max(length(rayStart - X)-max(d - dw, 0.0), 0.0);

		for(int k=0; k<n-1; ++k)
		{
			vec3 OT = T - O;
			float dist = length(OT);
			OT /= dist;
			float spotEffect = dot(D, OT);
			float edge = smoothstep(1, 1.05, spotEffect/w);
			float attenuation = 1.0/(1.0+dist*dist);
			Sin += delta * L * attenuation * edge * VSF(OT,V) * BeerLambert(t0 + t + dist);
			T += rayDelta;
			t += delta;
		}
	}

	return Sin * 10.0;
}