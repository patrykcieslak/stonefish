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
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLOceanParticles.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLCamera.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLAtmosphere.h"
#include "graphics/OpenGLOcean.h"
#include "entities/forcefields/Ocean.h"

namespace sf
{
    
GLSLShader* OpenGLOceanParticles::particleShader = NULL;

OpenGLOceanParticles::OpenGLOceanParticles(size_t numOfParticles, GLfloat visibleRange) : OpenGLParticles(numOfParticles), uniformd(0, 1.f), normald(0, 1.f)
{
    initialised = false;
    range = fabsf(visibleRange);
    lastEyePos = glm::vec3(0);
    
    glGenVertexArrays(1, &vao);
    OpenGLState::BindVertexArray(vao);
    
    static const GLfloat billboard[] = { 
         -0.5f, -0.5f, 0.0f,
          0.5f, -0.5f, 0.0f,
         -0.5f,  0.5f, 0.0f,
          0.5f,  0.5f, 0.0f,
    };
    
    glGenBuffers(1, &vboParticle);
    glBindBuffer(GL_ARRAY_BUFFER, vboParticle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(billboard), billboard, GL_STATIC_DRAW);

    glGenBuffers(1, &vboPositionSize);
    glBindBuffer(GL_ARRAY_BUFFER, vboPositionSize);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(glm::vec4), NULL, GL_STREAM_DRAW);

    OpenGLState::BindVertexArray(0);
    
    flakeTexture = OpenGLContent::LoadInternalTexture("flake.png", true, 0.f);
}
    
OpenGLOceanParticles::~OpenGLOceanParticles()
{
    if(vboParticle > 0) glDeleteBuffers(1, &vboParticle);
    if(vboPositionSize > 0) glDeleteBuffers(1, &vboPositionSize);
    if(vao > 0) glDeleteVertexArrays(1, &vao);
}

void OpenGLOceanParticles::Create(glm::vec3 eyePos)
{
    lastEyePos = eyePos;
    initialised = true;
    
    //Create particles randomly (uniformly) distributed inside a sphere
    for(size_t i=0; i<nParticles; ++i)
    {
        GLfloat r = cbrtf(uniformd(generator)) * range; //cbrtf for uniform distribution in sphere volume
        positionsSizes[i] = glm::vec4(r * glm::normalize(glm::vec3(normald(generator), normald(generator), normald(generator))) + eyePos, uniformd(generator)*0.01f + 0.002f);
        velocities[i] = 0.01f * glm::vec3(normald(generator), normald(generator), normald(generator));
    }
}
    
void OpenGLOceanParticles::Update(OpenGLCamera* cam, Ocean* ocn, GLfloat dt)
{
    glm::vec3 eyePos = cam->GetEyePosition();
    
    //Check if ever updated
    if(!initialised)
    {
        Create(eyePos);
        return;
    }
    
    //Simulate motion
    for(size_t i=0; i<nParticles; ++i)
    {
        positionsSizes[i].x += velocities[i].x * dt;
        positionsSizes[i].y += velocities[i].y * dt;
        positionsSizes[i].z += velocities[i].z * dt;
        
        Vector3 v = ocn->GetFluidVelocity(Vector3(positionsSizes[i].x, positionsSizes[i].y, positionsSizes[i].z));
        velocities[i] += glm::vec3((GLfloat)v.x(), (GLfloat)v.y(), (GLfloat)v.z()) * dt - 0.1f * velocities[i] * dt;
    }
    
    //Determine camera moving direction - can be used to generate particles in the direction of movement?
    /*
    glm::vec3 dP = eyePos - lastEyePos;
    lastEyePos = eyePos;
    bool cameraMoved = glm::length2(dP) > 0.01f*0.01f; //Camera moved more than 1 cm?
    if(cameraMoved) 
        dP = glm::normalize(dP);*/
        
    //Kill and create (relocate) particles 
    GLfloat range2 = range*range;
    
    //Relocate particles out of range to a random position on a sphere around camera
    for(size_t i=0; i<nParticles; ++i)
    {
        if(glm::length2(glm::vec3(positionsSizes[i]) - eyePos) > range2) //If particle is out of range
        {
            GLfloat r = cbrtf(uniformd(generator)) * range;
            positionsSizes[i] = glm::vec4(r * glm::normalize(glm::vec3(normald(generator), normald(generator), normald(generator))) + eyePos, uniformd(generator)*0.01f + 0.002f);
            velocities[i] = 0.01f * glm::vec3(normald(generator), normald(generator), normald(generator));
        }
    }
}
    
void OpenGLOceanParticles::Draw(OpenGLCamera* cam, OpenGLOcean* glOcn)
{
    glm::mat4 projection = cam->GetProjectionMatrix();
    glm::mat4 view = cam->GetViewMatrix();
    
    OpenGLState::BindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vboPositionSize);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(glm::vec4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(glm::vec4), &positionsSizes[0].x);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vboParticle);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vboPositionSize);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);
    
    OpenGLState::BindTexture(TEX_MAT_DIFFUSE, GL_TEXTURE_2D, flakeTexture);
    OpenGLState::EnableBlend();
    glBlendFunc(GL_ONE, GL_ONE);
    
    particleShader->Use();
    particleShader->SetUniform("MVP", projection * view);
    particleShader->SetUniform("camRight", glm::vec3(view[0][0], view[1][0], view[2][0]));
    particleShader->SetUniform("camUp", glm::vec3(view[0][1], view[1][1], view[2][1]));
    particleShader->SetUniform("FC", cam->GetLogDepthConstant());
    particleShader->SetUniform("eyePos", cam->GetEyePosition());
    particleShader->SetUniform("viewDir", cam->GetLookingDirection());
    particleShader->SetUniform("cWater", glOcn->getLightAttenuation());
    particleShader->SetUniform("bWater", glOcn->getLightScattering());
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)nParticles);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    OpenGLState::DisableBlend();
    OpenGLState::UseProgram(0);
    OpenGLState::BindVertexArray(0);
}
    
void OpenGLOceanParticles::Init()
{
    GLint compiled;
    std::string header1 = "#version 330\n";
	header1 += "#define MAX_POINT_LIGHTS " + std::to_string(MAX_POINT_LIGHTS) + "\n";
	header1 += "#define MAX_SPOT_LIGHTS " + std::to_string(MAX_SPOT_LIGHTS) + "\n";
    std::string header2 = "#version 330\n";
	header2 += "#define MAX_POINT_LIGHTS " + std::to_string(MAX_POINT_LIGHTS) + "\n";
	header2 += "#define MAX_SPOT_LIGHTS " + std::to_string(MAX_SPOT_LIGHTS) + "\n";
    
	std::vector<GLuint> commonMaterialShaders;
    commonMaterialShaders.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    
	GLuint lightingFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "lightingNoShadow.frag", header1, &compiled);
	commonMaterialShaders.push_back(lightingFragment);

    GLuint oceanOpticsFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "oceanOptics.frag", "", &compiled);
	commonMaterialShaders.push_back(oceanOpticsFragment);
    
    GLuint materialFragment = GLSLShader::LoadShader(GL_FRAGMENT_SHADER, "uwMaterial.frag", header2, &compiled);
    commonMaterialShaders.push_back(materialFragment);

    particleShader = new GLSLShader(commonMaterialShaders, "oceanParticle.frag", "billboard.vert");
    particleShader->AddUniform("MVP", ParameterType::MAT4);
    particleShader->AddUniform("camRight", ParameterType::VEC3);
    particleShader->AddUniform("camUp", ParameterType::VEC3);
    particleShader->AddUniform("FC", ParameterType::FLOAT);
    particleShader->AddUniform("eyePos", ParameterType::VEC3);
    particleShader->AddUniform("viewDir", ParameterType::VEC3);
    particleShader->AddUniform("color", ParameterType::VEC4);
    particleShader->AddUniform("tex", ParameterType::INT);
    particleShader->AddUniform("reflectivity", ParameterType::FLOAT);
    particleShader->AddUniform("cWater", ParameterType::VEC3);
    particleShader->AddUniform("bWater", ParameterType::VEC3);
    particleShader->AddUniform("transmittance_texture", ParameterType::INT);
    particleShader->AddUniform("scattering_texture", ParameterType::INT);
    particleShader->AddUniform("irradiance_texture", ParameterType::INT);
    particleShader->BindUniformBlock("SunSky", UBO_SUNSKY);
    particleShader->BindUniformBlock("Lights", UBO_LIGHTS);

    glDeleteShader(lightingFragment);
    glDeleteShader(oceanOpticsFragment);
    glDeleteShader(materialFragment);

    particleShader->Use();
    particleShader->SetUniform("tex", TEX_MAT_DIFFUSE);
    particleShader->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    particleShader->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    particleShader->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    particleShader->SetUniform("reflectivity", 0.f);
    particleShader->SetUniform("color", glm::vec4(0.f,0.f,0.f,1.f));
}

void OpenGLOceanParticles::Destroy()
{
    if(particleShader != NULL) delete particleShader;
}
    
}
