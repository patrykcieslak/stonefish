//
//  StaticEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "entities/StaticEntity.h"

#include "core/SimulationApp.h"
#include "utils/MathUtil.hpp"

using namespace sf;

StaticEntity::StaticEntity(std::string uniqueName, Material m, int _lookId) : Entity(uniqueName)
{
    mat = m;
	objectId = -1;
    lookId = _lookId;
    wireframe = false;
	rigidBody = NULL;
    mesh = NULL;
}

StaticEntity::~StaticEntity()
{
    if(mesh != NULL)
        delete mesh;
    
    rigidBody = NULL;
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

void StaticEntity::SetLook(int newLookId)
{
    lookId = newLookId;
}

void StaticEntity::SetWireframe(bool enabled)
{
    wireframe = enabled;
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
		item.model = glMatrixFromBtTransform(trans);
		items.push_back(item);
    }
	
	return items;
}

void StaticEntity::BuildGraphicalObject()
{
	if(mesh == NULL || !SimulationApp::getApp()->hasGraphics())
		return;
		
	objectId = OpenGLContent::getInstance()->BuildObject(mesh);	
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
    rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK | btCollisionObject::CF_STATIC_OBJECT);
	
	BuildGraphicalObject();
}

void StaticEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld *world)
{
    AddToDynamicsWorld(world, Transform::getIdentity());
}

void StaticEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const Transform& worldTransform)
{
    if(rigidBody != NULL)
    {
        btDefaultMotionState* motionState = new btDefaultMotionState(worldTransform);
        rigidBody->setMotionState(motionState);
        world->addRigidBody(rigidBody, MASK_STATIC, MASK_STATIC | MASK_DEFAULT);
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
