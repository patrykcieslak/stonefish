#version 430 core

uniform sampler2DArray texWaveFFT;
uniform sampler3D texSlopeVariance;
uniform vec4 gridSizes;
uniform vec4 choppyFactor;
uniform vec3 eyePos;
uniform mat3 MV;

in vec2 waveCoord;
in vec3 fragPos;
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;

// V, N in world space
/*float meanFresnel(vec3 V, vec3 N, vec2 sigmaSq) {
    vec2 v = V.xy; // view direction in wind space
    vec2 t = v * v / (1.0 - V.z * V.z); // cos^2 and sin^2 of view direction
    float sigmaV2 = dot(t, sigmaSq); // slope variance in view direction
    return meanFresnel(dot(V, N), sqrt(sigmaV2));
}*/

void main()
{
	vec3 toEye = normalize(eyePos - fragPos);
	
	//Wave slope (layers 1,2)
	vec2 slopes = texture(texWaveFFT, vec3(waveCoord/gridSizes.x, 1.0)).xy;
	slopes += texture(texWaveFFT, vec3(waveCoord/gridSizes.y, 1.0)).zw;
	slopes += texture(texWaveFFT, vec3(waveCoord/gridSizes.z, 2.0)).xy;
	slopes += texture(texWaveFFT, vec3(waveCoord/gridSizes.w, 2.0)).zw;
	
	//Choppy wave component (layers 5,6)
	// Jxx1..4 : partial Jxx
	float Jxx1 = texture(texWaveFFT, vec3(waveCoord/gridSizes.x, 5.0)).r;
	float Jxx2 = texture(texWaveFFT, vec3(waveCoord/gridSizes.y, 5.0)).g;
	float Jxx3 = texture(texWaveFFT, vec3(waveCoord/gridSizes.z, 5.0)).b;
	float Jxx4 = texture(texWaveFFT, vec3(waveCoord/gridSizes.w, 5.0)).a;
	float Jxxc = dot((choppyFactor), vec4(Jxx1, Jxx2, Jxx3, Jxx4));

	// Jyy1..4 : partial Jyy
	float Jyy1 = texture(texWaveFFT, vec3(waveCoord/gridSizes.x, 6.0)).r;
	float Jyy2 = texture(texWaveFFT, vec3(waveCoord/gridSizes.y, 6.0)).g;
	float Jyy3 = texture(texWaveFFT, vec3(waveCoord/gridSizes.z, 6.0)).b;
	float Jyy4 = texture(texWaveFFT, vec3(waveCoord/gridSizes.w, 6.0)).a;
	float Jyyc = dot((choppyFactor), vec4(Jyy1, Jyy2, Jyy3, Jyy4));

	slopes /= (1.0 + vec2(Jxxc, Jyyc));
	
	//Normals
	vec3 normal = normalize(vec3(-slopes.x, -slopes.y, 1.0)); //-1 or 1 ???
	if(dot(toEye, normal) < 0.0)
		normal = reflect(normal, toEye); //Reflect backfacing normals
	
	//
	float Jxx = dFdx(waveCoord.x);
	float Jxy = dFdy(waveCoord.x);
	float Jyx = dFdx(waveCoord.y);
	float Jyy = dFdy(waveCoord.y);
	float A = Jxx * Jxx + Jyx * Jyx;
	float B = Jxx * Jxy + Jyx * Jyy;
	float C = Jxy * Jxy + Jyy * Jyy;
	const float SCALE = 10.0;
	float ua = pow(A / SCALE, 0.25);
	float ub = 0.5 + 0.5 * B / sqrt(A * C);
	float uc = pow(C / SCALE, 0.25);
	vec2 sigmaSq = texture(texSlopeVariance, vec3(ua, ub, uc)).xw;
	sigmaSq = max(sigmaSq, 2e-5);

	vec3 Ty = normalize(vec3(0.0, normal.z, -normal.y));
	vec3 Tx = cross(Ty, normal);

	vec3 Rf = vec3(0.0);
	vec3 Rs = vec3(0.0);
	vec3 Ru = vec3(0.0);
	
	//float fresnel = 0.02 + 0.98 * meanFresnel(toEye, normal, sigmaSq);
	
	
	fragColor = vec3(1.0);//normal;
	fragNormal = normalize(MV * normal);
}