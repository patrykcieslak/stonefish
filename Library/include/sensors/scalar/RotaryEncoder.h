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
#include "actuators/Motor.h"
#include "actuators/Thruster.h"
#include "sensors/scalar/JointSensor.h"

namespace sf
{

class RotaryEncoder : public JointSensor
{
public:
    RotaryEncoder(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
    
    void AttachToMotor(Motor* m);
    void AttachToThruster(Thruster* th);
    
    virtual void InternalUpdate(Scalar dt);
    virtual void Reset();
    
protected:
    Scalar GetRawAngle();
    Scalar GetRawAngularVelocity();
    
    Motor* motor;
    Thruster* thrust;
    Scalar angle;
    Scalar lastAngle;
};

}

#endif
