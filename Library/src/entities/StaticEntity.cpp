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

StaticEntity::StaticEntity(std::string uniqueName, Material m, int _lookId) : Entity(uniqueName)
{
    mat = m;
	objectId = -1;
    lookId = _lookId;
    rigidBody = NULL;
    phyMesh = NULL;
}

StaticEntity::~StaticEntity()
{
    if(phyMesh != NULL) delete phyMesh;
}

EntityType StaticEntity::getType()
{
    return ENTITY_STATIC;
}

Material StaticEntity::getMaterial()
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

std::vector<Renderable> StaticEntity::Render()
{
	std::vector<Renderable> items(0);
	
    if(rigidBody != NULL && objectId >= 0 && isRenderable())
    {
		Transform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
		
		Renderable item;
        item.type = RenderableType::SOLID;
		item.objectId = objectId;
		item.lookId = lookId;
		item.model = glMatrixFromTransform(trans);
		items.push_back(item);
    }
	
	return items;
}

void StaticEntity::BuildGraphicalObject()
{
	if(phyMesh == NULL || !SimulationApp::getApp()->hasGraphics())
		return;
	
    objectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh);
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
        sm->getDynamicsWorld()->addRigidBody(rigidBody, MASK_STATIC, MASK_STATIC | MASK_DEFAULT);
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
