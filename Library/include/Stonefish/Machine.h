//
//  Machine.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__Machine__
#define __Stonefish__Machine__

#include "common.h"

typedef enum {MANIPULATOR, SUBMERSIBLE_VEHICLE, WHEELED_VEHICLE} MachineType;

//complex of entities, joints, sensors and actuators (pure virtual class)
class Machine
{
public:
    Machine(std::string uniqueName);
    virtual ~Machine(void);

    virtual MachineType getType() = 0;
    virtual void ApplyForces() = 0;
    virtual void Render() = 0;
    virtual btTransform GetTransform() = 0;
    virtual void SetTransform(const btTransform& trans) = 0;
    virtual void AddToDynamicsWorld(btDynamicsWorld* world) = 0;
    virtual void RemoveFromDynamicsWorld(btDynamicsWorld* world) = 0;
   
    std::string getName();
    
private:
    std::string name;
};

#endif