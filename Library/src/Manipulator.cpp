//
//  Manipulator.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Manipulator.h"

Manipulator::Manipulator(std::string uniqueName) : SystemEntity(uniqueName)
{
}

Manipulator::~Manipulator()
{
}

void Manipulator::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
    
    
}

void Manipulator::UpdateSensors(btScalar dt)
{
    
    
}

void Manipulator::UpdateControllers(btScalar dt)
{
    
    
}

void Manipulator::UpdateActuators(btScalar dt)
{

    
}

void Manipulator::ApplyGravity()
{
    
    
}

void Manipulator::ApplyFluidForces(Ocean* fluid)
{
    
    
}

void Manipulator::Render()
{
    
}
