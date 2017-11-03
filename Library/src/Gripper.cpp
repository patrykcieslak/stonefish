//
//  Gripper.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Gripper.h"

Gripper::Gripper(std::string uniqueName, Manipulator* m) : SystemEntity(uniqueName)
{
    manipulator = m;
    closed = false;
}

Gripper::~Gripper()
{
    delete mechanism;
    delete ft;
}

SystemType Gripper::getSystemType()
{
    return SYSTEM_GRIPPER;
}

btTransform Gripper::getTransform() const
{
    return mechanism->getMultiBody()->getBaseWorldTransform();
}

bool Gripper::isClosed()
{
    return closed;
}

ForceTorque* Gripper::getFT()
{
    return ft;
}

void Gripper::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
    mechanism->AddToDynamicsWorld(world, worldTransform);
	
    fix = new FixedJoint(getName() + "/Fix", mechanism, manipulator->getChain(), -1, manipulator->getNumOfLinks()-2);
    fix->AddToDynamicsWorld(world);
    
    ft = new ForceTorque(getName() + "/FT", fix, mechanism->getLink(0).solid, btTransform::getIdentity());
}

void Gripper::UpdateAcceleration(btScalar dt)
{
}

void Gripper::UpdateSensors(btScalar dt)
{
    ft->Update(dt);
}

void Gripper::ApplyGravity(const btVector3& g)
{
    mechanism->ApplyGravity(g);
}

void Gripper::ApplyDamping()
{
    mechanism->ApplyDamping();
}

void Gripper::GetAABB(btVector3& min, btVector3& max)
{
    mechanism->GetAABB(min, max);
}

std::vector<Renderable> Gripper::Render()
{
    return mechanism->Render();
}
    