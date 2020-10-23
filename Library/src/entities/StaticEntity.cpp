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
//  StaticEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#include "entities/StaticEntity.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

StaticEntity::StaticEntity(std::string uniqueName, std::string material, std::string look) : Entity(uniqueName)
{
    mat = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(material);
    if(SimulationApp::getApp()->hasGraphics())
        lookId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->getLookId(look);
    else
        lookId = -1;
    phyObjectId = -1;
    dm = DisplayMode::GRAPHICAL;
    rigidBody = NULL;
    phyMesh = NULL;
}

StaticEntity::~StaticEntity()
{
    if(phyMesh != NULL) delete phyMesh;
}

EntityType StaticEntity::getType() const
{
    return EntityType::STATIC;
}

Material StaticEntity::getMaterial() const
{
    return mat;
}

void StaticEntity::setTransform(const Transform& trans)
{
    if(rigidBody != NULL)
    {
        rigidBody->getMotionState()->setWorldTransform(trans);
        rigidBody->setCenterOfMassTransform(trans);
    }
}

Transform StaticEntity::getTransform()
{
    if(rigidBody != NULL)
    {
        Transform T;
        rigidBody->getMotionState()->getWorldTransform(T);
        return T;
    }
    else
        return Transform::getIdentity();
}

btRigidBody* StaticEntity::getRigidBody()
{
    return rigidBody;
}

void StaticEntity::getAABB(Vector3& min, Vector3& max)
{
    if(rigidBody != NULL)
        rigidBody->getAabb(min, max);
}

void StaticEntity::setDisplayMode(DisplayMode m)
{
    dm = m;
}

std::vector<Renderable> StaticEntity::Render()
{
    std::vector<Renderable> items(0);
    
    if(rigidBody != NULL && phyObjectId >= 0 && isRenderable())
    {
        Transform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
        
        Renderable item;
        item.type = RenderableType::SOLID;
        item.materialName = mat.name;
        item.objectId = phyObjectId;
        item.lookId = dm == DisplayMode::GRAPHICAL ? lookId : -1;
        item.model = glMatrixFromTransform(trans);
        items.push_back(item);
    }
    
    return items;
}

void StaticEntity::BuildGraphicalObject()
{
    if(phyMesh == NULL || !SimulationApp::getApp()->hasGraphics())
        return;
    
    phyObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh);
}

void StaticEntity::BuildRigidBody(btCollisionShape* shape)
{
    btDefaultMotionState* motionState = new btDefaultMotionState();
    
    shape->setMargin(0.0);
    
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(Scalar(0), motionState, shape, Vector3(0,0,0));
    rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = Scalar(0); //not used
    rigidBodyCI.m_linearDamping = rigidBodyCI.m_angularDamping = Scalar(0); //not used
    rigidBodyCI.m_linearSleepingThreshold = rigidBodyCI.m_angularSleepingThreshold = Scalar(0); //not used
    rigidBodyCI.m_additionalDamping = false;
    
    rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setUserPointer(this);
    rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    
    BuildGraphicalObject();
}

void StaticEntity::AddToSimulation(SimulationManager* sm)
{
    AddToSimulation(sm, Transform::getIdentity());
}

void StaticEntity::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    if(rigidBody != NULL)
    {
        btDefaultMotionState* motionState = new btDefaultMotionState(origin);
        rigidBody->setMotionState(motionState);
        sm->getDynamicsWorld()->addRigidBody(rigidBody, MASK_STATIC, MASK_DEFAULT);
    }
}

//Static members
void StaticEntity::GroupTransform(std::vector<StaticEntity*>& objects, const Transform& centre, const Transform& transform)
{
    for(unsigned int i=0; i<objects.size(); ++i)
    {
        Transform Tw = objects[i]->getTransform();
        Transform Tc = centre.inverse() * Tw;
        Tc = transform * Tc;
        Tw = centre * Tc;
        objects[i]->setTransform(Tw);
    }
}

}
