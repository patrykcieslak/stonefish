//
//  Gripper.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include <entities/systems/Gripper.h>

Gripper::Gripper(std::string uniqueName, Manipulator* m) : SystemEntity(uniqueName)
{
    manipulator = m;
    openFrac = btScalar(0);
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

ForceTorque* Gripper::getFT()
{
    return ft;
}

void Gripper::Open()
{
    SetState(btScalar(1));    
}

void Gripper::Close()
{
    SetState(btScalar(0));
} 

btScalar Gripper::GetState()
{
    return openFrac;
}

void Gripper::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
    FeatherstoneEntity* chain = manipulator->getChain();
    SolidEntity* lastLink = chain->getLink(manipulator->getNumOfLinks()-1).solid;    
    btTransform trans = lastLink->getTransform() * lastLink->getGeomToCOGTransform().inverse() * worldTransform;
    
    mechanism->AddToDynamicsWorld(world, trans);
	
    fix = new FixedJoint(getName() + "/Fix", mechanism, manipulator->getChain(), -1, manipulator->getNumOfLinks()-2, getTransform().getOrigin());
    fix->AddToDynamicsWorld(world);
    
    ft = new ForceTorque(getName() + "/FT", fix, mechanism->getLink(0).solid, btTransform::getIdentity());
}

void Gripper::UpdateAcceleration(btScalar dt)
{
    mechanism->UpdateAcceleration(dt);
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
    std::vector<Renderable> items = mechanism->Render();
    
    std::vector<Renderable> sens = ft->Render();
    items.insert(items.end(), sens.begin(), sens.end());
    
    return items;
}
    