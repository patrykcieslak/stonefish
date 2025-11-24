/*    
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

//
//  OpenGLOcean.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/07/17.
//  Copyright (c) 2017-2025 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLOcean.h"

#include <iostream>
#include <fstream>
#include "stb_image_write.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLCamera.h"
#include "graphics/OpenGLOceanParticles.h"
#include "graphics/OpenGLAtmosphere.h"
#include "utils/SystemUtil.hpp"
#include "entities/forcefields/Atmosphere.h"
#include "entities/forcefields/Uniform.h"
#include "entities/forcefields/Jet.h"
#include "entities/forcefields/Pipe.h"
#ifdef EMBEDDED_RESOURCES
#include <sstream>
#include "ResourceHandle.h"
#endif

namespace sf
{

OpenGLOcean::OpenGLOcean(GLfloat size) : gen_{rd_()}, randomGaussian_{0.f, 1.f}, randomUniform_{0.f, 1.f}
{
    cInfo("Generating ocean waves...");
    
    //Initialization
    lightAbsorption = glm::vec3(0.f);
    lightScattering = glm::vec3(0.f);
  
    //Params
    fftPasses_ = 8;
    slopeVarianceSize_ = 4;
    fftSize_ = 1 << fftPasses_;
    gridSizes_ = glm::vec4(893.f, 101.f, 21.f, 11.f);
    spectrum12_ = NULL;
    spectrum34_ = NULL;
    
    GLint layers = 4;
    oceanSize = size;
    particlesEnabled = true;
    waterTemperature = 15.f;

    //Create simulation textures
    oceanTextures[3] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D_ARRAY, glm::uvec3(fftSize_, fftSize_, layers), 
                                                 GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::TRILINEAR, true, true);
    oceanTextures[4] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D_ARRAY, glm::uvec3(fftSize_, fftSize_, layers), 
                                                 GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::TRILINEAR, true, true);
    GLfloat* data = ComputeButterflyLookupTable(fftSize_, fftPasses_);
    oceanTextures[5] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(fftSize_, fftPasses_, 0), 
                                                 GL_RGBA16F, GL_RGBA, GL_FLOAT, data, FilteringMode::NEAREST, false);
    delete[] data;

    //Framebuffers
    glGenFramebuffers(3, oceanFBOs);
    OpenGLState::BindFramebuffer(oceanFBOs[0]);
    GLenum drawBuffers[3] =
    {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, drawBuffers);
    
    for(int layer = 0; layer < 3; ++layer)
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer, oceanTextures[3], 0, layer);
    
    OpenGLState::BindFramebuffer(0);
    
    OpenGLState::BindFramebuffer(oceanFBOs[1]);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, oceanTextures[3], 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, oceanTextures[4], 0);
    OpenGLState::BindFramebuffer(0);
    
    //Computation
    oceanShaders["init"] = new GLSLShader("oceanInit.frag");
    oceanShaders["init"]->AddUniform("texSpectrum12", ParameterType::INT);
    oceanShaders["init"]->AddUniform("texSpectrum34", ParameterType::INT);
    oceanShaders["init"]->AddUniform("inverseGridSizes", ParameterType::VEC4);
    oceanShaders["init"]->AddUniform("fftSize", ParameterType::FLOAT);
    oceanShaders["init"]->AddUniform("t", ParameterType::FLOAT);
    
    std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "saq.vert"));
    sources.push_back(GLSLSource(GL_GEOMETRY_SHADER, "saq.geom"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanFFTX.frag"));
    oceanShaders["fftx"] = new GLSLShader(sources);
    oceanShaders["fftx"]->AddUniform("texButterfly", ParameterType::INT);
    oceanShaders["fftx"]->AddUniform("texSource", ParameterType::INT);
    oceanShaders["fftx"]->AddUniform("pass", ParameterType::FLOAT);
    
    sources.pop_back();
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanFFTY.frag"));
    oceanShaders["ffty"] = new GLSLShader(sources);
    oceanShaders["ffty"]->AddUniform("texButterfly", ParameterType::INT);
    oceanShaders["ffty"]->AddUniform("texSource", ParameterType::INT);
    oceanShaders["ffty"]->AddUniform("pass", ParameterType::FLOAT);
    
    oceanShaders["variance"] = new GLSLShader("oceanVariance.frag");
    oceanShaders["variance"]->AddUniform("texSpectrum12", ParameterType::INT);
    oceanShaders["variance"]->AddUniform("texSpectrum34", ParameterType::INT);
    oceanShaders["variance"]->AddUniform("varianceSize", ParameterType::FLOAT);
    oceanShaders["variance"]->AddUniform("fftSize", ParameterType::INT);
    oceanShaders["variance"]->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders["variance"]->AddUniform("slopeVarianceDelta", ParameterType::FLOAT);
    oceanShaders["variance"]->AddUniform("c", ParameterType::FLOAT);
    
    oceanShaders["spectrum"] = new GLSLShader("oceanSpectrum.frag", "texQuad.vert");
    oceanShaders["spectrum"]->AddUniform("texSpectrum12", ParameterType::INT);
    oceanShaders["spectrum"]->AddUniform("texSpectrum34", ParameterType::INT);
    oceanShaders["spectrum"]->AddUniform("invGridSizes", ParameterType::VEC4);
    oceanShaders["spectrum"]->AddUniform("fftSize", ParameterType::FLOAT);
    oceanShaders["spectrum"]->AddUniform("zoom", ParameterType::FLOAT);
    oceanShaders["spectrum"]->AddUniform("linear", ParameterType::FLOAT);
    oceanShaders["spectrum"]->AddUniform("rect", ParameterType::VEC4);
       
    //Mask background
    oceanShaders["mask_back"] = new GLSLShader("flat.frag", "oceanSurface.vert");
    oceanShaders["mask_back"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders["mask_back"]->AddUniform("FC", ParameterType::FLOAT);
    oceanShaders["mask_back"]->AddUniform("size", ParameterType::FLOAT);
    
    //Blur and bloom
    /*
    oceanShaders["downsample"] = new GLSLShader("downsample2x.frag");
    oceanShaders["downsample"]->AddUniform("source", INT);
    oceanShaders["downsample"]->AddUniform("srcViewport", VEC2);
    oceanShaders["gaussian"] = new GLSLShader("gaussianBlur.frag", "gaussianBlur.vert");
    oceanShaders["gaussian"]->AddUniform("source", INT);
    oceanShaders["gaussian"]->AddUniform("texelOffset", VEC2);
    oceanShaders["blur"] = new GLSLShader("oceanBlur.frag");
    oceanShaders["blur"]->AddUniform("cWater", ParameterType::VEC3);
    oceanShaders["blur"]->AddUniform("bWater", ParameterType::VEC3);
    oceanShaders["blur"]->AddUniform("blurScale", ParameterType::FLOAT);
    oceanShaders["blur"]->AddUniform("blurShape", ParameterType::VEC2);
    oceanShaders["blur"]->AddUniform("texScene", ParameterType::INT);
    oceanShaders["blur"]->AddUniform("texBlur", ParameterType::INT);
    oceanShaders["blur"]->AddUniform("texLinearDepth", ParameterType::INT);
    */

    //Background
    std::vector<GLuint> precompiled;
    precompiled.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    GLint compiled;
	GLuint oceanOpticsFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "oceanOptics.frag", "", &compiled);
    precompiled.push_back(oceanOpticsFragment);
    sources.clear();
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "oceanSurface.vert"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanBackground.frag"));
    oceanShaders["background"] = new GLSLShader(sources, precompiled);
    oceanShaders["background"]->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders["background"]->AddUniform("FC", ParameterType::FLOAT);
    oceanShaders["background"]->AddUniform("size", ParameterType::FLOAT);
    oceanShaders["background"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders["background"]->AddUniform("cWater", ParameterType::VEC3);
    oceanShaders["background"]->AddUniform("bWater", ParameterType::VEC3);
    oceanShaders["background"]->AddUniform("transmittance_texture", ParameterType::INT);
    oceanShaders["background"]->AddUniform("scattering_texture", ParameterType::INT);
    oceanShaders["background"]->AddUniform("irradiance_texture", ParameterType::INT);
    oceanShaders["background"]->BindUniformBlock("SunSky", UBO_SUNSKY);
    oceanShaders["background"]->BindUniformBlock("Lights", UBO_LIGHTS);

    oceanShaders["background"]->Use();
    oceanShaders["background"]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    oceanShaders["background"]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    oceanShaders["background"]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);    
    OpenGLState::UseProgram(0);
    
	glDeleteShader(oceanOpticsFragment);

    //Vector field
    sources.clear();
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "oceanVField.vert"));
    sources.push_back(GLSLSource(GL_GEOMETRY_SHADER, "oceanVField.geom"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanVField.frag"));
    oceanShaders["vectorfield"] = new GLSLShader(sources);
    oceanShaders["vectorfield"]->AddUniform("gridOrigin", ParameterType::IVEC3);
    oceanShaders["vectorfield"]->AddUniform("gridSize", ParameterType::IVEC3);
    oceanShaders["vectorfield"]->AddUniform("gridScale", ParameterType::FLOAT);
    oceanShaders["vectorfield"]->AddUniform("VP", ParameterType::MAT4);
    oceanShaders["vectorfield"]->AddUniform("vectorSize", ParameterType::FLOAT);
    oceanShaders["vectorfield"]->AddUniform("velocityMax", ParameterType::FLOAT);
    oceanShaders["vectorfield"]->AddUniform("eyePos", ParameterType::VEC3);
    oceanShaders["vectorfield"]->BindUniformBlock("OceanCurrents", UBO_OCEAN_CURRENTS);

    //Box around ocean (background)
    glm::vec3 v1(-0.5f, -0.5f, 0.5f);
    glm::vec3 v2(-0.5f,  0.5f, 0.5f);
    glm::vec3 v3(0.5f,   0.5f, 0.5f);
    glm::vec3 v4(0.5f,  -0.5f, 0.5f);
    glm::vec3 v5(0.5f,   0.5f,  0.f);
    glm::vec3 v6(0.5f,  -0.5f,  0.f);
    glm::vec3 v7(-0.5f, -0.5f,  0.f);
    glm::vec3 v8(-0.5f,  0.5f,  0.f);
    std::vector<glm::vec3> boxData;
    
    PlainMesh mesh;
    Vertex v;
    Face f;

    v.normal = glm::vec3(0.f, 0.f, -1.f);
    v.pos = v1; //0
    mesh.vertices.push_back(v);
    v.pos = v2; //1
    mesh.vertices.push_back(v);
    v.pos = v3; //2
    mesh.vertices.push_back(v);
    v.pos = v4; //3
    mesh.vertices.push_back(v);
    v.pos = v5; //4
    mesh.vertices.push_back(v);
    v.pos = v6; //5
    mesh.vertices.push_back(v);
    v.pos = v7; //6
    mesh.vertices.push_back(v);
    v.pos = v8; //7
    mesh.vertices.push_back(v);

    //Bottom        
    f.vertexID[0] = 0;
    f.vertexID[1] = 2;
    f.vertexID[2] = 1;
    mesh.faces.push_back(f);
    f.vertexID[0] = 0;
    f.vertexID[1] = 3;
    f.vertexID[2] = 2;
    mesh.faces.push_back(f);
    
    //Side1
    f.vertexID[0] = 4;
    f.vertexID[1] = 2;
    f.vertexID[2] = 3;
    mesh.faces.push_back(f);
    f.vertexID[0] = 4;
    f.vertexID[1] = 3;
    f.vertexID[2] = 5;
    mesh.faces.push_back(f);
    
    //Side2
    f.vertexID[0] = 6;
    f.vertexID[1] = 1;
    f.vertexID[2] = 7;
    mesh.faces.push_back(f);
    f.vertexID[0] = 6;
    f.vertexID[1] = 0;
    f.vertexID[2] = 1;
    mesh.faces.push_back(f);
    
    //Side3
    f.vertexID[0] = 4;
    f.vertexID[1] = 1;
    f.vertexID[2] = 2;
    mesh.faces.push_back(f);
    f.vertexID[0] = 4;
    f.vertexID[1] = 7;
    f.vertexID[2] = 1;
    mesh.faces.push_back(f);
    
    //Side4
    f.vertexID[0] = 3;
    f.vertexID[1] = 6;
    f.vertexID[2] = 5;
    mesh.faces.push_back(f);
    f.vertexID[0] = 3;
    f.vertexID[1] = 0;
    f.vertexID[2] = 6;
    mesh.faces.push_back(f);
    
    oceanBoxObj = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(&mesh);

    //Ocean currents
    oceanCurrentsUBOData.numCurrents = 0;
    oceanCurrentsUBOData.gravity = glm::vec3(0.f);
    glGenBuffers(1, &oceanCurrentsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, oceanCurrentsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(OceanCurrentsUBO), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, UBO_OCEAN_CURRENTS, oceanCurrentsUBO, 0, sizeof(OceanCurrentsUBO));
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(OceanCurrentsUBO), &oceanCurrentsUBOData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    //Load absorption coefficient table
#ifdef EMBEDDED_RESOURCES
    ResourceHandle rh(GetShaderPath() + "jerlov.dat");
    if(!rh.isValid())
        cCritical("Ocean water data could not be loaded!");    
    std::istringstream dataStream(rh.string());
    dataStream.read((char*)absorption, sizeof(absorption));
    dataStream.read((char*)scattering, sizeof(scattering));
#else
    std::ifstream dataFile(GetShaderPath() + "jerlov.dat", std::ios::in | std::ios::binary);
    if(!dataFile.is_open())
        cCritical("Ocean water data could not be loaded!");
    dataFile.read((char*)absorption, sizeof(absorption));
    dataFile.read((char*)scattering, sizeof(scattering));
    dataFile.close();
#endif


    params_.g = SimulationApp::getApp()->getSimulationManager()->getGravity().getZ();
    params_.t = 0.f;
    params_.propagate = false;
    UpdateOceanParams(params_);
}

OpenGLOcean::~OpenGLOcean()
{
    for(const auto& shader : oceanShaders)
        delete shader.second;
    
    glDeleteFramebuffers(3, oceanFBOs);
    glDeleteTextures(6, oceanTextures);
    glDeleteBuffers(1, &oceanCurrentsUBO);
    
    if(spectrum12_ != NULL) delete [] spectrum12_;
    if(spectrum34_ != NULL) delete [] spectrum34_;

    for(const auto& particles : oceanParticles)
        delete particles.second;
}

void OpenGLOcean::setWaterType(GLfloat t)
{
    t *= 63.f;
    float iPart;
    float fPart = modff(t, &iPart);
    int id = (int)truncf(iPart);
    lightAbsorption = absorption[id];
    lightScattering = scattering[id];
    if(fPart > 0.f) 
    {
        lightAbsorption += fPart * (absorption[id+1] - absorption[id]);
        lightScattering += fPart * (scattering[id+1] - scattering[id]);
    }
}

void OpenGLOcean::setWaterTemperature(GLfloat t)
{
    waterTemperature = t;
}

GLfloat OpenGLOcean::getWaterTemperature()
{
    return waterTemperature;
}
    
void OpenGLOcean::setParticles(bool enabled)
{
    particlesEnabled = enabled;
}

bool OpenGLOcean::getParticlesEnabled()
{
    return particlesEnabled;
}

glm::vec3 OpenGLOcean::getLightAttenuation()
{
    return lightAbsorption + getLightScattering();
}

glm::vec3 OpenGLOcean::getLightScattering()
{
    return lightScattering;
}
    
GLfloat OpenGLOcean::ComputeWaveHeight(GLfloat x, GLfloat y)
{
    return 0.f;
}

GLuint OpenGLOcean::getWaveTexture()
{
    return oceanTextures[3];
}

glm::vec4 OpenGLOcean::getWaveGridSizes()
{
    return gridSizes_;
}

OceanParams OpenGLOcean::getOceanParams() const
{
    return params_;
}

void OpenGLOcean::UpdateOceanParams(const OceanParams& p)
{
    params_ = p;
    params_.alpha = 0.076f * glm::pow(params_.g * params_.fetchLength / (params_.windSpeed * params_.windSpeed), -0.22f);
    params_.peakOmega = 22.f * glm::pow(params_.windSpeed * params_.fetchLength / (params_.g * params_.g), -0.33f);
    InitializeSimulation();
}

void OpenGLOcean::UpdateOceanCurrentsData(const OceanCurrentsUBO& data)
{
    memcpy(&oceanCurrentsUBOData, &data, sizeof(OceanCurrentsUBO));
}

void OpenGLOcean::InitializeSimulation()
{
    GenerateWavesSpectrum();
      
    //Create textures
    oceanTextures[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(fftSize_, fftSize_, 0), 
                                                 GL_RGBA16F, GL_RGBA, GL_FLOAT, spectrum12_, FilteringMode::NEAREST, true);
    oceanTextures[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(fftSize_, fftSize_, 0), 
                                                 GL_RGBA16F, GL_RGBA, GL_FLOAT, spectrum34_, FilteringMode::NEAREST, true);
    oceanTextures[2] = OpenGLContent::GenerateTexture(GL_TEXTURE_3D, glm::uvec3(slopeVarianceSize_, slopeVarianceSize_, slopeVarianceSize_), 
                                                 GL_RG16F, GL_RG, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    
    //Generate variances
    float slopeVarianceDelta = ComputeSlopeVariance();
    
    OpenGLState::BindFramebuffer(oceanFBOs[2]);
    OpenGLState::Viewport(0, 0, slopeVarianceSize_, slopeVarianceSize_);
    
    oceanShaders["variance"]->Use();
    oceanShaders["variance"]->SetUniform("texSpectrum12", TEX_POSTPROCESS1);
    oceanShaders["variance"]->SetUniform("texSpectrum34", TEX_POSTPROCESS2);
    oceanShaders["variance"]->SetUniform("varianceSize", (GLfloat)slopeVarianceSize_);
    oceanShaders["variance"]->SetUniform("fftSize", fftSize_);
    oceanShaders["variance"]->SetUniform("gridSizes", gridSizes_);
    oceanShaders["variance"]->SetUniform("slopeVarianceDelta", slopeVarianceDelta);
    
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[0]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, oceanTextures[1]);
    
    for(unsigned int layer = 0; layer < slopeVarianceSize_; ++layer)
    {
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, oceanTextures[2], 0, layer);
        oceanShaders["variance"]->SetUniform("c", (GLfloat)layer);
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    }
    
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    OpenGLState::BindFramebuffer(0);    
}

void OpenGLOcean::Simulate(GLfloat dt)
{	
    OpenGLState::DisableDepthTest();
    OpenGLState::DisableCullFace();
    
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BindBaseVertexArray();
    
    //Init -> one triangle -> multiple outputs
    OpenGLState::BindFramebuffer(oceanFBOs[0]);
    OpenGLState::Viewport(0, 0, fftSize_, fftSize_);
    oceanShaders["init"]->Use();
    oceanShaders["init"]->SetUniform("texSpectrum12", TEX_POSTPROCESS1);
    oceanShaders["init"]->SetUniform("texSpectrum34", TEX_POSTPROCESS2);
    oceanShaders["init"]->SetUniform("fftSize", (GLfloat)fftSize_);
    oceanShaders["init"]->SetUniform("inverseGridSizes", glm::vec4(2.f*M_PI*(GLfloat)fftSize_/gridSizes_[0],
                                                              2.f*M_PI*(GLfloat)fftSize_/gridSizes_[1],
                                                              2.f*M_PI*(GLfloat)fftSize_/gridSizes_[2],
                                                              2.f*M_PI*(GLfloat)fftSize_/gridSizes_[3]));
    oceanShaders["init"]->SetUniform("t", params_.t);
    params_.t += dt;
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[0]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, oceanTextures[1]);
    glDrawArrays(GL_TRIANGLES, 0, 3); //1 Layer
    
    //FFT passes -> multiple triangles -> multiple layers
    OpenGLState::BindFramebuffer(oceanFBOs[1]);
    OpenGLState::Viewport(0, 0, fftSize_, fftSize_);
    
    oceanShaders["fftx"]->Use();
    oceanShaders["fftx"]->SetUniform("texButterfly", TEX_POSTPROCESS1);
    oceanShaders["fftx"]->SetUniform("texSource", TEX_POSTPROCESS2);
    
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[5]);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    
    for(unsigned int i = 0; i < fftPasses_; ++i)
    {
        oceanShaders["fftx"]->SetUniform("pass", ((float)i + 0.5f)/(float)fftPasses_);
        if(i%2 == 0)
        {
            OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
        }
        else
        {
            OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
        }
        glDrawArrays(GL_TRIANGLES, 0, 3 * 3); //3 Layers
    }

    oceanShaders["ffty"]->Use();
    oceanShaders["ffty"]->SetUniform("texButterfly", TEX_POSTPROCESS1);
    oceanShaders["ffty"]->SetUniform("texSource", TEX_POSTPROCESS2);
    
    for(unsigned int i = fftPasses_; i < 2 * fftPasses_; ++i)
    {
        oceanShaders["ffty"]->SetUniform("pass", ((float)i - fftPasses_ + 0.5f)/(float)fftPasses_);
        if (i%2 == 0)
        {
            OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
        }
        else
        {
            OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
        }
        glDrawArrays(GL_TRIANGLES, 0, 3 * 3); //3 Layers
    }

    OpenGLState::UseProgram(0);
    OpenGLState::BindFramebuffer(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);

    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
       
    OpenGLState::EnableDepthTest();
    OpenGLState::EnableCullFace();
    
    OpenGLState::BindVertexArray(0);
    
    //Update currents uniform buffer
    glBindBuffer(GL_UNIFORM_BUFFER, oceanCurrentsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(OceanCurrentsUBO), &oceanCurrentsUBOData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    //Simulate particles (this is done every frame becasue even if the camera is not updated the particels need to move)
    if(particlesEnabled)
    {
        for(std::map<OpenGLView*, OpenGLOceanParticles*>::iterator it=oceanParticles.begin(); it!=oceanParticles.end(); ++it)
            it->second->Update(it->first, dt);
    }
}

void OpenGLOcean::DrawUnderwaterMask(OpenGLView* view)
{
    oceanShaders["mask_back"]->Use();
    oceanShaders["mask_back"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders["mask_back"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders["mask_back"]->SetUniform("size", oceanSize*0.5f);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetDrawingMode(DrawingMode::RAW);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawObject(oceanBoxObj, -1, glm::mat4(1.f));
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetDrawingMode(DrawingMode::UNDERWATER);
    OpenGLState::UseProgram(0);
}
    
void OpenGLOcean::DrawBackground(OpenGLView* view)
{
    oceanShaders["background"]->Use();
    oceanShaders["background"]->SetUniform("MVP", view->GetProjectionMatrix() * view->GetViewMatrix());
    oceanShaders["background"]->SetUniform("FC", view->GetLogDepthConstant());
    oceanShaders["background"]->SetUniform("size", oceanSize*0.5f);
    oceanShaders["background"]->SetUniform("eyePos", view->GetEyePosition());
    oceanShaders["background"]->SetUniform("cWater", getLightAttenuation());
    oceanShaders["background"]->SetUniform("bWater", getLightScattering());
    glCullFace(GL_FRONT);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetDrawingMode(DrawingMode::RAW);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawObject(oceanBoxObj, -1, glm::mat4(1.f));
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetDrawingMode(DrawingMode::UNDERWATER);
    glCullFace(GL_BACK);
    OpenGLState::UseProgram(0);
}

OpenGLOceanParticles* OpenGLOcean::AllocateParticles(OpenGLView* view)
{
    //Check if particles are already allocated for this view
    if(oceanParticles.find(view) != oceanParticles.end())
        return oceanParticles.at(view);

    OpenGLOceanParticles* particles = new OpenGLOceanParticles(STD_OCEAN_PARTICLES_COUNT, STD_OCEAN_PARTICLES_RADIUS);
    oceanParticles.insert(std::pair<OpenGLView*, OpenGLOceanParticles*>(view, particles));
    return particles;
}

void OpenGLOcean::AssignParticles(OpenGLView* view, OpenGLOceanParticles* particles)
{
    oceanParticles[view] = particles;
}

void OpenGLOcean::DrawParticles(OpenGLView* view)
{
    if(!particlesEnabled)
        return;
        
    try
    {
        oceanParticles.at(view)->Draw(view, this);
    }
    catch(const std::out_of_range& e)
    {
        cWarning("Particles missing!");
    }
}

void OpenGLOcean::DrawParticlesId(OpenGLView* view, GLushort id)
{
    if(!particlesEnabled)
        return;
        
    try
    {
        oceanParticles.at(view)->DrawId(view, id);
    }
    catch(const std::out_of_range& e)
    {
        cWarning("Particles missing!");
    }
}

void OpenGLOcean::DrawVelocityField(OpenGLView* view, GLfloat velocityMax)
{
    //1. Compute AABB of the limited viewera frustum
    glm::vec3 frustum[5];
    frustum[0] = view->GetEyePosition();
    glm::mat4 V = view->GetViewMatrix();
    glm::mat4 invP = glm::inverse(view->GetProjectionMatrix());
    glm::mat4 invV = glm::inverse(V);
    glm::vec4 proj[4];
    proj[0] = invP * glm::vec4(-1.f, -1.f, -1.f, 1.f);
    proj[1] = invP * glm::vec4(1.f, -1.f, -1.f, 1.f);
    proj[2] = invP * glm::vec4(1.f, 1.f, -1.f, 1.f);
    proj[3] = invP * glm::vec4(-1.f, 1.f, -1.f, 1.f);
    frustum[1] = invV * (proj[0]/proj[0].w);
    frustum[2] = invV * (proj[1]/proj[1].w);
    frustum[3] = invV * (proj[2]/proj[2].w);
    frustum[4] = invV * (proj[3]/proj[3].w);

    GLfloat scaling = 10.f/view->GetNearClip(); //5m of distance
    frustum[1] = frustum[0] + (frustum[1]-frustum[0])*scaling;
    frustum[2] = frustum[0] + (frustum[2]-frustum[0])*scaling;
    frustum[3] = frustum[0] + (frustum[3]-frustum[0])*scaling;
    frustum[4] = frustum[0] + (frustum[4]-frustum[0])*scaling;

    glm::vec3 aabbMin(BT_LARGE_FLOAT);
    glm::vec3 aabbMax(-BT_LARGE_FLOAT);

    for(unsigned int i=0; i<5; ++i)
    {
        if(frustum[i].x < aabbMin.x)
            aabbMin.x = frustum[i].x;
        if(frustum[i].x > aabbMax.x)
            aabbMax.x = frustum[i].x;
        if(frustum[i].y < aabbMin.y)
            aabbMin.y = frustum[i].y;
        if(frustum[i].y > aabbMax.y)
            aabbMax.y = frustum[i].y;
        if(frustum[i].z < aabbMin.z)
            aabbMin.z = frustum[i].z;
        if(frustum[i].z > aabbMax.z)
            aabbMax.z = frustum[i].z;
    }

    if(aabbMax.z < 0.f) return;
    if(aabbMin.z < 0.f) aabbMin.z = 0.f;

    //2. Snap to grid
    GLfloat gridScale = 0.2f;
    glm::ivec3 gridMin;
    glm::ivec3 gridMax;
    gridMin.x = (GLint)floorf(aabbMin.x/gridScale);
    gridMin.y = (GLint)floorf(aabbMin.y/gridScale);
    gridMin.z = (GLint)floorf(aabbMin.z/gridScale);
    gridMax.x = (GLint)ceilf(aabbMax.x/gridScale);
    gridMax.y = (GLint)ceilf(aabbMax.y/gridScale);
    gridMax.z = (GLint)ceilf(aabbMax.z/gridScale);

    glm::ivec3 gridSize = gridMax - gridMin + 1;
    GLsizei numPoints = gridSize.x*gridSize.y*gridSize.z;
    if(numPoints > 1e7) numPoints = 1e7;
    
    //3. Generate vertices and render
    oceanShaders["vectorfield"]->Use();
    oceanShaders["vectorfield"]->SetUniform("gridOrigin", gridMin);
    oceanShaders["vectorfield"]->SetUniform("gridSize", gridSize);
    oceanShaders["vectorfield"]->SetUniform("gridScale", gridScale);
    oceanShaders["vectorfield"]->SetUniform("vectorSize", 0.2f);
    oceanShaders["vectorfield"]->SetUniform("velocityMax", velocityMax);
    oceanShaders["vectorfield"]->SetUniform("VP", view->GetProjectionMatrix() * V);
    oceanShaders["vectorfield"]->SetUniform("eyePos", frustum[0]);
    OpenGLState::EnableBlend();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BindBaseVertexArray();
    glDrawArrays(GL_POINTS, 0, numPoints);
    OpenGLState::BindVertexArray(0);
    OpenGLState::UseProgram(0);
    OpenGLState::DisableBlend();
}

void OpenGLOcean::ApplySpecialEffects(OpenGLCamera* cam)
{
    /* SHADERS DISABLED */
    /*
    GLint* viewport = cam->GetViewport();
    
    //Downsample
    OpenGLState::BindFramebuffer(cam->getQuaterPostprocessFBO());
    OpenGLState::Viewport(0, 0, viewport[2]/4, viewport[3]/4);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cam->getColorTexture(1 - cam->getLastActiveColorBuffer()));
    oceanShaders["downsample"]->Use();
    oceanShaders["downsample"]->SetUniform("source", TEX_POSTPROCESS1);
    oceanShaders["downsample"]->SetUniform("srcViewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();

    //Blur
    oceanShaders["gaussian"]->Use();
    oceanShaders["gaussian"]->SetUniform("source", TEX_POSTPROCESS1);
    for(int i=0; i<9; ++i)
    {
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cam->getQuaterPostprocessTexture(0));
        oceanShaders["gaussian"]->SetUniform("texelOffset", glm::vec2(4.f/(GLfloat)viewport[2], 0.f));
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
        
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cam->getQuaterPostprocessTexture(1));
        oceanShaders["gaussian"]->SetUniform("texelOffset", glm::vec2(0.f, 4.f/(GLfloat)viewport[3]));
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    }
    OpenGLState::UseProgram(0);
    OpenGLState::BindFramebuffer(0);    

    //Apply
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cam->getColorTexture(1 - cam->getLastActiveColorBuffer()));
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, cam->getQuaterPostprocessTexture(0));
    OpenGLState::BindTexture(TEX_POSTPROCESS3, GL_TEXTURE_2D, cam->getLinearDepthTexture(true));

    OpenGLState::BindFramebuffer(cam->getRenderFBO());
    OpenGLState::Viewport(0, 0, viewport[2], viewport[3]);
    cam->SetRenderBuffers(cam->getLastActiveColorBuffer(), false, false);
    oceanShaders["blur"]->Use();
    oceanShaders["blur"]->SetUniform("cWater", getLightAttenuation());
    oceanShaders["blur"]->SetUniform("bWater", getLightScattering());
    oceanShaders["blur"]->SetUniform("blurScale", 0.002f);
    oceanShaders["blur"]->SetUniform("blurShape", glm::vec2(1.f/(GLfloat)viewport[2], 1.f/(GLfloat)viewport[3]));
    oceanShaders["blur"]->SetUniform("texScene", TEX_POSTPROCESS1);
    oceanShaders["blur"]->SetUniform("texBlur", TEX_POSTPROCESS2);
    oceanShaders["blur"]->SetUniform("texLinearDepth", TEX_POSTPROCESS3);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    
    OpenGLState::UnbindTexture(TEX_POSTPROCESS3);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    delete [] viewport;
    */
}

void OpenGLOcean::ShowSpectrum(glm::vec2 origin, GLfloat size)
{
    auto content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    glm::vec2 viewportSize = content->getViewportSize();

    GLfloat x = origin.x;
    GLfloat y = origin.y;
    GLfloat width = size;
    GLfloat height = size;
        
    y = viewportSize.y-y-height;

    oceanShaders["spectrum"]->Use();
    oceanShaders["spectrum"]->SetUniform("texSpectrum12", TEX_POSTPROCESS1);
    oceanShaders["spectrum"]->SetUniform("texSpectrum34", TEX_POSTPROCESS2);
    oceanShaders["spectrum"]->SetUniform("invGridSizes", glm::vec4(M_PI * fftSize_ / gridSizes_.x,
                                                          M_PI * fftSize_ / gridSizes_.y,
                                                          M_PI * fftSize_ / gridSizes_.z,
                                                          M_PI * fftSize_ / gridSizes_.w));
    oceanShaders["spectrum"]->SetUniform("fftSize", (GLfloat)fftSize_);
    oceanShaders["spectrum"]->SetUniform("zoom", 0.5f);
    oceanShaders["spectrum"]->SetUniform("linear", 0.f);
    oceanShaders["spectrum"]->SetUniform("rect", glm::vec4(x/viewportSize.x, y/viewportSize.y, width/viewportSize.x, height/viewportSize.y));
        
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[0]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, oceanTextures[1]);
     
    OpenGLState::Viewport(0, 0, viewportSize.x, viewportSize.y);
    content->SetViewportSize(viewportSize.x, viewportSize.y);
    content->BindBaseVertexArray();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    OpenGLState::BindVertexArray(0);

    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}

void OpenGLOcean::ShowTexture(int id, glm::vec4 rect)
{
    switch(id)
    {
        case 0:
        case 1:
            ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, oceanTextures[id], glm::vec4(1e6f));
            break;

        case 5:
            ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, oceanTextures[id]);
            break;
            
        case 3:
        case 4:
            ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, oceanTextures[id], 0, true);
            break;
            
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
            //((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(rect.x, rect.y, rect.z, rect.w, oceanViewTextures[id-30], glm::vec4(1000.f));
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

// Dispertion relation describes angular frequency for any given wave number,
// which tell us how fast a wave of a given waelength propagates.
GLfloat OpenGLOcean::DispertionRelation(GLfloat kMag)
{
    return glm::sqrt(params_.g * kMag * glm::tanh(kMag * params_.depth));
}

// Derivative of the dispertion relation for a finite depth
GLfloat OpenGLOcean::DispertionRelationDerivative(GLfloat kMag)
{
    GLfloat kh = kMag * params_.depth;
    GLfloat tanhKH = glm::tanh(kh);
    GLfloat sechKH = glm::sech(glm::clamp(kh, -9.f, 9.f)); // approx sech(x) = 1/cosh(x) (to maintain precision)
    return params_.g * (tanhKH + kh * sechKH * sechKH) / (2.f * glm::sqrt(params_.g * kMag * tanhKH));
}

// Texel-Marsel-Arsloe (TMA) correction to JONSWAP spectrum for finite depth
GLfloat OpenGLOcean::TMACorrection(GLfloat omega) 
{
    GLfloat omegaH = omega * glm::sqrt(params_.depth / params_.g);
	if (omegaH <= 1.0f)
		return 0.5f * omegaH * omegaH;
	else // omegaH > 1.0f
		return 1.0f - 0.5f * (2.0f - omegaH) * (2.0f - omegaH);
}

// Joint North Sea Wave Observation Project (JONSWAP) spectrum
GLfloat OpenGLOcean::JONSWAPSpectrum(GLfloat omega)
{
    // alpa and peakOmega computed once when parameters are set
    GLfloat sigma = (omega <= params_.peakOmega) ? 0.07f : 0.09f; // Surface tension coefficient [N]
	GLfloat r = glm::exp(- (omega - params_.peakOmega) * (omega - params_.peakOmega) 
                            / (2.0f * sigma * sigma * params_.peakOmega * params_.peakOmega));

	// Divisions & powers
	GLfloat oneOverOmega = 1.0f / (omega + 1e-6f);
	GLfloat peakOmegaOverOmega = params_.peakOmega / omega;
	GLfloat oneOverOmega5 = oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega;
    GLfloat peakOmegaOverOmega4 = peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega;

    // Final spectrum
    return params_.scale * TMACorrection(omega) 
        * params_.alpha * params_.g * params_.g * oneOverOmega5 * glm::exp(-1.25f * peakOmegaOverOmega4) * glm::pow(params_.gamma, r);
}

// Directional spreading
GLfloat OpenGLOcean::NormalizationFactor(GLfloat s) 
{
    GLfloat s2 = s * s;
    GLfloat s3 = s2 * s;
    GLfloat s4 = s3 * s;
    if (s < 5) 
        return -0.000564f * s4 + 0.00776f * s3 - 0.044f * s2 + 0.192f * s + 0.163f;
    else 
        return -4.80e-08f * s4 + 1.07e-05f * s3 - 9.53e-04f * s2 + 5.90e-02f * s + 3.93e-01f;
}

GLfloat OpenGLOcean::Cosine2s(GLfloat theta, GLfloat s) 
{
	return NormalizationFactor(s) * glm::pow(glm::abs(glm::cos(0.5f * theta)), 2.0f * s);
}

GLfloat OpenGLOcean::SpreadPower(GLfloat omega, GLfloat peakOmega) 
{
	if (omega > peakOmega)
		return 9.77f * glm::pow(glm::abs(omega / peakOmega), -2.5f);
	else
		return 6.97f * glm::pow(glm::abs(omega / peakOmega), 5.0f);
}

GLfloat OpenGLOcean::DirectionalSpectrum(GLfloat theta, GLfloat omega) 
{
	float s = SpreadPower(omega, params_.peakOmega) + 16.f * glm::tanh(glm::min(omega / params_.peakOmega, 20.f)) * params_.swell * params_.swell;
	
	return glm::lerp(2.0f / 3.1415f * glm::cos(theta) * glm::cos(theta), Cosine2s(theta - params_.windDirection, s), params_.spreadBlend);
}

GLfloat OpenGLOcean::ShortWavesFade(GLfloat kLength) 
{
	return glm::exp(-params_.shortWavesFade * params_.shortWavesFade * kLength * kLength);
}

GLfloat OpenGLOcean::FullSpectrum(glm::vec2 k, GLfloat kLength)
{
    GLfloat kAngle = glm::atan(k.y, k.x);
    GLfloat omega = DispertionRelation(kLength);
    return JONSWAPSpectrum(omega) * DirectionalSpectrum(kAngle, omega) * ShortWavesFade(kLength);
}

void OpenGLOcean::GetSpectrumSample(int i, int j, GLfloat lengthScale, GLfloat kMin, GLfloat* result)
{
    GLfloat dk = 2.0 * M_PI / lengthScale; // Used to discretize the frequency domain
    glm::vec2 k(i * dk, j * dk); // wave number
    GLfloat kLength = glm::length(k);
    if(kLength >= kMin)
    {
        GLfloat dOmegadk = DispertionRelationDerivative(kLength);
        GLfloat S = glm::sqrt(2.f * FullSpectrum(k, kLength) * glm::abs(dOmegadk) / kLength * dk * dk); // Spectrum
        GLfloat A = randomGaussian_(gen_); // Amplitude
        GLfloat phi = glm::two_pi<GLfloat>() * randomUniform_(gen_); // Phase
        glm::vec2 variate = A * glm::vec2(glm::cos(phi), glm::sin(phi));
        result[0] = variate.x * S;
        result[1] = variate.y * S;
    }
    else
    {
        result[0] = 0.f;
        result[1] = 0.f;
    }
}

// generates the waves spectrum
void OpenGLOcean::GenerateWavesSpectrum()
{
    if(spectrum12_ != NULL)
    {
        delete[] spectrum12_;
        delete[] spectrum34_;
    }
    spectrum12_ = new float[fftSize_ * fftSize_ * 4];
    spectrum34_ = new float[fftSize_ * fftSize_ * 4];

    for (int y = 0; y < fftSize_; ++y)
    {
        for (int x = 0; x < fftSize_; ++x)
        {
            int offset = 4 * (x + y * fftSize_);
            int i = x >= fftSize_ / 2 ? x - fftSize_ : x;
            int j = y >= fftSize_ / 2 ? y - fftSize_ : y;
            GetSpectrumSample(i, j, gridSizes_[0], M_PI / gridSizes_[0], spectrum12_ + offset);
            GetSpectrumSample(i, j, gridSizes_[1], M_PI * fftSize_ / gridSizes_[0], spectrum12_ + offset + 2);
            GetSpectrumSample(i, j, gridSizes_[2], M_PI * fftSize_ / gridSizes_[1], spectrum34_ + offset);
            GetSpectrumSample(i, j, gridSizes_[3], M_PI * fftSize_ / gridSizes_[2], spectrum34_ + offset + 2);
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
        theoreticSlopeVariance += k * k * FullSpectrum(glm::vec2(k, 0), k) * (nextK - k);
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
    for (int y = 0; y < fftSize_; ++y)
    {
        for (int x = 0; x < fftSize_; ++x)
        {
            int offset = 4 * (x + y * fftSize_);
            float i = 2.f * M_PI * (x >= fftSize_ / 2 ? x - fftSize_ : x);
            float j = 2.f * M_PI * (y >= fftSize_ / 2 ? y - fftSize_ : y);
            totalSlopeVariance += GetSlopeVariance(i / gridSizes_[0], j / gridSizes_[0], spectrum12_ + offset);
            totalSlopeVariance += GetSlopeVariance(i / gridSizes_[1], j / gridSizes_[1], spectrum12_ + offset + 2);
            totalSlopeVariance += GetSlopeVariance(i / gridSizes_[2], j / gridSizes_[2], spectrum34_ + offset);
            totalSlopeVariance += GetSlopeVariance(i / gridSizes_[3], j / gridSizes_[3], spectrum34_ + offset + 2);
        }
    }
    
    return theoreticSlopeVariance - totalSlopeVariance;
}

}
