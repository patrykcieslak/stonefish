//
//  RotaryEncoder.h
//  Stonefish
//
//  Created by Patryk Cieslak on 05/07/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_RotaryEncoder__
#define __Stonefish_RotaryEncoder__

#include "entities/FeatherstoneEntity.h"
#include "joints/RevoluteJoint.h"
#include "actuators/Motor.h"
#include "actuators/Thruster.h"
#include "sensors/SimpleSensor.h"

class RotaryEncoder : public SimpleSensor
{
public:
    RotaryEncoder(std::string uniqueName, RevoluteJoint* joint, btScalar frequency = btScalar(-1.), int historyLength = -1);
    RotaryEncoder(std::string uniqueName, FeatherstoneEntity* fe, unsigned int joint, btScalar frequency = btScalar(-1.), int historyLength = -1);
    RotaryEncoder(std::string uniqueName, Motor* m, btScalar frequency = btScalar(-1.), int historyLength = -1);
    RotaryEncoder(std::string uniqueName, Thruster* th, btScalar frequency = btScalar(-1.), int historyLength = -1);
    
    virtual void InternalUpdate(btScalar dt) = 0;
    virtual void Reset() = 0;
    
protected:
    btScalar GetRawAngle();
    btScalar GetRawAngularVelocity();
    
    RevoluteJoint* revolute;
    FeatherstoneEntity* multibody;
    unsigned int multibodyJoint;
    Motor* motor;
    Thruster* thrust;
};

#endif
