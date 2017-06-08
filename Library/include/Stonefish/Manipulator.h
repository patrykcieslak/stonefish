//
//  Manipulator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Manipulator__
#define __Stonefish_Manipulator__

#include "SystemEntity.h"
#include "FeatherstoneEntity.h"
#include "Gripper.h"

class Manipulator : public SystemEntity
{
public:
    Manipulator(std::string uniqueName);
    virtual ~Manipulator();
    
    virtual void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform = btTransform::getIdentity());
    
    virtual void UpdateSensors(btScalar dt);
    virtual void UpdateControllers(btScalar dt);
    virtual void UpdateActuators(btScalar dt);
    virtual void ApplyGravity();
    virtual void ApplyFluidForces(Ocean* fluid);
    virtual void Render();
    
private:
    FeatherstoneEntity* links;
    Gripper* gripper;
};

#endif
