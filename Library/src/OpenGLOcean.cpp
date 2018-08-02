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
    for(unsigned int i=0; i<6; ++i) oceanTextures[i] = 0;
	vao = 0;
    vbo = 0;
    turbidity = 1.0;
    lastTime = GetTimeInMicroseconds();
}

OpenGLOcean::~OpenGLOcean()
{
    for(unsigned int i=0; i<oceanShaders.size(); ++i) delete oceanShaders[i];
	glDeleteFramebuffers(3, oceanFBOs);
    glDeleteTextures(6, oceanTextures);
	glDeleteTextures(3, oceanViewTextures);
    glDeleteVertexArrays(1, &vaoMask);
	if(vboMask > 0) glDeleteBuffers(1, &vboMask);
    glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
    
    if(params.spectrum12 != NULL) delete[] params.spectrum12;
	if(params.spectrum34 != NULL) delete[] params.spectrum34;
}

void OpenGLOcean::setTurbidity(GLfloat t)
{
    turbidity = t;
}

GLfloat OpenGLOcean::getTurbidity()
{
    return turbidity;
}

void OpenGLOcean::setLightAbsorption(glm::vec3 la)
{
    lightAbsorption = la;
}

glm::vec3 OpenGLOcean::getLightAbsorption()
{
    return lightAbsorption;
}

void OpenGLOcean::Init()
{
	cInfo("Generating ocean waves...");
	
	//---textures
	params.passes = 8;
	params.slopeVarianceSize = 4;
	params.fftSize = 1 << params.passes;
	params.propagate = true;
	params.wind = 10.f;
	params.omega = 2.f;
	params.km = 370.f;
	params.cm = 0.23f;
	params.A = 0.1f;
	params.t = 0.f;
	params.gridSizes = glm::vec4(893.f, 101.f, 21.f, 11.f);
	params.spectrum12 = NULL;
	params.spectrum34 = NULL;
	GenerateWavesSpectrum();
	
	float maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	
	glGenTextures(6, oceanTextures);
	
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
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RG16F, params.slopeVarianceSize, params.slopeVarianceSize, params.slopeVarianceSize, 0, GL_RG, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_3D, 0);
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32F, params.fftSize, params.fftSize, 10);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	
	glGenTextures(3, oceanViewTextures);
	for(int i=0; i<3; ++i)
	{
		glTextureView(oceanViewTextures[i], GL_TEXTURE_2D, oceanTextures[3], GL_RGBA32F, 0, 1, i, 1);
		glBindTexture(GL_TEXTURE_2D, oceanViewTextures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32F, params.fftSize, params.fftSize, 10);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		
	//---framebuffers
	glGenFramebuffers(3, oceanFBOs);
	glBindFramebuffer(GL_FRAMEBUFFER, oceanFBOs[0]);
	GLenum drawBuffers[3] = 
	{
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2
	};
	glDrawBuffers(3, drawBuffers);
	
	for(int layer = 0; layer < 3; ++layer)
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer, oceanViewTextures[layer], 0);
		
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, oceanFBOs[1]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, oceanTextures[3], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, oceanTextures[4], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//---shaders
	std::vector<GLuint> precompiled;
	precompiled.push_back(OpenGLAtmosphere::getInstance()->getAtmosphereAPI());
	
    GLSLShader* shader;
	//shader = new GLSLShader(precompiled, "oceanSurface.frag", "quadTree.vert", "", std::make_pair("quadTree.tcs", "oceanSurface.tes"));
	shader = new GLSLShader(precompiled, "oceanSurface.frag", "infiniteSurface.vert");
    //shader->AddUniform("tessDiv", ParameterType::FLOAT);
	shader->AddUniform("texWaveFFT", ParameterType::INT);
	shader->AddUniform("texSlopeVariance", ParameterType::INT);
    shader->AddUniform("texReflection", ParameterType::INT);
	shader->AddUniform("MVP", ParameterType::MAT4);
	shader->AddUniform("gridSizes", ParameterType::VEC4);
	shader->AddUniform("eyePos", ParameterType::VEC3);
	shader->AddUniform("MV", ParameterType::MAT3);
    shader->AddUniform("viewport", ParameterType::VEC2);
	shader->AddUniform("transmittance_texture", ParameterType::INT);
	shader->AddUniform("scattering_texture", ParameterType::INT);
	shader->AddUniform("irradiance_texture", ParameterType::INT);
	shader->AddUniform("planetRadius", ParameterType::FLOAT);
	shader->AddUniform("sunDirection", ParameterType::VEC3);
    shader->AddUniform("whitePoint", ParameterType::VEC3);
    shader->AddUniform("cosSunSize", ParameterType::FLOAT);
	oceanShaders.push_back(shader); //0
    
	//shader = new GLSLShader(precompiled, "oceanBacksurface.frag", "quadTree.vert", "", std::make_pair("quadTree.tcs", "oceanSurface.tes"));
	shader = new GLSLShader(precompiled, "oceanBacksurface.frag", "infiniteSurface.vert");
    //shader->AddUniform("tessDiv", ParameterType::FLOAT);
	shader->AddUniform("texWaveFFT", ParameterType::INT);
	shader->AddUniform("texSlopeVariance", ParameterType::INT);
    shader->AddUniform("texReflection", ParameterType::INT);
	shader->AddUniform("MVP", ParameterType::MAT4);
	shader->AddUniform("gridSizes", ParameterType::VEC4);
	shader->AddUniform("eyePos", ParameterType::VEC3);
	shader->AddUniform("MV", ParameterType::MAT3);
    shader->AddUniform("viewport", ParameterType::VEC2);
	shader->AddUniform("lightAbsorption", ParameterType::VEC3);
	shader->AddUniform("turbidity", ParameterType::FLOAT);
    shader->AddUniform("transmittance_texture", ParameterType::INT);
	shader->AddUniform("scattering_texture", ParameterType::INT);
	shader->AddUniform("irradiance_texture", ParameterType::INT);
	shader->AddUniform("planetRadius", ParameterType::FLOAT);
	shader->AddUniform("sunDirection", ParameterType::VEC3);
    shader->AddUniform("whitePoint", ParameterType::VEC3);
    shader->AddUniform("cosSunSize", ParameterType::FLOAT);
	oceanShaders.push_back(shader); //1
    
	shader = new GLSLShader("oceanInit.frag"); //Using saq vertex shader
	shader->AddUniform("texSpectrum12", ParameterType::INT);
	shader->AddUniform("texSpectrum34", ParameterType::INT);
	shader->AddUniform("inverseGridSizes", ParameterType::VEC4);
	shader->AddUniform("fftSize", ParameterType::FLOAT);
	shader->AddUniform("t", ParameterType::FLOAT);
    oceanShaders.push_back(shader); //2
	
	shader = new GLSLShader("oceanFFTX.frag", "", "hbaoSaq.geom"); //Using saq vertex shader
	shader->AddUniform("texButterfly", ParameterType::INT);
	shader->AddUniform("texSource", ParameterType::INT);
	shader->AddUniform("pass", ParameterType::FLOAT);
	oceanShaders.push_back(shader); //3
    
	shader = new GLSLShader("oceanFFTY.frag", "", "hbaoSaq.geom"); //Using saq vertex shader
	shader->AddUniform("texButterfly", ParameterType::INT);
	shader->AddUniform("texSource", ParameterType::INT);
	shader->AddUniform("pass", ParameterType::FLOAT);
	oceanShaders.push_back(shader);
    
	shader = new GLSLShader("oceanVariance.frag"); //Using saq vertex shader
	shader->AddUniform("texSpectrum12", ParameterType::INT);
	shader->AddUniform("texSpectrum34", ParameterType::INT);
	shader->AddUniform("varianceSize", ParameterType::FLOAT);
	shader->AddUniform("fftSize", ParameterType::INT);
	shader->AddUniform("gridSizes", ParameterType::VEC4);
	shader->AddUniform("slopeVarianceDelta", ParameterType::FLOAT);
	shader->AddUniform("c", ParameterType::FLOAT);
	oceanShaders.push_back(shader);
    
	shader = new GLSLShader("oceanSpectrum.frag", "texQuad.vert");
	shader->AddUniform("texSpectrum12", ParameterType::INT);
	shader->AddUniform("texSpectrum34", ParameterType::INT);
	shader->AddUniform("invGridSizes", ParameterType::VEC4);
	shader->AddUniform("fftSize", ParameterType::FLOAT);
	shader->AddUniform("zoom", ParameterType::FLOAT);
	shader->AddUniform("linear", ParameterType::FLOAT);
	shader->AddUniform("rect", ParameterType::VEC4);
	oceanShaders.push_back(shader);
    
	shader = new GLSLShader("flat.frag", "pass.vert", "oceanVolume.geom");
	shader->AddUniform("axis", ParameterType::VEC2);
	shader->AddUniform("cellSize", ParameterType::VEC2);
	shader->AddUniform("MVP", ParameterType::MAT4);
	shader->AddUniform("texWaveFFT", ParameterType::INT);
	shader->AddUniform("gridSizes", ParameterType::VEC4);
	oceanShaders.push_back(shader);
    
    shader = new GLSLShader(precompiled, "oceanBackground.frag", "saq.vert");
    shader->AddUniform("lightAbsorption", ParameterType::VEC3);
	shader->AddUniform("eyePos", ParameterType::VEC3);
	shader->AddUniform("invProj", ParameterType::MAT4);
	shader->AddUniform("invView", ParameterType::MAT3);
	shader->AddUniform("transmittance_texture", ParameterType::INT);
	shader->AddUniform("scattering_texture", ParameterType::INT);
	shader->AddUniform("irradiance_texture", ParameterType::INT);
	shader->AddUniform("planetRadius", ParameterType::FLOAT);
	shader->AddUniform("whitePoint", ParameterType::VEC3);
	shader->AddUniform("sunDirection", ParameterType::VEC3);
	oceanShaders.push_back(shader);
    
    shader = new GLSLShader("oceanBlur.frag");
	shader->AddUniform("texScene", ParameterType::INT);
	shader->AddUniform("texLinearDepth", ParameterType::INT);
    oceanShaders.push_back(shader);
    
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
	
    for(unsigned int layer = 0; layer < params.slopeVarianceSize; ++layer)
    {
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, oceanTextures[2], 0, layer);
		oceanShaders[5]->SetUniform("c", (GLfloat)layer);
		OpenGLContent::getInstance()->DrawSAQ();
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
    //Surface (infinite plane)
    GLfloat surfData[12][4] = {{0.f, 0.f,  0.f, 1.f},
							  {1.f, 0.f,  0.f, 0.f},
							  {0.f, 1.f,  0.f, 0.f},
							  {0.f, 0.f,  0.f, 1.f},
							  {0.f, 1.f,  0.f, 0.f},
							  {-1.f, 0.f, 0.f, 0.f},
							  {0.f, 0.f,  0.f, 1.f},
							  {-1.f, 0.f, 0.f, 0.f},
							  {0.f, -1.f, 0.f, 0.f},
							  {0.f, 0.f,  0.f, 1.f},
							  {0.f, -1.f, 0.f, 0.f},
							  {1.f, 0.f,  0.f, 0.f}};
							  
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo); 
	
    glBindVertexArray(vao);	
	glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(surfData), surfData, GL_STATIC_DRAW); 
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);	
	
    glBindVertexArray(0);
    
	//---mask rendering
	glGenVertexArrays(1, &vaoMask);
	glBindVertexArray(vaoMask);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void OpenGLOcean::Simulate()
{	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	OpenGLContent::getInstance()->BindBaseVertexArray();
	
	//Init -> one triangle -> multiple outputs
	glBindFramebuffer(GL_FRAMEBUFFER, oceanFBOs[0]);
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
	glDrawArrays(GL_TRIANGLES, 0, 3); //1 Layer
	
    //FFT passes -> multiple triangles -> multiple layers
	glBindFramebuffer(GL_FRAMEBUFFER, oceanFBOs[1]);
	glViewport(0, 0, params.fftSize, params.fftSize);
	
	oceanShaders[3]->Use();
	oceanShaders[3]->SetUniform("texButterfly", TEX_POSTPROCESS1);
	oceanShaders[3]->SetUniform("texSource", TEX_POSTPROCESS2);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[5]);
	
	for(unsigned int i = 0; i < params.passes; ++i)
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
		glDrawArrays(GL_TRIANGLES, 0, 3 * 3); //3 Layers
	}

	oceanShaders[4]->Use();
	oceanShaders[4]->SetUniform("texButterfly", TEX_POSTPROCESS1);
	oceanShaders[4]->SetUniform("texSource", TEX_POSTPROCESS2);
	
	for(unsigned int i = params.passes; i < 2 * params.passes; ++i)
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
		glDrawArrays(GL_TRIANGLES, 0, 3 * 3); //3 Layers
	}

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	glBindVertexArray(0);
	
	//Advance time
	int64_t now = GetTimeInMicroseconds();
	params.t += (now-lastTime)/1000000.f;
	lastTime = now;
}

void OpenGLOcean::DrawSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint reflectionTexture, GLint* viewport)
{
    oceanShaders[0]->Use();
	oceanShaders[0]->SetUniform("MVP", projection * view);
	oceanShaders[0]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view))));
    oceanShaders[0]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
	oceanShaders[0]->SetUniform("eyePos", eyePos);
	oceanShaders[0]->SetUniform("gridSizes", params.gridSizes);
	oceanShaders[0]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
	oceanShaders[0]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
    oceanShaders[0]->SetUniform("texReflection", TEX_POSTPROCESS3);
	OpenGLAtmosphere::getInstance()->SetupOceanShader(oceanShaders[0]);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures[2]);
    glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS3, GL_TEXTURE_2D, reflectionTexture);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC_ALPHA);
	glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_3D, 0);
    glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS3, GL_TEXTURE_2D, 0);
    
	glUseProgram(0);
    
    
	//Delete old tree
	/*qt.leafs.clear();
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
	qt.Grow(eyePos, projection * view);
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
		data.push_back(origin + glm::vec3(-half.x, half.y, 0.f));
		data.push_back(origin + glm::vec3(-half, 0.f));
		data.push_back(origin + glm::vec3(half.x, -half.y, 0.f));
		
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
	oceanShaders[0]->SetUniform("MVP", projection * view);
	oceanShaders[0]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view))));
	oceanShaders[0]->SetUniform("eyePos", eyePos);
	oceanShaders[0]->SetUniform("tessDiv", 8.f);
	oceanShaders[0]->SetUniform("gridSizes", params.gridSizes);
	oceanShaders[0]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
	oceanShaders[0]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
	OpenGLAtmosphere::getInstance()->SetupOceanShader(oceanShaders[0]);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures[2]);
	
	glBindVertexArray(qt.vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, qt.vboVertex);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, qt.vboEdgeDiv);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glDrawArrays(GL_PATCHES, 0, data.size());
	
	glBindVertexArray(0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_3D, 0);
	glUseProgram(0);*/
	
	//printf("Ocean mesh vertices: %ld, time: %ld\n", data.size(), end-start);
}

void OpenGLOcean::DrawBacksurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint reflectionTexture, GLint* viewport)
{
    oceanShaders[1]->Use();
	oceanShaders[1]->SetUniform("MVP", projection * view);
	oceanShaders[1]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view))));
	oceanShaders[1]->SetUniform("eyePos", eyePos);
	oceanShaders[1]->SetUniform("tessDiv", 8.f);
	oceanShaders[1]->SetUniform("gridSizes", params.gridSizes);
    oceanShaders[1]->SetUniform("lightAbsorption", lightAbsorption);
	oceanShaders[1]->SetUniform("turbidity", turbidity);
	oceanShaders[1]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
	oceanShaders[1]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
	oceanShaders[1]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
    oceanShaders[1]->SetUniform("texReflection", TEX_POSTPROCESS3);
	OpenGLAtmosphere::getInstance()->SetupOceanShader(oceanShaders[1]);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures[2]);
    glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS3, GL_TEXTURE_2D, reflectionTexture);
	
	glDisable(GL_CULL_FACE);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glBindVertexArray(0);
	glEnable(GL_CULL_FACE);
    
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_3D, 0);
    glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS3, GL_TEXTURE_2D, 0);
	glUseProgram(0);
    
	//Delete old tree
	/*qt.leafs.clear();
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
	qt.Grow(eyePos, projection * view);
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
		data.push_back(origin + glm::vec3(-half.x, half.y, 0.f));
		data.push_back(origin + glm::vec3(-half, 0.f));
		data.push_back(origin + glm::vec3(half.x, -half.y, 0.f));
		
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
	glCullFace(GL_FRONT);
	
	oceanShaders[1]->Use();
	oceanShaders[1]->SetUniform("MVP", projection * view);
	oceanShaders[1]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view))));
	oceanShaders[1]->SetUniform("eyePos", eyePos);
	oceanShaders[1]->SetUniform("tessDiv", 8.f);
	oceanShaders[1]->SetUniform("gridSizes", params.gridSizes);
	oceanShaders[1]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
	oceanShaders[1]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
	OpenGLAtmosphere::getInstance()->SetupOceanShader(oceanShaders[1]);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_3D, oceanTextures[2]);
	
	glBindVertexArray(qt.vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, qt.vboVertex);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, qt.vboEdgeDiv);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glDrawArrays(GL_PATCHES, 0, data.size());
	
	glBindVertexArray(0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_3D, 0);
	glUseProgram(0);
	
	glCullFace(GL_BACK);*/
	//printf("Ocean mesh vertices: %ld, time: %ld\n", data.size(), end-start);
}

void OpenGLOcean::DrawBackground(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection)
{
	oceanShaders[8]->Use();
    oceanShaders[8]->SetUniform("lightAbsorption", lightAbsorption);
	oceanShaders[8]->SetUniform("eyePos", eyePos);
	oceanShaders[8]->SetUniform("invProj", glm::inverse(projection));
	oceanShaders[8]->SetUniform("invView", glm::inverse(glm::mat3(view)));
    OpenGLAtmosphere::getInstance()->SetupOceanShader(oceanShaders[8]);
	OpenGLContent::getInstance()->DrawSAQ();
	glUseProgram(0);
}

void OpenGLOcean::DrawVolume(GLuint sceneTexture, GLuint linearDepthTex)
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, sceneTexture);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, linearDepthTex);
	
	oceanShaders[9]->Use();
	oceanShaders[9]->SetUniform("texScene", TEX_POSTPROCESS1);
	oceanShaders[9]->SetUniform("texLinearDepth", TEX_POSTPROCESS2);
	OpenGLContent::getInstance()->DrawSAQ();
	glUseProgram(0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, 0);
	
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void OpenGLOcean::DrawVolumeMask(glm::vec3 eyePos, glm::vec3 lookingDir, glm::mat4 view, glm::mat4 projection)
{
	//Generate grid around camera near plane
	if(vboMask > 0)
		glDeleteBuffers(1, &vboMask);
	
	glm::vec3 forward = glm::normalize(glm::vec3(lookingDir.x, lookingDir.y, 0.0));
	glm::vec3 right = glm::normalize(glm::vec3(-lookingDir.y, lookingDir.x, 0.0));
	glm::vec2 gridSize(2.0, 2.0);
	
	std::vector<glm::vec3> data;
	for(int i=-50; i < 50; ++i)
		for(int h=-50; h < 50; ++h)
			data.push_back(glm::vec3(eyePos.x, eyePos.y, 0.0) 
						   +(float)i/50.f * right * gridSize.x
						   +(float)h/50.f * forward * gridSize.y);
	
	glGenBuffers(1, &vboMask);
	glBindBuffer(GL_ARRAY_BUFFER, vboMask);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*data.size(), &data[0].x, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	//Draw ocean volume
	oceanShaders[7]->Use();
	oceanShaders[7]->SetUniform("axis", glm::vec2(right.x, right.y));
	oceanShaders[7]->SetUniform("cellSize", gridSize/50.f);
	oceanShaders[7]->SetUniform("MVP", projection * view);
	oceanShaders[7]->SetUniform("gridSizes", params.gridSizes);
	oceanShaders[7]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
	
	glBindVertexArray(vaoMask);
	
	glBindBuffer(GL_ARRAY_BUFFER, vboMask);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glDrawArrays(GL_POINTS, 0, data.size());
	
	glBindVertexArray(0);
	
	glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, 0);
	glUseProgram(0);
}

void OpenGLOcean::ShowSpectrum(glm::vec2 viewportSize, glm::vec4 rect)
{
		GLfloat x = rect.x;
		GLfloat y = rect.y;
		GLfloat width = rect.z;
		GLfloat height = rect.w;
		
		y = viewportSize.y-y-height;

		oceanShaders[6]->Use();
		oceanShaders[6]->SetUniform("texSpectrum12", TEX_POSTPROCESS1);
		oceanShaders[6]->SetUniform("texSpectrum34", TEX_POSTPROCESS2);
		oceanShaders[6]->SetUniform("invGridSizes", glm::vec4(M_PI * params.fftSize / params.gridSizes.x,
															  M_PI * params.fftSize / params.gridSizes.y,
														      M_PI * params.fftSize / params.gridSizes.z,
														      M_PI * params.fftSize / params.gridSizes.w));
		oceanShaders[6]->SetUniform("fftSize", (GLfloat)params.fftSize);
		oceanShaders[6]->SetUniform("zoom", 0.5f);
		oceanShaders[6]->SetUniform("linear", 0.f);
		oceanShaders[6]->SetUniform("rect", glm::vec4(x/viewportSize.x, y/viewportSize.y, width/viewportSize.x, height/viewportSize.y));
		
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[0]);
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, oceanTextures[1]);
		
		//Build quad texture VBO
		GLfloat quadData[4][4] = {{-1.f, -1.f, 0.f, 0.f},
								{-1.f,  1.f, 0.f, 1.f},
								{ 1.f, -1.f, 1.f, 0.f},
								{ 1.f,  1.f, 1.f, 1.f}};
							  
		GLuint quadBuf = 0;
		glGenBuffers(1, &quadBuf); 
		glBindBuffer(GL_ARRAY_BUFFER, quadBuf); 
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW); 
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		OpenGLContent::getInstance()->BindBaseVertexArray();
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quadBuf); 
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
 		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(0);
		glBindVertexArray(0);
		
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS1, GL_TEXTURE_2D, 0);
		glBindMultiTextureEXT(GL_TEXTURE0 + TEX_POSTPROCESS2, GL_TEXTURE_2D, 0);
		glUseProgram(0);
}

void OpenGLOcean::ShowTexture(int id, glm::vec4 rect)
{
	switch(id)
	{
		case 0:
		case 1:
		case 5:
			OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, oceanTextures[id]);
			break;
			
		case 3:
		case 4:
			OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, oceanTextures[id], 1);
			break;
			
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
			OpenGLContent::getInstance()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, oceanViewTextures[id-30], glm::vec4(1000.f));
			break;
	}
	
	
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

	for(unsigned int i = 0; i < passes; ++i)
	{
		int nBlocks  = (int)powf(2.0, float(passes - 1 - (int)i));
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