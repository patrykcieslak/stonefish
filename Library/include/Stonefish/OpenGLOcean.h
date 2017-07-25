//
//  OpenGLOcean.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/07/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLOcean__
#define __Stonefish_OpenGLOcean__

#include "OpenGLContent.h"

struct OceanParams
{
	unsigned int passes;
	unsigned int slopeVarianceSize;
	int fftSize;
	glm::vec4 gridSizes;
	float* spectrum12;
	float* spectrum34;
	glm::vec4 choppyFactor;
	bool propagate;
	float wind;
	float omega;
	float km;
	float cm;
	float A;
	float t;
};

class OpenGLOcean 
{
public:
	OpenGLOcean();
	~OpenGLOcean();
	
	void InitOcean();
	void SimulateOcean();
	void DrawOceanSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection);
	void ShowOceanSpectrum(glm::vec2 viewportSize, glm::vec4 rect);
	void ShowOceanTexture(int id, glm::vec4 rect);

private:	
	GLfloat* ComputeButterflyLookupTable(unsigned int size, unsigned int passes);
	int bitReverse(int i, int N);
	void computeWeight(int N, int k, float &Wr, float &Wi);
	float ComputeSlopeVariance();
	float GetSlopeVariance(float kx, float ky, float *spectrumSample);
	void GenerateWavesSpectrum();
	void GetSpectrumSample(int i, int j, float lengthScale, float kMin, float *result);
	float spectrum(float kx, float ky, bool omnispectrum = false);
	float omega(float k);
	float sqr(float x);

	GLSLShader* oceanShaders[8]; //surface, volume, init, fftx, ffty, variance, choppy, show spectrum
	GLuint oceanFBOs[3];
	GLuint oceanTextures[7]; //spectrum12, spectrum34, slope, fft ping, fft pong, butterfly, gaussz
	GLuint oceanViewTextures[8];
	
	OceanParams params;
	QuadTree qt;
	int64_t lastTime;
};

#endif