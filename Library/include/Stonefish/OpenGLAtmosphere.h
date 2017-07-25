//
//  OpenGLAtmosphere.h
//  Stonefish
//
//  Created by Patryk Cieslak on 22/07/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLAtmosphere__
#define __Stonefish_OpenGLAtmosphere__

#include <functional>
#include "OpenGLPipeline.h"
#include "GLSLShader.h"

// An atmosphere layer of width 'width', and whose density is defined as
// 'exp_term' * exp('exp_scale' * h) + 'linear_term' * h + 'constant_term',
// clamped to [0,1], and where h is the altitude.
struct DensityProfileLayer 
{
	float width;
	float exp_term;
	float exp_scale;
	float linear_term;
	float constant_term;
};

// An atmosphere density profile made of several layers on top of each other
// (from bottom to top). The width of the last layer is ignored, i.e. it always
// extend to the top atmosphere boundary. The profile values vary between 0
// (null density) to 1 (maximum density).
struct DensityProfile 
{
	DensityProfileLayer layers[2];
};

struct Time
{
	unsigned short hour;
	unsigned short minute;
	unsigned short second;
};

enum AtmosphereTextures
{
	TRANSMITTANCE = 0,
	SCATTERING,
	IRRADIANCE,
	TEXTURE_COUNT
};

class OpenGLAtmosphere 
{
public: 
	OpenGLAtmosphere(unsigned int numOfPrecomputedWavelengths = 15, unsigned int numOfScatteringOrders = 4);
	~OpenGLAtmosphere();
	
	void DrawSkyAndSun(const OpenGLView* view);
	void ShowAtmosphereTexture(AtmosphereTextures id, glm::vec4 rect);
	void SetSunPosition(float azimuthDeg, float elevationDeg);
	void SetSunPosition(float latitude, float longitude, Time utc);                            
	
private:
	//Precomputation
	std::string EarthsAtmosphere(const glm::dvec3& lambdas);
	void Precompute();
	void PrecomputePass(GLuint fbo, GLuint delta_irradiance_texture, GLuint delta_rayleigh_scattering_texture,
						GLuint delta_mie_scattering_texture, GLuint delta_scattering_density_texture,
						GLuint delta_multiple_scattering_texture, const glm::dvec3& lambdas, const glm::dmat3& luminance_from_radiance, bool blend);
	void ComputeSpectralRadianceToLuminanceFactors(const std::vector<double>& wavelengths, 
												   const std::vector<double>& solarIrradiance,
												   double lambdaPower, double* k_r, double* k_g, double* k_b);
	void ConvertSpectrumToLinearSrgb(const std::vector<double>& wavelengths, 
									 const std::vector<double>& spectrum, 
									 double* r, double* g, double* b);
	void DrawBlendedSAQ(const std::vector<bool>& enableBlend);
	
	unsigned int nPrecomputedWavelengths;
	unsigned int nScatteringOrders;
	std::string glslDefinitions;
	std::string glslFunctions;
	
	//Rendering
	float sunAzimuth;
	float sunElevation;
	GLSLShader* skySunShader;
	GLuint textures[AtmosphereTextures::TEXTURE_COUNT];
};

#endif