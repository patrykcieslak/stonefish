//
//  ForcefieldEntity.cpp
//
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "entities/ForcefieldEntity.h"

#include "core/SimulationManager.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

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

void ForcefieldEntity::AddToSimulation(SimulationManager* sm)
{
    sm->getDynamicsWorld()->addCollisionObject(ghost, MASK_DEFAULT, MASK_DEFAULT);
}

std::vector<Renderable> ForcefieldEntity::Render()
{
	return std::vector<Renderable>(0);
}

void ForcefieldEntity::getAABB(Vector3& min, Vector3& max)
{
    min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
}

}
