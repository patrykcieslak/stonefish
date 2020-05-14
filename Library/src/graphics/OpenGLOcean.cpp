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
//  Copyright (c) 2017-2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLOcean.h"

#include <iostream>
#include <fstream>
#include "core/Console.h"
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
#include "utils/stb_image_write.h"
#include "entities/forcefields/Atmosphere.h"

namespace sf
{

OpenGLOcean::OpenGLOcean(GLfloat size)
{
    cInfo("Generating ocean waves...");
    
    //Initialization
    lightAbsorption = glm::vec3(0.f);
    lightScattering = glm::vec3(0.f);
  
    //Params
    params.passes = 8;
    params.slopeVarianceSize = 4;
    params.fftSize = 1 << params.passes;
    params.propagate = true;
    params.km = 370.f;
    params.cm = 0.23f;
    params.t = 0.f;
    params.gridSizes = glm::vec4(893.f, 101.f, 21.f, 11.f);
    params.spectrum12 = NULL;
    params.spectrum34 = NULL;
    GLint layers = 4;
    oceanSize = size;

    //Create simulation textures
    oceanTextures[3] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D_ARRAY, glm::uvec3(params.fftSize, params.fftSize, layers), 
                                                 GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::TRILINEAR, true, true);
    oceanTextures[4] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D_ARRAY, glm::uvec3(params.fftSize, params.fftSize, layers), 
                                                 GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, FilteringMode::TRILINEAR, true, true);
    GLfloat* data = ComputeButterflyLookupTable(params.fftSize, params.passes);
    oceanTextures[5] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(params.fftSize, params.passes, 0), 
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
    
    oceanShaders["fftx"] = new GLSLShader("oceanFFTX.frag", "", "saq.geom");
    oceanShaders["fftx"]->AddUniform("texButterfly", ParameterType::INT);
    oceanShaders["fftx"]->AddUniform("texSource", ParameterType::INT);
    oceanShaders["fftx"]->AddUniform("pass", ParameterType::FLOAT);
    
    oceanShaders["ffty"] = new GLSLShader("oceanFFTY.frag", "", "saq.geom");
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
    
    //Blur
    oceanShaders["blur"] = new GLSLShader("oceanBlur.frag");
    oceanShaders["blur"]->AddUniform("cWater", ParameterType::VEC3);
    oceanShaders["blur"]->AddUniform("bWater", ParameterType::VEC3);
    oceanShaders["blur"]->AddUniform("blurScale", ParameterType::FLOAT);
    oceanShaders["blur"]->AddUniform("blurShape", ParameterType::VEC2);
    oceanShaders["blur"]->AddUniform("texScene", ParameterType::INT);
    oceanShaders["blur"]->AddUniform("texLinearDepth", ParameterType::INT);
    
    //Background
    std::vector<GLuint> precompiled;
    precompiled.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    GLint compiled;
	GLuint oceanOpticsFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "oceanOptics.frag", "", &compiled);
    precompiled.push_back(oceanOpticsFragment);
    
    oceanShaders["background"] = new GLSLShader(precompiled, "oceanBackground.frag", "oceanSurface.vert");
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

    oceanShaders["background"]->Use();
    oceanShaders["background"]->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    oceanShaders["background"]->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    oceanShaders["background"]->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);    
    
	glDeleteShader(oceanOpticsFragment);
 
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
    
    boxData.push_back(v1);
    boxData.push_back(v3);
    boxData.push_back(v2);
    boxData.push_back(v1);
    boxData.push_back(v4);
    boxData.push_back(v3);
    
    boxData.push_back(v5);
    boxData.push_back(v3);
    boxData.push_back(v4);
    boxData.push_back(v5);
    boxData.push_back(v4);
    boxData.push_back(v6);
    
    boxData.push_back(v7);
    boxData.push_back(v2);
    boxData.push_back(v8);
    boxData.push_back(v7);
    boxData.push_back(v1);
    boxData.push_back(v2);
    
    boxData.push_back(v5);
    boxData.push_back(v2);
    boxData.push_back(v3);
    boxData.push_back(v5);
    boxData.push_back(v8);
    boxData.push_back(v2);
    
    boxData.push_back(v4);
    boxData.push_back(v7);
    boxData.push_back(v6);
    boxData.push_back(v4);
    boxData.push_back(v1);
    boxData.push_back(v7);
    
    glGenVertexArrays(1, &vaoMask);
    glGenBuffers(1, &vboMask);
    OpenGLState::BindVertexArray(vaoMask);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vboMask);
    glBufferData(GL_ARRAY_BUFFER, boxData.size()*sizeof(glm::vec3), &boxData[0].x, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGLState::BindVertexArray(0);
    
    //Load absorption coefficient table
    std::ifstream dataFile(GetShaderPath() + "jerlov.dat", std::ios::in | std::ios::binary);
    if(dataFile.is_open())
    {
        dataFile.read((char*)absorption, sizeof(absorption));
        dataFile.read((char*)scattering, sizeof(scattering));
        dataFile.close();
    }
    
    lastTime = GetTimeInMicroseconds();
}

OpenGLOcean::~OpenGLOcean()
{
    delete oceanShaders["init"];
    delete oceanShaders["fftx"];
    delete oceanShaders["ffty"];
    delete oceanShaders["variance"];
    delete oceanShaders["spectrum"];
    delete oceanShaders["mask_back"];
    delete oceanShaders["blur"];
    delete oceanShaders["background"];

    glDeleteFramebuffers(3, oceanFBOs);
    glDeleteTextures(6, oceanTextures);
    glDeleteVertexArrays(1, &vaoMask);
    glDeleteBuffers(1, &vboMask);
    
    if(params.spectrum12 != NULL) delete [] params.spectrum12;
    if(params.spectrum34 != NULL) delete [] params.spectrum34;

    for(std::map<OpenGLCamera*, OpenGLOceanParticles*>::iterator it=oceanParticles.begin(); it!=oceanParticles.end(); ++it)
        delete it->second;  
    oceanParticles.clear();
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
    return params.gridSizes;
}

void OpenGLOcean::InitializeSimulation()
{
    GenerateWavesSpectrum();
      
    //Create textures
    oceanTextures[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(params.fftSize, params.fftSize, 0), 
                                                 GL_RGBA16F, GL_RGBA, GL_FLOAT, params.spectrum12, FilteringMode::NEAREST, true);
    oceanTextures[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(params.fftSize, params.fftSize, 0), 
                                                 GL_RGBA16F, GL_RGBA, GL_FLOAT, params.spectrum34, FilteringMode::NEAREST, true);
    oceanTextures[2] = OpenGLContent::GenerateTexture(GL_TEXTURE_3D, glm::uvec3(params.slopeVarianceSize, params.slopeVarianceSize, params.slopeVarianceSize), 
                                                 GL_RG16F, GL_RG, GL_FLOAT, NULL, FilteringMode::BILINEAR, false);
    
    //Generate variances
    float slopeVarianceDelta = ComputeSlopeVariance();
    
    OpenGLState::BindFramebuffer(oceanFBOs[2]);
    OpenGLState::Viewport(0, 0, params.slopeVarianceSize, params.slopeVarianceSize);
    
    oceanShaders["variance"]->Use();
    oceanShaders["variance"]->SetUniform("texSpectrum12", TEX_POSTPROCESS1);
    oceanShaders["variance"]->SetUniform("texSpectrum34", TEX_POSTPROCESS2);
    oceanShaders["variance"]->SetUniform("varianceSize", (GLfloat)params.slopeVarianceSize);
    oceanShaders["variance"]->SetUniform("fftSize", params.fftSize);
    oceanShaders["variance"]->SetUniform("gridSizes", params.gridSizes);
    oceanShaders["variance"]->SetUniform("slopeVarianceDelta", slopeVarianceDelta);
    
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[0]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, oceanTextures[1]);
    
    for(unsigned int layer = 0; layer < params.slopeVarianceSize; ++layer)
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
    OpenGLState::Viewport(0, 0, params.fftSize, params.fftSize);
    oceanShaders["init"]->Use();
    oceanShaders["init"]->SetUniform("texSpectrum12", TEX_POSTPROCESS1);
    oceanShaders["init"]->SetUniform("texSpectrum34", TEX_POSTPROCESS2);
    oceanShaders["init"]->SetUniform("fftSize", (GLfloat)params.fftSize);
    oceanShaders["init"]->SetUniform("inverseGridSizes", glm::vec4(2.f*M_PI*(GLfloat)params.fftSize/params.gridSizes[0],
                                                              2.f*M_PI*(GLfloat)params.fftSize/params.gridSizes[1],
                                                              2.f*M_PI*(GLfloat)params.fftSize/params.gridSizes[2],
                                                              2.f*M_PI*(GLfloat)params.fftSize/params.gridSizes[3]));
    oceanShaders["init"]->SetUniform("t", params.t);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[0]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, oceanTextures[1]);
    glDrawArrays(GL_TRIANGLES, 0, 3); //1 Layer
    
    //FFT passes -> multiple triangles -> multiple layers
    OpenGLState::BindFramebuffer(oceanFBOs[1]);
    OpenGLState::Viewport(0, 0, params.fftSize, params.fftSize);
    
    oceanShaders["fftx"]->Use();
    oceanShaders["fftx"]->SetUniform("texButterfly", TEX_POSTPROCESS1);
    oceanShaders["fftx"]->SetUniform("texSource", TEX_POSTPROCESS2);
    
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[5]);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    
    for(unsigned int i = 0; i < params.passes; ++i)
    {
        oceanShaders["fftx"]->SetUniform("pass", ((float)i + 0.5f)/(float)params.passes);
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
    
    for(unsigned int i = params.passes; i < 2 * params.passes; ++i)
    {
        oceanShaders["ffty"]->SetUniform("pass", ((float)i - params.passes + 0.5f)/(float)params.passes);
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
    
    params.t += dt;
    
    for(std::map<OpenGLCamera*, OpenGLOceanParticles*>::iterator it=oceanParticles.begin(); it!=oceanParticles.end(); ++it)
        it->second->Update(it->first, SimulationApp::getApp()->getSimulationManager()->getOcean(), dt);
}

void OpenGLOcean::DrawUnderwaterMask(OpenGLCamera* cam)
{
    oceanShaders["mask_back"]->Use();
    oceanShaders["mask_back"]->SetUniform("MVP", cam->GetProjectionMatrix() * cam->GetViewMatrix());
    oceanShaders["mask_back"]->SetUniform("FC", cam->GetLogDepthConstant());
    oceanShaders["mask_back"]->SetUniform("size", oceanSize);
    OpenGLState::BindVertexArray(vaoMask);
    glDrawArrays(GL_TRIANGLES, 0, 30);
    OpenGLState::BindVertexArray(0);
    OpenGLState::UseProgram(0);
}
    
void OpenGLOcean::DrawBackground(OpenGLCamera* cam)
{
    oceanShaders["background"]->Use();
    oceanShaders["background"]->SetUniform("MVP", cam->GetProjectionMatrix() * cam->GetViewMatrix());
    oceanShaders["background"]->SetUniform("FC", cam->GetLogDepthConstant());
    oceanShaders["background"]->SetUniform("size", oceanSize);
    oceanShaders["background"]->SetUniform("eyePos", cam->GetEyePosition());
    oceanShaders["background"]->SetUniform("cWater", getLightAttenuation());
    oceanShaders["background"]->SetUniform("bWater", getLightScattering());
    glCullFace(GL_FRONT);
    OpenGLState::BindVertexArray(vaoMask);
    glDrawArrays(GL_TRIANGLES, 0, 30);
    OpenGLState::BindVertexArray(0);
    glCullFace(GL_BACK);
    OpenGLState::UseProgram(0);
}

void OpenGLOcean::DrawParticles(OpenGLCamera* cam)
{
    OpenGLOceanParticles* particles;
    
    try
    {
        particles = oceanParticles.at(cam);
    }
    catch(const std::out_of_range& e)
    {
        particles = new OpenGLOceanParticles(10000, 3.0);
        oceanParticles.insert(std::pair<OpenGLCamera*, OpenGLOceanParticles*>(cam, particles));
    }
    
    particles->Draw(cam, this);
}

void OpenGLOcean::DrawVolume(OpenGLCamera* cam, GLuint sceneTexture, GLuint linearDepthTex)
{
    GLint* viewport = cam->GetViewport();
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, sceneTexture);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, linearDepthTex);
    oceanShaders["blur"]->Use();
    oceanShaders["blur"]->SetUniform("cWater", getLightAttenuation());
    oceanShaders["blur"]->SetUniform("bWater", getLightScattering());
    oceanShaders["blur"]->SetUniform("blurScale", 0.002f);
    oceanShaders["blur"]->SetUniform("blurShape", glm::vec2(1.f, (GLfloat)viewport[2]/(GLfloat)viewport[3]));
    oceanShaders["blur"]->SetUniform("texScene", TEX_POSTPROCESS1);
    oceanShaders["blur"]->SetUniform("texLinearDepth", TEX_POSTPROCESS2);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    delete [] viewport;
}

void OpenGLOcean::DrawWaterline(OpenGLCamera* cam)
{
    /*
    // This is a concept but it became obsolete when the near plane of th camera is so close!
    GLint* viewport = cam->GetViewport();
    OpenGLState::DisableDepthTest();
    
    //1. Draw white SAQ to convert stencil to color texture in renderbuffer
    OpenGLState::BindFramebuffer(cam->getRenderFBO());
    GLenum renderBuffs[1] = {GL_COLOR_ATTACHMENT2};
    glDrawBuffers(1, renderBuffs);
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGLState::EnableStencilTest();
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    oceanShaders["waterline_flat"]->Use();
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::DisableStencilTest();

    //2. Use renderbuffer texture to draw downsampled, blurred image to postprocess buffer
    OpenGLState::BindFramebuffer(cam->getPostprocessHalfFBO());
    OpenGLState::Viewport(viewport[0], viewport[1], viewport[2]/2, viewport[3]/2);
    renderBuffs[0] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, renderBuffs);
    ///OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cam->getColorTexture(1));
    //Draw blur H
    //((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(cam->getColorTexture(1));

    //3. Do second pass of separable blur
    renderBuffs[0] = GL_COLOR_ATTACHMENT1;
    glDrawBuffers(1, renderBuffs);
    //OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cam->getPostprocessTexture(2));
    //Draw blur V
    //((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(cam->getPostprocessTexture(2));
   
    //4. Use the blurred result to draw waterline to the framebuffer with blending
    OpenGLState::BindFramebuffer(cam->getRenderFBO());
    OpenGLState::Viewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    renderBuffs[0] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, renderBuffs);
    // OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, cam->getPostprocessTexture(3));
    // OpenGLState::EnableBlend();
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // //Draw waterline
    // ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    // OpenGLState::DisableBlend();
    // OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedSAQ(cam->getPostprocessTexture(3));
    OpenGLState::BindFramebuffer(0);

    delete [] viewport;
    */
}

void OpenGLOcean::ShowSpectrum(glm::vec2 viewportSize, glm::vec4 rect)
{
    GLfloat x = rect.x;
    GLfloat y = rect.y;
    GLfloat width = rect.z;
    GLfloat height = rect.w;
        
    y = viewportSize.y-y-height;

    oceanShaders["spectrum"]->Use();
    oceanShaders["spectrum"]->SetUniform("texSpectrum12", TEX_POSTPROCESS1);
    oceanShaders["spectrum"]->SetUniform("texSpectrum34", TEX_POSTPROCESS2);
    oceanShaders["spectrum"]->SetUniform("invGridSizes", glm::vec4(M_PI * params.fftSize / params.gridSizes.x,
                                                          M_PI * params.fftSize / params.gridSizes.y,
                                                          M_PI * params.fftSize / params.gridSizes.z,
                                                          M_PI * params.fftSize / params.gridSizes.w));
    oceanShaders["spectrum"]->SetUniform("fftSize", (GLfloat)params.fftSize);
    oceanShaders["spectrum"]->SetUniform("zoom", 0.5f);
    oceanShaders["spectrum"]->SetUniform("linear", 0.f);
    oceanShaders["spectrum"]->SetUniform("rect", glm::vec4(x/viewportSize.x, y/viewportSize.y, width/viewportSize.x, height/viewportSize.y));
        
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, oceanTextures[0]);
    OpenGLState::BindTexture(TEX_POSTPROCESS2, GL_TEXTURE_2D, oceanTextures[1]);
    
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
        
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BindBaseVertexArray();
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, quadBuf);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(0);
    OpenGLState::BindVertexArray(0);
    
    OpenGLState::UnbindTexture(TEX_POSTPROCESS2);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    OpenGLState::UseProgram(0);
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

}
