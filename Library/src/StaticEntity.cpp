//
//  StaticEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "StaticEntity.h"
#include "MathsUtil.hpp"

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
	{
		delete mesh;
		mesh = NULL;
	}
	
    rigidBody = NULL;
}

EntityType StaticEntity::getType()
{
    return ENTITY_STATIC;
}

void StaticEntity::GetAABB(btVector3& min, btVector3& max)
{
	if(rigidBody != NULL)
		rigidBody->getAabb(min, max);
}

std::vector<Renderable> StaticEntity::Render()
{
	std::vector<Renderable> items(0);
	
    if(rigidBody != NULL && objectId >= 0 && isRenderable())
    {
		btTransform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
		
		Renderable item;
		item.objectId = objectId;
		item.lookId = lookId;
		item.dispCoordSys = false;
		item.model = item.csModel = glMatrixFromBtTransform(trans);
		items.push_back(item);
    }
	
	return items;
}

Material StaticEntity::getMaterial()
{
    return mat;
}

void StaticEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld *world)
{
    world->addRigidBody(rigidBody, MASK_STATIC, MASK_DEFAULT);
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