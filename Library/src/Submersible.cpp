//
//  Submersible.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Submersible.h"

Submersible::Submersible(std::string uniqueName) : Machine(uniqueName)
{
    frame = new CompoundEntity(uniqueName.append("-Frame"), false);
}

Submersible::~Submersible(void)
{
    
}

MachineType Submersible::getType()
{
    return SUBMERSIBLE_VEHICLE;
}

void Submersible::AddToDynamicsWorld(btDynamicsWorld* world)
{
    frame->AddToDynamicsWorld(world);
}

void Submersible::RemoveFromDynamicsWorld(btDynamicsWorld *world)
{
    frame->RemoveFromDynamicsWorld(world);
}

btTransform Submersible::GetTransform()
{
    return frame->getTransform();
}

void Submersible::SetTransform(const btTransform &trans)
{
    frame->setTransform(trans);
}

void Submersible::Render()
{
    frame->Render();
}

void Submersible::AddFramePart(SolidEntity *solid, const btTransform &location)
{
    frame->AddSolid(solid, location);
}

SolidEntity* Submersible::GetFramePart(unsigned int index)
{
    return frame->GetSolid(index);
}

unsigned int Submersible::FramePartsCount()
{
    return frame->SolidsCount();
}

void Submersible::AddSensor(Sensor *s, const btTransform &location)
{
    
}

void Submersible::AddThruster(Thruster *th, const btTransform &location)
{
    
}

void Submersible::AddControlSurface(ControlSurface *cs, const btTransform &location)
{
    
}

void Submersible::SetThrust(int thrusterId, btScalar value)
{
    
}

void Submersible::SetControlAngle(int controlSurfaceId, btScalar value)
{
    
}

void Submersible::ApplyForces()
{
    ApplyGravity();
    ApplyPropellingForces();
}

void Submersible::ApplyGravity()
{
    frame->ApplyGravity();
}

void Submersible::ApplyPropellingForces()
{
    
}

void Submersible::ApplyFluidForces()
{
    
}