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
//  OpenGLFall.h
//  Stonefish
//
//  Created by Patryk Cieslak on 02/07/24.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLFall.h"

#include "core/SimulationApp.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLCamera.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLAtmosphere.h"
#include "graphics/OpenGLOcean.h"
#include "entities/environment/Ocean.h"

namespace sf
{

GLSLShader* OpenGLFall::renderShader = nullptr;
GLSLShader* OpenGLFall::updateShader = nullptr;

OpenGLFall::OpenGLFall(GLuint maxParticles, GLfloat lifetime, glm::vec2 emitterSize, const std::vector<Mesh*>& meshes, 
    const Material& material, const Look& look) : OpenGLParticleSystem(maxParticles), lifetime(lifetime), emitterSize(emitterSize),
    material(material), look(look)
{
    //Sum mesh vertices
    size_t numVertices = 0;
    size_t numFaces = 0;
    for(size_t i=0; i<meshes.size(); ++i)
    {
        numVertices += meshes[i]->getNumOfVertices();
        numFaces += meshes[i]->faces.size();
    }
  
    //Generate and fill buffers
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVertexVBO);
    glGenBuffers(1, &particleIndexVBO);

    OpenGLState::BindVertexArray(particleVAO);	
    glEnableVertexAttribArray(0); //Position
    glEnableVertexAttribArray(1); //Normal
    if(meshes[0]->isTexturable())
    {
        glEnableVertexAttribArray(2); //UV
        glEnableVertexAttribArray(3); //Tangent
    }

    //Vertex data
    glBindBuffer(GL_ARRAY_BUFFER, particleVertexVBO);
    glBufferData(GL_ARRAY_BUFFER, meshes[0]->getVertexSize() * numVertices, NULL, GL_STATIC_DRAW);
    
    GLuint offset = 0;
    for(size_t i=0; i<meshes.size(); ++i)
    {
        Mesh* mesh = meshes[i];
        glBufferSubData(GL_ARRAY_BUFFER, offset, mesh->getVertexSize() * mesh->getNumOfVertices(), mesh->getVertexDataPointer());
        offset += mesh->getVertexSize() * mesh->getNumOfVertices();
    }
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, meshes[0]->getVertexSize(), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE,  meshes[0]->getVertexSize(), (void*)sizeof(glm::vec3));
    if(meshes[0]->isTexturable())
    {
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, meshes[0]->getVertexSize(), (void*)(sizeof(glm::vec3)*2));
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE,  meshes[0]->getVertexSize(), (void*)(sizeof(glm::vec3)*2 + sizeof(glm::vec2)));
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //Index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleIndexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face) * numFaces, NULL, GL_STATIC_DRAW);
    offset = 0;
    for(size_t i=0; i<meshes.size(); ++i)
    {
        Mesh* mesh = meshes[i];
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sizeof(Face) * mesh->faces.size(), mesh->faces.data());
        offset += sizeof(Face) * mesh->faces.size();
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    OpenGLState::BindVertexArray(0);

    //Particle data buffers (3D pose and twist)
    glGenBuffers(1, &poseSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, poseSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(glm::vec4) * maxParticles, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glGenBuffers(1, &twistSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, twistSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(glm::vec4) * maxParticles, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
        
OpenGLFall::~OpenGLFall()
{
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleVertexVBO);
    glDeleteBuffers(1, &particleIndexVBO);
    glDeleteBuffers(1, &poseSSBO);
    glDeleteBuffers(1, &twistSSBO);
}
        
void OpenGLFall::Setup(OpenGLCamera* cam)
{
}

void OpenGLFall::Update(OpenGLCamera* cam, GLfloat dt)
{
}

void OpenGLFall::Draw(OpenGLCamera* cam)
{
}

void OpenGLFall::Init()
{
    //Load shaders
	std::vector<GLuint> precompiled;
    precompiled.push_back(OpenGLAtmosphere::getAtmosphereAPI());
    
	std::vector<GLSLSource> sources;
    sources.push_back(GLSLSource(GL_COMPUTE_SHADER, "fallParticle.comp"));
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
    sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "fallParticle.frag"));
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
}

void OpenGLFall::Destroy()
{
    if(updateShader != nullptr) delete updateShader;
    if(renderShader != nullptr) delete renderShader;
}

}