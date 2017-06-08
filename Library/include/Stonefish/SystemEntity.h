//
//  SystemEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/05/2017.
//  Copyright © 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SystemEntity__
#define __Stonefish_SystemEntity__

#include "Entity.h"
#include "Ocean.h"

class SystemEntity : public Entity
{
public:
    SystemEntity(std::string uniqueName);
    virtual ~SystemEntity();
    
    //Base methods
    EntityType getType();
    virtual void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);
    
    //Pure virtual methods
    virtual void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform) = 0;
    virtual void UpdateAcceleration() = 0;
    virtual void UpdateSensors(btScalar dt) = 0;     //Update sensors in the system and its subsystems
    virtual void UpdateControllers(btScalar dt) = 0; //Update controllers in the system and its subsystems
    virtual void UpdateActuators(btScalar dt) = 0;   //Update actuators in the system and its subsystems
    virtual void ApplyGravity() = 0;
    virtual void ApplyFluidForces(Ocean* fluid) = 0;
    
    virtual btTransform getTransform() const = 0;
    
    virtual void Render() = 0;
};

#endif
