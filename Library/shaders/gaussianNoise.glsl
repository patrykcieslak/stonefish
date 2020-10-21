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

float rand(vec2 coord)
{
    // This one-liner can be found in many places, including:
    // http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
    // I can't find any explanation for it, but experimentally it does seem to
    // produce approximately uniformly distributed values in the interval [0,1].
    return fract(sin(dot(coord.xy, vec2(12.9898,78.233))) * 43758.5453);
}

float gaussian(vec2 coord, vec3 seed, float stddev, float mean)
{
    // Box-Muller method for sampling from the normal distribution
    // http://en.wikipedia.org/wiki/Normal_distribution#Generating_values_from_normal_distribution
    // This method requires 2 uniform random inputs and produces 2 
    // Gaussian random outputs. 3rd random variable is used to switch between the two outputs.
    float U, V, R, Z;
    // Add in the CPU-supplied random noiseSeed to generate the 3 random values.
    U = rand(coord + vec2(seed.x, seed.x))+0.0000001;
    V = rand(coord + vec2(seed.y, seed.y))+0.0000001;
    R = rand(coord + vec2(seed.z, seed.z))+0.0000001;
    // Switch between the two random outputs.
    if(R < 0.5)
        Z = sqrt(-2.0 * log(U)) * sin(6.2831853 * V);
    else
        Z = sqrt(-2.0 * log(U)) * cos(6.2831853 * V);

    // Apply the stddev and mean.
    Z = Z * stddev + mean;
    return Z;
}