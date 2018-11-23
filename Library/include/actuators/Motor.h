//
//  Motor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Motor__
#define __Stonefish_Motor__

#include "actuators/JointActuator.h"

namespace sf
{

class Motor : public JointActuator
{
public:
    Motor(std::string uniqueName);
    
    virtual void Update(btScalar dt);
    virtual void setIntensity(btScalar value);
	virtual btScalar getTorque();
    virtual btScalar getAngle();
	virtual btScalar getAngularVelocity();
    
protected:
	btScalar torque;
};

}

#endif
