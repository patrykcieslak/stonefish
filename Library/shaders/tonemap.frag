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
layout(location = 0) out vec4 fragColor;
uniform sampler2D texSource;
uniform sampler2D texExposure;
uniform float exposureComp;

#define RGB_TO_LUM vec3(0.2125, 0.7154, 0.0721)

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
vec3 ACESInput(vec3 c)
{
    vec3 cout;
    cout.r = dot(c, vec3(0.59719, 0.35458, 0.04823));
    cout.g = dot(c, vec3(0.07600, 0.90834, 0.01566));
    cout.b = dot(c, vec3(0.02840, 0.13383, 0.83777));
    return cout;
}

// ODT_SAT => XYZ => D60_2_D65 => sRGB
vec3 ACESOutput(vec3 c)
{
    vec3 cout;
    cout.r = dot(c, vec3(1.60475, -0.53108, -0.07367));
    cout.g = dot(c, vec3(-0.10208, 1.10813, -0.00605));
    cout.b = dot(c, vec3(-0.00327, -0.07276,  1.07602));
    return cout;
}

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 ACESFitted(vec3 color)
{
    color = ACESInput(color);
    color = RRTAndODTFit(color);  // Apply RRT and ODT
    color = ACESOutput(color);
    color = clamp(color, 0.0, 1.0);
    return color;
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

vec3 rgbToHsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsvToRgb(vec3 c)
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
    //Read exposure
    float exposure = clamp(texelFetch(texExposure, ivec2(0,0), 0).r, 0.0, 0.0004);
    //Tone mapping
    vec3 color = texture(texSource, texcoord).rgb; 
    color = accurateLinearToSRGB(exposure * exposureComp * color); //Gamma correction
    float lum = dot(color, RGB_TO_LUM); //FXAA
    color = ACESFitted(color); //Filmic tonemapping
    //Saturation bump-up
    vec3 hsv = rgbToHsv(color);
    hsv.y = clamp(hsv.y*1.1, 0.0, 1.0);
    color = hsvToRgb(hsv);
    //Final output
    fragColor = vec4(color, lum);
}