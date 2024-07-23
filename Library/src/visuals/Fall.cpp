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
//  Fall.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 01/07/24.
//  Copyright(c) 2024 Patryk Cieslak. All rights reserved.
//

#include "visuals/Fall.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "core/MaterialManager.h"
#include "graphics/OpenGLFall.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Fall::Fall(const std::string& uniqueName, unsigned int maxParticles, Scalar lifetime, Scalar emitterSizeX, Scalar emitterSizeY, 
            std::vector<std::string> particleModelPaths, const std::string& particleMaterial, const std::string& particleLook)
    : Visual(uniqueName)
{    
    Material material = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(particleMaterial);
    Look look = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->getLook(particleLook);
    bool texturableRequired = look.normalTexture > 0 || look.albedoTexture > 0;

    std::vector<Mesh*> meshes;
    for(size_t i=0; i<particleModelPaths.size(); ++i)
    {
        Mesh* mesh = OpenGLContent::LoadMesh(particleModelPaths[i], 1.f, false);
        if(mesh == nullptr)
            cCritical("Failed to load a mesh for the particle system! Path: " + particleModelPaths[i]);
        if(texturableRequired && !mesh->isTexturable())
            cCritical("The mesh for the particle system is not texturable!");
        if(!texturableRequired && mesh->isTexturable())
        {
            PlainMesh* plainMesh = OpenGLContent::ConvertToPlainMesh((TexturableMesh*)mesh);
            delete mesh;
            mesh = plainMesh;
        }
        meshes.push_back(mesh);
    }

    sizeX = emitterSizeX > Scalar(0) ? emitterSizeX : Scalar(1);
    sizeY = emitterSizeY > Scalar(0) ? emitterSizeY : Scalar(1);

    glFall = new OpenGLFall(maxParticles, lifetime, glm::vec2((GLfloat)sizeX, (GLfloat)sizeY), meshes, material, look);

    UpdateTransform();
    glFall->UpdateTransform();
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddParticleSystem(glFall);
}

Fall::~Fall()
{
    delete glFall;
}

VisualType Fall::getType() const
{
    return VisualType::FALL;
}

void Fall::UpdateTransform()
{
    Transform t = getVisualFrame();
    Vector3 p = t.getOrigin();
    Vector3 d = t.getBasis().getColumn(2);
    glm::vec3 pos((GLfloat)p.x(), (GLfloat)p.y(), (GLfloat)p.z());
    glm::vec3 dir((GLfloat)d.x(), (GLfloat)d.y(), (GLfloat)d.z());
    glFall->UpdateEmitter(pos, dir);
}

std::vector<Renderable> Fall::Render()
{
    std::vector<Renderable> items;
    
    //Render emitter
    Renderable item;
    item.model = glMatrixFromTransform(getVisualFrame());
    item.type = RenderableType::VISUAL_LINES;
    
    item.points.push_back(glm::vec3(-(GLfloat)sizeX/2.f, (GLfloat)sizeY/2.f, 0));
    item.points.push_back(glm::vec3((GLfloat)sizeX/2.f, (GLfloat)sizeY/2.f, 0));

    item.points.push_back(glm::vec3(-(GLfloat)sizeX/2.f, -(GLfloat)sizeY/2.f, 0));
    item.points.push_back(glm::vec3((GLfloat)sizeX/2.f, -(GLfloat)sizeY/2.f, 0));

    item.points.push_back(glm::vec3(-(GLfloat)sizeX/2.f, -(GLfloat)sizeY/2.f, 0));
    item.points.push_back(glm::vec3(-(GLfloat)sizeX/2.f, (GLfloat)sizeY/2.f, 0));

    item.points.push_back(glm::vec3((GLfloat)sizeX/2.f, -(GLfloat)sizeY/2.f, 0));
    item.points.push_back(glm::vec3((GLfloat)sizeX/2.f, (GLfloat)sizeY/2.f, 0));

    item.points.push_back(glm::vec3(0.f, 0.f, 0.f));
    item.points.push_back(glm::vec3(0.f, 0.f, (GLfloat)sqrtf(sizeX*sizeX + sizeY*sizeY)));

    items.push_back(item);    
    return items;
}

}