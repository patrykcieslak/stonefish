//
//  OpenGLOcean.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/07/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "OpenGLOcean.h"
#include "Console.h"
#include "SystemUtil.hpp"

OpenGLOcean::OpenGLOcean()
{
	qt.root.size = glm::vec2(10000.f,10000.f);
	for(unsigned short i=0; i<3; ++i) oceanFBOs[i] = 0;
	for(unsigned short i=0; i<8; ++i) oceanShaders[i] = NULL;
	for(unsigned short i=0; i<7; ++i) oceanTextures[i] = 0;
}

OpenGLOcean::~OpenGLOcean()
{
	for(unsigned short i=0; i<3; ++i) if(oceanFBOs[i] != 0) glDeleteFramebuffers(1, &oceanFBOs[i]);
	for(unsigned short i=0; i<8; ++i) if(oceanShaders[i] != NULL) delete oceanShaders[i];
	for(unsigned short i=0; i<7; ++i) if(oceanTextures[i] != 0) glDeleteTextures(1, &oceanTextures[i]);
	if(params.spectrum12 != NULL) delete[] params.spectrum12;
	if(params.spectrum34 != NULL) delete[] params.spectrum34;
}

void OpenGLOcean::InitOcean()
{
	cInfo("Generating ocean waves...");
	
	//---textures
	params.passes = 8;
	params.slopeVarianceSize = 4;
	params.fftSize = 1 << params.passes;
	params.choppy = true;
	params.propagate = true;
	params.wind = 1.f;
	params.omega = 1.f;
	params.km = 370.f;
	params.cm = 0.23f;
	params.A = 2.f;
	params.t = 0.f;
	params.gridSizes = glm::vec4(893.f, 101.f, 21.f, 11.f);
	params.spectrum12 = NULL;
	params.spectrum34 = NULL;
	GenerateWavesSpectrum();
	
	glGenTextures(7, oceanTextures);
	
	glBindTexture(GL_TEXTURE_2D, oceanTextures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, params.fftSize, params.fftSize, 0, GL_RGBA, GL_FLOAT, params.spectrum12);

	glBindTexture(GL_TEXTURE_2D, oceanTextures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, params.fftSize, params.fftSize, 0, GL_RGBA, GL_FLOAT, params.spectrum34);
	
	glBindTexture(GL_TEXTURE_2D, oceanTextures[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	GLfloat* data = ComputeButterflyLookupTable(params.fftSize, params.passes);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, params.fftSize, params.passes, 0, GL_RGBA, GL_FLOAT, data);
	delete[] data;
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBindTexture(GL_TEXTURE_3D, oceanTextures[2]);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE_ALPHA16F_ARB, params.slopeVarianceSize, params.slopeVarianceSize, params.slopeVarianceSize, 0, GL_LUMINANCE_ALPHA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_3D, 0);
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, params.fftSize, params.fftSize, 10, 0, GL_RGBA, GL_FLOAT, NULL); 
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, params.fftSize, params.fftSize, 10, 0, GL_RGBA, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		
	//---framebuffers
	glGenFramebuffers(3, oceanFBOs);
	glBindFramebuffer(GL_FRAMEBUFFER, oceanFBOs[0]);
	GLenum drawBuffers[8] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5,
		GL_COLOR_ATTACHMENT6,
		GL_COLOR_ATTACHMENT7
	};
	glDrawBuffers(8, drawBuffers);
	
	glBindFramebuffer(GL_FRAMEBUFFER, oceanFBOs[1]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, oceanTextures[3], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, oceanTextures[4], 0);
	
	//---shaders
	oceanShaders[0] = new GLSLShader("flat.frag", "flat.vert", "", std::make_pair("quadTree.tcs", "oceanSurface.tes"));
	oceanShaders[0]->AddUniform("MVP", ParameterType::MAT4);
	
	oceanShaders[2] = new GLSLShader("oceanInit.frag"); //Using saq vertex shader
	oceanShaders[2]->AddUniform("texSpectrum12", ParameterType::INT);
	oceanShaders[2]->AddUniform("texSpectrum34", ParameterType::INT);
	oceanShaders[2]->AddUniform("fftSize", ParameterType::FLOAT);
	oceanShaders[2]->AddUniform("inverseGridSizes", ParameterType::VEC4);
	oceanShaders[2]->AddUniform("t", ParameterType::FLOAT);
	
	oceanShaders[3] = new GLSLShader("oceanFFTX.frag", "", "oceanFFT.geom"); //Using saq vertex shader
	oceanShaders[3]->AddUniform("nLayers", ParameterType::INT);
	oceanShaders[3]->AddUniform("sLayer", ParameterType::INT);
	oceanShaders[3]->AddUniform("texButterfly", ParameterType::INT);
	oceanShaders[3]->AddUniform("texSource", ParameterType::INT);
	oceanShaders[3]->AddUniform("pass", ParameterType::FLOAT);
	
	oceanShaders[4] = new GLSLShader("oceanFFTY.frag", "", "oceanFFT.geom"); //Using saq vertex shader
	oceanShaders[4]->AddUniform("nLayers", ParameterType::INT);
	oceanShaders[4]->AddUniform("sLayer", ParameterType::INT);
	oceanShaders[4]->AddUniform("texButterfly", ParameterType::INT);
	oceanShaders[4]->AddUniform("texSource", ParameterType::INT);
	oceanShaders[4]->AddUniform("pass", ParameterType::FLOAT);
	
	oceanShaders[5] = new GLSLShader("oceanVariance.frag"); //Using saq vertex shader
	oceanShaders[5]->AddUniform("texSpectrum12", ParameterType::INT);
	oceanShaders[5]->AddUniform("texSpectrum34", ParameterType::INT);
	oceanShaders[5]->AddUniform("varianceSize", ParameterType::FLOAT);
	oceanShaders[5]->AddUniform("fftSize", ParameterType::INT);
	oceanShaders[5]->AddUniform("gridSizes", ParameterType::VEC4);
	oceanShaders[5]->AddUniform("slopeVarianceDelta", ParameterType::FLOAT);
	oceanShaders[5]->AddUniform("c", ParameterType::FLOAT);
	
	oceanShaders[7] = new GLSLShader("oceanSpectrum.frag", "texQuad.vert");
	oceanShaders[7]->AddUniform("texSpectrum12", ParameterType::INT);
	oceanShaders[7]->AddUniform("texSpectrum34", ParameterType::INT);
	oceanShaders[7]->AddUniform("invGridSizes", ParameterType::VEC4);
	oceanShaders[7]->AddUniform("fftSize", ParameterType::FLOAT);
	oceanShaders[7]->AddUniform("zoom", ParameterType::FLOAT);
	oceanShaders[7]->AddUniform("linear", ParameterType::FLOAT);
	oceanShaders[7]->AddUniform("rect", ParameterType::VEC4);
	
	//---generate variances
	float slopeVarianceDelta = ComputeSlopeVariance();
	
	glBindFramebuffer(GL_FRAMEBUFFER, oceanFBOs[2]);
    glViewport(0, 0, params.slopeVarianceSize, params.slopeVarianceSize);

    oceanShaders[5]->Use();
	oceanShaders[5]->SetUniform("texSpectrum12", TEX_POSTPROCESS1);
	oceanShaders[5]->SetUniform("texSpectrum34", TEX_POSTPROCESS2);
	oceanShaders[5]->SetUniform("varianceSize", (GLfloat)params.slopeVarianceSize);
	oceanShaders[5]->SetUniform("fftSize", params.fftSize);
	oceanShaders[5]->SetUniform("gridSizes", params.gridSizes);
	oceanShaders[5]->SetUniform("slopeVarianceDelta", slopeVarianceDelta);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[0]);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, oceanTextures[1]);
	
    for(int layer = 0; layer < params.slopeVarianceSize; ++layer)
    {
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, oceanTextures[2], 0, layer);
		oceanShaders[5]->SetUniform("c", (GLfloat)layer);
		OpenGLContent::getInstance()->DrawSAQ();
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int OpenGLOcean::bitReverse(int i, int N)
{
	int j = i;
	int M = N;
	int Sum = 0;
	int W = 1;
	M = M / 2;
	while (M != 0)
	{
		j = (i & M) > M - 1;
		Sum += j * W;
		W *= 2;
		M = M / 2;
	}
	return Sum;
}

void OpenGLOcean::computeWeight(int N, int k, float &Wr, float &Wi)
{
    Wr = cosl(2.f * M_PI * k / float(N));
    Wi = sinl(2.f * M_PI * k / float(N));
}

GLfloat* OpenGLOcean::ComputeButterflyLookupTable(unsigned int size, unsigned int passes)
{
	GLfloat *data = new GLfloat[size * passes * 4];

	for(int i = 0; i < passes; ++i)
	{
		int nBlocks  = (int)powf(2.0, float(passes - 1 - i));
		int nHInputs = (int)powf(2.0, float(i));
		for(int j = 0; j < nBlocks; ++j)
		{
		    for(int k = 0; k < nHInputs; ++k)
		    {
		        int i1, i2, j1, j2;
		        if(i == 0)
		        {
		            i1 = j * nHInputs * 2 + k;
		            i2 = j * nHInputs * 2 + nHInputs + k;
		            j1 = bitReverse(i1, size);
		            j2 = bitReverse(i2, size);
		        }
		        else
		        {
		            i1 = j * nHInputs * 2 + k;
		            i2 = j * nHInputs * 2 + nHInputs + k;
		            j1 = i1;
		            j2 = i2;
		        }

		        float wr, wi;
		        computeWeight(size, k * nBlocks, wr, wi);

		        int offset1 = 4 * (i1 + i * size);
		        data[offset1 + 0] = (j1 + 0.5) / size;
		        data[offset1 + 1] = (j2 + 0.5) / size;
		        data[offset1 + 2] = wr;
		        data[offset1 + 3] = wi;

		        int offset2 = 4 * (i2 + i * size);
		        data[offset2 + 0] = (j1 + 0.5) / size;
		        data[offset2 + 1] = (j2 + 0.5) / size;
		        data[offset2 + 2] = -wr;
		        data[offset2 + 3] = -wi;
		    }
		}
	}

	return data;
}

//Wave generation
float OpenGLOcean::sqr(float x)
{
    return x * x;
}

float OpenGLOcean::omega(float k)
{
    return sqrt(9.81 * k * (1.0 + sqr(k / params.km))); // Eq 24
}

// 1/kx and 1/ky in meters
float OpenGLOcean::spectrum(float kx, float ky, bool omnispectrum)
{
    float U10 = params.wind;
    float Omega = params.omega;

    // phase speed
    float k = sqrt(kx * kx + ky * ky);
    float c = omega(k) / k;

    // spectral peak
    float kp = 9.81 * sqr(Omega / U10); // after Eq 3
    float cp = omega(kp) / kp;

    // friction velocity
    float z0 = 3.7e-5 * sqr(U10) / 9.81 * pow(U10 / cp, 0.9f); // Eq 66
    float u_star = 0.41 * U10 / log(10.0 / z0); // Eq 60

    float Lpm = exp(- 5.0 / 4.0 * sqr(kp / k)); // after Eq 3
    float gamma = Omega < 1.0 ? 1.7 : 1.7 + 6.0 * log(Omega); // after Eq 3 // log10 or log??
    float sigma = 0.08 * (1.0 + 4.0 / pow(Omega, 3.0f)); // after Eq 3
    float Gamma = exp(-1.0 / (2.0 * sqr(sigma)) * sqr(sqrt(k / kp) - 1.0));
    float Jp = pow(gamma, Gamma); // Eq 3
    float Fp = Lpm * Jp * exp(- Omega / sqrt(10.0) * (sqrt(k / kp) - 1.0)); // Eq 32
    float alphap = 0.006 * sqrt(Omega); // Eq 34
    float Bl = 0.5 * alphap * cp / c * Fp; // Eq 31

    float alpham = 0.01 * (u_star < params.cm ? 1.0 + log(u_star / params.cm) : 1.0 + 3.0 * log(u_star / params.cm)); // Eq 44
    float Fm = exp(-0.25 * sqr(k / params.km - 1.0)); // Eq 41
    float Bh = 0.5 * alpham * params.cm / c * Fm; // Eq 40

    Bh *= Lpm; 

    if (omnispectrum)
    {
        return params.A * (Bl + Bh) / (k * sqr(k)); // Eq 30
    }

    float a0 = log(2.0) / 4.0;
    float ap = 4.0;
    float am = 0.13 * u_star / params.cm; // Eq 59
    float Delta = tanh(a0 + ap * pow(c / cp, 2.5f) + am * pow(params.cm / c, 2.5f)); // Eq 57

    float phi = atan2(ky, kx);

    if(params.propagate)
    {
        if (kx < 0.0)
        {
            return 0.0;
        }
        else
        {
            Bl *= 2.0;
            Bh *= 2.0;
        }
    }

	// remove waves perpendicular to wind dir
	float tweak = sqrtf( std::max( kx/sqrtf(kx*kx+ky*ky), 0.f) );
	tweak = 1.0f;
    return params.A * (Bl + Bh) * (1.0 + Delta * cos(2.0 * phi)) / (2.0 * M_PI * sqr(sqr(k))) * tweak; // Eq 67
}

void OpenGLOcean::GetSpectrumSample(int i, int j, float lengthScale, float kMin, float *result)
{
    static long seed = 1234;
    float dk = 2.0 * M_PI / lengthScale;
    float kx = i * dk;
    float ky = j * dk;
    if(fabsf(kx) < kMin && fabsf(ky) < kMin)
    {
        result[0] = 0.0;
        result[1] = 0.0;
    }
    else
    {
        float S = spectrum(kx, ky);
        float h = sqrtf(S / 2.0) * dk;
        float phi = frandom(&seed) * 2.0 * M_PI;
        result[0] = h * cos(phi);
        result[1] = h * sin(phi);
    }
}

// generates the waves spectrum
void OpenGLOcean::GenerateWavesSpectrum()
{
    if(params.spectrum12 != NULL)
    {
        delete[] params.spectrum12;
        delete[] params.spectrum34;
    }
    params.spectrum12 = new float[params.fftSize * params.fftSize * 4];
    params.spectrum34 = new float[params.fftSize * params.fftSize * 4];

    for (int y = 0; y < params.fftSize; ++y)
    {
        for (int x = 0; x < params.fftSize; ++x)
        {
            int offset = 4 * (x + y * params.fftSize);
            int i = x >= params.fftSize / 2 ? x - params.fftSize : x;
            int j = y >= params.fftSize / 2 ? y - params.fftSize : y;
            GetSpectrumSample(i, j, params.gridSizes[0], M_PI / params.gridSizes[0], params.spectrum12 + offset);
            GetSpectrumSample(i, j, params.gridSizes[1], M_PI * params.fftSize / params.gridSizes[0], params.spectrum12 + offset + 2);
            GetSpectrumSample(i, j, params.gridSizes[2], M_PI * params.fftSize / params.gridSizes[1], params.spectrum34 + offset);
            GetSpectrumSample(i, j, params.gridSizes[3], M_PI * params.fftSize / params.gridSizes[2], params.spectrum34 + offset + 2);
        }
    }
}

float OpenGLOcean::GetSlopeVariance(float kx, float ky, float *spectrumSample)
{
    float kSquare = kx * kx + ky * ky;
    float real = spectrumSample[0];
    float img = spectrumSample[1];
    float hSquare = real * real + img * img;
    return kSquare * hSquare * 2.0;
}

// precomputes filtered slope variances in a 3d texture, based on the wave spectrum
float OpenGLOcean::ComputeSlopeVariance()
{
    //slope variance due to all waves, by integrating over the full spectrum
    float theoreticSlopeVariance = 0.0;
    float k = 5e-3;
    while (k < 1e3)
    {
        float nextK = k * 1.001;
        theoreticSlopeVariance += k * k * spectrum(k, 0, true) * (nextK - k);
        k = nextK;
    }

    // slope variance due to waves, by integrating over the spectrum part
    // that is covered by the four nested grids. This can give a smaller result
    // than the theoretic total slope variance, because the higher frequencies
    // may not be covered by the four nested grid. Hence the difference between
    // the two is added as a "delta" slope variance in the "variances" shader,
    // to be sure not to lose the variance due to missing wave frequencies in
    // the four nested grids
    float totalSlopeVariance = 0.0;
    for (int y = 0; y < params.fftSize; ++y)
    {
        for (int x = 0; x < params.fftSize; ++x)
        {
            int offset = 4 * (x + y * params.fftSize);
            float i = 2.f * M_PI * (x >= params.fftSize / 2 ? x - params.fftSize : x);
            float j = 2.f * M_PI * (y >= params.fftSize / 2 ? y - params.fftSize : y);
            totalSlopeVariance += GetSlopeVariance(i / params.gridSizes[0], j / params.gridSizes[0], params.spectrum12 + offset);
            totalSlopeVariance += GetSlopeVariance(i / params.gridSizes[1], j / params.gridSizes[1], params.spectrum12 + offset + 2);
            totalSlopeVariance += GetSlopeVariance(i / params.gridSizes[2], j / params.gridSizes[2], params.spectrum34 + offset);
            totalSlopeVariance += GetSlopeVariance(i / params.gridSizes[3], j / params.gridSizes[3], params.spectrum34 + offset + 2);
        }
    }
	
	return theoreticSlopeVariance - totalSlopeVariance;
}

void OpenGLOcean::SimulateOcean(float dt)
{	
	//Init
	glBindFramebuffer(GL_FRAMEBUFFER, oceanFBOs[0]);
	for(unsigned short int i = 0; i < 8; ++i)
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, oceanTextures[3], 0, i);
		
	GLenum drawBuffers[8] =
	{
		GL_COLOR_ATTACHMENT0_EXT,
		GL_COLOR_ATTACHMENT1_EXT,
		GL_COLOR_ATTACHMENT2_EXT,
		GL_COLOR_ATTACHMENT3_EXT,
		GL_COLOR_ATTACHMENT4_EXT,
		GL_COLOR_ATTACHMENT5_EXT,
		GL_COLOR_ATTACHMENT6_EXT,
		GL_COLOR_ATTACHMENT7_EXT
	};
	
	glDrawBuffers(params.choppy ? 8 : 3, drawBuffers);
	glViewport(0, 0, params.fftSize, params.fftSize);
	
	oceanShaders[2]->Use();
	oceanShaders[2]->SetUniform("texSpectrum12", TEX_POSTPROCESS1);
	oceanShaders[2]->SetUniform("texSpectrum34", TEX_POSTPROCESS2);
	oceanShaders[2]->SetUniform("fftSize", (GLfloat)params.fftSize);
	oceanShaders[2]->SetUniform("inverseGridSizes", glm::vec4(2.f*M_PI*(GLfloat)params.fftSize/params.gridSizes[0],
                                                              2.f*M_PI*(GLfloat)params.fftSize/params.gridSizes[1],
															  2.f*M_PI*(GLfloat)params.fftSize/params.gridSizes[2],
															  2.f*M_PI*(GLfloat)params.fftSize/params.gridSizes[3]));
	oceanShaders[2]->SetUniform("t", params.t);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[0]);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, oceanTextures[1]);
	OpenGLContent::getInstance()->DrawSAQ();

    //FFT passes
	glBindFramebuffer(GL_FRAMEBUFFER, oceanFBOs[1]);
	
	oceanShaders[3]->Use();
	oceanShaders[3]->SetUniform("nLayers", params.choppy ? 8 : 3);
	oceanShaders[3]->SetUniform("sLayer", 0);
	oceanShaders[3]->SetUniform("texButterfly", TEX_POSTPROCESS1);
	oceanShaders[3]->SetUniform("texSource", TEX_POSTPROCESS2);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[5]);
	
	for(int i = 0; i < params.passes; ++i)
	{
		oceanShaders[3]->SetUniform("pass", ((float)i + 0.5f)/(float)params.passes);
		if(i%2 == 0)
		{
			glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
		    glDrawBuffer(GL_COLOR_ATTACHMENT1);
		}
		else
		{
		    glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
		    glDrawBuffer(GL_COLOR_ATTACHMENT0);
		}
		OpenGLContent::getInstance()->DrawSAQ();
	}

	oceanShaders[4]->Use();
	oceanShaders[4]->SetUniform("nLayers", params.choppy ? 8 : 3);
	oceanShaders[4]->SetUniform("sLayer", 0);
	oceanShaders[4]->SetUniform("texButterfly", TEX_POSTPROCESS1);
	oceanShaders[4]->SetUniform("texSource", TEX_POSTPROCESS2);
	
	for(int i = params.passes; i < 2 * params.passes; ++i)
	{
		oceanShaders[4]->SetUniform("pass", ((float)i - params.passes + 0.5f)/(float)params.passes);
		if (i%2 == 0)
		{
		    glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
		    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
		}
		else
		{
		    glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
		    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		}
		OpenGLContent::getInstance()->DrawSAQ();
	}

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
	
	params.t += dt;
}

void OpenGLOcean::DrawOceanSurface(glm::vec3 eyePos, glm::mat4 viewProjection)
{
	//Delete old tree
	qt.leafs.clear();
	if(!qt.root.leaf)
	{
		delete qt.root.child[0];
		delete qt.root.child[1];
		delete qt.root.child[2];
		delete qt.root.child[3];
	}
	
	//Build quad tree
	qt.maxLvl = 12;
	
	int64_t start = GetTimeInMicroseconds();
	qt.Grow(eyePos, viewProjection);
	int64_t end = GetTimeInMicroseconds();
	
	//Build mesh
	if(qt.vboVertex > 0)
	{
		glDeleteBuffers(1, &qt.vboVertex);
		glDeleteBuffers(1, &qt.vboEdgeDiv);
	}
	
	std::vector<glm::vec3> data;
	std::vector<glm::vec4> data2;
	for(unsigned int i=0; i < qt.leafs.size(); ++i)
	{
		glm::vec3 origin = qt.leafs[i]->origin;
		glm::vec2 half = qt.leafs[i]->size/2.f;
		data.push_back(origin + glm::vec3(half, 0.f));
		data.push_back(origin + glm::vec3(half.x, -half.y, 0.f));
		data.push_back(origin + glm::vec3(-half, 0.f));
		data.push_back(origin + glm::vec3(-half.x, half.y, 0.f));
		data2.push_back(qt.leafs[i]->edgeFactors);
	}
	
	glGenBuffers(1, &qt.vboVertex);
	glBindBuffer(GL_ARRAY_BUFFER, qt.vboVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*data.size(), &data[0].x, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &qt.vboEdgeDiv);
	glBindBuffer(GL_ARRAY_BUFFER, qt.vboEdgeDiv);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*data2.size(), &data2[0].x, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	//Draw mesh
	oceanShaders[0]->Use();
	oceanShaders[0]->SetUniform("MVP", viewProjection);
	
	glBindVertexArray(qt.vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, qt.vboVertex);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, qt.vboEdgeDiv);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glDrawArrays(GL_PATCHES, 0, data.size());
	
	glBindVertexArray(0);
	
	glUseProgram(0);
	
	//printf("Ocean mesh vertices: %ld, time: %ld\n", data.size(), end-start);
}

void OpenGLOcean::ShowOceanSpectrum(glm::vec4 rect)
{
	/*if(saqBuf != 0 && oceanShaders[7] != NULL)
	{
		GLfloat x = rect.x;
		GLfloat y = rect.y;
		GLfloat width = rect.z;
		GLfloat height = rect.w;
		
		OceanParams& params = ocean->getOceanParams();
		y = viewportSize.y-y-height;

		oceanShaders[7]->Use();
		oceanShaders[7]->SetUniform("texSpectrum12", TEX_POSTPROCESS1);
		oceanShaders[7]->SetUniform("texSpectrum34", TEX_POSTPROCESS2);
		oceanShaders[7]->SetUniform("invGridSizes", glm::vec4(M_PI * params.fftSize / params.gridSizes.x,
															  M_PI * params.fftSize / params.gridSizes.y,
														      M_PI * params.fftSize / params.gridSizes.z,
														      M_PI * params.fftSize / params.gridSizes.w));
		oceanShaders[7]->SetUniform("fftSize", (GLfloat)params.fftSize);
		oceanShaders[7]->SetUniform("zoom", 1.f);
		oceanShaders[7]->SetUniform("linear", 0.f);
		oceanShaders[7]->SetUniform("rect", glm::vec4(x/viewportSize.x, y/viewportSize.y, width/viewportSize.x, height/viewportSize.y));
		
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[0]);
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, oceanTextures[1]);
		
		glBindVertexArray(baseVertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, saqBuf); 
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
 		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, 0);
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}*/
}