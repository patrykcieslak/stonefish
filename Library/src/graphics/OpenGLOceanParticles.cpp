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
//  OpenGLOceanParticles.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/08/19.
//  Copyright (c) 2019-2026 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLOceanParticles.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLView.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLAtmosphere.h"
#include "graphics/OpenGLOcean.h"
#include "entities/forcefields/Ocean.h"
#include "utils/SystemUtil.hpp"

namespace sf
{
    
GLuint OpenGLOceanParticles::flakeTexture = 0;
GLuint OpenGLOceanParticles::noiseTexture = 0;
std::unique_ptr<GLSLShader> OpenGLOceanParticles::renderShader;
std::unique_ptr<GLSLShader> OpenGLOceanParticles::renderIdShader;
std::unique_ptr<GLSLShader> OpenGLOceanParticles::updateShader;

OpenGLOceanParticles::OpenGLOceanParticles(size_t numOfParticles, GLfloat visibleRange) : OpenGLParticles(numOfParticles), uniformd_(0, 1.f), normald_(0, 1.f)
{
    initialised_ = false;
    range_ = fabsf(visibleRange);
    lastEyePos_ = glm::vec3(0);
}
    
OpenGLOceanParticles::~OpenGLOceanParticles()
{
}

void OpenGLOceanParticles::Create(glm::vec3 eyePos)
{
    lastEyePos_ = eyePos;
    //Create particles randomly (uniformly) distributed inside a sphere
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlePosSSBO_);
    glm::vec4* positions = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * nParticles_, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    for(GLuint i=0; i<nParticles_; ++i) 
    {
        GLfloat r = cbrtf(uniformd_(generator_)) * range_; //cbrtf for uniform distribution in sphere volume
        *(positions++) = glm::vec4(r * glm::normalize(glm::vec3(normald_(generator_), normald_(generator_), normald_(generator_))) + eyePos, uniformd_(generator_)*0.005f + 0.002f);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleVelSSBO_);
    glm::vec4* velocities = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * nParticles_, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memset(velocities, 0, sizeof(glm::vec4) * nParticles_);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    initialised_ = true;
}
    
void OpenGLOceanParticles::Update(OpenGLView* view, GLfloat dt)
{
    glm::vec3 eyePos = view->GetEyePosition();
    lastEyePos_ = eyePos;

    //Check if ever updated
    if(!initialised_)
    {
        Create(eyePos);
        return;
    }

    //Move particles
    updateShader->Use();
    updateShader->SetUniform("dt", dt);
    updateShader->SetUniform("numParticles", nParticles_);
    updateShader->SetUniform("eyePos", eyePos);
    updateShader->SetUniform("R", range_);
    OpenGLState::BindTexture(TEX_MAT_ALBEDO, GL_TEXTURE_3D, noiseTexture);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_PARTICLE_POS, particlePosSSBO_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_PARTICLE_VEL, particleVelSSBO_);
    glDispatchCompute((GLuint)ceil(nParticles_/256.0), 1, 1);
    OpenGLState::UnbindTexture(TEX_MAT_ALBEDO);
    OpenGLState::UseProgram(0);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
    
void OpenGLOceanParticles::Draw(OpenGLView* view, OpenGLOcean* glOcn)
{
    glm::mat4 MV = view->GetViewMatrix();
    renderShader->Use();
    renderShader->SetUniform("MV", MV);
    renderShader->SetUniform("iMV", glm::inverse(MV));
    renderShader->SetUniform("P", view->GetProjectionMatrix());
    renderShader->SetUniform("FC", view->GetLogDepthConstant());
    renderShader->SetUniform("eyePos", view->GetEyePosition());
    renderShader->SetUniform("viewDir", view->GetLookingDirection());
    renderShader->SetUniform("cWater", glOcn->getLightAttenuation());
    renderShader->SetUniform("bWater", glOcn->getLightScattering());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_PARTICLE_POS, particlePosSSBO_);
    OpenGLState::BindTexture(TEX_MAT_ALBEDO, GL_TEXTURE_2D, flakeTexture);
    OpenGLState::EnableBlend();
    glBlendFunc(GL_ONE, GL_ONE);
    OpenGLState::BindVertexArray(particleVAO_);
    glDrawElements(GL_TRIANGLES, nParticles_ * 6, GL_UNSIGNED_INT, 0);
    OpenGLState::BindVertexArray(0);
    OpenGLState::DisableBlend();
    OpenGLState::UnbindTexture(TEX_MAT_ALBEDO);
    OpenGLState::UseProgram(0);
}

void OpenGLOceanParticles::DrawId(OpenGLView* view, GLushort id)
{
    glm::mat4 MV = view->GetViewMatrix();
    renderIdShader->Use();
    renderIdShader->SetUniform("MV", MV);
    renderIdShader->SetUniform("iMV", glm::inverse(MV));
    renderIdShader->SetUniform("P", view->GetProjectionMatrix());
    renderIdShader->SetUniform("FC", view->GetLogDepthConstant());
    renderIdShader->SetUniform("objectId", (GLuint)id);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_PARTICLE_POS, particlePosSSBO_);
    OpenGLState::BindTexture(TEX_MAT_ALBEDO, GL_TEXTURE_2D, flakeTexture);
    OpenGLState::BindVertexArray(particleVAO_);
    glDrawElements(GL_TRIANGLES, nParticles_ * 6, GL_UNSIGNED_INT, 0);
    OpenGLState::BindVertexArray(0);
    OpenGLState::UnbindTexture(TEX_MAT_ALBEDO);
    OpenGLState::UseProgram(0);
}
    
void OpenGLOceanParticles::Init()
{
    //Load shaders
	std::vector<GLuint> precompiled;
    precompiled.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    
	std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "oceanParticle.comp"));
    updateShader = std::make_unique<GLSLShader>(sources);
    updateShader->AddUniform("dt", ParameterType::FLOAT);
    updateShader->AddUniform("numParticles", ParameterType::UINT);
    updateShader->AddUniform("eyePos", ParameterType::VEC3);
    updateShader->AddUniform("R", ParameterType::FLOAT);
    updateShader->AddUniform("texNoise", ParameterType::INT);
    updateShader->AddUniform("invNoiseSize", ParameterType::FLOAT);
    updateShader->BindUniformBlock("OceanCurrents", UBO_OCEAN_CURRENTS);
    updateShader->BindShaderStorageBlock("Positions", SSBO_PARTICLE_POS);
    updateShader->BindShaderStorageBlock("Velocities", SSBO_PARTICLE_VEL);

    GLuint noiseSize = 16;
    updateShader->Use();
    updateShader->SetUniform("texNoise", TEX_MAT_ALBEDO);
    updateShader->SetUniform("invNoiseSize", 1.f/(GLfloat)noiseSize);
    OpenGLState::UseProgram(0);

    sources.clear();
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "particle.vert"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "lightingNoShadow.frag"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanOptics.frag"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanSurfaceFlat.glsl"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanParticle.frag"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "materialUUv.frag"));
    
    renderShader = std::make_unique<GLSLShader>(sources, precompiled);
    renderShader->AddUniform("MV", ParameterType::MAT4);
    renderShader->AddUniform("iMV", ParameterType::MAT4);
    renderShader->AddUniform("P", ParameterType::MAT4);
    renderShader->AddUniform("FC", ParameterType::FLOAT);
    renderShader->AddUniform("eyePos", ParameterType::VEC3);
    renderShader->AddUniform("viewDir", ParameterType::VEC3);
    renderShader->AddUniform("color", ParameterType::VEC4);
    renderShader->AddUniform("texAlbedo", ParameterType::INT);
    renderShader->AddUniform("enableAlbedoTex", ParameterType::BOOLEAN);
    renderShader->AddUniform("enableNormalTex", ParameterType::BOOLEAN);
    renderShader->AddUniform("reflectivity", ParameterType::FLOAT);
    renderShader->AddUniform("cWater", ParameterType::VEC3);
    renderShader->AddUniform("bWater", ParameterType::VEC3);
    renderShader->AddUniform("transmittance_texture", ParameterType::INT);
    renderShader->AddUniform("scattering_texture", ParameterType::INT);
    renderShader->AddUniform("irradiance_texture", ParameterType::INT);
    renderShader->BindUniformBlock("SunSky", UBO_SUNSKY);
    renderShader->BindUniformBlock("Lights", UBO_LIGHTS);
    renderShader->BindShaderStorageBlock("Positions", SSBO_PARTICLE_POS);

    renderShader->Use();
    renderShader->SetUniform("texAlbedo", TEX_MAT_ALBEDO);
    renderShader->SetUniform("enableAlbedoTex", true);
    renderShader->SetUniform("enableNormalTex", false);
    renderShader->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    renderShader->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    renderShader->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    renderShader->SetUniform("reflectivity", 0.f);
    renderShader->SetUniform("color", glm::vec4(0.f,0.f,0.f,0.3f));
    OpenGLState::UseProgram(0);

    sources.clear();
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "particle.vert"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanParticleSegmentation.frag"));
    
    renderIdShader = std::make_unique<GLSLShader>(sources);
    renderIdShader->AddUniform("MV", ParameterType::MAT4);
    renderIdShader->AddUniform("iMV", ParameterType::MAT4);
    renderIdShader->AddUniform("P", ParameterType::MAT4);
    renderIdShader->AddUniform("FC", ParameterType::FLOAT);
    renderIdShader->AddUniform("texAlbedo", ParameterType::INT);
    renderIdShader->AddUniform("objectId", ParameterType::UINT);
    renderIdShader->BindShaderStorageBlock("Positions", SSBO_PARTICLE_POS);

    renderIdShader->Use();
    renderIdShader->SetUniform("texAlbedo", TEX_MAT_ALBEDO);
    OpenGLState::UseProgram(0);

    //Load textures
    flakeTexture = OpenGLContent::LoadInternalTexture("flake.png", true, true);

    unsigned int seed = (unsigned int)GetTimeInMicroseconds();
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int8_t> dist(-127,127);
    glm::uvec3 noiseSize3(noiseSize, noiseSize, noiseSize);
    std::vector<int8_t> noiseData(noiseSize3.x * noiseSize3.y * noiseSize3.z * 4);
    int8_t* ptr = noiseData.data();
    for(unsigned int z=0; z<noiseSize3.z; ++z)
        for(unsigned int y=0; y<noiseSize3.y; ++y) 
            for(unsigned int x=0; x<noiseSize3.x; ++x) 
            {
              *ptr++ = dist(generator);
              *ptr++ = dist(generator);
              *ptr++ = dist(generator);
              *ptr++ = dist(generator);
            }
    noiseTexture = OpenGLContent::GenerateTexture(GL_TEXTURE_3D, noiseSize3,
                                                  GL_RGBA8_SNORM, GL_RGBA, GL_BYTE, noiseData.data(), FilteringMode::BILINEAR, true);
}

void OpenGLOceanParticles::Destroy()
{
    if(flakeTexture != 0) glDeleteTextures(1, &flakeTexture);
    if(noiseTexture != 0) glDeleteTextures(1, &noiseTexture);
    updateShader.reset();
    renderShader.reset();
    renderIdShader.reset();
}
    
}
