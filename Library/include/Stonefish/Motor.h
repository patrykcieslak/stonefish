//
//  Motor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__Motor__
#define __Stonefish__Motor__

#include "Actuator.h"
#include "RevoluteJoint.h"
#include "FeatherstoneEntity.h"

class Motor : public Actuator
{
public:
    Motor(std::string uniqueName, RevoluteJoint* revolute);
    Motor(std::string uniqueName, FeatherstoneEntity* mb, unsigned int child);
    
    void Update(btScalar dt);
    btVector3 Render();
    
    void setTorque(btScalar value);
    btScalar getTorque();
    ActuatorType getType();
    
private:
    btScalar getAngularVelocity();
    
    //output
    RevoluteJoint* revoluteOutput;
    FeatherstoneEntity* multibodyOutput;
    unsigned int multibodyChild;
    
    //states
    btScalar torque;
};




#endif /* defined(__Stonefish__Motor__) */
