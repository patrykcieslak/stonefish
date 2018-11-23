#version 330

uniform sampler2D texSpectrum12;
uniform sampler2D texSpectrum34;
uniform vec4 invGridSizes;
uniform float fftSize;
uniform float zoom;
uniform float linear;

in vec2 texcoord;
out vec4 fragColor;

void main() 
{
    vec2 st = texcoord - vec2(0.5);
    float r = length(st);
    float k = pow(10.0, -3.0 + 12.0 * r);

    vec2 kxy = st * pow(10.0, zoom * 3.0);

    float S = 0.0;
    if (abs(kxy.x) < invGridSizes.x && abs(kxy.y) < invGridSizes.x) 
	{
        st = 0.5 * kxy / invGridSizes.x + 0.5 / fftSize;
        S += length(texture(texSpectrum12, st).xy) * fftSize / invGridSizes.x;
    }
    if (abs(kxy.x) < invGridSizes.y && abs(kxy.y) < invGridSizes.y) 
	{
        st = 0.5 * kxy / invGridSizes.y + 0.5 / fftSize;
        S += length(texture(texSpectrum12, st).zw) * fftSize / invGridSizes.y;
    }
    if (abs(kxy.x) < invGridSizes.z && abs(kxy.y) < invGridSizes.z) 
	{
        st = 0.5 * kxy / invGridSizes.z + 0.5 / fftSize;
        S += length(texture(texSpectrum34, st).xy) * fftSize / invGridSizes.z;
    }
    if (abs(kxy.x) < invGridSizes.w && abs(kxy.y) < invGridSizes.w) 
	{
        st = 0.5 * kxy / invGridSizes.w + 0.5 / fftSize;
        S += length(texture(texSpectrum34, st).zw) * fftSize / invGridSizes.w;
    }
    S = S * S * 0.5;

    float s;
    if (linear > 0.0) 
	{
        s = S * 100.0; // linear scale in intensity
    } 
	else 
	{
        s = (log(S) / log(10.0) + 15.0) / 18.0; // logarithmic scale in intensity
    }

    fragColor = vec4(s);
}
