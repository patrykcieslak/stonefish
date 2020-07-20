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
//  AnimatedEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 15/07/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "entities/AnimatedEntity.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

AnimatedEntity::AnimatedEntity(std::string uniqueName, std::string graphicsFilename, 
                               Scalar graphicsScale, const Transform& graphicsOrigin, std::string material, std::string look) : MovingEntity(uniqueName, material, look)
{
    if(SimulationApp::getApp()->hasGraphics())
    {
        Mesh* mesh = OpenGLContent::LoadMesh(graphicsFilename, graphicsScale, false);
        OpenGLContent::TransformMesh(mesh, graphicsOrigin);
        graObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(mesh);
        delete mesh;
    }
    T_O = I4();
    vel = V0();
    aVel = V0();
    tg = nullptr;
}

AnimatedEntity::~AnimatedEntity()
{
    if(tg != nullptr)
        delete tg;
}

EntityType AnimatedEntity::getType() const
{
    return EntityType::ANIMATED;
}

Transform AnimatedEntity::getOTransform() const
{
    return T_O;
}

Transform AnimatedEntity::getCGTransform() const
{
    return T_O;
}

Vector3 AnimatedEntity::getLinearVelocity() const
{
    return V0();
}

Vector3 AnimatedEntity::getAngularVelocity() const
{
    return V0();
}

Vector3 AnimatedEntity::getLinearVelocityInLocalPoint(const Vector3& relPos) const
{
    return getLinearVelocity() + getAngularVelocity().cross(relPos);
}

Vector3 AnimatedEntity::getLinearAcceleration() const
{
    return V0();
}
        
Vector3 AnimatedEntity::getAngularAcceleration() const
{
    return V0();
}

void AnimatedEntity::getAABB(Vector3& min, Vector3& max)
{
    min = Vector3(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max = -min;
}

void AnimatedEntity::setOTransform(const Transform& trans)
{
    T_O = trans;
}

void AnimatedEntity::ConnectTrajectoryGenerator(TrajectoryGenerator* generator)
{
    tg = generator;
    T_O.setOrigin(tg->getCurrentPoint());
}

void AnimatedEntity::Update(Scalar dt)
{
    if(tg != nullptr)
    {
        tg->Advance(dt);
        T_O.setOrigin(tg->getCurrentPoint());
        vel = tg->getCurrentVelocity();
    }
}

void AnimatedEntity::AddToSimulation(SimulationManager* sm)
{
    AddToSimulation(sm, I4());
}
        
void AnimatedEntity::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    setOTransform(origin);
}

std::vector<Renderable> AnimatedEntity::Render()
{
    std::vector<Renderable> items(0);
    //Render animated object
    if(graObjectId >= 0 && isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SOLID;
        item.materialName = mat.name;
        item.objectId = graObjectId;
        item.lookId = dm == DisplayMode::GRAPHICAL ? lookId : -1;
        item.model = glMatrixFromTransform(getOTransform());
        items.push_back(item);
    }
    //Render object trajectory
    if(tg != nullptr)
    {
        const std::vector<Renderable>& tgItems = tg->Render();
        items.insert(items.begin(), tgItems.begin(), tgItems.end());
    }
    return items;
}

}
