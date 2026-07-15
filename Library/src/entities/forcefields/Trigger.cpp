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
//  Trigger.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 21/04/18.
//  Copyright(c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Trigger.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Trigger::Trigger(const std::string& uniqueName, Scalar radius, const Transform& worldTransform, const std::string& look) : ForcefieldEntity(uniqueName)
{
    ghost_->setCollisionFlags(ghost_->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    ghost_->setWorldTransform(worldTransform);
    collisionShape_ = std::make_unique<btSphereShape>(radius);
    ghost_->setCollisionShape(collisionShape_.get());
    active_ = false;
    
    std::unique_ptr<Mesh> mesh = OpenGLContent::BuildSphere((GLfloat)radius);
    
    if(SimulationApp::getApp()->hasGraphics())
    {
        objectId_ = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(mesh.get());
        lookId_ = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->getLookId(look);
    }
    else
    {
        objectId_ = 0;
        lookId_ = -1;
    }
}

Trigger::Trigger(const std::string& uniqueName, Scalar radius, Scalar length, const Transform& worldTransform, const std::string& look) : ForcefieldEntity(uniqueName)
{
    ghost_->setCollisionFlags(ghost_->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    ghost_->setWorldTransform(worldTransform);
    ghost_->setCollisionShape(new btCylinderShape(Vector3(radius, length/Scalar(2), radius)));
    active_ = false;
    
    std::unique_ptr<Mesh> mesh = OpenGLContent::BuildCylinder((GLfloat)radius, (GLfloat)length);
    
    if(SimulationApp::getApp()->hasGraphics())
    {
        objectId_ = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(mesh.get());
        lookId_ = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->getLookId(look);
    }
    else
    {
        objectId_ = 0;
        lookId_ = -1;
    }
}

Trigger::Trigger(const std::string& uniqueName, const Vector3& dimensions, const Transform& worldTransform, const std::string& look) : ForcefieldEntity(uniqueName)
{
    ghost_->setCollisionFlags(ghost_->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    ghost_->setWorldTransform(worldTransform);
    ghost_->setCollisionShape(new btBoxShape(dimensions/Scalar(2)));
    active_ = false;
    
    glm::vec3 halfExt((GLfloat)(dimensions.x()/Scalar(2)), (GLfloat)(dimensions.y()/Scalar(2)), (GLfloat)(dimensions.z()/Scalar(2)));
    std::unique_ptr<Mesh> mesh = OpenGLContent::BuildBox(halfExt);
    
    if(SimulationApp::getApp()->hasGraphics())
    {
        objectId_ = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(mesh.get());
        lookId_ = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->getLookId(look);
    }
    else
    {
        objectId_ = 0;
        lookId_ = -1;
    }
}

ForcefieldType Trigger::getForcefieldType()
{
    return ForcefieldType::TRIGGER;
}

void Trigger::AddActiveSolid(SolidEntity* solid)
{
    solids_.push_back(solid);
}

void Trigger::Activate(btCollisionObject* co)
{
    if(solids_.size() == 0)
        return;
    
    Entity* ent;
    btRigidBody* rb = btRigidBody::upcast(co);
    btMultiBodyLinkCollider* mbl = btMultiBodyLinkCollider::upcast(co);
    
    if(rb != 0)
    {
        if(rb->isStaticOrKinematicObject())
            return;
        else
            ent = (Entity*)rb->getUserPointer();
    }
    else if(mbl != 0)
    {
        if(mbl->isStaticOrKinematicObject())
            return;
        else
            ent = (Entity*)mbl->getUserPointer();
    }
    else
        return;
    
    if(ent->getType() == EntityType::SOLID)
    {
        SolidEntity* solid = (SolidEntity*)ent;
        for(unsigned int i=0; i<solids_.size(); ++i)
            if(solids_[i] == solid)
            {
                active_ = true;
                return;
            }
    }
}

void Trigger::Clear()
{
    active_ = false;
}

bool Trigger::isActive()
{
    return active_;
}

std::vector<Renderable> Trigger::Render()
{
    std::vector<Renderable> items(0);
    
    if(objectId_ >= 0 && isRenderable())
    {
        Transform trans = ghost_->getWorldTransform();
        Renderable item;
        item.type = RenderableType::SOLID;
        item.objectId = objectId_;
        item.lookId = lookId_;
        item.model = glMatrixFromTransform(trans);
        items.push_back(item);
    }
    
    return items;
}

}
