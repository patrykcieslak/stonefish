#version 430 core
uniform sampler2D texSpectrum12;
uniform sampler2D texSpectrum34;
uniform float varianceSize;
uniform int fftSize;
uniform vec4 gridSizes;
uniform float slopeVarianceDelta;
uniform float c;

in vec2 texcoord;
out vec2 fragColor;

#define M_PI 3.14159265

vec2 getSlopeVariances(vec2 k, float A, float B, float C, vec2 spectrumSample) 
{
    float w = 1.0 - exp(A * k.x * k.x + B * k.x * k.y + C * k.y * k.y);
    vec2 kw = k * w;
    return kw * kw * dot(spectrumSample, spectrumSample) * 2.0;
}

void main() 
{
    const float SCALE = 10.0;
    float a = floor(texcoord.x * varianceSize);
    float b = floor(texcoord.y * varianceSize);
    float A = pow(a / (varianceSize - 1.0), 4.0) * SCALE;
    float C = pow(c / (varianceSize - 1.0), 4.0) * SCALE;
    float B = (2.0 * b / (varianceSize - 1.0) - 1.0) * sqrt(A * C);
    A = -0.5 * A;
    B = - B;
    C = -0.5 * C;

    vec2 slopeVariances = vec2(slopeVarianceDelta);
    for (int y = 0; y < fftSize; ++y) 
	{
        for (int x = 0; x < fftSize; ++x) 
		{
            int i = x >= fftSize / 2 ? x - fftSize : x;
            int j = y >= fftSize / 2 ? y - fftSize : y;
            vec2 k = 2.0 * M_PI * vec2(i, j);

            vec4 spectrum12 = texture(texSpectrum12, vec2(float(x) + 0.5, float(y) + 0.5) / float(fftSize));
            vec4 spectrum34 = texture(texSpectrum34, vec2(float(x) + 0.5, float(y) + 0.5) / float(fftSize));

            slopeVariances += getSlopeVariances(k / gridSizes.x, A, B, C, spectrum12.xy);
            slopeVariances += getSlopeVariances(k / gridSizes.y, A, B, C, spectrum12.zw);
            slopeVariances += getSlopeVariances(k / gridSizes.z, A, B, C, spectrum34.xy);
            slopeVariances += getSlopeVariances(k / gridSizes.w, A, B, C, spectrum34.zw);
        }
    }
    fragColor = slopeVariances;
}
