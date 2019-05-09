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
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLOcean.h"

#include <iostream>
#include <fstream>
#include "core/Console.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLAtmosphere.h"
#include "utils/SystemUtil.hpp"
#include "utils/stb_image_write.h"
#include "entities/forcefields/Atmosphere.h"

namespace sf
{

OpenGLOcean::OpenGLOcean(float geometricWaves, SDL_mutex* hydrodynamics)
{
    cInfo("Generating ocean waves...");
    
    //Initialization
    lightAbsorption = glm::vec3(0.f);
    turbidity = 0.f;
    vao = 0;
    vbo = 0;
    vaoEdge = 0;
    vboEdge = 0;
    vaoMask = 0;
    vboMask = 0;
    tesselation = 8;
    
    //Params
    hydroMutex = hydrodynamics;
    qt = NULL;
    fftData = NULL;
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
    
    if(geometricWaves > 0.f)
    {
        params.wind = geometricWaves*5.f + 2.f;
        params.A = 1.f;
        params.omega = 5.f*expf(-geometricWaves) + 0.2f;
        waves = true;
    }
    else
    {
        params.wind = 5.f;
        params.A = 1.0f;
        params.omega = 2.f;
        waves = false;
    }
    
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
    
    GLint layers = 4;
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, params.fftSize, params.fftSize, layers, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, params.fftSize, params.fftSize, layers, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    
    //Framebuffers
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
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer, oceanTextures[3], 0, layer);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, oceanFBOs[1]);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, oceanTextures[3], 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, oceanTextures[4], 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //Shaders
    std::vector<GLuint> precompiled;
    precompiled.push_back(SimulationApp::getApp()->getSimulationManager()->getAtmosphere()->getOpenGLAtmosphere()->getAtmosphereAPI());
    
    //Surface rendering
    GLSLShader* shader;
    if(waves)
    {
        shader = new GLSLShader(precompiled, "oceanSurface.frag", "quadTree.vert", "", std::make_pair("quadTree.tesc", "oceanSurface.tese"));
        shader->AddUniform("tessDiv", ParameterType::FLOAT);
    }
    else
        shader = new GLSLShader(precompiled, "oceanSurface.frag", "infiniteSurface.vert");
    
    shader->AddUniform("texWaveFFT", ParameterType::INT);
    shader->AddUniform("texSlopeVariance", ParameterType::INT);
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
    
    //Backsurface rendering
    if(waves)
    {
        shader = new GLSLShader(precompiled, "oceanBacksurface.frag", "quadTree.vert", "", std::make_pair("quadTree.tesc", "oceanSurface.tese"));
        shader->AddUniform("tessDiv", ParameterType::FLOAT);
    }
    else
    {
        shader = new GLSLShader(precompiled, "oceanBacksurface.frag", "infiniteSurface.vert");
    }
    
    shader->AddUniform("texWaveFFT", ParameterType::INT);
    shader->AddUniform("texSlopeVariance", ParameterType::INT);
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
    
    //Computation
    shader = new GLSLShader("oceanInit.frag"); //Using saq vertex shader
    shader->AddUniform("texSpectrum12", ParameterType::INT);
    shader->AddUniform("texSpectrum34", ParameterType::INT);
    shader->AddUniform("inverseGridSizes", ParameterType::VEC4);
    shader->AddUniform("fftSize", ParameterType::FLOAT);
    shader->AddUniform("t", ParameterType::FLOAT);
    oceanShaders.push_back(shader); //2
    
    shader = new GLSLShader("oceanFFTX.frag", "", "saq.geom"); //Using saq vertex shader
    shader->AddUniform("texButterfly", ParameterType::INT);
    shader->AddUniform("texSource", ParameterType::INT);
    shader->AddUniform("pass", ParameterType::FLOAT);
    oceanShaders.push_back(shader); //3
    
    shader = new GLSLShader("oceanFFTY.frag", "", "saq.geom"); //Using saq vertex shader
    shader->AddUniform("texButterfly", ParameterType::INT);
    shader->AddUniform("texSource", ParameterType::INT);
    shader->AddUniform("pass", ParameterType::FLOAT);
    oceanShaders.push_back(shader); //4
    
    shader = new GLSLShader("oceanVariance.frag"); //Using saq vertex shader
    shader->AddUniform("texSpectrum12", ParameterType::INT);
    shader->AddUniform("texSpectrum34", ParameterType::INT);
    shader->AddUniform("varianceSize", ParameterType::FLOAT);
    shader->AddUniform("fftSize", ParameterType::INT);
    shader->AddUniform("gridSizes", ParameterType::VEC4);
    shader->AddUniform("slopeVarianceDelta", ParameterType::FLOAT);
    shader->AddUniform("c", ParameterType::FLOAT);
    oceanShaders.push_back(shader); //5
    
    shader = new GLSLShader("oceanSpectrum.frag", "texQuad.vert");
    shader->AddUniform("texSpectrum12", ParameterType::INT);
    shader->AddUniform("texSpectrum34", ParameterType::INT);
    shader->AddUniform("invGridSizes", ParameterType::VEC4);
    shader->AddUniform("fftSize", ParameterType::FLOAT);
    shader->AddUniform("zoom", ParameterType::FLOAT);
    shader->AddUniform("linear", ParameterType::FLOAT);
    shader->AddUniform("rect", ParameterType::VEC4);
    oceanShaders.push_back(shader); //6
    
    //Masking
    if(waves)
    {
        shader = new GLSLShader("flat.frag", "quadTree.vert", "", std::make_pair("quadTree.tesc", "oceanSurface.tese"));
        shader->AddUniform("tessDiv", ParameterType::FLOAT);
        shader->AddUniform("texWaveFFT", ParameterType::INT);
        shader->AddUniform("MVP", ParameterType::MAT4);
        shader->AddUniform("gridSizes", ParameterType::VEC4);
    }
    else
    {
        shader = new GLSLShader("flat.frag", "infiniteSurface.vert");
        shader->AddUniform("MVP", ParameterType::MAT4);
    }
    oceanShaders.push_back(shader); //7
    
    //Background
    shader = new GLSLShader("flat.frag", "infiniteSurface.vert");
    shader->AddUniform("MVP", ParameterType::MAT4);
    oceanShaders.push_back(shader); //8
    
    //Blur
    shader = new GLSLShader("oceanBlur.frag");
    shader->AddUniform("lightAbsorption", ParameterType::VEC3);
    shader->AddUniform("turbidity", ParameterType::FLOAT);
    shader->AddUniform("blurScale", ParameterType::FLOAT);
    shader->AddUniform("blurShape", ParameterType::VEC2);
    shader->AddUniform("texScene", ParameterType::INT);
    shader->AddUniform("texLinearDepth", ParameterType::INT);
    oceanShaders.push_back(shader); //9
    
    //Waterline
    shader = new GLSLShader("oceanEdge.frag", "pass.vert", "oceanEdge.geom");
    shader->AddUniform("MVP", ParameterType::MAT4);
    shader->AddUniform("texWaveFFT", ParameterType::INT);
    shader->AddUniform("gridSizes", ParameterType::VEC4);
    oceanShaders.push_back(shader); //10
    
    //Background
    shader = new GLSLShader(precompiled, "oceanBackground.frag", "infiniteSurface.vert");
    shader->AddUniform("MVP", ParameterType::MAT4);
    shader->AddUniform("eyePos", ParameterType::VEC3);
    shader->AddUniform("lightAbsorption", ParameterType::VEC3);
    shader->AddUniform("turbidity", ParameterType::FLOAT);
    shader->AddUniform("transmittance_texture", ParameterType::INT);
    shader->AddUniform("scattering_texture", ParameterType::INT);
    shader->AddUniform("irradiance_texture", ParameterType::INT);
    shader->AddUniform("planetRadius", ParameterType::FLOAT);
    shader->AddUniform("sunDirection", ParameterType::VEC3);
    shader->AddUniform("whitePoint", ParameterType::VEC3);
    shader->AddUniform("cosSunSize", ParameterType::FLOAT);
    oceanShaders.push_back(shader); //11
    
    //Generate variances
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
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, oceanTextures[0]);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_2D, oceanTextures[1]);
    
    for(unsigned int layer = 0; layer < params.slopeVarianceSize; ++layer)
    {
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, oceanTextures[2], 0, layer);
        oceanShaders[5]->SetUniform("c", (GLfloat)layer);
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
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
    
    //Box around ocean (background)
    glm::vec4 v1(-1.f, -1.f, -1.f, 0.f);
    glm::vec4 v2(-1.f,  1.f, -1.f, 0.f);
    glm::vec4 v3(1.f,   1.f, -1.f, 0.f);
    glm::vec4 v4(1.f,  -1.f, -1.f, 0.f);
    glm::vec4 v5(1.f,   1.f,  0.f, 0.f);
    glm::vec4 v6(1.f,  -1.f,  0.f, 0.f);
    glm::vec4 v7(-1.f, -1.f,  0.f, 0.f);
    glm::vec4 v8(-1.f,  1.f,  0.f, 0.f);
    std::vector<glm::vec4> boxData;
    
    //Z -
    boxData.push_back(v1);
    boxData.push_back(v2);
    boxData.push_back(v3);
    boxData.push_back(v1);
    boxData.push_back(v3);
    boxData.push_back(v4);
    
    boxData.push_back(v4);
    boxData.push_back(v3);
    boxData.push_back(v5);
    boxData.push_back(v4);
    boxData.push_back(v5);
    boxData.push_back(v6);
    
    boxData.push_back(v7);
    boxData.push_back(v8);
    boxData.push_back(v2);
    boxData.push_back(v7);
    boxData.push_back(v2);
    boxData.push_back(v1);
    
    boxData.push_back(v5);
    boxData.push_back(v3);
    boxData.push_back(v2);
    boxData.push_back(v5);
    boxData.push_back(v2);
    boxData.push_back(v8);
    
    boxData.push_back(v4);
    boxData.push_back(v6);
    boxData.push_back(v7);
    boxData.push_back(v4);
    boxData.push_back(v7);
    boxData.push_back(v1);
    
    glGenVertexArrays(1, &vaoMask);
    glGenBuffers(1, &vboMask);
    glBindVertexArray(vaoMask);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vboMask);
    glBufferData(GL_ARRAY_BUFFER, boxData.size()*sizeof(glm::vec4), &boxData[0].x, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    //Surface waves
    if(waves)
    {
        GLfloat oceanSize = 10000.f;
        qt = new QuadTree(glm::vec3(0), oceanSize, 15);
        fftData = new GLfloat[params.fftSize * params.fftSize * 4 * layers];
    }
    
    //Load absorption coefficient table
    std::ifstream dataFile(GetShaderPath() + "water_absorption.dat", std::ios::in | std::ios::binary);
    if(dataFile.is_open())
    {
        dataFile.read((char*)absorption, sizeof(absorption));
        dataFile.close();
    }
    
    lastTime = GetTimeInMicroseconds();
}

OpenGLOcean::~OpenGLOcean()
{
    for(size_t i=0; i<oceanShaders.size(); ++i) delete oceanShaders[i];
	glDeleteFramebuffers(3, oceanFBOs);
    glDeleteTextures(6, oceanTextures);
	if(vaoMask > 0) glDeleteVertexArrays(1, &vaoMask);
	if(vboMask > 0) glDeleteBuffers(1, &vboMask);
    if(vao > 0) glDeleteVertexArrays(1, &vao);
	if(vbo > 0) glDeleteBuffers(1, &vbo);
    
    if(params.spectrum12 != NULL) delete [] params.spectrum12;
	if(params.spectrum34 != NULL) delete [] params.spectrum34;
    
    if(qt != NULL)
        delete qt;
    
    if(fftData != NULL)
        delete [] fftData;
}

void OpenGLOcean::setWaterType(GLfloat t)
{
    t *= 63.f;
    float iPart;
    float fPart = modff(t, &iPart);
    int id = (int)truncf(iPart);
    lightAbsorption = absorption[id];
    if(fPart > 0.f) lightAbsorption += fPart * (absorption[id+1] - absorption[id]);
}
    
void OpenGLOcean::setTurbidity(GLfloat t)
{
    turbidity = t;
}
    
GLfloat OpenGLOcean::getTurbidity()
{
    return turbidity;
}

GLfloat OpenGLOcean::ComputeInterpolatedWaveData(GLfloat x, GLfloat y, GLuint channel)
{
    //BILINEAR INTERPOLATION ACCORDING TO OPENGL SPECIFICATION (4.5)
    //Calculate pixel cooridnates
    //x and y are already divided by the phyiscal dimensions of the texture (represented area in [m])
    //so they are directly texture coordinates
    float tmp;
    
    //First coordinate pair
    float i0f = modff(x - 0.5f/(float)params.fftSize, &tmp);
    float j0f = modff(y - 0.5f/(float)params.fftSize, &tmp);
    if(i0f < 0.f) i0f = 1.f - fabsf(i0f);
    if(j0f < 0.f) j0f = 1.f - fabsf(j0f);
    int i0 = (int)truncf(i0f * (float)params.fftSize);
    int j0 = (int)truncf(j0f * (float)params.fftSize);
    
    //Second coordinate pair
    float i1f = modff(x + 0.5f/(float)params.fftSize, &tmp);
    float j1f = modff(y + 0.5f/(float)params.fftSize, &tmp);
    if(i1f < 0.f) i1f = 1.f - fabsf(i1f);
    if(j1f < 0.f) j1f = 1.f - fabsf(j1f);
    int i1 = (int)truncf(i1f * (float)params.fftSize);
    int j1 = (int)truncf(j1f * (float)params.fftSize);
    
    //Calculate weigths
    float alpha = modff(i0f * (float)params.fftSize, &tmp);
    float beta = modff(j0f * (float)params.fftSize, &tmp);
    
    //Get texel values
    float t[4];
    t[0] = fftData[(j0 * params.fftSize + i0) * 4 + channel];
    t[1] = fftData[(j0 * params.fftSize + i1) * 4 + channel];
    t[2] = fftData[(j1 * params.fftSize + i0) * 4 + channel];
    t[3] = fftData[(j1 * params.fftSize + i1) * 4 + channel];
    
    //Interpolate
    float h = (1.f - alpha)*(1.f - beta)*t[0] + alpha*(1.f - beta)*t[1] + (1.f - alpha)*beta*t[2] + alpha*beta*t[3];
    
    return h;
}
    
GLfloat OpenGLOcean::getWaveHeight(GLfloat x, GLfloat y)
{
    if(waves)
    {
        //Z,X are reversed because the coordinate system used to draw ocean has Z axis pointing up!
        GLfloat z = 0.f;
        z -= ComputeInterpolatedWaveData(-x/params.gridSizes.x, y/params.gridSizes.x, 0);
        z -= ComputeInterpolatedWaveData(-x/params.gridSizes.y, y/params.gridSizes.y, 1);
        //The components below have low importance and were excluded to lower the computational cost
        //z -= ComputeInterpolatedWaveData(-x/params.gridSizes.z, y/params.gridSizes.z, 2);
        //z -= ComputeInterpolatedWaveData(-x/params.gridSizes.w, y/params.gridSizes.w, 3);
        return z;
    }
    else
        return 0.f;
}
    
glm::vec3 OpenGLOcean::getLightAbsorption()
{
    return lightAbsorption;
}

void OpenGLOcean::Simulate()
{	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BindBaseVertexArray();
	
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
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, oceanTextures[0]);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_2D, oceanTextures[1]);
	glDrawArrays(GL_TRIANGLES, 0, 3); //1 Layer
	
    //FFT passes -> multiple triangles -> multiple layers
	glBindFramebuffer(GL_FRAMEBUFFER, oceanFBOs[1]);
	glViewport(0, 0, params.fftSize, params.fftSize);
	
	oceanShaders[3]->Use();
	oceanShaders[3]->SetUniform("texButterfly", TEX_POSTPROCESS1);
	oceanShaders[3]->SetUniform("texSource", TEX_POSTPROCESS2);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, oceanTextures[5]);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_2D, 0);
    
	for(unsigned int i = 0; i < params.passes; ++i)
	{
		oceanShaders[3]->SetUniform("pass", ((float)i + 0.5f)/(float)params.passes);
		if(i%2 == 0)
		{
            glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
		}
		else
		{
            glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
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
            glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
		    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
		}
		else
		{
            glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
		    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		}
		glDrawArrays(GL_TRIANGLES, 0, 3 * 3); //3 Layers
	}

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[3]);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	glBindVertexArray(0);
	
    if(waves)
    {
        //Copy wave data to RAM for hydrodynamic computations
        if(SDL_TryLockMutex(hydroMutex) == 0)
        {
            glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
            glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
            glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_FLOAT, (GLvoid*)fftData);
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            SDL_UnlockMutex(hydroMutex);
        }
    }
    
	//Advance time
	int64_t now = GetTimeInMicroseconds();
	params.t += (now-lastTime)/1000000.f;
	lastTime = now;
}

void OpenGLOcean::UpdateSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection)
{
    if(!waves)
        return;
    
    qt->Cut();
    qt->Update(eyePos, projection * view);
}

void OpenGLOcean::DrawUnderwaterMask(glm::mat4 view, glm::mat4 projection, glm::mat4 infProjection)
{
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    
    if(waves)
    {
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
        glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
    
        oceanShaders[7]->Use();
        oceanShaders[7]->SetUniform("MVP", projection * view);
        oceanShaders[7]->SetUniform("tessDiv", (float)tesselation);
        oceanShaders[7]->SetUniform("gridSizes", params.gridSizes);
        oceanShaders[7]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
    
        //1. Draw surface to depth buffer
        qt->Draw();
        
        //2. Draw backsurface to depth and stencil buffer
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);
        glCullFace(GL_FRONT);
    
        qt->Draw();
    
        glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        
        //3. Draw box around whole ocean
        oceanShaders[8]->Use();
        oceanShaders[8]->SetUniform("MVP", infProjection * view);
        
        glBindVertexArray(vaoMask);
        glDrawArrays(GL_TRIANGLES, 0, 30);
        glBindVertexArray(0);
        
        glUseProgram(0);
        
        glCullFace(GL_BACK);
        glDisable(GL_STENCIL_TEST);
    }
    else
    {
        oceanShaders[7]->Use();
        oceanShaders[7]->SetUniform("MVP", infProjection * view);
        
        //1. Draw surface to depth buffer
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        glBindVertexArray(0);
        
        //2. Draw backsurface to depth and stencil buffer
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);
        glCullFace(GL_FRONT);
        
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        glBindVertexArray(0);
        
        //3. Draw box around whole ocean
        oceanShaders[8]->Use();
        oceanShaders[8]->SetUniform("MVP", infProjection * view);
        
        glBindVertexArray(vaoMask);
        glDrawArrays(GL_TRIANGLES, 0, 30);
        glBindVertexArray(0);
        
        glUseProgram(0);
        
        glCullFace(GL_BACK);
        glDisable(GL_STENCIL_TEST);
    }
    
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
}
    
void OpenGLOcean::DrawBackground(glm::vec3 eyePos, glm::mat4 view, glm::mat4 infProjection)
{
    oceanShaders[11]->Use();
    oceanShaders[11]->SetUniform("MVP", infProjection * view);
    oceanShaders[11]->SetUniform("eyePos", eyePos);
    oceanShaders[11]->SetUniform("lightAbsorption", lightAbsorption);
    oceanShaders[11]->SetUniform("turbidity", turbidity);
    SimulationApp::getApp()->getSimulationManager()->getAtmosphere()->getOpenGLAtmosphere()->SetupOceanShader(oceanShaders[11]);
    
    glCullFace(GL_FRONT);
    glBindVertexArray(vaoMask);
    glDrawArrays(GL_TRIANGLES, 0, 30);
    glBindVertexArray(0);
    glCullFace(GL_BACK);
    
    glUseProgram(0);
}

void OpenGLOcean::DrawSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLint* viewport)
{
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_3D, oceanTextures[2]);
    
    if(waves)
    {
        //Draw mesh
        oceanShaders[0]->Use();
        oceanShaders[0]->SetUniform("MVP", projection * view);
        oceanShaders[0]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view))));
        oceanShaders[0]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
        oceanShaders[0]->SetUniform("eyePos", eyePos);
        oceanShaders[0]->SetUniform("tessDiv", (float)tesselation);
        oceanShaders[0]->SetUniform("gridSizes", params.gridSizes);
        oceanShaders[0]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
        oceanShaders[0]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
        SimulationApp::getApp()->getSimulationManager()->getAtmosphere()->getOpenGLAtmosphere()->SetupOceanShader(oceanShaders[0]);
        
        //glCullFace(GL_BACK);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        qt->Draw();
        
        
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glUseProgram(0);
        
        //printf("Ocean mesh leafs: %ld, time: %ld\n", qt->leafs.size(), end-start);
        
        /* Works only for camera looking horizontally - needs a lot more work!
        //Draw water edge
        oceanShaders[10]->Use();
        oceanShaders[10]->SetUniform("MVP", projection * view);
        oceanShaders[10]->SetUniform("gridSizes", params.gridSizes);
        oceanShaders[10]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        GLsizei nEdgePoints = viewport[2]/5;
        glm::vec3 edgeData[nEdgePoints];
        for(GLsizei i=0; i<nEdgePoints; ++i)
            edgeData[i] = glm::vec3(-1.f + 2.f * (GLfloat)i/(GLfloat)(nEdgePoints-1), 0.f, 0.f);
        
        glGenBuffers(1, &vboEdge);
        glBindBuffer(GL_ARRAY_BUFFER, vboEdge);
        glBufferData(GL_ARRAY_BUFFER, sizeof(edgeData), &edgeData[0].x, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glBindVertexArray(vaoMask);
        glBindBuffer(GL_ARRAY_BUFFER, vboEdge);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glDrawArrays(GL_LINE_STRIP, 0, nEdgePoints);
        glBindVertexArray(0);
        
        glUseProgram(0);
        
        glDeleteBuffers(1, &vboEdge);
        
        //glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
         */
    }
    else //Flat surface (infinite plane)
    {
        oceanShaders[0]->Use();
        oceanShaders[0]->SetUniform("MVP", projection * view);
        oceanShaders[0]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view))));
        oceanShaders[0]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
        oceanShaders[0]->SetUniform("eyePos", eyePos);
        oceanShaders[0]->SetUniform("gridSizes", params.gridSizes);
        oceanShaders[0]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
        oceanShaders[0]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
        SimulationApp::getApp()->getSimulationManager()->getAtmosphere()->getOpenGLAtmosphere()->SetupOceanShader(oceanShaders[0]);
        
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        glBindVertexArray(0);
        
        glUseProgram(0);
    }
    
    glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void OpenGLOcean::DrawBacksurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLint* viewport)
{
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, oceanTextures[4]);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_3D, oceanTextures[2]);
    
    if(waves)
    {
        oceanShaders[1]->Use();
        oceanShaders[1]->SetUniform("MVP", projection * view);
        oceanShaders[1]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view))));
        oceanShaders[1]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
        oceanShaders[1]->SetUniform("eyePos", eyePos);
        oceanShaders[1]->SetUniform("tessDiv", (float)tesselation);
        oceanShaders[1]->SetUniform("gridSizes", params.gridSizes);
        oceanShaders[1]->SetUniform("lightAbsorption", lightAbsorption);
        oceanShaders[1]->SetUniform("turbidity", turbidity);
        oceanShaders[1]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
        oceanShaders[1]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
        SimulationApp::getApp()->getSimulationManager()->getAtmosphere()->getOpenGLAtmosphere()->SetupOceanShader(oceanShaders[1]);
        
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glCullFace(GL_FRONT);
        qt->Draw();
        glCullFace(GL_BACK);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        glUseProgram(0);
    }
    else
    {
        oceanShaders[1]->Use();
        oceanShaders[1]->SetUniform("MVP", projection * view);
        oceanShaders[1]->SetUniform("MV", glm::mat3(glm::transpose(glm::inverse(view))));
        oceanShaders[1]->SetUniform("eyePos", eyePos);
        oceanShaders[1]->SetUniform("gridSizes", params.gridSizes);
        oceanShaders[1]->SetUniform("lightAbsorption", lightAbsorption);
        oceanShaders[1]->SetUniform("turbidity", turbidity);
        oceanShaders[1]->SetUniform("viewport", glm::vec2((GLfloat)viewport[2], (GLfloat)viewport[3]));
        oceanShaders[1]->SetUniform("texWaveFFT", TEX_POSTPROCESS1);
        oceanShaders[1]->SetUniform("texSlopeVariance", TEX_POSTPROCESS2);
        SimulationApp::getApp()->getSimulationManager()->getAtmosphere()->getOpenGLAtmosphere()->SetupOceanShader(oceanShaders[1]);
    
        glDisable(GL_CULL_FACE);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        glBindVertexArray(0);
        glEnable(GL_CULL_FACE);
    }
    
    glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glUseProgram(0);
}

void OpenGLOcean::DrawVolume(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint sceneTexture, GLuint linearDepthTex, GLint* viewport)
{
	glDisable(GL_DEPTH_TEST);
	
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_2D, linearDepthTex);
	
	oceanShaders[9]->Use();
    oceanShaders[9]->SetUniform("lightAbsorption", lightAbsorption);
    oceanShaders[9]->SetUniform("turbidity", turbidity);
    oceanShaders[9]->SetUniform("blurScale", 0.002f);
    oceanShaders[9]->SetUniform("blurShape", glm::vec2(1.f, (GLfloat)viewport[2]/(GLfloat)viewport[3]));
    oceanShaders[9]->SetUniform("texScene", TEX_POSTPROCESS1);
	oceanShaders[9]->SetUniform("texLinearDepth", TEX_POSTPROCESS2);
	((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
	glUseProgram(0);
	
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, 0);
	
	glEnable(GL_DEPTH_TEST);
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
		
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, oceanTextures[0]);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS2);
    glBindTexture(GL_TEXTURE_2D, oceanTextures[1]);
		
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
    glBindVertexArray(0);
		
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + TEX_POSTPROCESS1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void OpenGLOcean::ShowTexture(int id, glm::vec4 rect)
{
	switch(id)
	{
		case 0:
		case 1:
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
