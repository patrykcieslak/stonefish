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
    material(material), look(look), nParticles(0)
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

    //Draw indirect command buffer
    DrawElementsIndirectCommand cmd;
    glGenBuffers(1, &particleDIB);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, particleDIB);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand) * maxParticles, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    //Generate draw commands based on mesh data
    offset = 0;
    GLuint offsetV = 0;
    for(size_t i=0; i<meshes.size(); ++i)
    {
        DrawElementsIndirectCommand cmd;
        Mesh* mesh = meshes[i];
        cmd.count = mesh->faces.size();
        cmd.instanceCount = 1;
        cmd.firstIndex = offset;
        cmd.baseVertex = offsetV;
        cmd.baseInstance = 0;
        drawCommands.push_back(cmd);

        offset += sizeof(Face) * mesh->faces.size();
        offsetV += mesh->getVertexSize() * mesh->getNumOfVertices();
    }

    //Generate particle data buffers
    glGenBuffers(1, &poseSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, poseSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(glm::vec4) * maxParticles, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glGenBuffers(1, &twistSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, twistSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(glm::vec4) * maxParticles, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glGenBuffers(1, &ageSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ageSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * maxParticles, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
        
OpenGLFall::~OpenGLFall()
{
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleVertexVBO);
    glDeleteBuffers(1, &particleIndexVBO);
    glDeleteBuffers(1, &poseSSBO);
    glDeleteBuffers(1, &twistSSBO);
    glDeleteBuffers(1, &ageSSBO);
}
        
void OpenGLFall::Setup(OpenGLCamera* cam)
{
    //Create particles randomly (uniformly) distributed over the emmiter surface
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, poseSSBO);
    ParticlePose* poses = (ParticlePose*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ParticlePose) * maxParticles, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    
    GLfloat diagonal = glm::length(emitterSize);
    GLuint numParticles = 0;
    do
    {
        // Generate point in a circle
        GLfloat r = sqrtf(uniformDist(randGen)) * diagonal;
        glm::vec2 p = glm::vec2(r * glm::normalize(glm::vec2(normalDist(randGen), normalDist(randGen))));

        // Test if point lies inside the emitter
        if(p.x > -emitterSize.x/2.f && p.x < emitterSize.x/2.f && p.y > -emitterSize.y/2.f && p.y < emitterSize.y/2.f)
        {
            // Set particle pose and scale
            poses[numParticles].posScaleX = glm::vec4(p.x, p.y, 0.0, 1.f + uniformDist(randGen)*0.25f);
            poses[numParticles].ori = glm::vec4(glm::normalize(glm::vec3(uniformDist(randGen), uniformDist(randGen), uniformDist(randGen))), uniformDist(randGen)*2.f*glm::pi<GLfloat>());
            ++numParticles;
        }
    }
    while(numParticles < maxParticles);
    
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    //Reset velocities and missing components of scale
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, twistSSBO);
    ParticleTwist* twists = (ParticleTwist*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ParticleTwist) * maxParticles, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);    

    for(GLuint i=0; i<maxParticles; ++i)
    {
        twists[i].velScaleY = glm::vec4(0.f, 0.f, 0.f, 1.f + uniformDist(randGen)*0.25f);
        twists[i].avelScaleZ = glm::vec4(0.f, 0.f, 0.f, 1.f + uniformDist(randGen)*0.25f);
    }
    
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    //Reset particle age
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ageSSBO);
    GLfloat* age = (GLfloat*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLfloat) * maxParticles, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memset(age, 0, sizeof(GLfloat) * maxParticles);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Generate draw commands - particles meshes are random but fixed
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, particleDIB);
    DrawElementsIndirectCommand* cmds = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, sizeof(DrawElementsIndirectCommand) * maxParticles, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    
    for(GLuint i=0; i<maxParticles; ++i)
    {
        size_t index = (size_t)roundf((uniformDist(randGen)+1.f)/2.f * (drawCommands.size()-1));
        cmds[i] = drawCommands[index];
        cmds[i].instanceCount = i;
    }

    glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    initialized = true;
}

void OpenGLFall::Update(OpenGLCamera* cam, GLfloat dt)
{
    if(!initialized)
    {
        Setup(cam);
        return;
    }

    //Update particles
    // updateShader->Use();
    // updateShader->SetUniform("dt", dt);
    // updateShader->SetUniform("numParticles", nParticles);
    


}

void OpenGLFall::Draw(OpenGLCamera* cam)
{

    //Draw particles
    // renderShader->Use();
    // renderShader->SetUniform("MV", cam->getMV());
    // renderShader->SetUniform("iMV", cam->getIMV());
    // renderShader->SetUniform("P", cam->getP());
    // renderShader->SetUniform("FC", cam->getFarClip());
    // renderShader->SetUniform("eyePos", cam->getPos());
    // renderShader->SetUniform("viewDir", cam->getDir());
    // renderShader->SetUniform("color", look.color);
    // renderShader->SetUniform("reflectivity", material.reflectivity);
    // renderShader->SetUniform("cWater", material.cWater);
    // renderShader->SetUniform("bWater", material.bWater);
    // renderShader->SetUniform("SunSky", UBO_SUNSKY);
    // renderShader->SetUniform("Lights", UBO_LIGHTS);
    // renderShader->SetUniform("Positions", SSBO_PARTICLE_POSE);
    // renderShader->SetUniform("enableAlbedoTex", look.textured);
    // renderShader->SetUniform("enableNormalTex", look.textured && look.normalMap != nullptr);
    
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, particleDIB);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)0, nParticles, 0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    // OpenGLState::UseProgram(0);
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
    updateShader->AddUniform("lifetime", ParameterType::FLOAT);
    updateShader->AddUniform("texNoise", ParameterType::INT);
    updateShader->AddUniform("invNoiseSize", ParameterType::FLOAT);
    updateShader->BindUniformBlock("OceanCurrents", UBO_OCEAN_CURRENTS);
    updateShader->BindShaderStorageBlock("Poses", SSBO_PARTICLE_POSE);
    updateShader->BindShaderStorageBlock("Twists", SSBO_PARTICLE_TWIST);
    updateShader->BindShaderStorageBlock("Age", SSBO_PARTICLE_AGE);

    updateShader->Use();
    updateShader->SetUniform("texNoise", TEX_MAT_ALBEDO);
    updateShader->SetUniform("invNoiseSize", 1.f/(GLfloat)noiseSize);
    OpenGLState::UseProgram(0);

    sources.clear();


    //sources.push_back(GLSLSource(GL_VERTEX_SHADER, "particle.vert"));
    // sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "lightingNoShadow.frag"));
    // sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanOptics.frag"));
    // sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "oceanSurfaceFlat.glsl"));
    // sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "fallParticle.frag"));
    // sources.push_back(GLSLSource(GL_FRAGMENT_SHADER, "materialUUv.frag"));
    
    // renderShader = new GLSLShader(sources, precompiled);
    // renderShader->AddUniform("MV", ParameterType::MAT4);
    // renderShader->AddUniform("iMV", ParameterType::MAT4);
    // renderShader->AddUniform("P", ParameterType::MAT4);
    // renderShader->AddUniform("FC", ParameterType::FLOAT);
    // renderShader->AddUniform("eyePos", ParameterType::VEC3);
    // renderShader->AddUniform("viewDir", ParameterType::VEC3);
    // renderShader->AddUniform("color", ParameterType::VEC4);
    // renderShader->AddUniform("texAlbedo", ParameterType::INT);
    // renderShader->AddUniform("enableAlbedoTex", ParameterType::BOOLEAN);
    // renderShader->AddUniform("enableNormalTex", ParameterType::BOOLEAN);
    // renderShader->AddUniform("reflectivity", ParameterType::FLOAT);
    // renderShader->AddUniform("cWater", ParameterType::VEC3);
    // renderShader->AddUniform("bWater", ParameterType::VEC3);
    // renderShader->AddUniform("transmittance_texture", ParameterType::INT);
    // renderShader->AddUniform("scattering_texture", ParameterType::INT);
    // renderShader->AddUniform("irradiance_texture", ParameterType::INT);
    // renderShader->BindUniformBlock("SunSky", UBO_SUNSKY);
    // renderShader->BindUniformBlock("Lights", UBO_LIGHTS);
    // renderShader->BindShaderStorageBlock("Positions", SSBO_PARTICLE_POSE);

    // renderShader->Use();
    // renderShader->SetUniform("texAlbedo", TEX_MAT_ALBEDO);
    // renderShader->SetUniform("enableAlbedoTex", true);
    // renderShader->SetUniform("enableNormalTex", false);
    // renderShader->SetUniform("transmittance_texture", TEX_ATM_TRANSMITTANCE);
    // renderShader->SetUniform("scattering_texture", TEX_ATM_SCATTERING);
    // renderShader->SetUniform("irradiance_texture", TEX_ATM_IRRADIANCE);
    // renderShader->SetUniform("reflectivity", 0.f);
    // renderShader->SetUniform("color", glm::vec4(0.f,0.f,0.f,0.3f));
    // OpenGLState::UseProgram(0);
}

void OpenGLFall::Destroy()
{
    if(updateShader != nullptr) delete updateShader;
    if(renderShader != nullptr) delete renderShader;
}

}