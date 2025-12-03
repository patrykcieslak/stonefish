/* 
    Diretional JONSWAP spectrum with TMA correction
*/

struct OceanSpectrum
{
    float scale;            // Overall scale factor
    float windSpeed;        // Wind speed [m/s]
    float windDirection;    // Wind direction [rad]
    float windFetchLength;  // Wind fetch length [m]
    float depth;            // Water depth [m]
    float peakOmega;        // Peak angular frequency [rad/s]
    float alpha;            // Phillips spectrum constant
    float gamma;            // Peak enhancement factor
    float swell;            // Swell factor
    float spreadBlend;      // Spreading blend factor
    float shortWavesFade;   // Short waves fade factor
    float g;                // Gravitational acceleration [m/s^2]
};

layout (std140) uniform OceanSpectrumParameters
{
    OceanSpectrum params;
};

// Dispertion relation describes angular frequency for any given wave number,
// which tell us how fast a wave of a given waelength propagates.
float DispertionRelation(float kMag)
{
    return sqrt(params.g * kMag * tanh(kMag * params.depth));
}

// Derivative of the dispertion relation for a finite depth
float DispertionRelationDerivative(float kMag)
{
    float kh = kMag * params.depth;
    float tanhKH = tanh(kh);
    float sechKH = 1.0 / cosh(clamp(kh, -9.0, 9.0)); // approx sech(x) = 1/cosh(x) (to maintain precision)
    return params.g * (tanhKH + kh * sechKH * sechKH) / (2.0 * sqrt(params.g * kMag * tanhKH));
}

// Texel-Marsel-Arsloe (TMA) correction to JONSWAP spectrum for finite depth
float TMACorrection(float omega) 
{
    float omegaH = omega * sqrt(params.depth / params.g);
	if (omegaH <= 1.0)
		return 0.5 * omegaH * omegaH;
	else if (omegaH < 2.0)
		return 1.0 - 0.5 * (2.0 - omegaH) * (2.0 - omegaH);
    else 
        return 1.0;
}

// Joint North Sea Wave Observation Project (JONSWAP) spectrum
float JONSWAPSpectrum(float omega)
{
    // alpha and peakOmega computed once when parameters are set
    float sigma = (omega <= params.peakOmega) ? 0.07 : 0.09; // Surface tension coefficient [N]
	float r = exp(- (omega - params.peakOmega) * (omega - params.peakOmega) 
                            / (2.0 * sigma * sigma * params.peakOmega * params.peakOmega));
    
	// Divisions & powers
	float oneOverOmega = 1.0 / (omega + 1e-6);
	float peakOmegaOverOmega = params.peakOmega / omega;
	float oneOverOmega5 = oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega;
    float peakOmegaOverOmega4 = peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega;

    // Final spectrum
    return params.scale * TMACorrection(omega) 
        * params.alpha * params.g * params.g * oneOverOmega5 * exp(-1.25 * peakOmegaOverOmega4) * pow(params.gamma, r);
}

// Directional spreading
float NormalizationFactor(float s) 
{
    float s2 = s * s;
    float s3 = s2 * s;
    float s4 = s3 * s;
    if (s < 5) 
        return -0.000564 * s4 + 0.00776 * s3 - 0.044 * s2 + 0.192 * s + 0.163;
    else 
        return -4.80e-08 * s4 + 1.07e-05 * s3 - 9.53e-04 * s2 + 5.90e-02 * s + 3.93e-01;
}

float Cosine2s(float theta, float s) 
{
	return NormalizationFactor(s) * pow(abs(cos(0.5 * theta)), 2.0 * s);
}

float SpreadPower(float omega, float peakOmega) 
{
	if (omega > peakOmega)
		return 9.77 * pow(abs(omega / peakOmega), -2.5);
	else
		return 6.97 * pow(abs(omega / peakOmega), 5.0);
}

float DirectionalSpreadingFunction(float theta, float omega) 
{
	float s = SpreadPower(omega, params.peakOmega) + 16.0 * tanh(min(omega / params.peakOmega, 20.0)) * params.swell * params.swell;
	
	return mix(TWO_OVER_PI * cos(theta) * cos(theta), Cosine2s(theta - params.windDirection, s), params.spreadBlend);
}

float ShortWavesFade(float kLength) 
{
	return exp(-params.shortWavesFade * params.shortWavesFade * kLength * kLength);
}

float DirectionalSpectrum(vec2 k, float kLength)
{
    float kAngle = atan(k.y, k.x);
    float omega = DispertionRelation(kLength);
    return JONSWAPSpectrum(omega) * DirectionalSpreadingFunction(kAngle, omega) * ShortWavesFade(kLength);
}
