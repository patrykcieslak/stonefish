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
layout(location = 0) out vec4 fragcolor;

uniform sampler2D texHDR;
uniform sampler2D texAverage;
uniform float exposureComp;

const vec3 rgbToLum = vec3(0.212671, 0.715160, 0.072169); // vec3(0.299, 0.587, 0.114)

float computeEV100(float aperture, float shutterTime , float ISO)
{
    //EV number is defined as:
    // 2^ EV_s = N ^2 / t    and     EV_s = EV_100 + log2 ( S /100)
    // This gives
    // EV_s = log2 ( N ^2 / t )
    // EV_100 + log2 ( S /100) = log2 ( N ^2 / t )
    // EV_100 = log2 ( N ^2 / t ) - log2 ( S /100)
    // EV_100 = log2 ( N ^2 / t . 100 / S )
    return log2(aperture*aperture/shutterTime*100.0/ISO);
}

float computeEV100FromAvgLuminance(float avgLuminance)
{
    // We later use the middle gray at 12.7% in order to have
    // a middle gray at 18% with a sqrt (2) room for specular highlights
    // But here we deal with the spot meter measuring the middle gray
    // which is fixed at 12.5 for matching standard camera
    // constructor settings ( i . e . calibration constant K = 12.5)
    // Reference: http://en.wikipedia.org/wiki/Film_speed
    return log2(avgLuminance*100.0/12.5);
}

float convertEV100ToExposure(float EV100)
{
    // Compute the maximum luminance possible with H_sbs sensitivity
    // maxLum = 78 / ( S * q ) * N ^2 / t
    //        = 78 / ( S * q ) * 2^ EV_100
    //        = 78 / (100 * 0.65) * 2^ EV_100
    //        = 1.2 * 2^ EV
    // Reference: http://en.wikipedia.org/wiki/Film_speed
    float maxLuminance = 1.2*pow(2.0, EV100);
    return 1.0/maxLuminance;
}

vec3 approximationSRGBToLinear(vec3 sRGBCol)
{
    return pow(sRGBCol, vec3(2.2));
}

vec3 approximationLinearToSRGB(vec3 linearCol)
{
    return pow(linearCol, vec3(1.0/2.2));
}

vec3 accurateSRGBToLinear(vec3 sRGBCol)
{
    vec3 linearRGBLo = sRGBCol/12.92;
    vec3 linearRGBHi = pow((sRGBCol + 0.055)/1.055, vec3(2.4));
    vec3 linearRGB = linearRGBHi;
    if(sRGBCol.r <= 0.04045) linearRGB.r = linearRGBLo.r;
    if(sRGBCol.g <= 0.04045) linearRGB.g = linearRGBLo.g;
    if(sRGBCol.b <= 0.04045) linearRGB.b = linearRGBLo.b;
    return linearRGB;
}

vec3 accurateLinearToSRGB(vec3 linearCol)
{
    vec3 sRGBLo = linearCol * 12.92;
    vec3 sRGBHi = (pow(abs(linearCol), vec3(1.0/2.4)) * 1.055) - 0.055;
    vec3 sRGB = sRGBHi;
    if(linearCol.r <= 0.0031308) sRGB.r = sRGBLo.r;
    if(linearCol.g <= 0.0031308) sRGB.g = sRGBLo.g;
    if(linearCol.b <= 0.0031308) sRGB.b = sRGBLo.b;
    return sRGB;
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// Convert ITU-R Rec. BT.709 linear RGB to XYZ tristimulus values.
vec3 rgbToXyz(vec3 c)
{
    vec3 cout;
    cout.x = dot(c, vec3(0.412453, 0.357580, 0.180423));
    cout.y = dot(c, vec3(0.212671, 0.715160, 0.072169));
    cout.z = dot(c, vec3(0.019334, 0.119193, 0.950227));
    return cout;
}

// Convert from XYZ tristimulus values to ITU-R Rec. BT.709 linear RGB.
vec3 xyzToRgb(vec3 c)
{
    vec3 cout;
    cout.r = dot(c, vec3(3.240479, -1.537150, -0.498535));
    cout.g = dot(c, vec3(-0.969256, 1.875991, 0.041556));
    cout.b = dot(c, vec3(0.055648, -0.204043, 1.057311));
    return cout;
}

vec3 Uncharted2TonemapFunction(const vec3 x)
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 Uncharted2Tonemap(const float exposure, const vec3 color)
{
    const float W = 11.2;
    vec3 curr = Uncharted2TonemapFunction(exposure * color);
    vec3 whiteScale = 1.0 / Uncharted2TonemapFunction(vec3(W));
    return curr * whiteScale;
}

vec3 Reinhard(vec3 color, float burn, float scale, float Lwhite)
{
    //Adjust burn
    burn = min(1.0, max(1e-8, 1.0 - burn));

    //Having the 'burn' parameter scale as 1/b^4 provides a nicely behaved knob.
    float invWp2 = 1.0 / (Lwhite * Lwhite * pow(burn, 4.0));

    //Convert to XYZ
    vec3 c = rgbToXyz(color);

    //Convert to xyY
    float normalization = 1.0/(c.x + c.y + c.z);
    float x = c.x * normalization;
    float y = c.y * normalization;
    float Lp = c.y * scale;
 
    //Apply the tonemapping transformation
    float Y = Lp * (1.0 + Lp * invWp2)/(1.0 + Lp);

    //Convert back to XYZ
    float ratio = Y/y;
    float X = ratio * x;
    float Z = ratio * (1.0 - x - y);

    //Convert back to RGB
    return xyzToRgb(vec3(X,Y,Z));
}

void main(void)
{
    //Read textures
    float Lavg = texture(texAverage, vec2(0.5, 0.5)).r;
    float Lmax = max( max(texelFetch(texAverage, ivec2(0,0), 0).g, texelFetch(texAverage, ivec2(1,0), 0).g),
                      max(texelFetch(texAverage, ivec2(0,1), 0).g, texelFetch(texAverage, ivec2(1,1), 0).g) );
    vec3 rgbColor = texture(texHDR, texcoord).rgb;
    
    //Expose
    float autoEV100 = computeEV100(8.0, 1.0/1000.0, 100.0); //computeEV100FromAvgLuminance(Lavg); 
    float exposure = convertEV100ToExposure(autoEV100) * exposureComp * 16.0;

    //Tonemap
    rgbColor = Uncharted2Tonemap(exposure, rgbColor); 
    /*float burn = 0.1;
    float scale = exposure / Lavg;
    float Lwhite = Lmax * scale;
    rgbColor = Reinhard(rgbColor, burn, scale, Lwhite);*/

    //Gamma correct
    rgbColor = accurateLinearToSRGB(rgbColor); //Constrains output to <0.0, 1.0>

    //FXAA luminance
    float lum = dot(rgbColor, rgbToLum); //sqrt for linear, without sqrt for gamma 2.0
    
    fragcolor = vec4(rgbColor, lum);
}
