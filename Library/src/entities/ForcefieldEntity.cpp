//
//  ForcefieldEntity.cpp
//
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "entities/ForcefieldEntity.h"

using namespace sf;

ForcefieldEntity::ForcefieldEntity(std::string uniqueName) : Entity(uniqueName)
{
    ghost = new btPairCachingGhostObject();
    ghost->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

ForcefieldEntity::~ForcefieldEntity()
{
}

EntityType ForcefieldEntity::getType()
{
    return ENTITY_FORCEFIELD;
}

btPairCachingGhostObject* ForcefieldEntity::getGhost()
{
    return ghost;
}

void ForcefieldEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld *world)
{
    world->addCollisionObject(ghost, MASK_DEFAULT, MASK_DEFAULT);
}

std::vector<Renderable> ForcefieldEntity::Render()
{
	return std::vector<Renderable>(0);
}

void ForcefieldEntity::GetAABB(btVector3& min, btVector3& max)
{
    min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
}
