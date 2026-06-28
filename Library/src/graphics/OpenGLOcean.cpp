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
//  Copyright (c) 2017-2024 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLOcean.h"

#include <iostream>
#include <fstream>
#include <memory>
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
    particlesEnabled = true;
    waterTemperature = 15.f;

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
}

OpenGLOcean::~OpenGLOcean()
{
    for(const auto& shader : oceanShaders)
        delete shader.second;
    
    glDeleteFramebuffers(3, oceanFBOs);
    glDeleteTextures(6, oceanTextures);
    glDeleteBuffers(1, &oceanCurrentsUBO);
    
    if(params.spectrum12 != NULL) delete [] params.spectrum12;
    if(params.spectrum34 != NULL) delete [] params.spectrum34;
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
    return params.gridSizes;
}

void OpenGLOcean::UpdateOceanCurrentsData(const OceanCurrentsUBO& data)
{
    memcpy(&oceanCurrentsUBOData, &data, sizeof(OceanCurrentsUBO));
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
    params.t += dt;
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
    
    //Update currents uniform buffer
    glBindBuffer(GL_UNIFORM_BUFFER, oceanCurrentsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(OceanCurrentsUBO), &oceanCurrentsUBOData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    //Simulate particles (this is done every frame becasue even if the camera is not updated the particels need to move)
    if(particlesEnabled)
    {
        for (const std::pair<OpenGLView*, std::shared_ptr<OpenGLOceanParticles>> particle: oceanParticles) {
            particle.second->Update(particle.first, dt);
        }
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

std::shared_ptr<OpenGLOceanParticles> OpenGLOcean::AllocateParticles(OpenGLView* view)
{
    //Check if particles are already allocated for this view
    if(oceanParticles.find(view) != oceanParticles.end())
        return oceanParticles.at(view);

    std::shared_ptr<OpenGLOceanParticles> particles = std::make_shared<OpenGLOceanParticles>(STD_OCEAN_PARTICLES_COUNT, STD_OCEAN_PARTICLES_RADIUS);
    oceanParticles.insert(std::pair<OpenGLView*, std::shared_ptr<OpenGLOceanParticles>>(view, particles));
    return particles;
}

void OpenGLOcean::AssignParticles(OpenGLView* view, std::shared_ptr<OpenGLOceanParticles> particles)
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
