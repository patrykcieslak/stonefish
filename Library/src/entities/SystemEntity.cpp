//
//  SystemEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/13/13.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "entities/SystemEntity.h"

SystemEntity::SystemEntity(std::string uniqueName) : Entity(uniqueName)
{
}

SystemEntity::~SystemEntity()
{
}

EntityType SystemEntity::getType()
{
	return ENTITY_SYSTEM;
}

void SystemEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world)
{
	AddToDynamicsWorld(world, btTransform::getIdentity());
}

void SystemEntity::UpdateSensors(btScalar dt)
{    
}

void SystemEntity::UpdateControllers(btScalar dt)
{
}
 
void SystemEntity::UpdateActuators(btScalar dt)
{
}
  
