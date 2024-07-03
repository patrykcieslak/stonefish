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
//  OpenGLMarineSnow.h
//  Stonefish
//
//  Created by Patryk Cieslak on 04/08/19.
//  Copyright (c) 2019-2024 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLMarineSnow.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLCamera.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLAtmosphere.h"
#include "graphics/OpenGLOcean.h"
#include "entities/environment/Ocean.h"
#include "utils/SystemUtil.hpp"

namespace sf
{
    
GLuint OpenGLMarineSnow::flakeTexture = 0;
GLSLShader* OpenGLMarineSnow::renderShader = nullptr;
GLSLShader* OpenGLMarineSnow::updateShader = nullptr;

OpenGLMarineSnow::OpenGLMarineSnow(GLuint maxParticles, GLfloat visibleRange) 
    : OpenGLParticleSystem(maxParticles)
{
    initialised = false;
    range = fabsf(visibleRange);
    lastEyePos = glm::vec3(0);

    glGenBuffers(1, &poseSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, poseSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * maxParticles, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glGenBuffers(1, &twistSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, twistSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * maxParticles, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glGenBuffers(1, &particleEAB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleEAB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6 * maxParticles, NULL, GL_STATIC_DRAW);
    GLuint* indices = (GLuint*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * 6 * maxParticles, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    for(GLuint i=0; i<maxParticles; ++i) 
    {
        GLuint index = GLuint(i<<2);
        *(indices++) = index;
        *(indices++) = index+2;
        *(indices++) = index+1;
        *(indices++) = index;
        *(indices++) = index+3;
        *(indices++) = index+2;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glGenVertexArrays(1, &particleVAO);
    OpenGLState::BindVertexArray(particleVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleEAB);
    OpenGLState::BindVertexArray(0);
}
    
OpenGLMarineSnow::~OpenGLMarineSnow()
{
    glDeleteBuffers(1, &poseSSBO);
    glDeleteBuffers(1, &twistSSBO);
    glDeleteBuffers(1, &particleEAB);
    glDeleteVertexArrays(1, &particleVAO);
}

void OpenGLMarineSnow::Setup(OpenGLCamera* cam)
{
    lastEyePos = cam->GetEyePosition();

    //Create particles randomly (uniformly) distributed inside a sphere
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, poseSSBO);
    glm::vec4* positions = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * maxParticles, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    for(GLuint i=0; i<maxParticles; ++i) 
    {
        GLfloat r = cbrtf(uniformDist(randGen)) * range; //cbrtf for uniform distribution in sphere volume
        *(positions++) = glm::vec4(r * glm::normalize(glm::vec3(normalDist(randGen), normalDist(randGen), normalDist(randGen))) + lastEyePos, uniformDist(randGen)*0.005f + 0.002f);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, twistSSBO);
    glm::vec4* velocities = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * maxParticles, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memset(velocities, 0, sizeof(glm::vec4) * maxParticles);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    initialised = true;
}
    
void OpenGLMarineSnow::Update(OpenGLCamera* cam, GLfloat dt)
{
    //Check if ever updated
    if(!initialised)
    {
        Setup(cam);
        return;
    }
    
    lastEyePos = cam->GetEyePosition();;

    //Move particles
    updateShader->Use();
    updateShader->SetUniform("dt", dt);
    updateShader->SetUniform("numParticles", maxParticles);
    updateShader->SetUniform("eyePos", lastEyePos);
    updateShader->SetUniform("R", range);
    OpenGLState::BindTexture(TEX_MAT_ALBEDO, GL_TEXTURE_3D, noiseTexture);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_PARTICLE_POS, poseSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_PARTICLE_VEL, twistSSBO);
    glDispatchCompute((GLuint)ceil(maxParticles/256.0), 1, 1);
    OpenGLState::UnbindTexture(TEX_MAT_ALBEDO);
    OpenGLState::UseProgram(0);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
    
void OpenGLMarineSnow::Draw(OpenGLCamera* cam)
{
    OpenGLOcean* glOcn = (OpenGLOcean*)SimulationApp::getApp()->getSimulationManager()->getOcean()->getOpenGLOcean();
    if(glOcn == nullptr)
        return;

    glm::mat4 MV = cam->GetViewMatrix();
    renderShader->Use();
    renderShader->SetUniform("MV", MV);
    renderShader->SetUniform("iMV", glm::inverse(MV));
    renderShader->SetUniform("P", cam->GetProjectionMatrix());
    renderShader->SetUniform("FC", cam->GetLogDepthConstant());
    renderShader->SetUniform("eyePos", cam->GetEyePosition());
    renderShader->SetUniform("viewDir", cam->GetLookingDirection());
    renderShader->SetUniform("cWater", glOcn->getLightAttenuation());
    renderShader->SetUniform("bWater", glOcn->getLightScattering());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_PARTICLE_POS, poseSSBO);
    OpenGLState::BindTexture(TEX_MAT_ALBEDO, GL_TEXTURE_2D, flakeTexture);
    OpenGLState::EnableBlend();
    glBlendFunc(GL_ONE, GL_ONE);
    OpenGLState::BindVertexArray(particleVAO);
    glDrawElements(GL_TRIANGLES, maxParticles * 6, GL_UNSIGNED_INT, 0);
    OpenGLState::BindVertexArray(0);
    OpenGLState::DisableBlend();
    OpenGLState::UnbindTexture(TEX_MAT_ALBEDO);
    OpenGLState::UseProgram(0);
}
    
void OpenGLMarineSnow::Init()
{
    //Load shaders
	std::vector<GLuint> precompiled;
    precompiled.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    
	std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "marineSnowParticle.comp"));
    updateShader = new GLSLShader(sources);
    updateShader->AddUniform("dt", ParameterType::FLOAT);
    updateShader->AddUniform("numParticles", ParameterType::UINT);
    updateShader->AddUniform("eyePos", ParameterType::VEC3);
    updateShader->AddUniform("R", ParameterType::FLOAT);
    updateShader->AddUniform("texNoise", ParameterType::INT);
    updateShader->AddUniform("invNoiseSize", ParameterType::FLOAT);
    updateShader->BindUniformBlock("OceanCurrents", UBO_OCEAN_CURRENTS);
    updateShader->BindShaderStorageBlock("Positions", SSBO_PARTICLE_POS);
    updateShader->BindShaderStorageBlock("Velocities", SSBO_PARTICLE_VEL);

    updateShader->Use();
    updateShader->SetUniform("texNoise", TEX_MAT_ALBEDO);
    updateShader->SetUniform("invNoiseSize", 1.f/(GLfloat)noiseSize);
    OpenGLState::UseProgram(0);

    sources.clear();
    sources.push_back(GLSLSource(GL_VERTEX_SHADER, "particle.vert"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "lightingNoShadow.frag"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanOptics.frag"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanSurfaceFlat.glsl"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "marineSnowParticle.frag"));
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "materialUUv.frag"));
    
    renderShader = new GLSLShader(sources, precompiled);
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

    //Load textures
    flakeTexture = OpenGLContent::LoadInternalTexture("flake.png", true, true);
}

void OpenGLMarineSnow::Destroy()
{
    if(updateShader != nullptr) delete updateShader;
    if(renderShader != nullptr) delete renderShader;
    if(flakeTexture != 0) glDeleteTextures(1, &flakeTexture);
}
    
}
