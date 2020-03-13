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

in vec2 texcoord;
layout(location = 0) out float sonarData;

uniform sampler2D texRangeIntensity;
uniform int beamSamples;
uniform float beamSampleStep;
uniform float binRangeStep;
uniform float rangeMin;
uniform float halfFOV;
uniform float beamAngleStep;
uniform float d;
uniform vec3 noiseSeed;
uniform vec2 noiseStddev;

#define M_PI 3.1415926535897932384626433832795
const vec2 samples[5]=vec2[](
    vec2(-0.5, 0.023792),
    vec2(-0.25, 0.094907),
    vec2(0.0,  0.150342),
    vec2(0.25,  0.094907),
    vec2(0.5,  0.023792)
);

float sigmoid(float x)
{
    float t = (x - 0.6666667) * 18.0;
    return (0.5 * tanh(0.5 * t) + 0.5);
}

float rand(vec2 coord)
{
  // This one-liner can be found in many places, including:
  // http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
  // I can't find any explanation for it, but experimentally it does seem to
  // produce approximately uniformly distributed values in the interval [0,1].
  return fract(sin(dot(coord.xy, vec2(12.9898,78.233))) * 43758.5453);
}

float gaussian(vec2 coord, float stddev, float mean)
{
  // Box-Muller method for sampling from the normal distribution
  // http://en.wikipedia.org/wiki/Normal_distribution#Generating_values_from_normal_distribution
  // This method requires 2 uniform random inputs and produces 2 
  // Gaussian random outputs. 3rd random variable is used to switch between the two outputs.
  float U, V, R, Z;
  // Add in the CPU-supplied random noiseSeed to generate the 3 random values.
  U = rand(coord + vec2(noiseSeed.x, noiseSeed.x))+0.0000001;
  V = rand(coord + vec2(noiseSeed.y, noiseSeed.y))+0.0000001;
  R = rand(coord + vec2(noiseSeed.z, noiseSeed.z))+0.0000001;
  // Switch between the two random outputs.
  if(R < 0.5)
    Z = sqrt(-2.0 * log(U)) * sin(2.0 * M_PI * V);
  else
    Z = sqrt(-2.0 * log(U)) * cos(2.0 * M_PI * V);

  // Apply the stddev and mean.
  Z = Z * stddev + mean;

  // Return it as a vec4, to be added to the input ("true") color.
  return Z;
}

void main()
{
    //Calculate beam texture coordinate
    float angle = gl_FragCoord.x * beamAngleStep - halfFOV;
    vec2 sampleCoord;
    //sampleCoord.x = d * tan(angle) + 0.5;
    
    float angleCoord[5];
    for(int i=0; i<5; ++i)
    {
        angleCoord[i] = d * tan(angle + samples[i].x * beamAngleStep) + 0.5;
        angleCoord[i] = clamp(angleCoord[i], 0.0, 1.0);
    }
    
    //Calculate current bin ranges
    float binRangeMin = (gl_FragCoord.y-0.5) * binRangeStep + rangeMin;
    float binRangeMax = binRangeMin + binRangeStep;
    
    //Sample beam vertically
    float binHits = 0.0;
    sonarData = 0.0;
    sampleCoord.y = 0.0;
    
    for(int i=0; i<beamSamples; ++i)
    {
        for(int k=0; k<5; ++k)
        {
            vec2 rangeIntensity = texture(texRangeIntensity, vec2(angleCoord[k], sampleCoord.y)).xy;
            
            if(rangeIntensity.x >= binRangeMin && rangeIntensity.x < binRangeMax)
            {
                sonarData += samples[k].y * sigmoid(rangeIntensity.y);
                binHits += samples[k].y;
            }
        }
        
        sampleCoord.y += beamSampleStep;
    }
    
    if(binHits > 0.0)
        sonarData = sonarData/binHits*gaussian(texcoord.xx, noiseStddev.x, 1.0); //Multiplicative noise
    sonarData += gaussian(-texcoord, noiseStddev.y, 0.0); //Additive noise
    sonarData = clamp(sonarData, 0.0, 1.0);
}
